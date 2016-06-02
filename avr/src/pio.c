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
#include "pio_dev.h"
#include "base/uartutil.h"
#include "net/net.h"

#ifdef DEV_ENC28J60
#include "enc28j60.h"
#endif

// the table of available devices
static const pio_dev_ptr_t PROGMEM devices[] = {
#ifdef DEV_ENC28J60
  &pio_dev_enc28j60,
#endif
};
#define NUM_DEVICES  (sizeof(devices) / sizeof(pio_dev_ptr_t))

// index of active device
static uint8_t dev_id;
static pio_dev_ptr_t cur_dev;

uint8_t pio_set_device(uint8_t id)
{
  if(id < NUM_DEVICES) {
    dev_id = id;
    return 1;
  }
  // invalid device selected
  return 0;
}

uint8_t pio_init(const uint8_t mac[6],uint8_t flags)
{
  // get current device
  cur_dev = (pio_dev_ptr_t)pgm_read_word(devices + dev_id);

  // show hello
  // NOTE: UART - time_stamp_spc() pio: init: pio_dev_name(cur_dev)

  // call init
  uint8_t result = pio_dev_init(cur_dev, mac, flags);
  if(result == PIO_OK) {
		// NOTE: UART - : ok! mac=mac, flags=flags
    // show revision
    uint8_t rev;
    result = pio_dev_status(cur_dev, PIO_STATUS_VERSION, &rev);
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
  pio_dev_exit(cur_dev);
}

uint8_t pio_send(const uint8_t *buf, uint16_t size)
{
  return pio_dev_send(cur_dev, buf, size);
}

uint8_t pio_recv(uint8_t *buf, uint16_t max_size, uint16_t *got_size)
{
  return pio_dev_recv(cur_dev, buf, max_size, got_size);
}

uint8_t pio_has_recv(void)
{
  return pio_dev_has_recv(cur_dev);
}

uint8_t pio_status(uint8_t status_id, uint8_t *value)
{
  return pio_dev_status(cur_dev, status_id, value);
}

uint8_t pio_control(uint8_t control_id, uint8_t value)
{
  return pio_dev_control(cur_dev, control_id, value);
}
