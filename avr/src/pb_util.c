/*
 * pb_util.c: plipbox protocol high level helpers
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

#include "pb_util.h"

#include "pb_proto.h"
#include "base/timer.h"
#include "stats.h"
#include "dump.h"
#include "main.h"

uint8_t pb_util_handle(void)
{
  const pb_proto_stat_t *ps = &pb_proto_stat;

  // call protocol handler (low level transmit)
  uint8_t status = pb_proto_handle();
  if(status == PBPROTO_STATUS_IDLE)
    return PBPROTO_STATUS_IDLE; // Nothing done... return

  if(status == PBPROTO_STATUS_OK) {
		// Everything went OK
    // Update stats
    stats_update_ok(ps->stats_id, ps->size, ps->rate);
    if(global_verbose)
      dump_pb_cmd(ps); // In interactive (verbose) mode show result
  }
  else {
		// PB proto failed with an error
    // Dump error
    dump_pb_cmd(ps);
    // Update stats
    stats_get(ps->stats_id)->err++;
  }
  return status;
}
