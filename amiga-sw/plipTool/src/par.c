#include "par.h"
#include <stdio.h>
#include <resources/misc.h>
#include <clib/misc_protos.h>

struct Library *MiscBase = NULL;

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
	FreeMiscResource(MR_PARALLELPORT);
	FreeMiscResource(MR_PARALLELBITS);
}

void parWaitBusyHi(void) {
	volatile UBYTE ubBusy;
	UWORD uwTimeout;
	
	uwTimeout = 1;
	do {
		ubBusy = PAR_STATUS_VAL & _BV(PAR_BUSY);
		++uwTimeout;
	} while(!ubBusy && uwTimeout);
	
	if(!uwTimeout) {
		printf("ERR: waitBusyHi timeout!\n");
	}
}

void parWaitBusyLo(void) {
	volatile UBYTE ubBusy;
	UWORD uwTimeout;
	
	uwTimeout = 1;
	do {
		ubBusy = PAR_STATUS_VAL & _BV(PAR_BUSY);
		++uwTimeout;
	} while(ubBusy && uwTimeout);
	
	if(!uwTimeout) {
		printf("ERR: waitBusyLo timeout!\n");
	}
}

