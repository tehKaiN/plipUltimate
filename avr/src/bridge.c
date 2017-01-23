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

#include "global.h"

#include "pkt_buf.h"
#include "pb_proto.h"
#include "base/uartutil.h"
#include "param.h"
#include "base/timer.h"
#include "stats.h"
#include "base/util.h"
#include "bridge.h"
#include "pio_util.h"
#include "pio.h"
#include "net/eth.h"
#include "net/net.h"
#include "spi/enc28j60.h"
#include "cmd.h"
#include "pinout.h"

// Set if plipUltimate has its eth online
#define FLAG_ONLINE            1
// Set if there is need to send magic packet to Amiga
#define FLAG_SEND_MAGIC        2
// Set if first transfer (which?) is yet to be made
#define FLAG_FIRST_TRANSFER    4
// Set if there is cmd response pending for Amiga
#define FLAG_SEND_CMD_RESPONSE 8

uint8_t s_ubFlags;
static uint8_t req_is_pending;

static void bridgeRequestResponseRead(void)
{
  if(!req_is_pending) {
    req_is_pending = 1;
    parRequestAmiRead();
  }
}

// ----- magic packets -----

/**
 * Enables ethernet communication, also sets MAC address to value specified
 * in eth frame.
 * @param buf Pointer to magic packet.
 */
static void bridgeCommOnline(const uint8_t *buf)
{
	// NOTE: UART - time_stamp_spc() [MAGIC] online \r\n
  s_ubFlags |= FLAG_ONLINE | FLAG_FIRST_TRANSFER;

  // validate mac address and if it does not match then reconfigure PIO
  const uint8_t *src_mac = eth_get_src_mac(buf);
  if(!net_compare_mac(g_sConfig.mac_addr, src_mac)) {
    // update mac param and save
    net_copy_mac(src_mac, g_sConfig.mac_addr);
    param_save();

    // re-configure PIO
    enc28j60_exit();
    enc28j60_init(g_sConfig.mac_addr, PIO_INIT_BROAD_CAST);
  }
}

/**
 * Disables ethernet communication.
 */
static void bridgeCommOffline(void)
{
	// NOTE: UART - time_stamp_spc() [MAGIC] offline
  s_ubFlags &= ~FLAG_ONLINE;
}

static void bridgeLoopback(uint16_t size)
{
  s_ubFlags |= FLAG_SEND_MAGIC;
  bridgeRequestResponseRead();
}

static void request_magic(void)
{
	// NOTE: UART - time_stamp_spc() [MAGIC] request\r\n

  // request receive
  s_ubFlags |= FLAG_SEND_MAGIC | FLAG_FIRST_TRANSFER;
  bridgeRequestResponseRead();
}

// ----- packet callbacks -----

// the Amiga requests a new packet

uint8_t bridgeFillPacket(uint16_t *pFilledSize) {
  if((s_ubFlags & FLAG_SEND_MAGIC) == FLAG_SEND_MAGIC) {
		// Send magic packet to Amiga
    s_ubFlags &= ~FLAG_SEND_MAGIC;

    // Build magic packet header
    // Target (bcast) MAC, src (plipbox) MAC, 0xFFFF => pFilledSize: 14 bytes
    net_copy_bcast_mac(g_pDataBuffer + ETH_OFF_TGT_MAC);
    net_copy_mac(g_sConfig.mac_addr, g_pDataBuffer + ETH_OFF_SRC_MAC);
    net_put_word(g_pDataBuffer + ETH_OFF_TYPE, ETH_TYPE_MAGIC_ONLINE);

    *pFilledSize = ETH_HDR_SIZE;
  }
  else if((s_ubFlags & FLAG_SEND_CMD_RESPONSE) == FLAG_SEND_CMD_RESPONSE) {
    // Send CMD response - it's already in buffer
    *pFilledSize = g_uwCmdResponseSize;
  }
  else {
		// Receive packet buffer with data from ENC28j60 if pending
    pio_util_recv_packet(pFilledSize);

    if(s_ubFlags & FLAG_FIRST_TRANSFER) {
			// report first packet transfer
      // NOTE: UART - time_stamp_spc() FIRST TRANSFER!\r\n
      s_ubFlags &= ~FLAG_FIRST_TRANSFER;
    }
  }

  req_is_pending = 0;

  return PBPROTO_STATUS_OK;
}

/**
 * Handles packet sent by Amiga.
 * There are basically 4 possible packet types, all defined
 * by ETH_TYPE_* defines.
 * Custom "Magic" packets are defined as topmost EtherType values.
 * @param uwSize Packet length
 * @return Always PBPROTO_STATUS_OK
 */
uint8_t bridgeProcessPacket(uint16_t uwSize) {
  // get eth type
  uint16_t eth_type = eth_get_pkt_type(g_pDataBuffer);
  switch(eth_type) {
    case ETH_TYPE_MAGIC_ONLINE:
      bridgeCommOnline(g_pDataBuffer);
      break;
    case ETH_TYPE_MAGIC_OFFLINE:
      bridgeCommOffline();
      break;
    case ETH_TYPE_MAGIC_LOOPBACK:
      bridgeLoopback(uwSize);
      break;
		case ETH_TYPE_MAGIC_CMD:
			cmdProcess(uwSize);
			s_ubFlags |= FLAG_SEND_CMD_RESPONSE;
			req_is_pending = 0;
			bridgeRequestResponseRead();
			break;
    default:
      // send packet via pio
      pio_util_send_packet(uwSize);
      // if a packet arrived and we are not online then request online state
      if((s_ubFlags & FLAG_ONLINE)==0) {
        request_magic();
      }
      break;
  }
  return PBPROTO_STATUS_OK;
}

/**
 * Main plip loop.
 * Monitors parallel port and ENC28J60 traffic.
 */
void bridgeLoop(void)
{
  // Associate protocol fns with given ptrs
  parInit();

  // Init ENC28j60
  enc28j60_init(g_sConfig.mac_addr, pio_util_get_init_flags());

  // Reset stats
  stats_reset();

  // Reset flags & request state
  s_ubFlags = 0;
  req_is_pending = 0;

  uint8_t flow_control = g_sConfig.flow_ctl;
  uint8_t limit_flow = 0;
  uint8_t ubDisplayPacketInfo = 1;
  uint8_t ubPacketCount;
  while(1) {
    // NOTE: UART command handling was here

    // Calls pb_proto_handle - this is where PAR communication is done
    pb_proto_handle();

    // Handle packets coming from network
		ubPacketCount = enc28j60_has_recv();
    if(ubPacketCount) {
      if(ubDisplayPacketInfo) {
        // NOTE: UART - time_stamp_spc() FIRST INCOMING!\r\n
        ubDisplayPacketInfo = 0;
      }

      if(s_ubFlags & FLAG_ONLINE) {
				// Comm online: let Amiga know about new packet
        bridgeRequestResponseRead();
      }
      else {
				// Comm offline: read packet from ENC28j60 and drop it
        uint16_t size;
        pio_util_recv_packet(&size);
        // NOTE: UART - time_stamp_spc() OFFLINE DROP: hex_word(size)\r\n
      }
    }

    // flow control
    if(flow_control) {
      // flow limited
      if(limit_flow) {
        // disable again?
        if(!ubPacketCount) {
          enc28j60_control(PIO_CONTROL_FLOW, 0);
          limit_flow = 0;
        }
      }
      // no flow limit
      else {
        // enable?
        if(ubPacketCount) {
          enc28j60_control(PIO_CONTROL_FLOW, 1);
          limit_flow = 1;
        }
      }
    }
  }
}
