/*
 * spi.h - SPI setup
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

#ifndef SPI_H
#define SPI_H

#include <avr/io.h>
#include "../global.h"

#define SPI_SS_MASK		PB2
#define SPI_MOSI_MASK	PB3
#define SPI_MISO_MASK	PB4
#define SPI_SCK_MASK	PB5

extern void spi_init(void);

inline void spi_out(u08 data)
{
  SPDR = data;
  while (!(SPSR&(1<<SPIF)));
}

inline u08 spi_in(void)
{
  SPDR = 0x00;
  while (!(SPSR&(1<<SPIF)));
  return SPDR;
}

inline void spi_enable_eth(void) { PORTB &= ~SPI_SS_MASK; }
inline void spi_disable_eth(void) { PORTB |= SPI_SS_MASK; }

#endif
