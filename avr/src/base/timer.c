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

#include <avr/interrupt.h>

#define T2_vect TIMER2_COMPA_vect

// timer counter
volatile uint16_t timer_100us; /// Number of 100us intervals passed
volatile uint16_t timer_10ms;  /// Number of 10-ms intervals passed
volatile uint32_t time_stamp;  /// Time stamp since beginning of runtime - 1 tick equals 100us
static uint16_t count;         /// Number of 100us intervals to increment 10-ms count

/// 100us is 1/10k of a second
/// NOTE(KaiN#): time was reduced by 1, should it be?
#define T2_100us ((F_CPU/8)/10000)

void timer_init(void) {
  cli();

	/// Set timer 2 to CTC, prescaler 8 & compare value: 100us
  TCCR2A = _BV(WGM21);  // CTC
  TCCR2B = _BV(CS21);   // Prescaler 8
  OCR2A = T2_100us;     // Output compare
  TCNT2  = 0x00;        // Reset timer state
  TIMSK2 = _BV(OCIE2A); // Enable compare interrupt

	/// Is it used?
  // ----- TIMER1 (16bit) -----
  // prescale 64
  // 16 MHz -> 250 KHz = 4 us timer

  // set to CTC on OCR1A with prescale 8
  TCCR1A = 0x00;
  TCCR1B = _BV(CS11) | _BV(CS10); // prescale 64
  TCCR1C = 0x00;

  // reset timer
  TCNT1 = 0;

  timer_100us = 0;
  timer_10ms = 0;
  time_stamp = 0;
  count = 0;

  sei();
}

// timer2 compare A handler
ISR(T2_vect) {
  ++timer_100us;
  ++time_stamp;
  ++count;
  if(count >= 1000) {
    count = 0;
    timer_10ms++;
  }
}

/// Busy-wait for supplied number of 10ms intervals
void timer_delay_10ms(uint16_t uwIntervalCount) {
	timer_10ms=0;
	while(timer_10ms<uwIntervalCount);
}

/// Busy-wait for supplied number of 100us intervals
void timer_delay_100us(uint16_t uwIntervalCount) {
	timer_100us=0;
	while(timer_100us<uwIntervalCount);
}

/// Calculates bitrate based on transferred byte count and elapsed time
uint16_t timer_hw_calc_rate_kbs(uint16_t bytes, uint16_t delta) {
  if(delta != 0) {
    uint32_t nom = 1000 * (uint32_t)bytes * 100;
    uint32_t denom = (uint32_t)delta * 4;
    uint32_t rate = nom / denom;
    return (uint16_t)rate;
  }
	else
		return 0;
}
