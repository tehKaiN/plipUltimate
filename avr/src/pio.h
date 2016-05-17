/*
 * pio.h - generic interface for packet I/O
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

#ifndef PIO_H
#define PIO_H

#include "global.h"

/* result values */
#define PIO_OK            0
#define PIO_NOT_FOUND     1
#define PIO_TOO_LARGE     2
#define PIO_IO_ERR        3

/* init flags */
#define PIO_INIT_FULL_DUPLEX    1
#define PIO_INIT_LOOP_BACK      2
#define PIO_INIT_BROAD_CAST     4
#define PIO_INIT_FLOW_CONTROL   8

/* status flags */
#define PIO_STATUS_VERSION      0
#define PIO_STATUS_LINK_UP      1

/* control ids */
#define PIO_CONTROL_FLOW        0

/* --- API --- */

extern uint8_t pio_set_device(uint8_t id);
extern uint8_t pio_init(const uint8_t mac[6],uint8_t flags);
extern void pio_exit(void);

extern uint8_t pio_send(const uint8_t *buf, uint16_t size);
extern uint8_t pio_recv(uint8_t *buf, uint16_t max_size, uint16_t *got_size);
extern uint8_t pio_has_recv(void);
extern uint8_t pio_status(uint8_t status_id, uint8_t *value);
extern uint8_t pio_control(uint8_t control_id, uint8_t value);

#endif
