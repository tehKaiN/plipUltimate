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

#ifndef SPI_H
#define SPI_H

#include <avr/io.h>
#include "../global.h"
#include "../pinout.h"

extern void spiInit(void);

inline void spiWriteByte(uint8_t data) {
  SPDR = data;
  while (!(SPSR&(1<<SPIF)));
}

inline uint8_t spiReadByte(void) {
  SPDR = 0x00;
  while (!(SPSR&(1<<SPIF)));
  return SPDR;
}

inline void spiEnableEth(void) {
	SPI_PORT |= SD_CS;
	SPI_PORT &= ~ETH_CS;
}
inline void spiDisableEth(void) {
	SPI_PORT |= ETH_CS;
}
inline void spiEnableSd(void) {
	SPI_PORT |= ETH_CS;
	SPI_PORT &= ~SD_CS;
}
inline void spiDisableSd(void) {
	SPI_PORT |= SD_CS;
}
inline void spiDisableAll(void) {
	SPI_PORT |= ETH_CS;
	SPI_PORT |= SD_CS;
}

#endif
