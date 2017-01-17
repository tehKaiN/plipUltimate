#include <stdio.h>
#include <string.h>
#include <clib/exec_protos.h>
#include "ack.h"
#include "buildver.h"
#include "data.h"
#include "par.h"
#include "cmd.h"

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
	setbuf(stdout, NULL);
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
	
	ackFree();
	parFree();
	
	return 0;
}