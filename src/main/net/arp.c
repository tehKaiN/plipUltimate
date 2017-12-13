/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#include <main/net/arp.h>
#include <main/net/net.h>

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
