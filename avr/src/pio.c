/*
 * pio.c - generic interface for packet I/O
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

#include "pio.h"
#include "base/uartutil.h"
#include "net/net.h"
#include "spi/enc28j60.h"

uint8_t pio_init(const uint8_t mac[6],uint8_t flags)
{
  // show hello
  // NOTE: UART - time_stamp_spc() pio: init: pio_dev_name(cur_dev)

  // call init
  uint8_t result = enc28j60_init(mac, flags);
  if(result == PIO_OK) {
		// NOTE: UART - : ok! mac=mac, flags=flags
    // show revision
    uint8_t rev;
    result = enc28j60_status(PIO_STATUS_VERSION, &rev);
    if(result == PIO_OK) {
			// NOTE: UART - rev=hex_byte(rev)
    }
  } else {
		// NOTE: UART = ERROR: hex_byte(result)
  }
  // NOTE: UART - \r\n
  return result;
}

void pio_exit(void)
{
	// NOTE: UART - time_stamp_spc() pio: exit\r\n
	enc28j60_exit();
}

uint8_t pio_send(const uint8_t *buf, uint16_t size)
{
  return enc28j60_send(buf, size);
}

uint8_t pio_recv(uint8_t *buf, uint16_t max_size, uint16_t *got_size)
{
  return enc28j60_recv(buf, max_size, got_size);
}

uint8_t pio_has_recv(void)
{
  return enc28j60_has_recv();
}

uint8_t pio_status(uint8_t status_id, uint8_t *value)
{
  return enc28j60_status(status_id, value);
}

uint8_t pio_control(uint8_t control_id, uint8_t value)
{
  return enc28j60_control(control_id, value);
}
