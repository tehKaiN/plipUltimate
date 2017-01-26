/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef IP_H
#define IP_H

#include "../global.h"
#include "net.h"

/* list of IP protocol numbers */
#define IP_PROTOCOL_ICMP    0x01
#define IP_PROTOCOL_TCP     0x06
#define IP_PROTOCOL_UDP     0x11

#define IP_MIN_HDR_SIZE     20

#define IP_CHECKSUM_OFF     10

#define UDP_CHECKSUM_OFF    6
#define TCP_CHECKSUM_OFF    16

inline const uint8_t *ip_get_src_ip(const uint8_t *buf) { return buf + 12; }
inline const uint8_t *ip_get_tgt_ip(const uint8_t *buf) { return buf + 16; }
inline uint16_t ip_get_total_length(const uint8_t *buf) { return (uint16_t)buf[2] << 8 | (uint16_t)buf[3]; }
inline uint8_t ip_get_hdr_length(const uint8_t *buf) { return (buf[0] & 0xf) * 4; }
inline uint8_t ip_get_protocol(const uint8_t *buf) { return buf[9]; }

#endif
