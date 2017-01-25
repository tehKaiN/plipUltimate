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

#ifndef _CONFIG_H
#define _CONFIG_H

#include "global.h"

/**
 * Main config struct.
 * This struct must be 16-bit aligned to reflect Amiga arch
 */
typedef struct _tConfig {
  uint8_t mac_addr[6]; ///< Card's MAC address.

  uint8_t flow_ctl;    ///< Flow control?
  uint8_t full_duplex; ///< Ethernet full duplex mode.

  uint16_t test_plen;  ///< Test packet length.
  uint16_t test_ptype; ///< Test packet EtherType.
  uint8_t test_ip[4];  ///< Plipbox IP? Used in ARP check.
  uint16_t test_port;
  uint8_t zzPad;
  uint8_t test_mode;
} tConfig;

extern tConfig g_sConfig;

// param result
#define CONFIG_OK                  0
#define CONFIG_EEPROM_NOT_READY    1
#define CONFIG_EEPROM_CRC_MISMATCH 2

// init parameters. try to load from eeprom or use default
void configInit(void);
// save param to eeprom (returns param result)
uint8_t configSaveToRom(void);
// load param from eeprom (returns param result)
uint8_t configLoadFromRom(void);
// reset param
void configReset(void);

#endif // _CONFIG_H
