/*
 * pb_proto.c - avr implementation of the plipbox protocol
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

#include <avr/interrupt.h>
#include <util/delay_basic.h>

#include "pb_proto.h"
#include "par_low.h"
#include "base/timer.h"
#include "stats.h"

#include "base/uartutil.h"
#include "pinout.h"

// define symbolic names for protocol
#define SET_RAK         par_low_set_busy_hi
#define CLR_RAK         par_low_set_busy_lo
#define GET_REQ         par_low_get_pout
#define GET_SELECT      par_low_get_select

// delay loop for recv_burst
// at least 2us
#if (F_CPU == 16000000)
	// 3 cycles per call
	#define BURST_DELAY _delay_loop_1(6)
#else
	#error Delay loop not defined for F_CPU
#endif

// recv funcs
static pb_proto_fill_func fill_func;
static pb_proto_proc_func proc_func;
static uint8_t *pb_buf;
static uint16_t pb_buf_size;
static uint32_t trigger_ts;

uint16_t pb_proto_timeout = 5000; // = 500ms in 100us ticks

// public stat func
pb_proto_stat_t pb_proto_stat;

// ----- Init -----

void pb_proto_init(pb_proto_fill_func ff, pb_proto_proc_func pf, uint8_t *buf, uint16_t buf_size)
{
  fill_func = ff;
  proc_func = pf;
  pb_buf = buf;
  pb_buf_size = buf_size;

  // init signals
  PAR_DATA_DDR = 0x00;
  PAR_STATUS_PIN &= ~BUSY;
}

uint8_t pb_proto_get_line_status(void) {
	uint8_t ubIn, ubStrobe, ubSelect, ubPOut;
	ubIn = PAR_STATUS_PIN;
  ubStrobe = (ubIn & NSTROBE) >> POUT_PIN;
  ubSelect = (ubIn & SEL)     >> SEL_PIN;
  ubPOut   = (ubIn & POUT)    >> NSTROBE_PIN;
  return ((ubPOut << 2) | (ubSelect << 1) | ubStrobe);
}

void pb_proto_request_recv(void)
{
  par_low_pulse_ack(1);
  trigger_ts = time_stamp;
}

// ----- HELPER -----

static uint8_t wait_req(uint8_t ubReqValue, uint8_t ubStateFlag) {
  // wait for new REQ value
  timer_100us = 0;
  while(timer_100us < pb_proto_timeout) {
		uint8_t ubIn = PAR_STATUS_PIN;
    uint8_t ubPOut = (ubIn & POUT) >> POUT_PIN;
    if(ubReqValue == ubPOut)
      return PBPROTO_STATUS_OK;
    // during transfer client aborted and removed SEL
    if(!(ubIn & SEL))
      return PBPROTO_STATUS_LOST_SELECT | ubStateFlag;
  }
  return PBPROTO_STATUS_TIMEOUT | ubStateFlag;
}

static uint8_t wait_sel(uint8_t select_state, uint8_t state_flag) {
  timer_100us = 0;
  while(timer_100us < pb_proto_timeout) {
    if(((PAR_STATUS_PIN & SEL) >> SEL_PIN) == select_state)
      return PBPROTO_STATUS_OK;
  }
  return PBPROTO_STATUS_TIMEOUT | state_flag;
}

// ---------- Handler ----------

// amiga wants to send a packet
static uint8_t cmd_send(uint16_t *pReadSize)
{
  uint8_t ubStatus;
  uint16_t uwSize;

  // --- get size hi ---
  ubStatus = wait_req(1, PBPROTO_STAGE_SIZE_HI);
  if(ubStatus != PBPROTO_STATUS_OK)
    return ubStatus;
  uwSize = PAR_DATA_PIN << 8;
  PAR_STATUS_PORT &= ~BUSY;

  // --- get size lo ---
  ubStatus = wait_req(0, PBPROTO_STAGE_SIZE_LO);
  if(ubStatus != PBPROTO_STATUS_OK)
    return ubStatus;
  uwSize |= PAR_DATA_PIN;
  PAR_STATUS_PORT ^= BUSY;

  // Check with buffer size
  if(uwSize > pb_buf_size) {
    return PBPROTO_STATUS_PACKET_TOO_LARGE;
  }

  // Original plipbox had following loop operating on words, so size has to be
  // rounded up
  // TODO(KaiN#9): Make odd transfers safe?
  uwSize = (uwSize+1)&0xFFFE;

  // Packet read loop
  uint16_t uwReadSize = 0;
  uint8_t *ptr = pb_buf;
  uint8_t ubPOutWait = 1;
  while(uwSize--) {
    ubStatus = wait_req(ubPOutWait, PBPROTO_STAGE_DATA);
    if(ubStatus != PBPROTO_STATUS_OK)
      break;
    *(ptr++) = PAR_DATA_PIN;
    PAR_STATUS_PORT ^= BUSY;
    ubPOutWait ^= 1;
    uwReadSize++;
	}

  *pReadSize = uwReadSize;
  return ubStatus;
}

// amiga wants to receive a packet
static uint8_t cmd_recv(uint16_t uwSize, uint16_t *pWriteSize)
{
	uint8_t ubStatus;

  PAR_DATA_DDR = 0xFF;

  // Send packet size - high part
  ubStatus = wait_req(1, PBPROTO_STAGE_SIZE_HI);
  if(ubStatus != PBPROTO_STATUS_OK) {
    return ubStatus;
    // NOTE(KaiN): return without DDR switchback
  }
  PAR_DATA_PORT = uwSize >> 8;
  PAR_STATUS_PORT &= ~BUSY;

  // Send packet size - low part
  ubStatus = wait_req(0, PBPROTO_STAGE_SIZE_LO);
  if(ubStatus != PBPROTO_STATUS_OK) {
    return ubStatus;
    // NOTE(KaiN): return without DDR switchback
  }
  PAR_DATA_PORT = uwSize & 0xFF;
  PAR_STATUS_PORT ^= BUSY;

  // --- data ---
  uint16_t uwWriteSize = 0;
  const uint8_t *ptr = pb_buf;
  uint8_t ubPOutWait = 1;
  // Original plipbox had following loop operating on words, so size has to be
  // rounded up
  // TODO(KaiN#9): Make odd transfers safe?
  uwSize = (uwSize+1)&0xFFFE;
  while(uwSize--) {
    ubStatus = wait_req(ubPOutWait, PBPROTO_STAGE_DATA);
    if(ubStatus != PBPROTO_STATUS_OK)
      break;
    PAR_DATA_PORT = *(ptr++);
    PAR_STATUS_PORT ^= BUSY;
    ++uwWriteSize;
    ubPOutWait ^= 1;
  }

  // Final wait
  if(ubStatus == PBPROTO_STATUS_OK)
    ubStatus = wait_req(1, PBPROTO_STAGE_LAST_DATA);

  // [IN]
  PAR_DATA_DDR = 0x00;

  *pWriteSize = uwWriteSize;
  return ubStatus;
}

// ---------- BURST ----------

static uint8_t cmd_send_burst(uint16_t *ret_size)
{
  uint16_t uwSize;
  uint8_t ubStatus;

  // --- packet size hi ---
  ubStatus = wait_req(1, PBPROTO_STAGE_SIZE_HI);
  if(ubStatus != PBPROTO_STATUS_OK)
    return ubStatus;

  uwSize = PAR_DATA_PIN << 8;
  PAR_STATUS_PORT &= ~BUSY;

  // --- packet size lo ---
  ubStatus = wait_req(0, PBPROTO_STAGE_SIZE_LO);
  if(ubStatus != PBPROTO_STATUS_OK)
    return ubStatus;

  uwSize |= PAR_DATA_PIN;
  // delay SET_RAK until burst begin...

  // check size
  if(uwSize > pb_buf_size)
    return PBPROTO_STATUS_PACKET_TOO_LARGE;

  // round to even and convert to words
  uint16_t words = (uwSize +1) >> 1;
  uint16_t i;
  uint8_t result = PBPROTO_STATUS_OK;
  uint8_t *ptr = pb_buf;

  // ----- burst loop -----
  // BEGIN TIME CRITICAL
  cli();
  PAR_STATUS_PORT ^= BUSY; // trigger start of burst
  for(i=0;i<words;i++) {
    // wait REQ == 1
    while(!(PAR_STATUS_PIN & POUT) && (PAR_STATUS_PIN & SEL))
		if(!(PAR_STATUS_PIN & SEL))
			break;
    *(ptr++) = PAR_DATA_PIN;

    // wait REQ == 0
    while((PAR_STATUS_PIN & POUT) && (PAR_STATUS_PIN & SEL))
		if(!(PAR_STATUS_PIN & SEL))
			break;
    *(ptr++) = PAR_DATA_PIN;
  }
  sei();
  // END TIME CRITICAL

  do {
		// Wait for POUT == 1
		while(!(PAR_STATUS_PIN & POUT) && (PAR_STATUS_PIN & SEL));
		if(!(PAR_STATUS_PIN & SEL))
			continue;

		PAR_STATUS_PORT ^= BUSY;
		// Wait for POUT == 0
		while((PAR_STATUS_PIN & POUT) && (PAR_STATUS_PIN & SEL));
  } while(!(PAR_STATUS_PIN & SEL));

  // error?
  if(i<words)
    result = PBPROTO_STATUS_TIMEOUT | PBPROTO_STAGE_DATA;

  // final ACK
	PAR_STATUS_PORT ^= BUSY;

  *ret_size = i << 1;
  return result;
}

/**
 * Sends data from AVR to Amiga in burst mode
 * AVR doesn't acknowledge sending next part of data, so Amiga just reads
 * as fast as it can and acks every byte read
 */
static uint8_t cmd_recv_burst(uint16_t size, uint16_t *ret_size) {
  uint8_t status;

  // --- set packet size hi
  status = wait_req(1, PBPROTO_STAGE_SIZE_HI);
  if(status != PBPROTO_STATUS_OK)
    return status;

	PAR_DATA_DDR = 0xFF;
	PAR_DATA_PORT = size >> 8;
	PAR_STATUS_PORT &= ~BUSY;

  // --- set packet size lo ---
  status = wait_req(0, PBPROTO_STAGE_SIZE_LO);
  if(status != PBPROTO_STATUS_OK)
    return status;

	PAR_DATA_PORT = size & 0xFF;
	PAR_STATUS_PORT ^= BUSY;

  // --- burst ready? ---
  status = wait_req(1, PBPROTO_STAGE_DATA);
  if(status != PBPROTO_STATUS_OK)
    return status;

  // round to even and convert to words
  uint16_t words = (size + 1) >> 1;
  uint8_t result = PBPROTO_STATUS_OK;
  uint16_t i;
  uint8_t *ptr = pb_buf;

  // ----- burst loop -----
  // BEGIN TIME CRITICAL
  cli();
	PAR_STATUS_PORT ^= BUSY;
  for(i=0;i<words;i++) {
    BURST_DELAY;
    PAR_DATA_PORT = *(ptr++);

    // wait REQ == 0
    while((PAR_STATUS_PIN & POUT) && (PAR_STATUS_PIN & SEL));
		if(!(PAR_STATUS_PIN & SEL))
			break;

    BURST_DELAY;
    PAR_DATA_PORT = *(ptr++);

    // wait REQ == 1
    while(!(PAR_STATUS_PIN & POUT) && (PAR_STATUS_PIN & SEL));
		if(!(PAR_STATUS_PIN & SEL))
			break;
  }
  recv_burst_exit:
  sei();
  // END TIME CRITICAL

	// TODO(KaiN#9): WTF with label/goto
  // final wait REQ == 0
  while(PAR_STATUS_PIN & POUT)
    if(!(PAR_STATUS_PIN & SEL))
			goto recv_burst_exit;

	PAR_STATUS_PORT |= BUSY;

  // final wait REQ == 1
  while(!(PAR_STATUS_PIN & POUT))
    if(!(PAR_STATUS_PIN & SEL))
			goto recv_burst_exit;

  // error?
  if(i<words)
    result = PBPROTO_STATUS_TIMEOUT | PBPROTO_STAGE_DATA;

  // final ACK
	PAR_STATUS_PORT &= ~BUSY;

  // [IN]
  PAR_DATA_DDR = 0x00;

  *ret_size = i << 1;
  return result;
}

uint8_t pb_proto_handle(void) {
  uint8_t result;
  pb_proto_stat_t *ps = &pb_proto_stat;

  // handle server side of plipbox protocol
  ps->cmd = 0;

  // make sure that SEL == 1
  if(!(PAR_STATUS_PIN & SEL)) {
    ps->status = PBPROTO_STATUS_IDLE;
    return PBPROTO_STATUS_IDLE;
  }

  // make sure that REQ == 0
  if(!(PAR_STATUS_PIN & POUT)) {
    ps->status = PBPROTO_STATUS_IDLE;
    return PBPROTO_STATUS_IDLE;
  }

  // read command byte and execute it
  uint8_t cmd = PAR_DATA_PIN;

  // fill buffer for recv command
  uint16_t pkt_size = 0;
  if((cmd == PBPROTO_CMD_RECV) || (cmd == PBPROTO_CMD_RECV_BURST)) {
    uint8_t res = fill_func(pb_buf, pb_buf_size, &pkt_size);
    if(res != PBPROTO_STATUS_OK) {
      ps->status = res;
      return res;
    }
  }

  // start timer
  uint32_t ts = time_stamp;
  timer_hw_reset();

  // confirm cmd with RAK = 1
  PAR_STATUS_PORT |= BUSY;

  uint16_t ret_size = 0;
  switch(cmd) {
    case PBPROTO_CMD_RECV:
      result = cmd_recv(pkt_size, &ret_size);
      break;
    case PBPROTO_CMD_SEND:
      result = cmd_send(&ret_size);
      break;
    case PBPROTO_CMD_RECV_BURST:
      result = cmd_recv_burst(pkt_size, &ret_size);
      break;
    case PBPROTO_CMD_SEND_BURST:
      result = cmd_send_burst(&ret_size);
      break;
    default:
      result = PBPROTO_STATUS_INVALID_CMD;
      break;
  }

  // wait for SEL == 0
  wait_sel(0, PBPROTO_STAGE_END_SELECT);

  // reset RAK = 0
  PAR_STATUS_PORT &= ~BUSY;

  // read timer
  uint16_t delta = timer_hw_get();

  // process buffer for send command
  if(result == PBPROTO_STATUS_OK) {
    if((cmd == PBPROTO_CMD_SEND) || (cmd == PBPROTO_CMD_SEND_BURST)) {
      result = proc_func(pb_buf, ret_size);
    }
  }

  // fill in stats
  ps->cmd = cmd;
  ps->status = result;
  ps->size = ret_size;
  ps->delta = delta;
  ps->rate = timer_hw_calc_rate_kbs(ret_size, delta);
  ps->ts = ts;
  ps->is_send = (cmd == PBPROTO_CMD_SEND) || (cmd == PBPROTO_CMD_SEND_BURST);
  ps->stats_id = ps->is_send ? STATS_ID_PB_TX : STATS_ID_PB_RX;
  ps->recv_delta = ps->is_send ? 0 : (uint16_t)(ps->ts - trigger_ts);
  return result;
}
