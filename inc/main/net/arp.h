/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef ARP_H
#define ARP_H

#include <main/global.h>
#include <main/net/net.h>

#define ARP_OFF_HW_TYPE   0
#define ARP_OFF_PROT_TYPE 2
#define ARP_OFF_HW_SIZE   4
#define ARP_OFF_PROT_SIZE 5
#define ARP_OFF_OP        6
#define ARP_OFF_SRC_MAC   8
#define ARP_OFF_SRC_IP    14
#define ARP_OFF_TGT_MAC   18
#define ARP_OFF_TGT_IP    24

#define ARP_SIZE          28

#define ARP_REQUEST       1
#define ARP_REPLY         2

extern uint8_t arp_is_ipv4(const uint8_t *buf, uint16_t len);
extern void arp_make_reply(uint8_t *buf, const uint8_t *my_mac, const uint8_t *my_ip);

/* getter */
inline uint16_t arp_get_op(const uint8_t *buf) { return net_get_word(buf + ARP_OFF_OP); }
inline const uint8_t* arp_get_src_mac(const uint8_t *buf) { return buf + ARP_OFF_SRC_MAC; }
inline const uint8_t* arp_get_src_ip(const uint8_t *buf) { return buf + ARP_OFF_SRC_IP; }
inline const uint8_t* arp_get_tgt_mac(const uint8_t *buf) { return buf + ARP_OFF_TGT_MAC; }
inline const uint8_t* arp_get_tgt_ip(const uint8_t *buf) { return buf + ARP_OFF_TGT_IP; }

#endif
