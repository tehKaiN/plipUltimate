/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#include <pliptool/timer.h>
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
