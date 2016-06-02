/*
 * stats.c - manage device statistics
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

#include "stats.h"
#include "base/uartutil.h"
#include "base/uart.h"

stats_t stats[STATS_ID_NUM];

void stats_reset(void)
{
  for(uint8_t i=0;i<STATS_ID_NUM;i++) {
    stats_t *s = &stats[i];
    s->bytes = 0;
    s->cnt = 0;
    s->err = 0;
    s->drop = 0;
    s->max_rate = 0;
  }
}

void stats_update_ok(uint8_t id, uint16_t size, uint16_t rate)
{
  stats_t *s = &stats[id];
  s->cnt++;
  s->bytes += size;
  if(rate > s->max_rate) {
    s->max_rate = rate;
  }
}

static void dump_line(uint8_t id)
{
//  const stats_t *s = &stats[id];

  // NOTE: UART - hex_word(s->cnt) hex_word(s->bytes) hex_word(s->err) hex_word(s->drop) hex_word(s->max_rate)

//  PGM_P str;
  switch(id) {
    case STATS_ID_PB_RX:
			// NOTE: UART - rx plipbox
      break;
    case STATS_ID_PIO_RX:
			// NOTE: UART - rx_pio
      break;
    case STATS_ID_PB_TX:
    case STATS_ID_PIO_TX:
			// NOTE: UART - tx
      break;
    default:
			// NOTE: UART - ?
      break;
  }
}

static void dump_header(void)
{
	// NOTE: UART - cnt  bytes    err  drop rate\r\n
}

void stats_dump_all(void)
{
  dump_header();
  for(uint8_t i=0;i<STATS_ID_NUM;i++) {
    dump_line(i);
  }
}

void stats_dump(uint8_t pb, uint8_t pio)
{
  dump_header();
  if(pb) {
    dump_line(STATS_ID_PB_RX);
    dump_line(STATS_ID_PB_TX);
  }
  if(pio) {
    dump_line(STATS_ID_PIO_RX);
    dump_line(STATS_ID_PIO_TX);
  }
}
