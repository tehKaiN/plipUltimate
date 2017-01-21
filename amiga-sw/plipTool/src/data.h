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
