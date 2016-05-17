/*
 * dump.h - helper functions for debugging
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

#ifndef DUMP_H
#define DUMP_H

#include "global.h"

#include "pb_proto.h"

extern void dump_eth_pkt(const uint8_t *eth_buf, uint16_t size);
extern void dump_arp_pkt(const uint8_t *arp_buf);
extern void dump_ip_pkt(const uint8_t *ip_buf);
extern void dump_ip_protocol(const uint8_t *ip_buf);

extern void dump_pb_cmd(const pb_proto_stat_t *ps);

extern void dump_line(const uint8_t *eth_buf, uint16_t size);

#endif
