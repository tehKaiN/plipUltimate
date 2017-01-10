/*
 * cmd_table.c - command table
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

#include "cmd_table.h"
#include "base/uartutil.h"
#include "base/util.h"

#include "net/net.h"
#include "param.h"
#include "stats.h"

COMMAND(cmd_quit)
{
  return CMD_QUIT;
}

COMMAND(cmd_device_reset)
{
  return CMD_RESET;
}

COMMAND(cmd_version)
{
	// NOTE: UART - VERSION BUILD_DATE\r\n
  return CMD_OK;
}

COMMAND(cmd_param_dump)
{
  param_dump();
  return CMD_OK;
}

COMMAND(cmd_param_save)
{
  uint8_t result = param_save();
  if(result == PARAM_OK) {
    return CMD_OK;
  } else {
    return CMD_MASK_ERROR | result;
  }
}

COMMAND(cmd_param_load)
{
  uint8_t result = param_load();
  if(result == PARAM_OK) {
    return CMD_OK;
  } else {
    return CMD_MASK_ERROR | result;
  }
}

COMMAND(cmd_param_reset)
{
  param_reset();
  return CMD_OK;
}

COMMAND(cmd_param_toggle)
{
  uint8_t group = argv[0][0];
  uint8_t type = argv[0][1];
  uint8_t *val = 0;
  uint8_t result = CMD_OK;

  if(group == 't') {
    switch(type) {
      case 'm': val = &g_sConfig.test_mode; break;
      default: return CMD_PARSE_ERROR;
    }
  }
  else if(group == 'f') {
    switch(type) {
      case 'd': val = &g_sConfig.full_duplex; result = CMD_OK_RESTART; break;
      case 'c': val = &g_sConfig.flow_ctl; result = CMD_OK_RESTART; break;
      default: return CMD_PARSE_ERROR;
    }
  }
  else {
    return CMD_PARSE_ERROR;
  }

  if(argc == 1) {
    // toggle value if no argument is given
    *val = *val ? 0 : 1;
  } else {
    uint8_t new_val;
    if(utilParseByteHex((char*)argv[1],&new_val)) {
      *val = new_val;
    } else {
      return CMD_PARSE_ERROR;
    }
  }
  return result;
}

COMMAND(cmd_param_word)
{
  uint8_t group = argv[0][0];
  uint8_t type = argv[0][1];
  uint16_t *val = 0;

  if(group == 't') {
    switch(type) {
      case 'l': val = &g_sConfig.test_plen; break;
      case 't': val = &g_sConfig.test_ptype; break;
      case 'p': val = &g_sConfig.test_port; break;
      default: return CMD_PARSE_ERROR;
    }
  }
  else {
    return CMD_PARSE_ERROR;
  }

  if(argc == 1) {
    return CMD_PARSE_ERROR;
  } else {
    uint16_t new_val;
    if(utilParseWordHex((char*)argv[1],&new_val)) {
      *val = new_val;
    } else {
      return CMD_PARSE_ERROR;
    }
  }
  return CMD_OK;
}

COMMAND(cmd_param_mac_addr)
{
  uint8_t mac[6];

  if(net_parse_mac((char*)argv[1], mac)) {
    net_copy_mac(mac, g_sConfig.mac_addr);
    return CMD_OK;
  } else {
    return CMD_PARSE_ERROR;
  }
}

COMMAND(cmd_param_ip_addr)
{
  uint8_t ip[4];

  if(net_parse_ip((char*)argv[1], ip)) {
    net_copy_ip(ip, g_sConfig.test_ip);
    return CMD_OK;
  } else {
    return CMD_PARSE_ERROR;
  }
}

COMMAND(cmd_stats_dump)
{
  stats_dump_all();
  return CMD_OK;
}

COMMAND(cmd_stats_reset)
{
  stats_reset();
  return CMD_OK;
}

// ----- Names -----
CMD_NAME("q", cmd_quit, "quit command mode");
CMD_NAME("r", cmd_device_reset, "soft reset device");
CMD_NAME("v", cmd_version, "print firmware version");
  // param
CMD_NAME("p", cmd_param_dump, "print parameters");
CMD_NAME("ps", cmd_param_save, "save parameters to EEPROM");
CMD_NAME("pl", cmd_param_load, "load parameters from EEPROM" );
CMD_NAME("pr", cmd_param_reset, "reset parameters to default" );
  // stats
CMD_NAME("sd", cmd_stats_dump, "dump statistics" );
CMD_NAME("sr", cmd_stats_reset, "reset statistics" );
  // options
CMD_NAME("m", cmd_gen_m, "mac address of device <mac>" );
CMD_NAME("fd", cmd_gen_fd, "set full duple mode [on]" );
CMD_NAME("fc", cmd_gen_fc, "set flow control [on]" );
  // test
CMD_NAME("tl", cmd_gen_tl,  "test packet length <n>");
CMD_NAME("tt", cmd_gen_tt, "test packet eth type <n>" );
CMD_NAME("ti", cmd_gen_ti, "test IP address <ip>" );
CMD_NAME("tp", cmd_gen_tp, "test UDP port <n>" );
CMD_NAME("tm", cmd_gen_tm, "test mode [0|1]" );

// ----- Entries -----
const cmd_table_t PROGMEM cmd_table[] = {
  CMD_ENTRY(cmd_quit),
  CMD_ENTRY(cmd_device_reset),
  CMD_ENTRY(cmd_version),
  // param
  CMD_ENTRY(cmd_param_dump),
  CMD_ENTRY(cmd_param_save),
  CMD_ENTRY(cmd_param_load),
  CMD_ENTRY(cmd_param_reset),
  // stats
  CMD_ENTRY(cmd_stats_dump),
  CMD_ENTRY(cmd_stats_reset),
  // options
  CMD_ENTRY_NAME(cmd_param_mac_addr, cmd_gen_m),
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_fd),
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_fc),
  // test
  CMD_ENTRY_NAME(cmd_param_word, cmd_gen_tl),
  CMD_ENTRY_NAME(cmd_param_word, cmd_gen_tt),
  CMD_ENTRY_NAME(cmd_param_ip_addr, cmd_gen_ti),
  CMD_ENTRY_NAME(cmd_param_word, cmd_gen_tp),
  CMD_ENTRY_NAME(cmd_param_toggle, cmd_gen_tm),
  { 0,0 } // last entry
};
