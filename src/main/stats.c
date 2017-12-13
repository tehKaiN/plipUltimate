/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#include <main/stats.h>
#include <main/base/uartutil.h>
#include <main/base/uart.h>

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
