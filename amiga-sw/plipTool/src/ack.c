#include <stdio.h>
#include <clib/exec_protos.h>
#include <clib/cia_protos.h>
#include <resources/cia.h>
#include <hardware/cia.h>
#include <exec/interrupts.h>

volatile UBYTE g_ubAckEdge;
struct CIABase *s_pCiaBase;
struct Interrupt *s_pInt;
WORD s_wICRMask;

/**
 *  Parallel port ACK Interrupt handler.
 *  @param pState Pointer to ACK edge detection variable.
 */
__amigainterrupt __saveds void ackIntServer(__reg("a1") UBYTE *pState) {
	*pState = 1;
}

UBYTE ackReserve(void) {
	struct Interrupt *pOldInt;
	// Open CIA resource
	s_pCiaBase = OpenResource(CIAANAME);
	if(!s_pCiaBase) {
		printf("ERR: Couldn't open CIA Resource\n");
		return 0;
	}
	
	// Alloc & fill int struct
	s_pInt = AllocMem(sizeof(struct Interrupt), MEMF_CHIP | MEMF_PUBLIC);
	if(!s_pInt) {
		printf("ERR: Out of mem\n");
		return 0;
	}
	s_pInt->is_Node.ln_Type = NT_INTERRUPT;
	s_pInt->is_Node.ln_Pri  = 0;
	s_pInt->is_Node.ln_Name = "plipTool";
	s_pInt->is_Data         = (APTR)&g_ubAckEdge;
	s_pInt->is_Code         = (APTR)ackIntServer;
	
	// Register int
	pOldInt = AddICRVector((struct Library*)s_pCiaBase, CIAICRB_FLG, s_pInt);
	if(pOldInt) {
		printf(
			"Couldn't register ICR int vector. Current owner: %s\n",
			pOldInt->is_Node.ln_Name
		);
		FreeMem(s_pInt, sizeof(struct Interrupt));
		return 0;
	}
	
	// Get current CIA int mask
	s_wICRMask = AbleICR((struct Library*)s_pCiaBase, 0);
	// Reset CIA ACK int
	SetICR((struct Library*)s_pCiaBase, CIAICRF_FLG);
	// Enable CIA ACK int
	AbleICR((struct Library*)s_pCiaBase, CIAICRF_SETCLR | CIAICRF_FLG);
	
	return 1;
}

void ackFree(void) {
	// Disable CIA ACK int
	AbleICR((struct Library*)s_pCiaBase, CIAICRF_FLG);
	// Remove CIA ACK int vector
	RemICRVector((struct Library*)s_pCiaBase, CIAICRB_FLG, s_pInt);
	// Mem cleanup
	FreeMem(s_pInt, sizeof(struct Interrupt));
}