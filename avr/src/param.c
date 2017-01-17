/*
 * param.c - handle device parameters
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plipbox.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "param.h"
#include "base/uartutil.h"
#include "base/uart.h"
#include "net/net.h"

#include <avr/eeprom.h>
#include <util/crc16.h>
#include <avr/pgmspace.h>
#include <string.h>

//TODO(KaiN#7): Inverse config function return value logic

// current memory RAM param
tConfig g_sConfig;

// eeprom param
tConfig s_sEepromConfig EEMEM;
uint16_t s_uwEepromCrc EEMEM;

// default
static const tConfig PROGMEM sc_sDefaultConfig = {
  .mac_addr = { 0x1a,0x11,0xaf,0xa0,0x47,0x11},

  .flow_ctl = 0,
  .full_duplex = 0,

  .test_plen = 1514,
  .test_ptype = 0xfffd,
  .test_ip = { 192,168,2,222 },
  .test_port = 6800,
  .test_mode = 0
};

static void dump_byte(PGM_P str, const uint8_t val)
{
	// NOTE: UART - str hex_byte(val) \r\n
}

static void dump_word(PGM_P str, const uint16_t val)
{
	// NOTE: UART - str, hex_word(val) \r\n
}

// dump all params
void param_dump(void)
{
  // mac address
  // NOTE: UART - m: mac address, param.mac_addr \r\n

  // options
  // NOTE: UART - \r\n fd: full duplex param.full_duplex
  // NOTE: UART - \r\n fc: flow control param.flow_ctl

  // test
	// NOTE: UART - \r\n tl: packet len param.test_plen
	// NOTE: UART - \r\n tt: packet type param.test_ptype
	// NOTE: UART - \r\n ti: ip address param.test_ip
	// NOTE: UART - \r\n tp: udp port param.test_port
	// NOTE: UART - \r\n tm: test mode param.test_mode
	// NOTE: UART - \r\n
}

// build check sum for parameter block
static uint16_t calc_crc16(tConfig *p)
{
  uint16_t crc16 = 0xffff;
  uint8_t *data = (uint8_t *)p;
  uint16_t i;
  for(i=0;i<sizeof(tConfig);i++) {
    crc16 = _crc16_update(crc16,*data);
    data++;
  }
  return crc16;
}

uint8_t param_save(void)
{
  // check that eeprom is writable
  if(!eeprom_is_ready())
    return PARAM_EEPROM_NOT_READY;

  // write current param to eeprom
  eeprom_write_block(&g_sConfig,&s_sEepromConfig,sizeof(tConfig));

  // calc current parameter crc
  uint16_t crc16 = calc_crc16(&g_sConfig);
  eeprom_write_word(&s_uwEepromCrc,crc16);

  return PARAM_OK;
}

uint8_t param_load(void)
{
  // check that eeprom is readable
  if(!eeprom_is_ready())
    return PARAM_EEPROM_NOT_READY;

  // read param
  eeprom_read_block(&g_sConfig,&s_sEepromConfig,sizeof(tConfig));

  // read crc16
  uint16_t crc16 = eeprom_read_word(&s_uwEepromCrc);
  uint16_t my_crc16 = calc_crc16(&g_sConfig);
  if(crc16 != my_crc16) {
    param_reset();
    return PARAM_EEPROM_CRC_MISMATCH;
  }

  return PARAM_OK;
}

void param_reset(void)
{
	uint8_t i;
  // restore default param
  uint8_t *out = (uint8_t *)&g_sConfig;
  const uint8_t *in = (const uint8_t *)&sc_sDefaultConfig;
  for(i=0;i<sizeof(tConfig);i++) {
    *(out++) = pgm_read_byte_near(in++);
  }
}

void param_init(void)
{
  if(param_load()!=PARAM_OK)
    param_reset();
}
