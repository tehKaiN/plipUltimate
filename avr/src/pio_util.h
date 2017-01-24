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

#ifndef PIO_UTIL_H
#define PIO_UTIL_H

#include "global.h"

/* get the configured init flags for PIO */
extern uint8_t pio_util_get_init_flags(void);

/* receive packet from current PIO and store in pkt_buf.
   also update stats and is verbose if enabled.
   only call if pio_has_recv() ist not 0!
   returns packet size and pio status.
*/
extern uint8_t pio_util_recv_packet(uint16_t *size);

/* send packet to current PIO from pkt_buf
   aöso updates stats and is verbose if enabled.
   return pio status.
*/
extern uint8_t pio_util_send_packet(uint16_t size);


#endif
