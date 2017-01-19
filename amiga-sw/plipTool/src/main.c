#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <clib/exec_protos.h>
#include "ack.h"
#include "buildver.h"
#include "data.h"
#include "par.h"
#include "cmd.h"

#define BOOT_PAGE_BYTE_SIZE 128
#define BOOT_PAGE_COUNT 256
#define BOOT_NRWW_PAGE_COUNT 32
#define BOOT_RWW_PAGE_COUNT (BOOT_PAGE_COUNT - BOOT_NRWW_PAGE_COUNT)

typedef struct _tPage {
	UBYTE ubIdx;
	UBYTE pData[BOOT_PAGE_BYTE_SIZE];
} tPage;

void printUsage(void) {
	
}

UBYTE parseMac(tConfig *pConfig, char *szMac) {
	UWORD uwA, uwB, uwC, uwD, uwE, uwF;
	if(strlen(szMac) != 6*2+5)
		return 0;
	if(
		sscanf(
			szMac, "%hx:%hx:%hx:%hx:%hx:%hx", &uwA, &uwB, &uwC, &uwD, &uwE, &uwF
		) < 6
	)
		return 0;
	pConfig->mac_addr[0] = uwA;
	pConfig->mac_addr[1] = uwB;
	pConfig->mac_addr[2] = uwC;
	pConfig->mac_addr[3] = uwD;
	pConfig->mac_addr[4] = uwE;
	pConfig->mac_addr[5] = uwF;
	
	return 1;
}

void configDisplay(tConfig *pConfig) {
	printf(
		"MAC addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
		pConfig->mac_addr[0], pConfig->mac_addr[1], pConfig->mac_addr[2],
		pConfig->mac_addr[3], pConfig->mac_addr[4], pConfig->mac_addr[5]
	);
	printf("Flow control: %s\n", pConfig->flow_ctl ? "enabled" : "disabled");
	printf(
		"Ethernet full duplex: %s\n",
		pConfig->full_duplex ? "enabled" : "disabled"
	);
	printf(
		"Test IP: %hu.%hu.%hu.%hu\n",
		pConfig->test_ip[0], pConfig->test_ip[1],
		pConfig->test_ip[2], pConfig->test_ip[3]
	);
}

int main(int lArgCount, char **pArgs) {
	printf("plipTool ver. %d.%d.%d.\n", BUILD_YEAR, BUILD_MONTH, BUILD_DAY);
	
	if(!parReserve("plipTool")) {		
		return 1;
	}
	if(!ackReserve()) {
		parFree();
		return 1;
	}
	
	if(lArgCount < 2) {
		printf("ERR: no commands specified\n");
		printUsage();
		ackFree();
		parFree();
		return 0;
	}

	if(!strcmp(pArgs[1], "reboot")) {
		cmdReset();
	}
	else if(!strcmp(pArgs[1], "config")) {
		tConfig sConfig;
		if(!cmdConfigGet(&sConfig)) {
			printf("ERR: Read config error");
		}
		else {
			if(lArgCount == 2) {
				configDisplay(&sConfig);
			}
			else {
				UBYTE i, ubErr, ubWriteType;
				ubWriteType = WRITE_TYPE_CURRENT;
				
				ubErr = 0;
				for(i = 2; i != lArgCount; ++i) {
					if(!strcmp(pArgs[i], "default"))
						ubWriteType = WRITE_TYPE_DEFAULT;
					else if(!strcmp(pArgs[i], "mac")) {
						if(!parseMac(&sConfig, pArgs[i+1])) {
							printf("ERR: Invalid MAC address\n");
							ubErr = 1;
							break;
						}
						++i;
					}
				}
				if(!ubErr)
					cmdConfigSet(&sConfig, ubWriteType);
			}
		}
	}
	else if(!strcmp(pArgs[1], "flash")) {
		UBYTE ubPageCount, i;
		FILE *pPufFile;
		tPage *pPages;
		
		if(lArgCount < 3)
			printf("ERR: No PUF file specified\n");
		else {
			pPufFile = fopen(pArgs[2], "rb");
			if(!pPufFile)
				printf("ERR: couldn't open PUF file: %s", pArgs[2]);
			else {
				// Read all pages from file
				fread(&ubPageCount, 1, 1, pPufFile);
				pPages = malloc(sizeof(tPage)*ubPageCount);
				printf("Reading %u pages...\n", ubPageCount);
				for(i = 0; i != ubPageCount; ++i) {
					fread(&pPages[i].ubIdx, 1, 1, pPufFile);
					fread(&pPages[i].pData, BOOT_PAGE_BYTE_SIZE, 1, pPufFile);
					printf("Read page %u\n", i);
				}
				
				// Send pages to Ami
				cmdReset();
				PAR_STATUS_DDR &= ~PAR_POUT;
				PAR_STATUS_DDR |= PAR_BUSY;
				ULONG ulTimer = 7000000;
				while(ulTimer && !(PAR_STATUS_VAL & PAR_POUT))
					--ulTimer;
				if(!ulTimer) {
					printf("ERR: Boot loader doesn't respond\n");
				}
				else {
					UBYTE ubDoHiPout, ubByte;
					
					// Make BUSY pulse to confirm flash mode
					PAR_STATUS_VAL |= PAR_BUSY;
					PAR_STATUS_VAL &= ~PAR_BUSY;
					
					// Switch DDR to normal
					PAR_STATUS_DDR &= ~PAR_BUSY;
					PAR_STATUS_DDR |= PAR_POUT;
					
					PAR_DATA_DDR = 0xFF;
					for(i = 0; i != ubPageCount; ++i) {
						// Clear ACK detection
						g_ubAckEdge = 0;
						
						// Send page write request
						PAR_DATA_VAL = PBPROTO_CMD_SEND_BOOT;
						PAR_STATUS_VAL |= PAR_SEL;
						parWaitBusyHi();
						
						// Send page number
						PAR_DATA_VAL = pPages[i].ubIdx;
						PAR_STATUS_VAL |= PAR_POUT;
						parWaitBusyLo();
						
						// Send page bytes
						ubDoHiPout = 0;
						for(ubByte = 0; ubByte != BOOT_PAGE_BYTE_SIZE; ++ubByte) {
							PAR_DATA_VAL = pPages[i].pData[ubByte];
							if(ubDoHiPout) {
								PAR_STATUS_VAL |= PAR_POUT;
								parWaitBusyLo();
								ubDoHiPout = 0;
							}
							else {
								PAR_STATUS_VAL &= ~PAR_POUT;
								parWaitBusyHi();
								ubDoHiPout = 1;
							}
						}
						
						// Finish page with POUT and SEL low
						PAR_STATUS_VAL &= ~(PAR_POUT|PAR_SEL);
						PAR_DATA_VAL = 0;
						
						// Wait for ACK
						ulTimer = 7000000;
						while(ulTimer && !g_ubAckEdge)
							--ulTimer;
						if(!g_ubAckEdge) {
							printf("ERR: No response from plip after sending page number %u\n", i);
							break;
						}
					}
					printf("plipUltimate flashed!\n");
				}
				
				free(pPufFile);
			}
		}
	}
	ackFree();
	parFree();
	
	return 0;
}