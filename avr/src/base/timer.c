/*
 * timer.c - hw timer
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

#include "../global.h"

#include <avr/io.h>
#include <avr/interrupt.h>

// timer s_uw10msCounterer
volatile uint16_t g_uwTimer100us; ///< Number of 100us intervals passed
volatile uint16_t g_uwTimer10ms;  ///< Number of 10-ms intervals passed
volatile uint32_t g_uwTimeStamp;  ///< Time stamp since beginning of runtime.
                                  ///  1 tick equals 100us.
static uint16_t s_uw10msCounter;  ///< Number of 100us intervals, stored for
                                  ///  10-ms s_uw10msCounterer increments

/// 100us is 1/10k of a second
/// NOTE(KaiN#): time was reduced by 1, should it be?
#define T1_100us F_CPU/10000

void timerInit(void) {
  cli();

	/// Set timer 2 to CTC, prescaler 8 & compare value: 100us
  TCCR1A = _BV(WGM12);  // CTC
  TCCR1B = _BV(CS10);   // Prescaler 1
  OCR1A = T1_100us;     // Output compare
  TCNT1  = 0;           // Reset timer state
  TIMSK1 = _BV(OCIE1A); // Enable compare interrupt

  g_uwTimer100us = 0;
  g_uwTimer10ms = 0;
  g_uwTimeStamp = 0;
  s_uw10msCounter = 0;

  sei();
}

/**
 * Timer interrupt handler.
 * Increments time stamp and time interval vars accordingly.
 */
ISR(TIMER1_COMPA_vect) {
	TCNT1 = 0; // Reset timer s_uw10msCounterer
  ++g_uwTimer100us;
  ++g_uwTimeStamp;
  ++s_uw10msCounter;
  if(s_uw10msCounter >= 100) {
    s_uw10msCounter = 0;
    g_uwTimer10ms++;
  }
}

/// Busy-wait for supplied number of 10ms intervals
void timerDelay10ms(uint16_t uwCount) {
	g_uwTimer10ms=0;
	while(g_uwTimer10ms<uwCount);
}

/// Busy-wait for supplied number of 100us intervals
void timerDelay100us(uint16_t uwCount) {
	g_uwTimer100us=0;
	while(g_uwTimer100us<uwCount);
}

// TODO(KaiN#9): timerCalculateKbps() is completely messed up
/**
 * Calculates bitrate based on transferred byte count and elapsed time.
 */
uint16_t timerCalculateKbps(uint16_t bytes, uint16_t delta) {
  if(delta != 0) {
    uint32_t nom = 1000 * (uint32_t)bytes * 100;
    uint32_t denom = (uint32_t)delta * 4;
    uint32_t rate = nom / denom;
    return (uint16_t)rate;
  }
	else
		return 0;
}
