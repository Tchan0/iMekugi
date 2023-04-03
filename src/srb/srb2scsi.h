/*******************************************************************************
* Used to send SRB requests over TCP
*
*******************************************************************************/
#ifndef SRB2SCSI_H_DEFINED__
#define SRB2SCSI_H_DEFINED__

#include "scsi/scsi.h"

DWORD executeSCSICommand (PSRB_ExecSCSICmd pSRB, int isWin32Call);

#endif
