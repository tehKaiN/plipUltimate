/*
 * arp.c - handle ARP protocol
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

#include "arp.h"
#include "net.h"

uint8_t arp_is_ipv4(const uint8_t *buf, uint16_t len)
{
  if(len < ARP_SIZE) {
    return 0;
  }

  uint16_t hw_type = net_get_word(buf + ARP_OFF_HW_TYPE);
  uint16_t pt_type = net_get_word(buf + ARP_OFF_PROT_TYPE);
  uint8_t hw_size = buf[ARP_OFF_HW_SIZE];
  uint8_t pt_size = buf[ARP_OFF_PROT_SIZE];

  return (hw_type == 1) && (pt_type == 0x800) && (hw_size == 6) && (pt_size == 4);
}

void arp_make_reply(uint8_t *buf, const uint8_t *my_mac, const uint8_t *my_ip)
{
	// make a reply
	net_put_word(buf + ARP_OFF_OP, ARP_REPLY);
	net_copy_mac(buf + ARP_OFF_SRC_MAC, buf + ARP_OFF_TGT_MAC);
	net_copy_ip(buf + ARP_OFF_SRC_IP, buf + ARP_OFF_TGT_IP);
	net_copy_mac(my_mac, buf + ARP_OFF_SRC_MAC);
	net_copy_ip(my_ip, buf + ARP_OFF_SRC_IP);
}
