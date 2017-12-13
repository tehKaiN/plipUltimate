/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#include <main/global.h>

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
