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
