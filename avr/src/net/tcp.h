/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef tcp_H
#define tcp_H

#include "../global.h"
#include "net.h"
#include "ip.h"

   // byte offsets in TCP packets
#define TCP_SRC_PORT_OFF  0
#define TCP_TGT_PORT_OFF  2
#define TCP_SEQ_NUM_OFF   4
#define TCP_ACK_NUM_OFF   8
#define TCP_DATA_SIZE_OFF 12
#define TCP_FLAGS_OFF     12
#define TCP_WINDOW_OFF    14

  // flag masks
#define TCP_FLAGS_FIN     0x001
#define TCP_FLAGS_SYN     0x002
#define TCP_FLAGS_RST     0x004
#define TCP_FLAGS_PSH     0x008
#define TCP_FLAGS_ACK     0x010
#define TCP_FLAGS_URG     0x020
#define TCP_FLAGS_ECE     0x040
#define TCP_FLAGS_CWR     0x080
#define TCP_FLAGS_NS      0x100

inline const uint8_t *tcp_get_data_ptr(const uint8_t *tcp_buf) { return tcp_buf + (tcp_buf[TCP_DATA_SIZE_OFF] >> 4) * 4; }
inline uint16_t  tcp_get_src_port(const uint8_t *tcp_buf) { return net_get_word(tcp_buf + TCP_SRC_PORT_OFF); }
inline uint16_t  tcp_get_tgt_port(const uint8_t *tcp_buf) { return net_get_word(tcp_buf + TCP_TGT_PORT_OFF); }
inline uint32_t  tcp_get_seq_num(const uint8_t *tcp_buf) { return net_get_long(tcp_buf + TCP_SEQ_NUM_OFF); }
inline uint32_t  tcp_get_ack_num(const uint8_t *tcp_buf) { return net_get_long(tcp_buf + TCP_ACK_NUM_OFF); }
inline uint16_t  tcp_get_flags(const uint8_t *tcp_buf) { return net_get_word(tcp_buf + TCP_FLAGS_OFF) & 0x1ff; }
inline uint16_t  tcp_get_window_size(const uint8_t *tcp_buf) { return net_get_word(tcp_buf + TCP_WINDOW_OFF); }

#endif
