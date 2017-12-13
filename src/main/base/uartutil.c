/*
 * uartutil.c - serial utility routines
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

/*
#include <main/base/uart.h>
#include <main/base/util.h>
#include <main/base/uartutil.h>
#include <main/base/timer.h>

void uart_send_pstring(PGM_P data)
{
  while(1) {
    uint8_t c = pgm_read_byte_near(data);
    if(c == 0) {
      break;
    }
    uart_send(c);
    data++;
  }
}

void uart_send_string(const char *str)
{
  while(*str) {
    uart_send(*str);
    str++;
  }
}

void uart_send_data(uint8_t *data,uint8_t len)
{
	uint8_t i;
  for(i=0;i<len;i++)
    uart_send(data[i]);
}

void uart_send_crlf(void)
{
  uart_send_pstring(PSTR("\r\n"));
}

void uart_send_spc(void)
{
  uart_send((uint8_t)' ');
}

static uint8_t buf[12];

void uart_send_time_stamp_spc(void)
{
  uint32_t ts = time_stamp;
  dword_to_dec(ts, buf, 10, 4);
  buf[11] = ' ';
  uart_send_data(buf,12);
}

void uart_send_time_stamp_spc_ext(uint32_t ts)
{
  dword_to_dec(ts, buf, 10, 4);
  buf[11] = ' ';
  uart_send_data(buf,12);
}

void uart_send_rate_kbs(uint16_t kbs)
{
  dword_to_dec(kbs, buf, 6, 2);
  uart_send_data(buf,7);
  uart_send_pstring(PSTR(" KB/s"));
}

void uart_send_delta(uint32_t delta)
{
  // huge -> show upper hex
  if(delta > 0xffff) {
    buf[0] = '!';
    word_to_hex((uint16_t)(delta >> 16), buf+1);
  }
  // for too large numbers use hex
  else if(delta > 9999) {
    buf[0] = '>';
    word_to_hex(delta, buf+1);
  }
  // for smaller numbers use decimal
  else {
    buf[0] = '+';
    dword_to_dec(delta, buf+1, 4, 4);
  }
  uart_send_data(buf,5);
}

void uart_send_hex_byte(uint8_t data)
{
  byte_to_hex(data,buf);
  uart_send_data(buf,2);
}

void uart_send_hex_word(uint16_t data)
{
  word_to_hex(data,buf);
  uart_send_data(buf,4);
}

void uart_send_hex_dword(uint32_t data)
{
  dword_to_hex(data,buf);
  uart_send_data(buf,8);
}

#ifdef DEBUG
void uart_send_free_stack(void)
{
  uint16_t free = stack_free();
  uart_send_pstring(PSTR("free stack:"));
  uart_send_hex_word(free);
  uart_send_crlf();
}
#endif
*/
