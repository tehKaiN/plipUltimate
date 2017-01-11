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
