/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#include <main/spi/spi.h>

void spiInit(void)
{
	// output: CS, MOSI, SCK, input: MISO, card detect, write protect
	SPI_DDR |= SPI_SCK | SPI_MOSI | SD_CS | ETH_CS;
	SPI_DDR &= ~(SPI_MISO | SD_DETECT | SD_LOCK);

	// MOSI, SCK = 0, Eth CS = 1
	SPI_PORT &= ~(SPI_MOSI | SPI_SCK);
	spiDisableAll();

  SPCR = _BV(SPE) | _BV(MSTR); // 8 MHz @ 16
	SPSR = _BV(SPI2X);
}
