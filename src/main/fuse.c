/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#include <avr/io.h>

FUSES = {
    .low = 0xE0,
    .high = 0xD8,
    .extended = 0xFF
};
