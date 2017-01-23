/*
 * This file is part of PlipUltimate.
 * Based on PlipBox by Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
