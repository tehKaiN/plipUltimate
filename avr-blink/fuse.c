#include <avr/io.h>

FUSES = {
    .low = 0xE2,
    .high = 0xD9,
    .extended = EFUSE_DEFAULT
};
