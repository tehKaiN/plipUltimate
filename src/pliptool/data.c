/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#include <pliptool/data.h>
#include <string.h>
#include <stdio.h>
#include <pliptool/par.h>
#include <pliptool/ack.h>

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

UBYTE g_pRecvBfr[RECV_BFR_SIZE];
UWORD g_uwRecvSize;

void dataRecv(void) {
	UWORD i;
	UBYTE ubDoHiPout;

	g_uwRecvSize = 0;
	memset(g_pRecvBfr, 0, RECV_BFR_SIZE);

	PAR_STATUS_DDR &= ~PAR_STATUS_MASK;   // Clear status DDR
	PAR_STATUS_DDR |= PAR_POUT | PAR_SEL; // Set status DDR
	PAR_STATUS_VAL &= ~PAR_STATUS_MASK;   // Clear status pins

	// Send recv request
	PAR_DATA_DDR = 0xFF;             // Set data DDR to output
	PAR_DATA_VAL = PBPROTO_CMD_RECV; // Send recv request cmd
	PAR_STATUS_VAL |= PAR_SEL;       // Trigger plip
	if(!parWaitBusy(1))              // Wait for plip ack
		goto sendFail;

	// Receive data
	PAR_DATA_DDR = 0x00;               // Set data DDR to input

	PAR_STATUS_VAL |= PAR_POUT;        // Let plip know that we're ready for more
	if(!parWaitBusy(0))                // Wait for BUSY change
		goto sendFail;
	g_uwRecvSize |= PAR_DATA_VAL << 8; // Read upper size byte

	PAR_STATUS_VAL &= ~PAR_POUT;
	if(!parWaitBusy(1))
		goto sendFail;
	g_uwRecvSize |= PAR_DATA_VAL;      // Read lower size byte

	ubDoHiPout = 1;
	for(i = 0; i != g_uwRecvSize; ++i) {
		if(ubDoHiPout) {
			PAR_STATUS_VAL |= PAR_POUT;
			if(!parWaitBusy(0))
				goto sendFail;
			ubDoHiPout = 0;
		}
		else {
			PAR_STATUS_VAL &= ~PAR_POUT;
			if(!parWaitBusy(1))
				goto sendFail;
			ubDoHiPout = 1;
		}
		g_pRecvBfr[i] = PAR_DATA_VAL;
	}

	// Make all pins low
	parMakeLow();

sendFail:
	parMakeLow();
}

/**
 *  Sends data to plipUltimate in double acked way.
 *  This is the most robust method of sending stuff.
 */
void dataSend(const UBYTE *pData, UWORD uwSize) {
	UWORD i;

	// Reset ACK edge detection
	g_ubAckEdge = 0;

	PAR_DATA_DDR = 0xFF;                            // Set data DDR
	PAR_STATUS_DDR &= ~PAR_STATUS_MASK;             // Clear status DDR
	PAR_STATUS_DDR |= PAR_POUT | PAR_SEL;           // Set status DDR
	PAR_STATUS_VAL &= ~PAR_STATUS_MASK;             // Clear status pins

	// Send send request
	PAR_DATA_VAL = PBPROTO_CMD_SEND;
	PAR_STATUS_VAL |= PAR_SEL;
	parWaitBusy(1);

	// Send size
	PAR_DATA_VAL = uwSize >> 8;
	PAR_STATUS_VAL |= PAR_POUT;
	parWaitBusy(0);
	PAR_DATA_VAL = uwSize & 0xFF;
	PAR_STATUS_VAL &= ~PAR_POUT;
	parWaitBusy(1);

	// Send rest of data
	UBYTE ubDoHiPout = 1;
	for(i = 0; i != uwSize; ++i) {
		PAR_DATA_VAL = pData[i];
		if(ubDoHiPout) {
			PAR_STATUS_VAL |= PAR_POUT;
			parWaitBusy(0);
			ubDoHiPout = 0;
		}
		else {
			PAR_STATUS_VAL &= ~PAR_POUT;
			parWaitBusy(1);
			ubDoHiPout = 1;
		}
	}

	// Make all pins low
	parMakeLow();
}

void dataDump(const UBYTE *pData, const UWORD uwDataSize) {
	UWORD i;
	printf("Data size: %u", uwDataSize);
	for(i = 0; i != uwDataSize; ++i) {
		if(!(i % 10))
			putc('\n', stdout);
		else
			putc(' ', stdout);
		printf("%02x", pData[i]);
	}
	putc('\n', stdout);
}
