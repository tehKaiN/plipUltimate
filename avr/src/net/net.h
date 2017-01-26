/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef NET_H
#define NET_H

#include "../global.h"

/* typedefs for generic packet tx */
typedef void (*net_tx_packet_func)(const uint8_t *buf, uint16_t);

extern void net_copy_mac(const uint8_t *in, uint8_t *out);
extern void net_copy_ip(const uint8_t *in, uint8_t *out);

extern uint8_t  net_compare_mac(const uint8_t *a, const uint8_t *b);
extern uint8_t  net_compare_ip(const uint8_t *a, const uint8_t *b);

extern uint16_t  net_get_word(const uint8_t *buf);
extern void net_put_word(uint8_t *buf, uint16_t value);

extern uint32_t  net_get_long(const uint8_t *buf);
extern void net_put_long(uint8_t *buf, uint32_t value);

extern void net_dump_mac(const uint8_t *in);
extern void net_dump_ip(const uint8_t *in);

extern uint8_t net_parse_ip(const char *buf, uint8_t *ip);
extern uint8_t net_parse_mac(const char *buf, uint8_t *mac);

/* constants */
extern const uint8_t net_bcast_mac[6];
extern const uint8_t net_zero_mac[6];
extern const uint8_t net_zero_ip[4];
extern const uint8_t net_ones_ip[4];

/* convenience functions */
inline void net_copy_bcast_mac(uint8_t *out) { net_copy_mac(net_bcast_mac, out); }
inline void net_copy_zero_mac(uint8_t *out) { net_copy_mac(net_zero_mac, out); }

inline void net_copy_zero_ip(uint8_t *out) { net_copy_ip(net_zero_ip, out); }
inline uint8_t net_compare_bcast_ip(const uint8_t *in) { return net_compare_ip(net_ones_ip, in); }

inline uint8_t net_compare_bcast_mac(const uint8_t *in) { return net_compare_mac(net_bcast_mac, in); }
inline uint8_t net_compare_zero_mac(const uint8_t *in) { return net_compare_mac(net_zero_mac, in); }

#endif
