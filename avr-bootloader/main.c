/*
 * This file is part of PlipUltimate.
 * Based on PlipBox by Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <avr/io.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#define F_CPU 20000000
#include <util/delay.h>
#include "pinout.h"

/**
 * ATmega 328p has 256 pages, 64 words each.
 * First page's address is 0x0000, AVR operates natively on WORD address, but
 * SPM functions use byte addresses.
 */
#define BOOT_PAGE_COUNT 256
#define BOOT_PAGE_WORD_SIZE 64
#define BOOT_PAGE_BYTE_SIZE (BOOT_PAGE_WORD_SIZE*2)
uint16_t g_pPageBuffer[BOOT_PAGE_WORD_SIZE];
uint8_t g_ubPageIdx;

/**
 * Boot write mode to differentiate from regular plip operation
 */
#define CMD_SEND_BOOT 0x88

/**
 * Waits approximately 2s for given status pin value.
 * @param ubStatusPin Pin number to monitor.
 * @param ubValue     Desired pin value. 1 for high, 0 for low.
 * @return 1 if monitored pin reaches its desired state, otherwise 0.
 */
uint8_t waitForStatusPin(uint8_t ubStatusPin, uint8_t ubValue) {
	uint16_t uwCounter = 0;
	while(uwCounter < 20000) {
		if(((PAR_STATUS_PIN >> ubStatusPin) & 1) == ubValue)
			return 1;
		++uwCounter;
		_delay_us(100);
	}
	return 0;
}

/**
 * Blinks status LED a given number of times.
 * Each blink takes 1 second. A series of blinks is preceeded and followed by
 * 1 second of low state.
 * @param ubCount Blink count.
 */
void blink(uint8_t ubCount) {
	LED_PORT &= ~LED_STATUS;
	_delay_ms(1000);
	while(ubCount--) {
		LED_PORT |= LED_STATUS;
		_delay_ms(500);
		LED_PORT &= ~LED_STATUS;
		_delay_ms(500);
	}
	_delay_ms(1000);
}

/**
 * Exits bootloader with given result code.
 * Result code is indicated by status LED.
 * @param ubCode Result code.
 */
void bootExit(uint8_t ubCode) {
	LED_DDR |= LED_STATUS;
	blink(ubCode);
	LED_PORT = 0;
	LED_DDR = 0;
	asm("jmp 0000");
}

/**
 * Reads flash mem page from Ami. Protocol is as follows:
 * - Ami sends page idx, which confirms by setting POUT high
 * - Plip reads page idx, confirming it by setting BUSY low
 * - next, 128 bytes are sent by Ami, each of them confirmed by alternating POUT
 *   state, which is followed by Plip read confirm by alternating BUSY.
 * After successful sending, last POUT state should be high, but Ami is expected
 * to set it low afterwards.
 */
uint8_t bootReadPageFromAmi(void) {
  uint8_t *pPageByteBfr = (uint8_t*)g_pPageBuffer;
  uint8_t ubPOutWait = 0;
  uint8_t i;

	// Read page idx
  if(!waitForStatusPin(POUT_PIN, 1)) // Wait for ready state from Ami
		return 0;
  g_ubPageIdx = PAR_DATA_PIN;        // Read byte
  PAR_STATUS_PORT &= ~BUSY;          // Confirm read

  // Read page bytes
  for(i = 0; i != BOOT_PAGE_BYTE_SIZE; ++i) {
    if(!waitForStatusPin(POUT_PIN, ubPOutWait))
      return 0;
    pPageByteBfr[i] = PAR_DATA_PIN;
    PAR_STATUS_PORT ^= BUSY;
    ubPOutWait ^= 1;
	}

	waitForStatusPin(POUT_PIN, 0);
  return 1;
}

/**
 * Writes prepared flash page.
 * This function expects received data to be already in little-endian.
 */
void bootFlashPage(void) {
	uint8_t ubSreg;
	uint32_t ulPageAddr;
	uint8_t i;

	// Disable interrupts
	ubSreg = SREG;
	cli();

	// Calculate page address
	ulPageAddr = g_ubPageIdx * BOOT_PAGE_BYTE_SIZE;

	// Erase page & wait for completion
  boot_page_erase_safe(ulPageAddr);
  boot_spm_busy_wait();

  // Fill page buffer
  for(i = 0; i != BOOT_PAGE_WORD_SIZE; ++i)
		boot_page_fill(ulPageAddr + (i<<1), g_pPageBuffer[i]);

  // Write page
  boot_page_write_safe(ulPageAddr);
	boot_rww_enable_safe();
  ubSreg = SREG;
}

int main(void) {
	volatile uint8_t ubRequest;
	uint8_t i;

	// Disable watchdog & interrupts
	cli();
	wdt_reset();
	MCUSR=0;
	WDTCSR|=_BV(WDCE) | _BV(WDE);
	WDTCSR=0;

	// Set DDR & pullups
	PAR_STATUS_DDR |= NACK | BUSY;
  PAR_STATUS_PORT |= NSTROBE | SEL | POUT | NACK;
	PAR_DATA_DDR = 0;

	// Do a bootloader handshake to init programming
	for(i = 0; i != 3; ++i) {
		if(!waitForStatusPin(POUT_PIN, 1))
			bootExit(1);
		PAR_STATUS_PORT &= ~NACK;
		_delay_ms(5);
		PAR_STATUS_PORT |= NACK;
		if(!waitForStatusPin(POUT_PIN, 0))
			bootExit(1);
		PAR_STATUS_PORT &= ~NACK;
		_delay_ms(5);
		PAR_STATUS_PORT |= NACK;
	}

	while(1) {
		// Wait for read request from Ami
		if(!waitForStatusPin(SEL_PIN, 1))
			bootExit(3);
		ubRequest = PAR_DATA_PIN;
		PAR_STATUS_PORT |= BUSY;

		// Abort if request is not flash-related
		if(ubRequest != CMD_SEND_BOOT)
			bootExit(4); // 4

		// Read page from Ami
		if(!bootReadPageFromAmi())
			bootExit(5);

		// Program page on flash & wait for Ami read
		bootFlashPage();

		// Let Ami know that bootloader is ready for more
		PAR_STATUS_PORT |= NACK;
		_delay_ms(2);
		PAR_STATUS_PORT &= ~NACK;
	}
	bootExit(8);
	return 0;
}
