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

#include "pio_util.h"

#include "base/timer.h"
#include "pio.h"
#include "base/uartutil.h"
#include "stats.h"
#include "pkt_buf.h"
#include "param.h"

#include "net/net.h"
#include "net/eth.h"
#include "net/arp.h"
#include "net/ip.h"
#include "net/udp.h"
#include "spi/enc28j60.h"

uint8_t pio_util_get_init_flags()
{
  uint8_t flags = PIO_INIT_BROAD_CAST;

  if(g_sConfig.flow_ctl) {
    flags |= PIO_INIT_FLOW_CONTROL;
  }
  if(g_sConfig.full_duplex) {
    flags |= PIO_INIT_FULL_DUPLEX;
  }

  return flags;
}

/**
 * Receives data from ENC28j60, calculates stats & does logging.
 * @param pDataSize Pointer to addr to be filled with read data size.
 */
uint8_t pio_util_recv_packet(uint16_t *pDataSize)
{
  // Fetch packet from ENC28j60, measure elapsed time
  timerReset();
  uint8_t ubRecvResult = enc28j60_recv(g_pDataBuffer, DATABUF_SIZE, pDataSize);
  uint16_t uwTimeDelta = timerGetState();
  uint16_t uwDataRate = timerCalculateKbps(*pDataSize, uwTimeDelta);

  if(ubRecvResult == PIO_OK) {
		// Update stats - write new data size & rate
    stats_update_ok(STATS_ID_PIO_RX, *pDataSize, uwDataRate);
  }
  else {
		// Update stats - increase error count
    stats_get(STATS_ID_PIO_RX)->err++;
  }

  return ubRecvResult;
}

uint8_t pio_util_send_packet(uint16_t size)
{
  timerReset();
  uint8_t result = enc28j60_send(g_pDataBuffer, size);
  // NOTE(KaiN#7): Is it really that short?
  uint16_t delta = timerGetState();

  uint16_t rate = timerCalculateKbps(size, delta);
  if(result == PIO_OK) {
    stats_update_ok(STATS_ID_PIO_TX, size, rate);
  }
  else {
    stats_get(STATS_ID_PIO_TX)->err++;
  }

  return result;
}

/**
 * Handles ARP request.
 * @param uwPacketSize Received ethernet frame size.
 */
uint8_t pio_util_handle_arp(uint16_t uwPacketSize)
{
	// Sanity check
  uint16_t uwEtherType = eth_get_pkt_type(g_pDataBuffer);
  if(uwEtherType != ETH_TYPE_ARP) {
    return 0;
  }
  if(uwPacketSize <= ETH_HDR_SIZE) {
    return 0;
  }

  // Payload buffer pointer & size
  uint8_t *pPayloadBuffer = g_pDataBuffer + ETH_HDR_SIZE;
  uint16_t uwPayloadSize = uwPacketSize - ETH_HDR_SIZE;

  // Is it an ARP request?
  if(
		arp_is_ipv4(pPayloadBuffer, uwPayloadSize) &&
		arp_get_op(pPayloadBuffer) == ARP_REQUEST
	) {
    // Is plipbox being searched?
    const uint8_t *pTargetIp = arp_get_tgt_ip(pPayloadBuffer);
    if(net_compare_ip(pTargetIp, g_sConfig.test_ip)) {
      arp_make_reply(pPayloadBuffer, g_sConfig.mac_addr, g_sConfig.test_ip);
      eth_make_bcast(g_pDataBuffer, g_sConfig.mac_addr);
      pio_util_send_packet(uwPacketSize);
    }
  }

  return 1;
}

uint8_t pio_util_handle_udp_test(uint16_t size)
{
  uint8_t *ip_buf = g_pDataBuffer + ETH_HDR_SIZE;
  uint8_t *udp_buf = ip_buf + ip_get_hdr_length(ip_buf);
  const uint8_t *dst_ip = ip_get_tgt_ip(ip_buf);
  uint16_t dst_port = udp_get_tgt_port(udp_buf);
  const uint8_t *data_ptr = udp_get_data_ptr(udp_buf);

  // for us?
  if(net_compare_ip(g_sConfig.test_ip, dst_ip) && (dst_port == g_sConfig.test_port)) {
    // send UDP packet back again
    // flip IP/UDP
    const uint8_t *src_ip = ip_get_src_ip(ip_buf);
    net_copy_ip(src_ip, ip_buf + 16); // set tgt ip
    net_copy_ip(g_sConfig.test_ip, ip_buf + 12); // set src ip
    uint16_t src_port = udp_get_src_port(udp_buf);
    net_put_word(udp_buf + UDP_SRC_PORT_OFF, dst_port);
    net_put_word(udp_buf + UDP_TGT_PORT_OFF, src_port);

    // flip eth
    net_copy_mac(g_pDataBuffer + ETH_OFF_SRC_MAC, g_pDataBuffer + ETH_OFF_TGT_MAC);
    net_copy_mac(g_sConfig.mac_addr, g_pDataBuffer + ETH_OFF_SRC_MAC);

    return 1;
 } else {
 	return 0;
 }
}


