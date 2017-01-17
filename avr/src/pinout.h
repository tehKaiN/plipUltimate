/*
 * pinout.h - AVR Pinout configuration
 *
 * Written by
 *  Marcin Nowosad <kain@piwnica.ws>
 *
 * This file is part of plipUltimate.
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

#ifndef PINOUT_H
#define PINOUT_H

/// Parallel status bits
#define NSTROBE_PIN PC0
#define NACK_PIN    PC1
#define BUSY_PIN    PC2
#define POUT_PIN    PC3
#define SEL_PIN     PC4

/// LED-related defines
#define LED_DDR        DDRC
#define LED_PORT       PORTC
#define LED_STATUS_PIN PC5
#define LED_STATUS     _BV(LED_STATUS_PIN)

#define NSTROBE _BV(NSTROBE_PIN)
#define NACK    _BV(NACK_PIN)
#define BUSY    _BV(BUSY_PIN)
#define POUT    _BV(POUT_PIN)
#define SEL     _BV(SEL_PIN)

#define PAR_STATUS_PORT PORTC
#define PAR_STATUS_DDR DDRC
#define PAR_STATUS_PIN PINC
#define PAR_STATUS_MASK (NSTROBE | NACK | BUSY | POUT | SEL)

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

