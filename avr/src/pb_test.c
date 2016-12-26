/*
 * pb_test.c: plipbox test mode
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

#include "pb_test.h"

#include "base/uartutil.h"
#include "pb_proto.h"
#include "pb_util.h"
#include "param.h"
#include "stats.h"
#include "main.h"
#include "base/timer.h"
#include "net/net.h"
#include "pkt_buf.h"
#include "dump.h"

static uint8_t toggle_request;
static uint8_t auto_mode;
static uint8_t silent_mode;

// ----- Packet Callbacks -----

static uint8_t fill_pkt(uint16_t *pFilledSize)
{
  *pFilledSize = param.test_plen;
  if(*pFilledSize > DATABUF_SIZE) {
    return PBPROTO_STATUS_PACKET_TOO_LARGE;
  }

  net_copy_mac(net_bcast_mac, g_pDataBuffer);
  net_copy_mac(param.mac_addr, g_pDataBuffer+6);

  uint8_t ptype_hi = (uint8_t)(param.test_ptype >> 8);
  uint8_t ptype_lo = (uint8_t)(param.test_ptype & 0xff);
  g_pDataBuffer[12] = ptype_hi;
  g_pDataBuffer[13] = ptype_lo;

  uint8_t *ptr = g_pDataBuffer + 14;
  uint16_t num = *pFilledSize - 14;
  uint8_t val = 0;
  while(num > 0) {
    *ptr = val;
    ptr++;
    val++;
    num--;
  }

  return PBPROTO_STATUS_OK;
}

static uint8_t proc_pkt(uint16_t uwSize)
{
  uint16_t errors = 0;

  // check packet size
  if(uwSize != param.test_plen) {
    errors = 1;
    // NOTE: UART - ERR: uwSize\r\n
  }

  // +0: check dst mac
  if(!net_compare_mac(g_pDataBuffer, net_bcast_mac)) {
    errors++;
    // NOTE: UART - ERR: dst mac\r\n
  }
  // +6: check src mac
  if(!net_compare_mac(g_pDataBuffer+6, param.mac_addr)) {
    errors++;
    // NOTE: UART - ERR: src mac\r\n
  }
  // +12,+13: pkt type
  uint8_t ptype_hi = (uint8_t)(param.test_ptype >> 8);
  uint8_t ptype_lo = (uint8_t)(param.test_ptype & 0xff);
  if((g_pDataBuffer[12] != ptype_hi) || (g_pDataBuffer[13] != ptype_lo)) {
    errors++;
		// NOTE: UART - ERR: pkt type\r\n
  }

  // +14: data
  const uint8_t *ptr = g_pDataBuffer + 14;
  uint16_t num = uwSize - 14;
  uint8_t val = 0;
  while(num > 0) {
    if(*ptr != val) {
			// NOTE: UART - ERR: data @hex_word(num)\r\n
    }
    val++;
    ptr++;
    num--;
  }

#if 0
  for(int i=0;i<16;i++) {
    uart_send_hex_byte(buf[i]);
    uart_send_spc();
  }
  uart_send_crlf();
#endif

  if(errors > 0) {
		// NOTE: UART - TOTAL ERRORS hex_word(errors)\r\n
    return PBPROTO_STATUS_ERROR;
  } else {
    return PBPROTO_STATUS_OK;
  }
}

// ----- function table -----

static void pb_test_worker(void)
{
  uint8_t status = pb_util_handle();

  // ok!
  if(status == PBPROTO_STATUS_OK) {

    // always dump I/O
    if(!silent_mode) {
      dump_pb_cmd(&pb_proto_stat);
    }

    // next iteration?
    if(pb_proto_stat.is_send) {
      if(auto_mode) {
        // next iteration after
        pb_test_send_packet(1);
      } else {
        silent_mode = 0;
      }
    }
  }
  // pb proto failed with an error
  else if(status != PBPROTO_STATUS_IDLE) {
    // disable auto mode
    if(auto_mode) {
      pb_test_toggle_auto();
    }
  }
}

void pb_test_loop(void) {
	// NOTE: time_stamp_spc() [PB_TEST] on\r\n

  stats_reset();

  // setup handlers for pb testing
  pb_proto_init(fill_pkt, proc_pkt);
  auto_mode = 0;
  toggle_request = 0;
  silent_mode = 0;

  // test loop
  while(run_mode == RUN_MODE_PB_TEST) {
    // command line handling
    // NOTE: UART cmd_worker handling was here, reset by loop break

    pb_test_worker();
  }

  stats_dump(1,0);

	// NOTE: UART - time_stamp_spc() [PB_TEST] off\r\n
}

void pb_test_send_packet(uint8_t silent)
{
  silent_mode = silent;
  pb_proto_request_recv();
}

void pb_test_toggle_auto(void)
{
  auto_mode = !auto_mode;

	// NOTE: UART - time_stamp_spc() [AUTO]
  if(auto_mode) {
		// NOTE: UART - on
  } else {
		// NOTE: UART - off
  }

  if(auto_mode) {
    // send first packet
    pb_test_send_packet(1);
    // clear stats
    stats_reset();
  }
}
