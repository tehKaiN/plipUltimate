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

#ifndef ETH_H
#define ETH_H

#include "../global.h"
#include "net.h"

#define ETH_OFF_TGT_MAC   0
#define ETH_OFF_SRC_MAC   6
#define ETH_OFF_TYPE      12

#define ETH_HDR_SIZE  14

#define ETH_TYPE_IPV4 0x800
#define ETH_TYPE_ARP  0x806

/**
 * Custom EtherType values.
 * ETH_TYPE_MAGIC_ONLINE:
 *   Request by Amiga to go online
 * ETH_TYPE_MAGIC_OFFLINE:
 *   Request by Amiga to go offline
 * ETH_TYPE_MAGIC_LOOPBACK:
 *   TODO: document it.
 * ETH_TYPE_MAGIC_CMD:
 *   Used as non-ethernet commands, such as SD access
 *   or reboot request.
 */
#define ETH_TYPE_MAGIC_ONLINE	  0xffff
#define ETH_TYPE_MAGIC_OFFLINE  0xfffe
#define ETH_TYPE_MAGIC_LOOPBACK 0xfffd
#define ETH_TYPE_MAGIC_CMD      0xfffc

/**
 * Returns pointer to target MAC address in given eth frame.
 * @param pkt Pointer to eth frame.
 * @return Pointer to target MAC address.
 */
inline const uint8_t* eth_get_tgt_mac(const uint8_t *pkt) {
	return pkt + ETH_OFF_TGT_MAC;
}

/**
 * Returns pointer to source MAC address in given eth frame.
 * @param pkt Pointer to eth frame.
 * @return Pointer to source MAC address.
 */
inline const uint8_t *eth_get_src_mac(const uint8_t *pkt) {
	return pkt + ETH_OFF_SRC_MAC;
}

/**
 * Returns EtherType field value in given eth frame.
 * @param pkt Pointer to eth frame.
 * @return 2-byte EtherType field value.
 */
inline uint16_t eth_get_pkt_type(const uint8_t *pkt) {
	return net_get_word(pkt + ETH_OFF_TYPE);
}

inline uint8_t eth_is_arp_pkt(const uint8_t *pkt) {
	return eth_get_pkt_type(pkt) == ETH_TYPE_ARP;
}

inline uint8_t  eth_is_ipv4_pkt(const uint8_t *pkt) {
	return eth_get_pkt_type(pkt) == ETH_TYPE_IPV4;
}

inline void eth_set_pkt_type(uint8_t *pkt, uint16_t type) {
	net_put_word(pkt + ETH_OFF_TYPE, type);
}

inline void eth_make_bcast(uint8_t *pkt, const uint8_t *my_mac)
{
	net_copy_mac(net_bcast_mac, pkt + ETH_OFF_TGT_MAC);
	net_copy_mac(my_mac, pkt + ETH_OFF_SRC_MAC);
}

#endif
