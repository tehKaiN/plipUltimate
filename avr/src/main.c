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

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "global.h"
#include "base/uart.h"
#include "base/uartutil.h"
#include "base/timer.h"
#include "config.h"

#include "bridge.h"
#include "pinout.h"
#include "base/util.h"

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
