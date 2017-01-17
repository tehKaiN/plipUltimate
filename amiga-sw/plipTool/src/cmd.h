#ifndef GUARD_CMD_H
#define GUARD_CMD_H

#include <clib/exec_protos.h>

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
UBYTE cmdConfigGet(tConfig *pConfig);
void cmdConfigSet(tConfig *pConfig, UBYTE ubWriteType);
UBYTE cmdReadResponse(UBYTE ubResponseCode);

#endif // GUARD_CMD_H
