#include "cmd.h"
#include <string.h>
#include "global.h"
#include "pkt_buf.h"
#include "base/util.h"
#include "net/eth.h"
#include "param.h"

uint16_t g_uwCmdResponseSize;

// Local fn decls
static void cmdReboot(void);
static void cmdGetConfig(void);
static void cmdSetConfig(void);
static void cmdGetLog(void);
static void cmdGetSdInfo(void);
static void cmdSdRead(void);
static void cmdSdWrite(void);

/**
 * PlipUltimate command process function.
 * This function should be called when cmd packet has been received.
 */
void cmdProcess(uint16_t uwPacketSize) {
	uint8_t ubCmdType = g_pDataBuffer[0];
	g_pDataBuffer[0] |= CMD_RESPONSE;
	switch(ubCmdType) {
		case CMD_REBOOT:    cmdReboot();    return;
		case CMD_GETLOG:    cmdGetLog();    return;
		case CMD_GETCONFIG: cmdGetConfig(); return;
		case CMD_SETCONFIG: cmdSetConfig(); return;
		case CMD_SDINFO:    cmdGetSdInfo(); return;
		case CMD_SDREAD:    cmdSdRead();    return;
		case CMD_SDWRITE:   cmdSdWrite();   return;
	}
}

static void cmdReboot(void) {
	utilReset();
}

static void cmdGetLog(void) {
	// TODO(KaiN#9): implement cmdGetLog()
}

static void cmdGetConfig(void) {
	memcpy(&g_pDataBuffer[ETH_HDR_SIZE], &g_sConfig, sizeof(tConfig));
	g_uwCmdResponseSize = ETH_HDR_SIZE + sizeof(tConfig);
}

static void cmdSetConfig(void) {
	// TODO(KaiN#9): implement cmdSetConfig()
}

static void cmdGetSdInfo(void) {
	// TODO(KaiN#9): implement cmdGetSdInfo()
}

static void cmdSdRead(void) {
	// TODO(KaiN#9): implement cmdSdRead()
}

static void cmdSdWrite(void) {
	// TODO(KaiN#9): implement cmdSdWrite()
}
