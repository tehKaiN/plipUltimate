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

#ifndef UDP_H
#define UDP_H

#include "../global.h"
#include "net.h"
#include "ip.h"

   // byte offsets in UDP packets
#define UDP_SRC_PORT_OFF  0
#define UDP_TGT_PORT_OFF  2
#define UDP_LENGTH_OFF    4
#define UDP_CHECKSUM_OFF  6
#define UDP_DATA_OFF      8

inline const uint8_t *udp_get_data_ptr(const uint8_t *udp_buf) { return udp_buf + UDP_DATA_OFF; }
inline uint16_t  udp_get_src_port(const uint8_t *udp_buf) { return net_get_word(udp_buf + UDP_SRC_PORT_OFF); }
inline uint16_t  udp_get_tgt_port(const uint8_t *udp_buf) { return net_get_word(udp_buf + UDP_TGT_PORT_OFF); }
inline uint16_t  udp_get_length(const uint8_t *udp_buf) { return net_get_word(udp_buf + UDP_LENGTH_OFF); }
inline uint16_t  udp_get_checksum(const uint8_t *udp_buf) { return net_get_word(udp_buf + UDP_CHECKSUM_OFF); }

#endif
