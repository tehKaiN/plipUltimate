/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef BRIDGE_H
#define BRIDGE_H

#include "global.h"

void bridgeLoop(void);
uint8_t bridgeFillPacket(uint16_t *pFilledSize);
uint8_t bridgeProcessPacket(uint16_t uwSize);

#endif
