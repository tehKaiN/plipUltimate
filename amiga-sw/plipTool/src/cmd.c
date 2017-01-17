#include "cmd.h"
#include <stdio.h>
#include <string.h>
#include "ack.h"
#include "data.h"


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