/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef PINOUT_H
#define PINOUT_H

/// LED-related defines
#define LED_DDR        DDRC
#define LED_PORT       PORTC
#define LED_STATUS_PIN PC5
#define LED_STATUS     _BV(LED_STATUS_PIN)

/// Parallel status bits
#define PAR_NSTROBE_PIN PC0
#define PAR_NACK_PIN    PC1
#define PAR_BUSY_PIN    PC2
#define PAR_POUT_PIN    PC3
#define PAR_SEL_PIN     PC4

#define PAR_NSTROBE _BV(PAR_NSTROBE_PIN)
#define PAR_NACK    _BV(PAR_NACK_PIN)
#define PAR_BUSY    _BV(PAR_BUSY_PIN)
#define PAR_POUT    _BV(PAR_POUT_PIN)
#define PAR_SEL     _BV(PAR_SEL_PIN)

#define PAR_STATUS_PORT PORTC
#define PAR_STATUS_DDR DDRC
#define PAR_STATUS_PIN PINC
#define PAR_STATUS_MASK (PAR_NSTROBE | PAR_NACK | PAR_BUSY | PAR_POUT | PAR_SEL)

/// Parallel data bits
#define PAR_DATA_PORT PORTD
#define PAR_DATA_DDR DDRD
#define PAR_DATA_PIN PIND

/// SPI devices pins
#define SD_LOCK   _BV(PB0)
#define SD_CS     _BV(PB1)
#define ETH_CS    _BV(PB2)
#define SPI_MOSI  _BV(PB3)
#define SPI_MISO  _BV(PB4)
#define SPI_SCK   _BV(PB5)
// PB6 is used as XTAL1
#define SD_DETECT _BV(PB7)

#define SPI_PORT PORTB
#define SPI_PIN  PINB
#define SPI_DDR  DDRB

#endif // PINOUT_H

