/*
 * enc28j60.h - pio_dev implementaton for Microchip ENC28J60
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

#ifndef ENC28J60_H
#define ENC28J60_H

uint8_t enc28j60_init(const uint8_t macaddr[6], uint8_t flags);
void enc28j60_exit(void);
uint8_t enc28j60_send(const uint8_t *data, uint16_t size);
uint8_t enc28j60_recv(uint8_t *data, uint16_t max_size, uint16_t *got_size);
uint8_t enc28j60_has_recv(void);
uint8_t enc28j60_status(uint8_t status_id, uint8_t *value);
uint8_t enc28j60_control(uint8_t control_id, uint8_t value);

#endif
