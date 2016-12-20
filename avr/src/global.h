/*
 * global.h - global project settings
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plipbox.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>

/// Version macros
#define VERSION_MAJ "0"
#define VERSION_MIN "6"
#define VERSION VERSION_MAJ "." VERSION_MIN
#define BUILD_DATE "BUILD_DATE"

/// CPU cycle count per microsecond
#define F_CPU 20000000UL
#define CYCLES_PER_US ((F_CPU+500000)/1000000)

#endif
