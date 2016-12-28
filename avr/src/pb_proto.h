/*
 * pb_proto.h - avr implementation of the plipbox protocol
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

#ifndef PBPROTO_H
#define PBPROTO_H

#include "global.h"

// status return
#define PBPROTO_STATUS_IDLE              0
#define PBPROTO_STATUS_OK                1
#define PBPROTO_STATUS_TIMEOUT           2
#define PBPROTO_STATUS_LOST_SELECT       3
#define PBPROTO_STATUS_INVALID_CMD       4
#define PBPROTO_STATUS_PACKET_TOO_LARGE  5
#define PBPROTO_STATUS_ERROR             6

// protocol stages for error reprots
#define PBPROTO_STAGE_END_SELECT         0x10
#define PBPROTO_STAGE_SIZE_HI            0x20
#define PBPROTO_STAGE_SIZE_LO            0x30
#define PBPROTO_STAGE_DATA               0x40
#define PBPROTO_STAGE_LAST_DATA          0x50
#define PBPROTO_STAGE_BURST_LO           0x60
#define PBPROTO_STAGE_BURST_HI           0x70
#define PBPROTO_STAGE_INPUT              0x80

// commands
#define PBPROTO_CMD_SEND       0x11   // amiga wants to send a packet
#define PBPROTO_CMD_RECV       0x22   // amiga wants to receive a packet
#define PBPROTO_CMD_SEND_BURST 0x33
#define PBPROTO_CMD_RECV_BURST 0x44

// line status
#define PBPROTO_LINE_OFF       0x0
#define PBPROTO_LINE_DISABLED  0x7
#define PBPROTO_LINE_OK        0x1

// callbacks
typedef uint8_t (*pb_proto_fill_func)(uint16_t *uwSize);
typedef uint8_t (*pb_proto_proc_func)(uint16_t uwSize);

typedef struct {
  uint8_t cmd;		// received pb proto command
  uint8_t status;   // status after processing the command
  uint8_t is_send;    // was a transmit command (amiga send?)
  uint8_t stats_id; // what id to use for stats recording
  uint16_t size;     // packet size
  uint16_t delta;    // hw timing for transmit
  uint16_t rate;     // delta converted to transfer rate
  uint16_t recv_delta; // delta after recv was requested
  uint32_t ts;       // time stamp of transfer
} pb_proto_stat_t;

extern pb_proto_stat_t pb_proto_stat; // filled by pb_proto_handle()

// ----- Parameter -----

extern uint16_t pb_proto_rx_timeout; // timeout for next byte in 100us

// ----- API -----

extern void pb_proto_init(pb_proto_fill_func fill_func, pb_proto_proc_func proc_func);
extern uint8_t  parGetStatusLines(void);
extern uint8_t  pb_proto_handle(void); // side effect: fill pb_proto_stat!
extern void parRequestAmiRead(void);

#endif
