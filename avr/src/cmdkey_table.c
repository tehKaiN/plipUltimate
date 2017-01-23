/*
 * cmdkey_table.c - command key table
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

#include "cmdkey_table.h"

#include "stats.h"
#include "base/uartutil.h"

COMMAND_KEY(cmd_dump_stats)
{
  stats_dump_all();
}

CMDKEY_HELP(cmd_enter_bridge_mode, "enter bridge mode");
CMDKEY_HELP(cmd_enter_bridge_test_mode, "enter bridge test mode");
CMDKEY_HELP(cmd_enter_pio_test_mode, "enter PIO test mode");
CMDKEY_HELP(cmd_enter_pb_test_mode, "enter PB test mode");
CMDKEY_HELP(cmd_dump_stats, "dump statistics");
CMDKEY_HELP(cmd_reset_stats, "reset statistics");
CMDKEY_HELP(cmd_toggle_verbose, "toggle verbose output");
CMDKEY_HELP(cmd_send_test_packet, "send a test packet (pbtest mode)");
CMDKEY_HELP(cmd_send_test_packet_silent, "send a test packet (silent) (pbtest mode)");
CMDKEY_HELP(cmd_toggle_auto_mode, "toggle auto send (pbtest mode)");

const cmdkey_table_t PROGMEM cmdkey_table[] = {
  CMDKEY_ENTRY('s', cmd_dump_stats),
  { 0,0 }
};
