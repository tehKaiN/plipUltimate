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

#ifndef STATS_H
#define STATS_H

#include "global.h"

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
