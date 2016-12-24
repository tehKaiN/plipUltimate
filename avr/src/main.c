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
#include "param.h"
#include "base/cmd.h"

#include "pb_test.h"
#include "pio_test.h"
#include "bridge_test.h"
#include "bridge.h"
#include "main.h"
#include "pinout.h"

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
 *   Removed Packet IO HAL: (Prg: -232 B, Data: -3 B)
 *   Simplified data buffer access (Prg: -100 B, Data: -4 B)
 *   Other minor changes
 *
 * Sizes before UART removal:
 * Program: 15872 (48.4%)
 * Data:     1767 (86.3%)
 * EEPROM:     21 ( 2.1%)
 *
 * Current sizes after last change:
 * Program: 9884 (30.2%)
 * Data: 1689 (82.5%)
 * EEPROM: 21 (2.1%)
 *
 */

/**
 * Initilizes AVR hardware.
 * Parallel interface is configured as follows:
 * 	NStrobe: input,  pulled high
 * 	Select:  input,  pulled high
 * 	Busy:    output, default: 0
 * 	POut:    input,  pulled high
 * 	NAck:    output, default: 1
 */
static void init_hw(void)
{
	// Disable watchdog
	wdt_disable();
	sei();

	// Setup timers
	timer_init();

	// Zero DDR and PORT status
  PAR_STATUS_DDR &= ~PAR_STATUS_MASK;
  PAR_STATUS_PORT &= ~PAR_STATUS_MASK;

  // Set them correctly
  PAR_STATUS_DDR |= BUSY | NACK;
  PAR_STATUS_PORT |= NSTROBE | SEL | POUT | NACK;

  PAR_DATA_DDR = 0xFF;
}

int main(void)
{
	init_hw();

	// Send welcome message
	// NOTE: UART - \r\nWelcome to plipbox " VERSION " " BUILD_DATE "\r\n

	// Load & display parameters (config)
	param_init();
	param_dump();

	// Display help message
	// NOTE: UART - Press <return> to enter command mode or <?> for key help\r\n

	#ifdef DEBUG
		uart_send_free_stack();
	#endif

	// Enter main loop depending on current run mode
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

	// Wait a bit and do a reset
	timer_delay_10ms(10);
	wdt_enable(WDTO_15MS);
	while(1);

  return 0;
}
