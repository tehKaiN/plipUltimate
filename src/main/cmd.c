/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#include <main/cmd.h>
#include <string.h>
#include <main/global.h>
#include <main/pkt_buf.h>
#include <main/base/util.h>
#include <main/net/eth.h>
#include <main/config.h>
#include <main/spi/enc28j60.h>
#include <main/pio_util.h>

/**
 * Config write types.
 *  WRITE_TYPE_CURRENT - write into config stored in RAM
 *  WRITE_TYPE_DEFAULT - write into config stored in ROM
 */
#define WRITE_TYPE_INVALID 0
#define WRITE_TYPE_CURRENT 1
#define WRITE_TYPE_DEFAULT 2

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
	uint8_t ubResult = 1;

	if(
		g_pDataBuffer[1] != WRITE_TYPE_CURRENT &&
		g_pDataBuffer[1] != WRITE_TYPE_DEFAULT
	) {
		ubResult |= 0b10;
	}
	else {
		// Update current config
		memcpy(&g_sConfig, &g_pDataBuffer[ETH_HDR_SIZE], sizeof(tConfig));

		// Update ROM config
		if(g_pDataBuffer[1] == WRITE_TYPE_DEFAULT) {
			if(configSaveToRom())
				ubResult |= 0b100;
		}

		// Reconfigure plip
		enc28j60_exit();
		enc28j60_init(g_sConfig.mac_addr, pio_util_get_init_flags());
	}

	// Prepare response
	g_pDataBuffer[1] = ubResult;
	g_uwCmdResponseSize = ETH_HDR_SIZE;
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
