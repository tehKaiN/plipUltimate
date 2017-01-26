/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef GUARD_PAR_H
#define GUARD_PAR_H

#include <clib/exec_protos.h>
#include <hardware/cia.h>

extern struct CIA ciaa, ciab;

/**
 *  ciab.ciaddrb: DDR for data line
 *  ciaa.prb: PORT/PIN for data line
 *  ciab.ciaddra: DDR for status:
 *  	bit 0: BUSY
 *  	bit 1: POUT
 *  	bit 2: SEL
 *  ciab.ciapra PORT/PIN for status:
 *  	bit 0: BUSY
 *  	bit 1: POUT
 *  	bit 2: SEL
 *  About ACK and STROBE lines:
 *  	http://www.verycomputer.com/2_75ca7b7b48180355_1.htm
 *  	STROBE (out): goes low 3 CIA cycles after data is written/read
 *  	ACK (in): hi-state triggers interrupt
 */
 
#define _BV(x)          (1 << x)
#define PAR_STATUS_DDR  ciab.ciaddra
#define PAR_STATUS_MASK (1|2|4)
#define PAR_STATUS_VAL  ciab.ciapra
#define PAR_DATA_DDR    ciaa.ciaddrb
#define PAR_DATA_VAL    ciaa.ciaprb
#define PAR_BUSY_PIN    0
#define PAR_POUT_PIN    1
#define PAR_SEL_PIN     2
#define PAR_BUSY        _BV(PAR_BUSY_PIN)
#define PAR_POUT        _BV(PAR_POUT_PIN)
#define PAR_SEL         _BV(PAR_SEL_PIN)

UBYTE parReserve(char *szOwnerName);
void parFree(void);
UBYTE parWaitBusy(UBYTE ubVal);
void parMakeLow(void);

#endif // GUARD_PAR_H
