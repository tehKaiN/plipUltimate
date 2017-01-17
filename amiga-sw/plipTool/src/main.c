#include <stdio.h>
#include <string.h>
#include <clib/exec_protos.h>
#include "ack.h"
#include "buildver.h"
#include "data.h"
#include "par.h"

/**
 *  Magic EtherType value
 */
#define ETH_TYPE_MAGIC_CMD 0xfffc

/**
 *  PlipUltimate command types
 */
#define CMD_INVALID 0
#define CMD_RESET 1
#define CMD_GETCONFIG 3
#define CMD_SETCONFIG  4
#define CMD_RESPONSE 128

typedef struct _tConfig {
  UBYTE mac_addr[6]; ///< Card's MAC address.

  UBYTE flow_ctl;    ///< Flow control?
  UBYTE full_duplex; ///< Ethernet full duplex mode.

  UWORD test_plen;   ///< Test packet length.
  UWORD test_ptype;  ///< Test packet EtherType.
  UBYTE test_ip[4];  ///< Plipbox IP? Used in ARP check.
  UWORD test_port;
  UBYTE test_mode;
} tConfig;

const UBYTE s_pResetCmd[14] = {
	CMD_RESET, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	ETH_TYPE_MAGIC_CMD>>8, ETH_TYPE_MAGIC_CMD&0xFF
};

const UBYTE s_pGetConfigCmd[14] = {
	CMD_GETCONFIG, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	ETH_TYPE_MAGIC_CMD>>8, ETH_TYPE_MAGIC_CMD&0xFF
};

void plipReset(void) {
	printf("Sending CMD_RESET...");
	dataSend(s_pResetCmd, 14);
	printf("OK\n");
}

UBYTE plipGetConfig(void) {
	UWORD uwTimeout = 1;
	
	// Reset ACK edge detection
	g_ubAckEdge = 0;
	
	// Send config request
	printf("Requesting plip config...");
	dataSend(s_pGetConfigCmd, 14);
	printf("OK\n");
	
	// Wait for plip's reaction
	while(!g_ubAckEdge && !uwTimeout)
		++uwTimeout;
	
	if(!g_ubAckEdge) {
		// Edge not detected
		printf("ERR: No response from plip!\n");
		return 0;
	}
	
	// Receive data from plip
	dataRecv();
	dataDump(g_pRecvBfr, g_uwRecvSize); // DEBUG
	
	if(g_pRecvBfr[0] != (CMD_GETCONFIG | CMD_RESPONSE)) {
		printf(
			"ERR: Wrong response from plip, got %hu, expected %hu\n",
			g_pRecvBfr[0], CMD_GETCONFIG | CMD_RESPONSE
		);
		return 0;
	}
	puts("Received config from plip");
	printf(
		"packet size: %u, struct data size: %d, Ami struct size: %lu\n",
		g_uwRecvSize, g_uwRecvSize-14, sizeof(tConfig)
	);

	tConfig *pConfig = (tConfig*)&g_pRecvBfr[14];
	// Display struct
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
		"Test IP: %hu.%hu%.hu.%hu\n",
		pConfig->test_ip[0], pConfig->test_ip[1],
		pConfig->test_ip[2], pConfig->test_ip[3]
	);
	
	return 1;
}

void printUsage(void) {
	
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
		plipReset();
	}
	else if(!strcmp(pArgs[1], "getconfig")) {
		plipGetConfig();
	}
	
	ackFree();
	parFree();
	
	return 0;
}