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

uint8_t waitForStatusPin(uint8_t ubStatusPin, uint8_t ubValue) {
	uint16_t uwCounter = 0;
	while((uwCounter < 1000)) {
		if(((PAR_STATUS_PIN >> ubStatusPin) & 1) == ubValue)
			return 1;
		++uwCounter;
		_delay_us(1000);
	}
	return 0;
}

void bootExit(void) {
	LED_DDR |= _BV(PC5);
	_delay_ms(20);
	LED_DDR = 0;
	LED_PORT = 0;
	_delay_ms(20);
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
	// Disable watchdog & interrupts
	cli();
	wdt_reset();
	MCUSR=0;
	WDTCSR|=_BV(WDCE) | _BV(WDE);
	WDTCSR=0;

	// Bootloader ready for flashing - set POUT hi for 1 second and wait for SEL
	LED_PORT = _BV(PC5);
	PAR_STATUS_DDR = POUT;
	PAR_STATUS_PORT = POUT;

	// Exit bootloader if Ami doesn't respond
	if(!waitForStatusPin(BUSY_PIN, 1))
		bootExit();

	PAR_STATUS_PORT = 0;
	PAR_STATUS_DDR = BUSY | NACK;
	uint8_t ubRequest;
	while(1) {
		// Wait for request from Ami
		if((PAR_STATUS_PORT & SEL) && !(PAR_STATUS_PORT & POUT)) {
			_delay_ms(2);
			continue;
		}

		// Abort if request is not flash-related
		ubRequest = PAR_DATA_PIN;
		PAR_STATUS_PORT |= BUSY;
		if(ubRequest != CMD_SEND_BOOT)
			break;
		// Read page from Ami
		if(!bootReadPageFromAmi())
			continue;

		// Program page on flash & wait for Ami read
		bootFlashPage();

		// Let Ami know that bootloader is ready for more
		PAR_STATUS_PORT |= NACK;
		_delay_ms(2);
		PAR_STATUS_PORT &= ~NACK;
	}
	bootExit();
	return 0;
}
