/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef STATS_H
#define STATS_H

#include <main/global.h>

#define STATS_ID_PB_RX  0
#define STATS_ID_PB_TX  1
#define STATS_ID_PIO_RX 2
#define STATS_ID_PIO_TX 3
#define STATS_ID_NUM    4

typedef struct {
  uint32_t bytes;
  uint16_t cnt;
  uint16_t err;
  uint16_t drop;
  uint16_t max_rate;
} stats_t;

extern stats_t stats[STATS_ID_NUM];

extern void stats_reset(void);
extern void stats_dump_all(void);
extern void stats_dump(uint8_t pb, uint8_t pio);
extern void stats_update_ok(uint8_t id, uint16_t size, uint16_t rate);

inline stats_t *stats_get(uint8_t id)
{
  return &stats[id];
}

#endif
