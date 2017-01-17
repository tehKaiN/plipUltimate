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
 
#define PAR_STATUS_DDR  ciab.ciaddra
#define PAR_STATUS_MASK (1|2|4)
#define PAR_STATUS_VAL  ciab.ciapra
#define PAR_DATA_DDR    ciaa.ciaddrb
#define PAR_DATA_VAL    ciaa.ciaprb
#define PAR_BUSY        0
#define PAR_POUT        1
#define PAR_SEL         2
#define _BV(x)          (1 << x)

UBYTE parReserve(char *szOwnerName);
void parFree(void);
void parWaitBusyHi(void);
void parWaitBusyLo(void);

#endif // GUARD_PAR_H
