#include <avr/io.h>

FUSES = {
    .low = 0xE0,
    .high = 0xD8,
    .extended = EFUSE_DEFAULT
};