/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef SPI_H
#define SPI_H

#include <avr/io.h>
#include <main/global.h>
#include <main/pinout.h>

extern void spiInit(void);

static inline void spiWriteByte(uint8_t data) {
  SPDR = data;
  while (!(SPSR&(1<<SPIF)));
}

static inline uint8_t spiReadByte(void) {
  SPDR = 0x00;
  while (!(SPSR&(1<<SPIF)));
  return SPDR;
}

static inline void spiEnableEth(void) {
	SPI_PORT |= SD_CS;
	SPI_PORT &= ~ETH_CS;
}
static inline void spiDisableEth(void) {
	SPI_PORT |= ETH_CS;
}
static inline void spiEnableSd(void) {
	SPI_PORT |= ETH_CS;
	SPI_PORT &= ~SD_CS;
}
static inline void spiDisableSd(void) {
	SPI_PORT |= SD_CS;
}
static inline void spiDisableAll(void) {
	SPI_PORT |= ETH_CS;
	SPI_PORT |= SD_CS;
}

#endif
