/*
 * This file is part of PlipUltimate.
 * Based on PlipBox by Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "cmd.h"
#include <string.h>
#include "global.h"
#include "pkt_buf.h"
#include "base/util.h"
#include "net/eth.h"
#include "param.h"
#include "spi/enc28j60.h"
#include "pio_util.h"

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
			if(param_save())
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
