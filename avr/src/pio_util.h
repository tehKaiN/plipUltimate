/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef PIO_UTIL_H
#define PIO_UTIL_H

#include "global.h"

/* get the configured init flags for PIO */
extern uint8_t pio_util_get_init_flags(void);

/* receive packet from current PIO and store in pkt_buf.
   also update stats and is verbose if enabled.
   only call if pio_has_recv() ist not 0!
   returns packet size and pio status.
*/
extern uint8_t pio_util_recv_packet(uint16_t *size);

/* send packet to current PIO from pkt_buf
   aöso updates stats and is verbose if enabled.
   return pio status.
*/
extern uint8_t pio_util_send_packet(uint16_t size);


#endif
