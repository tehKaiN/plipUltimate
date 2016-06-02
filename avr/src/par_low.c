/*
 * par_low.c - low_level routines to access amiga parallel port
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

#include "par_low.h"
#include <avr/interrupt.h>
#include <util/delay_basic.h>
#include "pinout.h"


void par_low_init(void) {
	// Zero status direction and pin state
  PAR_STATUS_DDR &= ~PAR_STATUS_MASK;
  PAR_STATUS_PORT &= ~PAR_STATUS_MASK;

	// NStrobe: input, pulled high
	// Select: input, pulled high
	// Busy: output, default: 0
	// POut: input, pulled high
	// NAck: output, default: 1
  PAR_STATUS_DDR |= BUSY | NACK;
  PAR_STATUS_PORT |= NSTROBE | SEL | POUT | NACK;

  PAR_DATA_DDR = 0xFF;
}

// data bus

void par_low_pulse_ack(uint8_t delay)
{
  PAR_STATUS_PORT &= ~NACK;
  _delay_loop_1(delay);
  PAR_STATUS_PORT |= NACK;
}
