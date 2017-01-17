#include <stdio.h>
#include <string.h>
#include <resources/misc.h>
#include <clib/exec_protos.h>
#include <clib/misc_protos.h>
#include <hardware/cia.h>
#include "ack.h"
#include "buildver.h"

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
#define CMD_GETCONFIG 3
#define CMD_SETCONFIG  4
#define CMD_RESPONSE 128

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

typedef struct _tConfig {
  UBYTE mac_addr[6]; ///< Card's MAC address.

  UBYTE flow_ctl;    ///< Flow control?
  UBYTE full_duplex; ///< Ethernet full duplex mode.

  UWORD test_plen;  ///< Test packet length.
  UWORD test_ptype; ///< Test packet EtherType.
  UBYTE test_ip[4];  ///< Plipbox IP? Used in ARP check.
  UWORD test_port;
  UBYTE test_mode;
} tConfig;

#define RECV_BFR_SIZE 1600
UBYTE s_pRecvBfr[RECV_BFR_SIZE];
UWORD s_uwRecvSize;

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
	PAR_DATA_DDR = 0x00;
	PAR_STATUS_VAL &= ~PAR_STATUS_MASK;
}

void plipRecv(void) {
	UWORD i;
	UBYTE ubDoHiPout;
	
	s_uwRecvSize = 0;
	memset(s_pRecvBfr, 0, RECV_BFR_SIZE);// DEBUG
	
	PAR_STATUS_DDR &= ~PAR_STATUS_MASK;             // Clear status DDR
	PAR_STATUS_DDR |= _BV(PAR_POUT) | _BV(PAR_SEL); // Set status DDR
	PAR_STATUS_VAL &= ~PAR_STATUS_MASK;             // Clear status pins
	
	// Send recv request
	PAR_DATA_DDR = 0xFF;             // Set data DDR to output
	PAR_DATA_VAL = PBPROTO_CMD_RECV; // Send recv request cmd
	PAR_STATUS_VAL |= _BV(PAR_SEL);  // Trigger plip
	waitBusyHi();                    // Wait for plip ack
	
	// Receive data
	PAR_DATA_DDR = 0x00;               // Set data DDR to input
	
	PAR_STATUS_VAL |= _BV(PAR_POUT);   // Let plip know that we're ready for more
	waitBusyLo();                      // Wait for BUSY change
	s_uwRecvSize |= PAR_DATA_VAL << 8; // Read upper size byte
	
	PAR_STATUS_VAL &= ~_BV(PAR_POUT);
	waitBusyHi();
	s_uwRecvSize |= PAR_DATA_VAL;      // Read lower size byte
	
	ubDoHiPout = 1;
	for(i = 0; i != s_uwRecvSize; ++i) {
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
		s_pRecvBfr[i] = PAR_DATA_VAL;
	}
	
	// Make all pins low
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

UBYTE plipGetConfig(void) {
	UWORD uwTimeout = 1;
	
	// Reset ACK edge detection
	g_ubAckEdge = 0;
	
	// Send config request
	printf("Requesting plip config...");
	plipSend(s_pGetConfigCmd, 14);
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
	plipRecv();
	dataDump(s_pRecvBfr, s_uwRecvSize); // DEBUG
	
	if(s_pRecvBfr[0] != (CMD_GETCONFIG | CMD_RESPONSE)) {
		printf(
			"ERR: Wrong response from plip, got %hu, expected %hu\n",
			s_pRecvBfr[0], CMD_GETCONFIG | CMD_RESPONSE
		);
		return 0;
	}
	puts("Received config from plip");
	printf(
		"packet size: %u, struct data size: %d, Ami struct size: %lu\n",
		s_uwRecvSize, s_uwRecvSize-14, sizeof(tConfig)
	);

	tConfig *pConfig = (tConfig*)&s_pRecvBfr[14];
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