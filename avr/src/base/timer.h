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

#ifndef TIMER_H
#define TIMER_H

#include "../global.h"

#include <avr/io.h>

// init timers
void timerInit(void);

// a 100 Hz = 10ms timer
// 16bit: 0,10ms...~10hours
extern volatile uint16_t g_uwTimer10ms;

// a 99.83ms ~ 100ms timer
// 16bit: 0,100us...6.5s
extern volatile uint16_t g_uwTimer100us;

// in 100us
extern volatile uint32_t g_uwTimeStamp;

// busy wait with 10ms timer
extern void timerDelay10ms(uint16_t uwCount);

// busy wait with 100us timer
extern void timerDelay100us(uint16_t uwCount);

// ----- hardware timer -----

// 16 bit hw timer with 4us resolution
inline void timerReset(void) { TCNT1 = 0; }
inline uint16_t  timerGetState(void) { return TCNT1; }
extern uint16_t timerCalculateKbps(uint16_t bytes, uint16_t delta);


#endif

