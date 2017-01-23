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

#ifndef GUARD_CMD_H
#define GUARD_CMD_H

#include <clib/exec_protos.h>
#include "main.h"

/**
 *  Config write types.
 *  WRITE_TYPE_CURRENT - write into config stored in RAM
 *  WRITE_TYPE_DEFAULT - write into config stored in ROM
 */
#define WRITE_TYPE_INVALID 0
#define WRITE_TYPE_CURRENT 1
#define WRITE_TYPE_DEFAULT 2

/**
 *  Amiga has 16-bit alignment, AVR has 8-bit.
 *  Thus received config's test_mode must be shifted by one byte.
 */
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

void cmdReset(void);
UBYTE cmdFlash(tPage *pPages, UBYTE ubPageCount);
UBYTE cmdConfigGet(tConfig *pConfig);
void cmdConfigSet(tConfig *pConfig, UBYTE ubWriteType);
UBYTE cmdReadResponse(UBYTE ubResponseCode);

#endif // GUARD_CMD_H
