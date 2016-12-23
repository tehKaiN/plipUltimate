/*
 * bridge.c: bridge packets from plip to eth and back
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

#include "global.h"

#include "pkt_buf.h"
#include "pb_proto.h"
#include "base/uartutil.h"
#include "param.h"
#include "dump.h"
#include "base/timer.h"
#include "stats.h"
#include "base/util.h"
#include "bridge.h"
#include "main.h"
#include "base/cmd.h"
#include "pb_util.h"
#include "pio_util.h"
#include "pio.h"
#include "net/eth.h"
#include "net/net.h"
#include "spi/enc28j60.h"

#define FLAG_ONLINE         1
#define FLAG_SEND_MAGIC     2
#define FLAG_FIRST_TRANSFER 4

static uint8_t flags;
static uint8_t req_is_pending;

static void trigger_request(void)
{
  if(!req_is_pending) {
    req_is_pending = 1;
    pb_proto_request_recv();
    if(global_verbose) {
			// NOTE: UART - time_stamp_spc() REQ\r\n
    }
  } else {
    if(global_verbose) {
			// NOTE: UART - time_stamp_spc() REQ IGN\r\n
    }
  }
}

// ----- magic packets -----

/**
 * Enables ethernet communication, also sets MAC address to value specified
 * in eth frame.
 * @param buf Pointer to magic packet.
 */
static void magic_online(const uint8_t *buf)
{
	// NOTE: UART - time_stamp_spc() [MAGIC] online \r\n
  flags |= FLAG_ONLINE | FLAG_FIRST_TRANSFER;

  // validate mac address and if it does not match then reconfigure PIO
  const uint8_t *src_mac = eth_get_src_mac(buf);
  if(!net_compare_mac(param.mac_addr, src_mac)) {
    // update mac param and save
    net_copy_mac(src_mac, param.mac_addr);
    param_save();

    // re-configure PIO
    enc28j60_exit();
    enc28j60_init(param.mac_addr, PIO_INIT_BROAD_CAST);
  }
}

/**
 * Disables ethernet communication.
 */
static void magic_offline(void)
{
	// NOTE: UART - time_stamp_spc() [MAGIC] offline
  flags &= ~FLAG_ONLINE;
}

static void magic_loopback(uint16_t size)
{
  flags |= FLAG_SEND_MAGIC;
  trigger_request();
}

static void request_magic(void)
{
	// NOTE: UART - time_stamp_spc() [MAGIC] request\r\n

  // request receive
  flags |= FLAG_SEND_MAGIC | FLAG_FIRST_TRANSFER;
  trigger_request();
}

// ----- packet callbacks -----

// the Amiga requests a new packet

static uint8_t fill_pkt(uint8_t *buf, uint16_t max_size, uint16_t *size)
{
  // need to send a magic?
  if((flags & FLAG_SEND_MAGIC) == FLAG_SEND_MAGIC) {
    flags &= ~FLAG_SEND_MAGIC;

    // Build magic packet
    // Target (bcast) MAC, src (plipbox) MAC, 0xFFFF => size: 14 bytes
    net_copy_bcast_mac(pkt_buf + ETH_OFF_TGT_MAC);
    net_copy_mac(param.mac_addr, pkt_buf + ETH_OFF_SRC_MAC);
    net_put_word(pkt_buf + ETH_OFF_TYPE, ETH_TYPE_MAGIC_ONLINE);

    *size = ETH_HDR_SIZE;
  }
  else {
    // pending PIO packet?
    pio_util_recv_packet(size);

    // report first packet transfer
    if(flags & FLAG_FIRST_TRANSFER) {
      flags &= ~FLAG_FIRST_TRANSFER;
      // NOTE: UART - time_stamp_spc() FIRST TRANSFER!\r\n
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
 * @param buf Buffer containing sent data
 * @param size Packet length
 * @return Always PBPROTO_STATUS_OK
 */
static uint8_t proc_pkt(const uint8_t *buf, uint16_t size)
{
  // get eth type
  uint16_t eth_type = eth_get_pkt_type(buf);
  switch(eth_type) {
    case ETH_TYPE_MAGIC_ONLINE:
      magic_online(buf);
      break;
    case ETH_TYPE_MAGIC_OFFLINE:
      magic_offline();
      break;
    case ETH_TYPE_MAGIC_LOOPBACK:
      magic_loopback(size);
      break;
    default:
      // send packet via pio
      pio_util_send_packet(size);
      // if a packet arrived and we are not online then request online state
      if((flags & FLAG_ONLINE)==0) {
        request_magic();
      }
      break;
  }
  return PBPROTO_STATUS_OK;
}

// ---------- loop ----------

uint8_t bridge_loop(void)
{
  uint8_t result = CMD_WORKER_IDLE;

  // NOTE: UART - time_stamp_spc() [BRIDGE] on\r\n

  // Associate protocol fns with given ptrs
  pb_proto_init(fill_pkt, proc_pkt, pkt_buf, PKT_BUF_SIZE);

  // Init ENC28j60
  enc28j60_init(param.mac_addr, pio_util_get_init_flags());

  // Reset stats
  stats_reset();

  // Reset flags & request state
  flags = 0;
  req_is_pending = 0;

  uint8_t flow_control = param.flow_ctl;
  uint8_t limit_flow = 0;
  uint8_t ubDisplayPacketInfo = 1;
  while(run_mode == RUN_MODE_BRIDGE) {
    // NOTE: UART command handling was here

    // Calls pb_proto_handle - this is where PAR communication is done
    pb_util_handle();

    // Handle packets coming from network
    uint8_t ubPacketCount = enc28j60_has_recv();
    if(ubPacketCount) {
      if(ubDisplayPacketInfo) {
        // NOTE: UART - time_stamp_spc() FIRST INCOMING!\r\n
        ubDisplayPacketInfo = 0;
      }

      if(flags & FLAG_ONLINE) {
				// Comm online: let Amiga know about new packet
        trigger_request();
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
          if(global_verbose) {
						// NOTE: UART - time_stamp_spc() FLOW off\r\n
          }
        }
      }
      // no flow limit
      else {
        // enable?
        if(ubPacketCount) {
          enc28j60_control(PIO_CONTROL_FLOW, 1);
          limit_flow = 1;
          if(global_verbose) {
						// NOTE: UART - time_stamp_spc() FLOW on\r\n
          }
        }
      }
    }
  }

  stats_dump_all();
  enc28j60_exit();

	// NOTE: UART - time_stamp_spc() [BRIDGE] off\r\n

  return result;
}
