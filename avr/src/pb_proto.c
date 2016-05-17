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
#include "../pinout.h"

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
static u08 *pb_buf;
static u16 pb_buf_size;
static u32 trigger_ts;

u16 pb_proto_timeout = 5000; // = 500ms in 100us ticks

// public stat func
pb_proto_stat_t pb_proto_stat;

// ----- Init -----

void pb_proto_init(pb_proto_fill_func ff, pb_proto_proc_func pf, u08 *buf, u16 buf_size)
{
  fill_func = ff;
  proc_func = pf;
  pb_buf = buf;
  pb_buf_size = buf_size;

  // init signals
  PAR_DATA_DDR = 0x00;
  PAR_STATUS_PIN &= ~BUSY;
}

u08 pb_proto_get_line_status(void) {
  u08 strobe = PAR_STATUS_PIN & NSTROBE;
  u08 select = PAR_STATUS_PIN & SEL;
  u08 pout = PAR_STATUS_PIN & POUT;
  return ((pout >> (POUT_PIN-2)) | (select >> (SEL_PIN-1)) | (strobe >> NSTROBE_PIN));
}

void pb_proto_request_recv(void)
{
  par_low_pulse_ack(1);
  trigger_ts = time_stamp;
}

// ----- HELPER -----

static u08 wait_req(u08 toggle_expect, u08 state_flag) {
  // wait for new REQ value
  timer_100us = 0;
  while(timer_100us < pb_proto_timeout) {
    u08 pout = (PAR_STATUS_PIN & POUT) >> POUT_PIN;
    if((toggle_expect && pout) || (!toggle_expect && !pout))
      return PBPROTO_STATUS_OK;
    // during transfer client aborted and removed SEL
    if(!(PAR_STATUS_PIN & SEL))
      return PBPROTO_STATUS_LOST_SELECT | state_flag;
  }
  return PBPROTO_STATUS_TIMEOUT | state_flag;
}

static u08 wait_sel(u08 select_state, u08 state_flag) {
  timer_100us = 0;
  while(timer_100us < pb_proto_timeout) {
    if((PAR_STATUS_PIN & SEL) >> SEL_PIN == select_state)
      return PBPROTO_STATUS_OK;
  }
  return PBPROTO_STATUS_TIMEOUT | state_flag;
}

// ---------- Handler ----------

// amiga wants to send a packet
static u08 cmd_send(u16 *pReadSize)
{
  u08 ubStatus;
  u16 uwSize;

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
  // TODO: Make odd transfers safe?
  uwSize = (uwSize+1)&0xFFFE;

  // Packet read loop
  u16 uwReadSize = 0;
  u08 *ptr = pb_buf;
  u08 ubPOutWait = 1;
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
static u08 cmd_recv(u16 uwSize, u16 *pWriteSize)
{
	u08 ubStatus;

  PAR_DATA_DDR = 0xFF;

  // Send packet size - high part
  ubStatus = wait_req(1, PBPROTO_STAGE_SIZE_HI);
  if(ubStatus != PBPROTO_STATUS_OK) {
    return ubStatus;
    // NOTE: return without DDR switchback
  }
  PAR_DATA_PORT = uwSize >> 8;
  PAR_STATUS_PORT &= ~BUSY;

  // Send packet size - low part
  ubStatus = wait_req(0, PBPROTO_STAGE_SIZE_LO);
  if(ubStatus != PBPROTO_STATUS_OK) {
    return ubStatus;
    // NOTE: return without DDR switchback
  }
  PAR_DATA_PORT = uwSize & 0xFF;
  PAR_STATUS_PORT ^= BUSY;

  // --- data ---
  u16 uwWriteSize = 0;
  const u08 *ptr = pb_buf;
  u08 ubPOutWait = 1;
  // Original plipbox had following loop operating on words, so size has to be
  // rounded up
  // TODO: Make odd transfers safe?
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

static u08 cmd_send_burst(u16 *ret_size)
{
  u16 uwSize;
  u08 ubStatus;

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
  u16 words = (uwSize +1) >> 1;
  u16 i;
  u08 result = PBPROTO_STATUS_OK;
  u08 *ptr = pb_buf;

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
static u08 cmd_recv_burst(u16 size, u16 *ret_size)
{
  u08 status;

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
  u16 words = (size + 1) >> 1;
  u08 result = PBPROTO_STATUS_OK;
  u16 i;
  u08 *ptr = pb_buf;

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

	// TODO: WTF with label/goto
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

u08 pb_proto_handle(void)
{
  u08 result;
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
  u08 cmd = PAR_DATA_PIN;

  // fill buffer for recv command
  u16 pkt_size = 0;
  if((cmd == PBPROTO_CMD_RECV) || (cmd == PBPROTO_CMD_RECV_BURST)) {
    u08 res = fill_func(pb_buf, pb_buf_size, &pkt_size);
    if(res != PBPROTO_STATUS_OK) {
      ps->status = res;
      return res;
    }
  }

  // start timer
  u32 ts = time_stamp;
  timer_hw_reset();

  // confirm cmd with RAK = 1
  PAR_STATUS_PORT |= BUSY;

  u16 ret_size = 0;
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
  u16 delta = timer_hw_get();

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
  ps->recv_delta = ps->is_send ? 0 : (u16)(ps->ts - trigger_ts);
  return result;
}
