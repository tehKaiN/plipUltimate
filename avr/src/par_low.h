/*
 * par_low.h - low_level routines to access amiga parallel port
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

#ifndef PAR_LOW_H
#define PAR_LOW_H

#include "global.h"

#include <avr/io.h>

/// Parallel status bits
#define NSTROBE_PIN PC0
#define NACK_PIN    PC1
#define BUSY_PIN    PC2
#define POUT_PIN    PC3
#define SEL_PIN     PC4

#define NSTROBE _BV(NSTROBE_PIN)
#define NACK    _BV(NACK_PIN)
#define BUSY    _BV(BUSY_PIN)
#define POUT    _BV(POUT_PIN)
#define SEL     _BV(SEL_PIN)

#define PAR_STATUS_PORT PORTC
#define PAR_STATUS_DDR DDRC
#define PAR_STATUS_PIN PINC
#define PAR_STATUS_MASK (NSTROBE | NACK | BUSY | POUT | SEL)

/// Parallel data bits
#define PAR_DATA_PORT PORTD
#define PAR_DATA_DDR DDRD
#define PAR_DATA_PIN PIND


// ----- Input Buffer Handling -----

#define PAR_IN_BUF_BITS     4
#define PAR_IN_BUF_SIZE     (1 << PAR_IN_BUF_BITS)
#define PAR_IN_BUF_MASK     (PAR_IN_BUF_SIZE - 1)

// ----- Functions -----

extern void par_low_init(void);

extern void par_low_pulse_ack(u08 delay);

#endif
