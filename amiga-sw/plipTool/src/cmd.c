#include "cmd.h"
#include <stdio.h>
#include <string.h>
#include "ack.h"
#include "data.h"
#include "par.h"
#include "timer.h"

/**
 *  Magic EtherType value
 */
#define ETH_TYPE_MAGIC_CMD 0xfffc

/**
 *  PlipUltimate command types
 */
#define CMD_INVALID   0
#define CMD_RESET     1
#define CMD_GETCONFIG 3
#define CMD_SETCONFIG 4
#define CMD_RESPONSE  128

UBYTE cmdReadResponse(UBYTE ubResponseCode) {
	ULONG ulTimeout = 7000000; // Longer timeouts - especially for ROM writes
	
	// Wait for plip's reaction
	while(!g_ubAckEdge && ulTimeout)
		--ulTimeout;
	
	if(!g_ubAckEdge) {
		// Edge not detected
		printf("ERR: No response from plip!\n");
		return 0;
	}
	
	// Receive data from plip
	dataRecv();
	// dataDump(g_pRecvBfr, g_uwRecvSize); // DEBUG
		
	// Check response code
	if(g_pRecvBfr[0] != (ubResponseCode | CMD_RESPONSE)) {
		printf(
			"ERR: Wrong response from plip, got %hu, expected %d\n",
			g_pRecvBfr[0], ubResponseCode | CMD_RESPONSE
		);
		return 0;
	}
	return 1;
}

void cmdReset(void) {
	const UBYTE pResetPacket[14] = {
		CMD_RESET, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		ETH_TYPE_MAGIC_CMD>>8, ETH_TYPE_MAGIC_CMD&0xFF
	};
	
	printf("Sending CMD_RESET...");
	dataSend(pResetPacket, 14);
	printf("OK\n");
}

UBYTE cmdFlash(tPage *pPages, UBYTE ubPageCount) {
	ULONG ulTimer;
	UBYTE i, ubDoHiPout, ubByte;
		
	// Reset to enter boot mode
	cmdReset();
	
	PAR_STATUS_DDR &= ~PAR_STATUS_MASK;
	PAR_STATUS_DDR |= PAR_POUT | PAR_SEL;
	PAR_DATA_DDR = 0xFF;
	PAR_DATA_VAL = 0x00;
	
	// Plip takes ~65ms to wake up
	timerDelayMs(70);

	// Do a bootloader handshake: PApAPApAPApA
	printf("Handshaking bootloader: ");
	for(i = 0; i != 3; ++i) {
		// POut hi
		g_ubAckEdge = 0;
		PAR_STATUS_VAL |= PAR_POUT;
		putc('P', stdout);
		
		// Wait for Ack edge
		ulTimer = 100000;
		while(--ulTimer && !g_ubAckEdge);
		if(!ulTimer)
			goto sendFail;
		putc('A', stdout);
		
		// POut lo
		g_ubAckEdge = 0;
		PAR_STATUS_VAL &= ~PAR_POUT;
		putc('p', stdout);
		
		// Wait for Ack edge
		ulTimer = 100000;
		while(--ulTimer && !g_ubAckEdge);
		if(!ulTimer)
			goto sendFail;
		putc('A', stdout);
	}
	printf(" OK\n");
		
	for(i = 0; i != ubPageCount; ++i) {
		// Clear ACK detection
		g_ubAckEdge = 0;
		
		// Send page write request
		PAR_DATA_VAL = PBPROTO_CMD_SEND_FLASH;
		PAR_STATUS_VAL |= PAR_SEL;
				
		if(!parWaitBusy(1))
			goto sendFail;
		
		// Send page number
		PAR_DATA_VAL = pPages[i].ubIdx;
		PAR_STATUS_VAL |= PAR_POUT;
		if(!parWaitBusy(0))
			goto sendFail;
		
		// Send page bytes
		ubDoHiPout = 0;
		for(ubByte = 0; ubByte != BOOT_PAGE_BYTE_SIZE; ++ubByte) {
			PAR_DATA_VAL = pPages[i].pData[ubByte];
			if(!ubDoHiPout) {
				PAR_STATUS_VAL &= ~PAR_POUT;
				if(!parWaitBusy(1))
					goto sendFail;
				ubDoHiPout = 1;
			}
			else {
				PAR_STATUS_VAL |= PAR_POUT;
				if(!parWaitBusy(0))
					goto sendFail;
				ubDoHiPout = 0;
			}
		}
		
		// Finish page with POUT and SEL low
		PAR_STATUS_VAL &= ~(PAR_POUT|PAR_SEL);
		PAR_DATA_VAL = 0;
		
		// Wait for ACK
		ulTimer = 100000;
		while(ulTimer && !g_ubAckEdge)
			--ulTimer;
		if(!g_ubAckEdge) {
			printf("ERR: No response from plip after sending page number %u\n", i);
			goto sendFail;
		}
	}
	parMakeLow();
	return 1;
	
sendFail:
	parMakeLow();
	return 0;
}

UBYTE cmdConfigGet(tConfig *pConfig) {
	const UBYTE pGetConfigPacket[14] = {
		CMD_GETCONFIG, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		ETH_TYPE_MAGIC_CMD>>8, ETH_TYPE_MAGIC_CMD&0xFF
	};
	
	// Send config request
	printf("Requesting plip config...");
	dataSend(pGetConfigPacket, 14);
	printf("OK\n");
	
	if(cmdReadResponse(CMD_GETCONFIG)) {
		memcpy(pConfig, &g_pRecvBfr[14], sizeof(tConfig));
		return 1;
	}
	return 0;	
}

void cmdConfigSet(tConfig *pConfig, UBYTE ubWriteType) {
	UBYTE pSetConfigPacket[14+sizeof(tConfig)] = {
		CMD_SETCONFIG, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		ETH_TYPE_MAGIC_CMD>>8, ETH_TYPE_MAGIC_CMD&0xFF, 0
	};
	
	if(ubWriteType == WRITE_TYPE_INVALID) {
		printf("ERR: Invalid write type\n");
		return;
	}
	
	// Config type - default (rom) vs current (ram)
	pSetConfigPacket[1] = ubWriteType;
	
	memcpy(&pSetConfigPacket[14+0], pConfig, sizeof(tConfig));
	printf(
		"Writing %s config to plip...",
		ubWriteType == WRITE_TYPE_CURRENT ? "current" : "default"
	);
	dataSend(pSetConfigPacket, 14+sizeof(tConfig));
	printf("OK\n");
	if(cmdReadResponse(CMD_SETCONFIG)) {
		if(g_pRecvBfr[1] == 1)
			printf("Config changed successfully\n");
		else {
			if(g_pRecvBfr[1] & 2)
				printf("Invalid config write type\n");
			else if(g_pRecvBfr[1] & 4)
				printf("Couldn't write config to ROM\n");
		}
	}
}