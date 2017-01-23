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

#ifndef _SRC_CMD_H
#define _SRC_CMD_H

#include "global.h"

/**
 * Commands are sent  to PlipUltimate as standard eth packet, where EtherType
 * field has value TODO. MAC address fields are used to store command type and
 * basic params.
 *
 * First byte of target MAC addr is used as command type. Supported commands
 * are listed as CMD_* defines.
 */

/**
 * Commands implemented by PlipUltimate
 */
#define CMD_INVALID    0
#define CMD_REBOOT     1
#define CMD_GETLOG     2
#define CMD_GETCONFIG  3
#define CMD_SETCONFIG  4
#define CMD_SDINFO     5
#define CMD_SDREAD     6
#define CMD_SDWRITE    7
#define CMD_RESPONSE 128

extern void cmdProcess(uint16_t uwPacketSize);

extern uint16_t g_uwCmdResponseSize;

#endif // _SRC_CMD_H
