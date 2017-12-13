/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>

/// CPU cycle count per microsecond
#define F_CPU 20000000UL
#define CYCLES_PER_US ((F_CPU+500000)/1000000)

/// Uncomment this for no-ENC28j60 dev mode
//#define NOENC

#endif
