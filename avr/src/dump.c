/*
 * dump.c - helper functions for debugging
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of pbprotobox.
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

#include "dump.h"
#include "base/uart.h"
#include "base/uartutil.h"
#include "net/net.h"
#include "net/arp.h"
#include "net/eth.h"
#include "net/ip.h"
#include "net/udp.h"
#include "net/tcp.h"
#include "param.h"
#include "base/util.h"
#include "pb_proto.h"

void dump_eth_pkt(const uint8_t *eth_buf, uint16_t size)
{
  uint8_t buf[4];

  // NOTE: UART - [dword_to_dec(size), hex_word(eth_get_pkt_type(eth_buf)), eth_get_src_mac(eth_buf) > eth_get_tgt_mac(eth_buf)]
}

void dump_arp_pkt(const uint8_t *arp_buf)
{
	// NOTE: UART - [ARP:

  // ARP op
  uint16_t op = arp_get_op(arp_buf);
  if(op == ARP_REQUEST) {
		// NOTE: UART - REQ
  } else if(op == ARP_REPLY) {
		// NOTE: UART - REPL
  } else {
		// NOTE: UART - hex_word(op)
  }

  // src-tgt pair
  // NOTE: UART - ,(net_dump_mac(arp_get_src_mac(arp_buf)), net_dump_ip(arp_get_src_ip(arp_buf))) > (net_dump_mac(arp_get_tgt_mac(arp_buf)) , net_dump_ip(arp_get_tgt_ip(arp_buf)))]
}

void dump_ip_pkt(const uint8_t *ip_buf)
{
	// NOTE: UART - [IP4:

  // size
  // NOTE: UART - hex_word(ip_get_total_length(ip_buf))

  // ip proto
  uint8_t proto = ip_get_protocol(ip_buf);
  if(proto == IP_PROTOCOL_ICMP) {
		// NOTE: UART - ,ICMP
  } else if(proto == IP_PROTOCOL_TCP) {
		// NOTE: UART - ,TCP
  } else if(proto == IP_PROTOCOL_UDP) {
		// NOTE: UART - UDP
  } else {
		// NOTE: UART - , hex_word(proto)
  }

  // src/tgt ip
  // NOTE: UART - , net_dump_ip(ip_get_src_ip(ip_buf)) > net_dump_ip(ip_get_tgt_ip(ip_buf)) ]
}

static void dump_udp_port(uint16_t port)
{
  if(port == 67) {
		// NOTE: UART - BOOTPS
  } else if(port == 68) {
		// NOTE: UART - BOOTPC
  } else {
		// NOTE: UART - hex_word(port)
  }
}

static void dump_tcp_port(uint16_t port)
{
  if(port == 21) {
		// NOTE: UART - FTPctl
  } else if(port == 20) {
		// NOTE: UART - FTPdat
  } else {
		// NOTE: UART - hex_word(port)
  }
}

extern void dump_ip_protocol(const uint8_t *ip_buf)
{
  const uint8_t *proto_buf = ip_buf + ip_get_hdr_length(ip_buf);
  uint8_t proto = ip_get_protocol(ip_buf);
  if(proto == IP_PROTOCOL_UDP) {
		// NOTE: UART - [UDP:
    uint16_t src_port = udp_get_src_port(proto_buf);
    uint16_t tgt_port = udp_get_tgt_port(proto_buf);
    // NOTE: UART - src_port > tgt_port]
  }
  else if(proto == IP_PROTOCOL_TCP) {
		// NOTE: UART -   [TCP:
    uint16_t src_port = tcp_get_src_port(proto_buf);
    uint16_t tgt_port = tcp_get_tgt_port(proto_buf);
    // NOTE: UART - src_port > tgt_port

    uint16_t flags = tcp_get_flags(proto_buf);
		// NOTE: UART - ,flags= hex_word(flags)

    uint32_t seq = tcp_get_seq_num(proto_buf);
    // NOTE: UART - ,seq= hex_word(seq)

    if(flags & TCP_FLAGS_ACK) {
      uint32_t ack = tcp_get_ack_num(proto_buf);
      // NOTE: UART - ,ack= hex_word(ack)
    }

    // NOTE: UART - ]
  }
}

extern void dump_line(const uint8_t *eth_buf, uint16_t size)
{
  dump_eth_pkt(eth_buf, size);

  const uint8_t *ip_buf = eth_buf + ETH_HDR_SIZE;
  uint16_t type = eth_get_pkt_type(eth_buf);
  if(type == ETH_TYPE_ARP) {
    dump_arp_pkt(ip_buf);
  } else if(type == ETH_TYPE_IPV4) {
    dump_ip_pkt(ip_buf);
    dump_ip_protocol(ip_buf);
  }
}

void dump_pb_cmd(const pb_proto_stat_t *ps)
{
  uint8_t buf[4];

  // NOTE: UART - time_stamp_spc()

  // show command
  uint8_t cmd = ps->cmd;
  uint8_t is_valid = 1;
  switch(cmd) {
    case PBPROTO_CMD_SEND:
    case PBPROTO_CMD_SEND_BURST:
      break;
    case PBPROTO_CMD_RECV:
    case PBPROTO_CMD_RECV_BURST:
      break;
    default:
      is_valid = 0;
      break;
  }

  uint8_t status = ps->status;

  // invalid command
  if(!is_valid) {
		// NOTE: UART - cmd=cmd?? ERR:status\r\n
    return;
  }

  // NOTE: UART - ps->is_send ? [TX: : [RX: hex_byte(cmd)

  // result
  if(status == PBPROTO_STATUS_OK) {
		// NOTE: UART - ] ok
  } else {
		// NOTE: UART - ] ERR: hex_byte(status)
  }

  // packet size
  // NOTE: UART - n= dword_to_dec(ps->size, buf, 4, 4)

  // packet delta
  // NOTE: UART - d= dword_to_dec(ps->delta, buf, 4, 4)

  // speed
  // NOTE: UART - v= uart_send_rate_kbs(ps->rate)

  // request delay
  if(!ps->is_send) {
		// NOTE: UART - +req= uart_send_delta(ps->recv_delta);
  }

  // NOTE: UART - \r\n
}
