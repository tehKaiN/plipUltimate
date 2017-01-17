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

#include "global.h"
#include "pb_proto.h"
#include "base/timer.h"
#include "stats.h"

#include "base/uartutil.h"
#include "pkt_buf.h"
#include "pinout.h"

// define symbolic names for protocol
#define SET_RAK         par_low_set_busy_hi
#define CLR_RAK         par_low_set_busy_lo
#define GET_REQ         par_low_get_pout
#define GET_SELECT      par_low_get_select

/**
 * Delay loop for recv_burst - should be at least 2us
 * Each loop takes 3 cycles, e.g. arg = 6 takes 18 cycles + overhead
 */
#if (F_CPU == 16000000)
	#define BURST_DELAY _delay_loop_1(6)
#elif (F_CPU == 20000000UL)
	#define BURST_DELAY _delay_loop_1(15)
#else
	#error Delay loop not defined for F_CPU
#endif

// recv funcs
static pb_proto_fill_func packetFillFn;
static pb_proto_proc_func packetProcessFn;
static uint32_t trigger_ts;

uint16_t pb_proto_timeout = 5000; // = 500ms in 100us ticks

// public stat func
pb_proto_stat_t pb_proto_stat;

// ----- Init -----

void pb_proto_init(pb_proto_fill_func _packetFillFn, pb_proto_proc_func _packetProcessFn)
{
  packetFillFn = _packetFillFn;
  packetProcessFn = _packetProcessFn;

  // Set data DDR to input, make BUSY low
  PAR_DATA_DDR = 0x00;
  PAR_STATUS_PORT &= ~BUSY;
}

uint8_t parGetStatusLines(void) {
	uint8_t ubIn, ubStrobe, ubSelect, ubPOut;
	ubIn = PAR_STATUS_PIN;
  ubStrobe = (ubIn & NSTROBE) >> POUT_PIN;
  ubSelect = (ubIn & SEL)     >> SEL_PIN;
  ubPOut   = (ubIn & POUT)    >> NSTROBE_PIN;
  return ((ubPOut << 2) | (ubSelect << 1) | ubStrobe);
}

/**
 * Sends information to Amiga that data is ready.
 * Done as 1ms pulse on ACK line.
 */
void parRequestAmiRead(void) {
  PAR_STATUS_PORT &= ~NACK;
  timerDelay100us(20);
  PAR_STATUS_PORT |= NACK;
  trigger_ts = g_uwTimeStamp;
}

// ----- HELPER -----

/**
 * Waits for PaperOut pin state specified by ubReqValue, for ubStateFlag purposes.
 * @param ubReqValue Requested PaperOut pin state (1: hi, 0: lo)
 * @param ubStateFlag For debugging purposes. Flag is appended to return value.
 * @return wait result - PBPROTO_STATUS_OK on success, otherwise error occured.
 */
static uint8_t parWaitForPout(uint8_t ubReqValue, uint8_t ubStateFlag) {
  g_uwTimer100us = 0;
  while(g_uwTimer100us < pb_proto_timeout) {
		uint8_t ubIn = PAR_STATUS_PIN;
    uint8_t ubPOut = (ubIn & POUT) >> POUT_PIN;
    if(ubReqValue == ubPOut)
      return PBPROTO_STATUS_OK;
    // During transfer client aborted and removed SEL
    if(!(ubIn & SEL))
      return PBPROTO_STATUS_LOST_SELECT | ubStateFlag;
  }
  return PBPROTO_STATUS_TIMEOUT | ubStateFlag;
}

static uint8_t parWaitForSel(uint8_t select_state, uint8_t state_flag) {
  g_uwTimer100us = 0;
  while(g_uwTimer100us < pb_proto_timeout) {
    if(((PAR_STATUS_PIN & SEL) >> SEL_PIN) == select_state)
      return PBPROTO_STATUS_OK;
  }
  return PBPROTO_STATUS_TIMEOUT | state_flag;
}

// ---------- Handler ----------

// amiga wants to send a packet
static uint8_t parHandleAmiWrite(uint16_t *pReadSize)
{
  uint8_t ubStatus;
  uint16_t uwSize;

  // --- get size hi ---
  ubStatus = parWaitForPout(1, PBPROTO_STAGE_SIZE_HI);
  if(ubStatus != PBPROTO_STATUS_OK)
    return ubStatus;
  uwSize = PAR_DATA_PIN << 8;
  PAR_STATUS_PORT &= ~BUSY;

  // --- get size lo ---
  ubStatus = parWaitForPout(0, PBPROTO_STAGE_SIZE_LO);
  if(ubStatus != PBPROTO_STATUS_OK)
    return ubStatus;
  uwSize |= PAR_DATA_PIN;
  PAR_STATUS_PORT ^= BUSY;

  // Check with buffer size
  if(uwSize > DATABUF_SIZE) {
    return PBPROTO_STATUS_PACKET_TOO_LARGE;
  }

  // Original plipbox had following loop operating on words, so size has to be
  // rounded up
  // TODO(KaiN#9): Make odd transfers safe?
  uwSize = (uwSize+1)&0xFFFE;

  // Packet read loop
  uint16_t uwReadSize = 0;
  uint8_t *ptr = g_pDataBuffer;
  uint8_t ubPOutWait = 1;
  while(uwSize--) {
    ubStatus = parWaitForPout(ubPOutWait, PBPROTO_STAGE_DATA);
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
/**
 * Sends data to Amiga in normal (non-burst) way.
 * Algorithm is as following:
 * - send size hiWord and set BUSY=0
 * - send size loWord and set BUSY=1
 * - send all message bytes
 * Amiga must indicate its ready state by alternating PaperOut line,
 * then PlipBox sends next byte and alternates BUSY line.
 * Not sure if it matters but BUSY is set contrary to POUT.
 */
static uint8_t parHandleAmiRead(uint16_t uwSize, uint16_t *pWriteSize)
{
	uint8_t ubStatus;

  PAR_DATA_DDR = 0xFF;

  // Send packet size - high part
  ubStatus = parWaitForPout(1, PBPROTO_STAGE_SIZE_HI);
  if(ubStatus != PBPROTO_STATUS_OK) {
    return ubStatus;
    // NOTE(KaiN): return without DDR switchback
  }
  PAR_DATA_PORT = uwSize >> 8;
  PAR_STATUS_PORT &= ~BUSY;

  // Send packet size - low part
  ubStatus = parWaitForPout(0, PBPROTO_STAGE_SIZE_LO);
  if(ubStatus != PBPROTO_STATUS_OK) {
    return ubStatus;
    // NOTE(KaiN): return without DDR switchback
  }
  PAR_DATA_PORT = uwSize & 0xFF;
  PAR_STATUS_PORT |= BUSY;

  // --- data ---
  uint16_t uwWriteSize = 0;
  const uint8_t *ptr = g_pDataBuffer;
  uint8_t ubPOutWait = 1;
  // Original plipbox had following loop operating on words, so size has to be
  // rounded up
  // TODO(KaiN#9): Make odd transfers safe?
  uwSize = (uwSize+1)&0xFFFE;
  while(uwSize--) {
    ubStatus = parWaitForPout(ubPOutWait, PBPROTO_STAGE_DATA);
    if(ubStatus != PBPROTO_STATUS_OK)
      break;
    PAR_DATA_PORT = *(ptr++);
    PAR_STATUS_PORT ^= BUSY;
    ++uwWriteSize;
    ubPOutWait ^= 1;
  }

  // Final wait
  if(ubStatus == PBPROTO_STATUS_OK)
    ubStatus = parWaitForPout(1, PBPROTO_STAGE_LAST_DATA);

  // [IN]
  PAR_DATA_DDR = 0x00;

  *pWriteSize = uwWriteSize;
  return ubStatus;
}

// ---------- BURST ----------

static uint8_t parHandleAmiWriteBurst(uint16_t *ret_size)
{
  uint16_t uwSize;
  uint8_t ubStatus;

  // --- packet size hi ---
  ubStatus = parWaitForPout(1, PBPROTO_STAGE_SIZE_HI);
  if(ubStatus != PBPROTO_STATUS_OK)
    return ubStatus;

  uwSize = PAR_DATA_PIN << 8;
  PAR_STATUS_PORT &= ~BUSY;

  // --- packet size lo ---
  ubStatus = parWaitForPout(0, PBPROTO_STAGE_SIZE_LO);
  if(ubStatus != PBPROTO_STATUS_OK)
    return ubStatus;

  uwSize |= PAR_DATA_PIN;
  // delay SET_RAK until burst begin...

  // check size
  if(uwSize > DATABUF_SIZE)
    return PBPROTO_STATUS_PACKET_TOO_LARGE;

  // round to even and convert to words
  uint16_t words = (uwSize +1) >> 1;
  uint16_t i;
  uint8_t result = PBPROTO_STATUS_OK;
  uint8_t *ptr = g_pDataBuffer;

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
static uint8_t parHandleAmiReadBurst(uint16_t size, uint16_t *ret_size) {
  uint8_t status;

  // --- set packet size hi
  status = parWaitForPout(1, PBPROTO_STAGE_SIZE_HI);
  if(status != PBPROTO_STATUS_OK)
    return status;

	PAR_DATA_DDR = 0xFF;
	PAR_DATA_PORT = size >> 8;
	PAR_STATUS_PORT &= ~BUSY;

  // --- set packet size lo ---
  status = parWaitForPout(0, PBPROTO_STAGE_SIZE_LO);
  if(status != PBPROTO_STATUS_OK)
    return status;

	PAR_DATA_PORT = size & 0xFF;
	PAR_STATUS_PORT ^= BUSY;

  // --- burst ready? ---
  status = parWaitForPout(1, PBPROTO_STAGE_DATA);
  if(status != PBPROTO_STATUS_OK)
    return status;

  // round to even and convert to words
  uint16_t words = (size + 1) >> 1;
  uint8_t result = PBPROTO_STATUS_OK;
  uint16_t i;
  uint8_t *ptr = g_pDataBuffer;

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

/**
 * Handles communication with Amiga.
 * This function does the following:
 * - checks if SEL=1 and POUT=0, if not then aborts
 * - reads send/recv normal/burst byte
 * - sets BUSY=1
 * - prepares response for recv
 * - does normal/burst comm with Amiga
 * - waits a bit for SEL=0, then sets BUSY=0
 * - processes packet sent by Amiga
 */
uint8_t pb_proto_handle(void) {
  uint8_t result;
  pb_proto_stat_t *ps = &pb_proto_stat;

  // handle server side of plipbox protocol
  ps->cmd = 0;

  // make sure that SEL == 1 and POUT == 0
  if(!(PAR_STATUS_PIN & SEL) || (PAR_STATUS_PIN & POUT)) {
    ps->status = PBPROTO_STATUS_IDLE;
    return PBPROTO_STATUS_IDLE;
  }

  // Read command byte
  uint8_t cmd = PAR_DATA_PIN;

  // Amiga wants to receive data - prepare
  uint16_t pkt_size = 0;
  if((cmd == PBPROTO_CMD_RECV) || (cmd == PBPROTO_CMD_RECV_BURST)) {
    uint8_t res = packetFillFn(&pkt_size);
    if(res != PBPROTO_STATUS_OK) {
      ps->status = res;
      return res;
    }
  }

  // start timer
  uint32_t ts = g_uwTimeStamp;
  timerReset();

  // confirm cmd with BUSY = 1
  PAR_STATUS_PORT |= BUSY;

  uint16_t uwParDataSize = 0;
  switch(cmd) {
    case PBPROTO_CMD_RECV:
      result = parHandleAmiRead(pkt_size, &uwParDataSize);
      break;
    case PBPROTO_CMD_SEND:
      result = parHandleAmiWrite(&uwParDataSize);
      break;
    case PBPROTO_CMD_RECV_BURST:
      result = parHandleAmiReadBurst(pkt_size, &uwParDataSize);
      break;
    case PBPROTO_CMD_SEND_BURST:
      result = parHandleAmiWriteBurst(&uwParDataSize);
      break;
    default:
      result = PBPROTO_STATUS_INVALID_CMD;
      break;
  }

  // wait for SEL == 0
  parWaitForSel(0, PBPROTO_STAGE_END_SELECT);

  // reset BUSY = 0
  PAR_STATUS_PORT &= ~BUSY;

  // Read timer - assuming transfer will be much shorter than 100us
  // TODO(KaiN#7): is it really that short?
  uint16_t uwTimeDelta = timerGetState();

  // Amiga sent data - process it
  if(result == PBPROTO_STATUS_OK) {
    if((cmd == PBPROTO_CMD_SEND) || (cmd == PBPROTO_CMD_SEND_BURST)) {
      result = packetProcessFn(uwParDataSize);
    }
  }

  // fill in stats
  ps->cmd = cmd;
  ps->status = result;
  ps->size = uwParDataSize;
  ps->delta = uwTimeDelta;
  ps->rate = timerCalculateKbps(uwParDataSize, uwTimeDelta);
  ps->ts = ts;
  ps->is_send = (cmd == PBPROTO_CMD_SEND) || (cmd == PBPROTO_CMD_SEND_BURST);
  ps->stats_id = ps->is_send ? STATS_ID_PB_TX : STATS_ID_PB_RX;
  ps->recv_delta = ps->is_send ? 0 : (uint16_t)(ps->ts - trigger_ts);
  return result;
}
