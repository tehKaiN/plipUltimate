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


// ----- Input Buffer Handling -----

#define PAR_IN_BUF_BITS     4
#define PAR_IN_BUF_SIZE     (1 << PAR_IN_BUF_BITS)
#define PAR_IN_BUF_MASK     (PAR_IN_BUF_SIZE - 1)

// ----- Functions -----

extern void par_low_init(void);

extern void par_low_pulse_ack(uint8_t delay);

#endif
