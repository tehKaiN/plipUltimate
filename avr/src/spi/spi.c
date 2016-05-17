/*
 * spi.c - SPI setup
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

#include "spi.h"

void spi_init(void)
{
	// output: CS, MOSI, SCK, input: MISO, card detect, write protect
	SPI_DDR |= SPI_SCK | SPI_MOSI | SD_CS | ETH_CS;
	SPI_DDR &= ~(SPI_MISO | SD_DETECT | SD_LOCK);

	// MOSI, SCK = 0, Eth CS = 1
	SPI_PORT &= ~(SPI_MOSI | SPI_SCK);
	SPI_PORT |= ETH_CS;

  SPCR = _BV(SPE) | _BV(MSTR); // 8 MHz @ 16
	SPSR = _BV(SPI2X);
}
