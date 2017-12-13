/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#ifndef GUARD_MAIN_H
#define GUARD_MAIN_H

#include <clib/exec_protos.h>

#define BOOT_PAGE_BYTE_SIZE 128
#define BOOT_PAGE_COUNT 256
#define BOOT_NRWW_PAGE_COUNT 32
#define BOOT_RWW_PAGE_COUNT (BOOT_PAGE_COUNT - BOOT_NRWW_PAGE_COUNT)

typedef struct _tPage {
	UBYTE ubIdx;
	UBYTE pData[BOOT_PAGE_BYTE_SIZE];
} tPage;

#endif // GUARD_MAIN_H
