/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
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
