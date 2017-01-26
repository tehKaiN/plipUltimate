/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#include <exec/types.h>

extern volatile UBYTE g_ubAckEdge; ///< ACK edge detection state.

/**
 *  Reserves parallel ACK interrupt.
 *  @return 1 on success, otherwise 0.
 */
UBYTE ackReserve(void);

/**
 *  Frees parallel ACK interrupt.
 */
void ackFree(void);
