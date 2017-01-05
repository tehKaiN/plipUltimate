#include <stdio.h>
#include <resources/misc.h>
#include <clib/exec_protos.h>
#include <clib/misc_protos.h>
#include <hardware/cia.h>
#include "ack.h"

struct Library *MiscBase = NULL;
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

/**
 *  Magic EtherType value
 */
#define ETH_TYPE_MAGIC_CMD 0xfffc

/**
 *  PlipUltimate command types
 */
#define CMD_INVALID 0
#define CMD_RESET 1

/**
 *  Transmission types:
 *  	PBPROTO_CMD_SEND - double acked send to plip
 *  	PBPROTO_CMD_RECV - double acked recv from plip
 *  	PBPROTO_CMD_SEND_BURST - burst send to plip
 *  	PBPROTO_CMD_RECV_BURST - burst recv from plip
 */
#define PBPROTO_CMD_SEND       0x11
#define PBPROTO_CMD_RECV       0x22
#define PBPROTO_CMD_SEND_BURST 0x33
#define PBPROTO_CMD_RECV_BURST 0x44

const UBYTE s_pResetCmd[14] = {
	CMD_RESET, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	ETH_TYPE_MAGIC_CMD>>8, ETH_TYPE_MAGIC_CMD&0xFF
};

void waitBusyHi(void) {
	volatile UBYTE ubBusy;
	UWORD uwTimeout;
	
	uwTimeout = 1;
	do {
		ubBusy = PAR_STATUS_VAL & _BV(PAR_BUSY);
		++uwTimeout;
	} while(!ubBusy && uwTimeout);
	
	if(!uwTimeout) {
		printf("ERR: waitBusyHi timeout!\n");
	}
}

void waitBusyLo(void) {
	volatile UBYTE ubBusy;
	UWORD uwTimeout;
	
	uwTimeout = 1;
	do {
		ubBusy = PAR_STATUS_VAL & _BV(PAR_BUSY);
		++uwTimeout;
	} while(ubBusy && uwTimeout);
	
	if(!uwTimeout) {
		printf("ERR: waitBusyLo timeout!\n");
	}
}

/**
 *  Sends data to plipUltimate in double acked way.
 *  This is the most robust method of sending stuff.
 */
void plipSend(const UBYTE *pData, UWORD uwSize) {
	UWORD i;
	
	PAR_DATA_DDR = 0xFF;                            // Set data DDR
	PAR_STATUS_DDR &= ~PAR_STATUS_MASK;             // Clear status DDR
	PAR_STATUS_DDR |= _BV(PAR_POUT) | _BV(PAR_SEL); // Set status DDR
	PAR_STATUS_VAL &= ~PAR_STATUS_MASK;             // Clear status pins
	
	PAR_DATA_VAL = PBPROTO_CMD_SEND;
	PAR_STATUS_VAL |= _BV(PAR_SEL);
	waitBusyHi();
	
	// Send size
	PAR_DATA_VAL = uwSize >> 8;
	PAR_STATUS_VAL |= _BV(PAR_POUT);
	waitBusyLo();
	PAR_DATA_VAL = uwSize & 0xFF;
	PAR_STATUS_VAL &= ~_BV(PAR_POUT);
	waitBusyHi();
	
	// Send rest of data
	UBYTE ubDoHiPout = 1;
	for(i = 0; i != uwSize; ++i) {
		PAR_DATA_VAL = pData[i];
		if(ubDoHiPout) {
			PAR_STATUS_VAL |= _BV(PAR_POUT);
			waitBusyLo();
			ubDoHiPout = 0;
		}
		else {
			PAR_STATUS_VAL &= ~_BV(PAR_POUT);
			waitBusyHi();
			ubDoHiPout = 1;
		}
	}
	
	// Make all pins low
	PAR_DATA_VAL = 0;
	PAR_STATUS_VAL &= ~PAR_STATUS_MASK;
}

/**
 *  Reserves parallel port for exclusive usage.
 *  Based on C example from http://wiki.amigaos.net/wiki/Misc_Resource
 *  @param szOwnerName Pointer to app name string
 */
UBYTE parReserve(char *szOwnerName) {
	char *pOwner;
	
	MiscBase = (struct Library*)OpenResource(MISCNAME);
	if(!MiscBase) {
		printf("ERR: Couldn't open %s\n", MISCNAME);
		return 0;
	}
	
	pOwner = AllocMiscResource(MR_PARALLELPORT, szOwnerName);
	if(pOwner) {
		printf("ERR: Parallel data lines access blocked by: %s\n", pOwner);
		return 0;
	}
	pOwner = AllocMiscResource(MR_PARALLELBITS, szOwnerName);
	if(pOwner) {
		printf("ERR: Parallel status lines access blocked by: %s\n", pOwner);
		// Free what was already allocated
		FreeMiscResource(MR_PARALLELPORT);
	}
	return 1;
}

void parFree(void) {
	FreeMiscResource(MR_PARALLELPORT);
	FreeMiscResource(MR_PARALLELBITS);
}

void plipReset(void) {
	printf("Sending CMD_RESET...");
	plipSend(s_pResetCmd, 14);
	printf("OK\n");
}

void printUsage(void) {
	
}

int main(int lArgCount, char **pArgs) {
	setbuf(stdout, NULL);
	printf("plipTool ver. 2016-12-26.\n");
	
	if(!parReserve("plipTool")) {		
		return 1;
	}
	
	if(lArgCount < 2) {
		printf("ERR: no commands specified\n");
		printUsage();
	}
	else {
		// TODO: determine commands
		plipReset();
	}
	
	parFree();
	
	return 0;
}