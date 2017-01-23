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

#ifndef GUARD_DATA_H
#define GUARD_DATA_H

#include <clib/exec_protos.h>

#define PBPROTO_CMD_SEND_FLASH 0x88

#define RECV_BFR_SIZE 1600
extern UBYTE g_pRecvBfr[RECV_BFR_SIZE];
extern UWORD g_uwRecvSize;


void dataRecv(void);
void dataSend(const UBYTE *pData, UWORD uwSize);
void dataDump(const UBYTE *pData, const UWORD uwDataSize);

#endif //GUARD_DATA_H
