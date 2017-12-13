/*
 * This file is part of PlipUltimate.
 * License: GPLv2
 * Full license: https://github.com/tehKaiN/plipUltimate/blob/master/LICENSE
 * Authors list: https://github.com/tehKaiN/plipUltimate/blob/master/AUTHORS
 */

#include "par.h"
#include <stdio.h>
#include <resources/misc.h>
#include <clib/misc_protos.h>

struct Library *MiscBase = NULL;

/**
 *  Makes all parallel pins low.
 *  Plip data lines aren't pulled down, so to prevent floating state Ami must
 *  pull pins low whenever possible.
 */
void parMakeLow(void) {
	// Make all pins low
	PAR_STATUS_DDR |= PAR_STATUS_MASK;
	PAR_STATUS_VAL &= ~PAR_STATUS_MASK;
	PAR_DATA_DDR = 0xFF;
	PAR_DATA_VAL = 0;
}

/**
 *  Reserves parallel port for exclusive usage.
 *  Based on C example from http://wiki.amigaos.net/wiki/Misc_Resource
 *  @param szOwnerName Pointer to app name string
 */
UBYTE parReserve(char *szOwnerName) {
	char *pOwner;
	
	MiscBase = (struct Library*)OpenResource(MISCNAME);
	if(!MiscBase) {
		printf("ERR: Couldn't open %s\n", MISCNAME);
		return 0;
	}
	
	pOwner = AllocMiscResource(MR_PARALLELPORT, szOwnerName);
	if(pOwner) {
		printf("ERR: Parallel data lines access blocked by: %s\n", pOwner);
		return 0;
	}
	pOwner = AllocMiscResource(MR_PARALLELBITS, szOwnerName);
	if(pOwner) {
		printf("ERR: Parallel status lines access blocked by: %s\n", pOwner);
		// Free what was already allocated
		FreeMiscResource(MR_PARALLELPORT);
	}
	return 1;
}

void parFree(void) {
	parMakeLow();
	FreeMiscResource(MR_PARALLELPORT);
	FreeMiscResource(MR_PARALLELBITS);
}

UBYTE parWaitBusy(UBYTE ubVal) {
	volatile UBYTE ubBusy;
	UWORD uwTimeout;
	
	uwTimeout = 1;
	do {
		ubBusy = ((PAR_STATUS_VAL & PAR_BUSY) != 0);
		++uwTimeout;
	} while(ubBusy != ubVal && uwTimeout);
	
	if(ubBusy == ubVal)
		return 1;
	printf("ERR: BUSY=%u timeout!\n",ubVal);
	return 0;
}
