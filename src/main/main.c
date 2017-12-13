/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <main/global.h>
#include <main/base/uart.h>
#include <main/base/uartutil.h>
#include <main/base/timer.h>
#include <main/config.h>

#include <main/bridge.h>
#include <main/pinout.h>
#include <main/base/util.h>

uint8_t g_ubVerboseMode = 0;

/**
 * ORIGINAL:
 * Program: 16544 (50.5%)
 * Data:     1767 (86.3%)
 * EEPROM:     21 ( 2.1%)
 *
 * Changes:
 *   Removed util/delay.h (Prg: -48 B)
 *   Simplified parallel interface init (Prg: -564 B)
 *   Added SPI SD Card support
 *   Watchdog based on wdt functions (Prg: -24 B)
 *   Removed Packet IO HAL: (Prg: -232 B, Data: -3 B)
 *   Simplified data buffer access (Prg: -100 B, Data: -4 B)
 *   Added Amiga command interface (Prg: +8 B)
 *   Other minor changes
 *
 * Sizes before UART removal:
 * Program: 15872 (48.4%)
 * Data:     1767 (86.3%)
 * EEPROM:     21 ( 2.1%)
 *
 * Current sizes after last change:
 * Program: 9912 (30.2%)
 * Data: 1689 (82.5%)
 * EEPROM: 21 (2.1%)
 *
 */

int main(void)
{
	// Disable watchdog
	cli();
	wdt_reset();
	MCUSR=0;
	WDTCSR|=_BV(WDCE) | _BV(WDE);
	WDTCSR=0;
	sei();

	// Setup timers
	timerInit();

  // Initialize status LED
  LED_PORT |= LED_STATUS;

	// Load config
	configInit();

	// Do the main loop
	bridgeLoop();

  return 0;
}
