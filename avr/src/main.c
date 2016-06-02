/*
 * main.c - main loop
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plipbox.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "global.h"
#include "base/uart.h"
#include "base/uartutil.h"
#include "base/timer.h"
#include "par_low.h"
#include "param.h"
#include "base/cmd.h"

#include "pb_test.h"
#include "pio_test.h"
#include "bridge_test.h"
#include "bridge.h"
#include "main.h"

uint8_t run_mode = RUN_MODE_BRIDGE;
uint8_t global_verbose = 0;

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
 *   Other minor changes
 *
 * Sizes before UART removal:
 * Program: 15872 (48.4%)
 * Data:     1767 (86.3%)
 * EEPROM:     21 ( 2.1%)
 *
 * Current sizes after last change:
 * Program: 10230 (31.2%)
 * Data: 1696 (82.8%)
 * EEPROM: 21 (2.1%)
 *
 */

static void init_hw(void)
{
	/// Disable watchdog
	cli();
	wdt_disable();
	sei();
	/// Setup other HW stuff
	timer_init();                   // Setup timer
	par_low_init();                 // Setup parallel interface
}

int main(void)
{
  while(1) {
		init_hw();

		// send welcome
		// NOTE: UART - \r\nWelcome to plipbox " VERSION " " BUILD_DATE "\r\n

		// param init
		param_init();
		param_dump();
		// NOTE: UART - \r\n

		// help info
		// NOTE: UART - Press <return> to enter command mode or <?> for key help\r\n

		#ifdef DEBUG
			uart_send_free_stack();
		#endif

		// select main loop depending on current run mode
		uint8_t result = CMD_WORKER_IDLE;
		while(result != CMD_WORKER_RESET)
			switch(run_mode) {
				case RUN_MODE_PB_TEST:
					result = pb_test_loop();
					break;
				case RUN_MODE_PIO_TEST:
					result = pio_test_loop();
					break;
				case RUN_MODE_BRIDGE_TEST:
					result = bridge_test_loop();
					break;
				case RUN_MODE_BRIDGE:
				default:
					result = bridge_loop();
					break;
			}

		// wait a bit
		// NOTE: UART - resetting...\r\n
		timer_delay_10ms(10);

	}
  return 0;
}
