/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef PIO_H
#define PIO_H

#include <main/global.h>

/* result values */
#define PIO_OK            0
#define PIO_NOT_FOUND     1
#define PIO_TOO_LARGE     2
#define PIO_IO_ERR        3

/* init flags */
#define PIO_INIT_FULL_DUPLEX    1
#define PIO_INIT_LOOP_BACK      2
#define PIO_INIT_BROAD_CAST     4
#define PIO_INIT_FLOW_CONTROL   8

/* status flags */
#define PIO_STATUS_VERSION      0
#define PIO_STATUS_LINK_UP      1

/* control ids */
#define PIO_CONTROL_FLOW        0

/* --- API --- */


#endif
