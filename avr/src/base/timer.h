/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
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

