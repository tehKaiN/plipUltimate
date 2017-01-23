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

#ifndef BOOTLOADER_PINOUT_H
#define BOOTLOADER_PINOUT_H

/// Parallel status bits
#define NSTROBE_PIN PC0
#define NACK_PIN    PC1
#define BUSY_PIN    PC2
#define POUT_PIN    PC3
#define SEL_PIN     PC4

#define NSTROBE _BV(NSTROBE_PIN)
#define NACK    _BV(NACK_PIN)
#define BUSY    _BV(BUSY_PIN)
#define POUT    _BV(POUT_PIN)
#define SEL     _BV(SEL_PIN)

#define PAR_STATUS_PORT PORTC
#define PAR_STATUS_DDR DDRC
#define PAR_STATUS_PIN PINC
#define PAR_STATUS_MASK (NSTROBE | NACK | BUSY | POUT | SEL)

/// LED-related defines
#define LED_DDR        DDRC
#define LED_PORT       PORTC
#define LED_STATUS_PIN PC5
#define LED_STATUS     _BV(LED_STATUS_PIN)

/// Parallel data bits
#define PAR_DATA_PORT PORTD
#define PAR_DATA_DDR DDRD
#define PAR_DATA_PIN PIND

#endif // BOOTLOADER_PINOUT_H
