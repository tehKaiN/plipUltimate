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
