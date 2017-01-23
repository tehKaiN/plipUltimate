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

#include "timer.h"
#include <stdio.h>
#include <clib/timer_protos.h>

struct Device* TimerBase;
static struct IORequest timereq;

void timerCreate(void) {
  OpenDevice("timer.device", 0, &timereq, 0);
  TimerBase = timereq.io_Device;
}

void timerDestroy(void) {
  CloseDevice(&timereq);	
}

void timerDelayMs(UWORD uwMs) {
	struct timeval sTimeStart, sTimeCurr;

  GetSysTime(&sTimeStart);
	do {
		GetSysTime(&sTimeCurr);
		SubTime(&sTimeCurr, &sTimeStart);
	}
	while(sTimeCurr.tv_secs*1000 + sTimeCurr.tv_micro/1000 < uwMs);
}
