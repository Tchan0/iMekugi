/*******************************************************************************
*
*
*******************************************************************************/
#include "srb.h"
#include "scsi/scsi.h"

/*******************************************************************************
* 
*******************************************************************************/
char* srbGetCodeDesc (char code, SRB_CODE_DESC* descArray, int max_entries, char* szDefaultDesc)
{
 char* pchar = szDefaultDesc;
 int i;

 for (i=0; i < max_entries; i++){
	 if (descArray[i].code == code){
		 pchar = descArray[i].desc;
		 return (pchar);
	 }
 };

 return (pchar);
}

/*******************************************************************************
* 
*******************************************************************************/
char* srbGetStatusDesc (char status)
{
 return (srbGetCodeDesc(status, srbStatusCodeDesc, SRB_STATUS_CODE_MAX, "SRB status code unknown"));
}

/*******************************************************************************
* 
*******************************************************************************/
char* srbGetCommandDesc (char command)
{
 return (srbGetCodeDesc(command, srbCommandCodeDesc, SRB_COMMAND_CODE_MAX, "SRB command code unknown"));
}

/*******************************************************************************
* TODO horrible hack - redefinition of scsiGetCommandDesc to avoid include problems
*******************************************************************************/
char* srbGetSCSICommandDesc (char command)
{
 return (srbGetCodeDesc(command, (SRB_CODE_DESC*)scsiCommandCodeDesc, SCSI_COMMAND_CODE_MAX, "SCSI command code unknown"));
}

/*******************************************************************************
* returns the size of the SRB in bytes, including all related data/sense/... buffers
*******************************************************************************/
DWORD srbGetSize (LPSRB pSRB, int isWin32Call)
{
 switch (pSRB->SRB_Cmd) {
	case SC_HA_INQUIRY:			//0x00
		return sizeof(SRB_HAInquiry);
	case SC_GET_DEV_TYPE:		//0x01
		return (isWin32Call ? sizeof(SRB_GDEVBlock) : sizeof(SRB_GDEVBlock_DOS)) ;
	case SC_EXEC_SCSI_CMD:		//0x02
		if (isWin32Call){
			PSRB_ExecSCSICmd pSRBSCSI = (PSRB_ExecSCSICmd)pSRB;
			return ((sizeof(SRB_ExecSCSICmd) + pSRBSCSI->SRB_BufLen));
		} else {
			PSRB_ExecSCSICmd_DOS pSRBSCSIDOS = (PSRB_ExecSCSICmd_DOS)pSRB;
			return ((sizeof(SRB_ExecSCSICmd_DOS) + pSRBSCSIDOS->SRB_BufLen + pSRBSCSIDOS->SRB_SenseLen + 
						pSRBSCSIDOS->SRB_CDBLen - 1));
		}
	case SC_ABORT_SRB:			//0x03
		return (isWin32Call ? sizeof(SRB_Abort) : sizeof(SRB_Abort_DOS));
	case SC_RESET_DEV:			//0x04
		return (isWin32Call ? sizeof(SRB_BusDeviceReset) : sizeof(SRB_BusDeviceReset_DOS));
	case SC_SET_HA_PARMS:		//0x05 - only in aspidos, NOT in aspi32 !
		return sizeof(SRB_SetHostAdapterParams_DOS);
	case SC_GET_DISK_INFO:		//0x06
		return sizeof(SRB_GetDiskInfo);
	case SC_RESCAN_SCSI_BUS:	//0x07 - NOT in aspidos - this cmd code was "reserved for future expansion"
		return sizeof(SRB_RescanPort);
	case SC_GETSET_TIMEOUTS:	//0x08 - NOT in aspidos - this cmd code was "reserved for future expansion"
		return sizeof(SRB_GetSetTimeouts);
	default:
		break;
 }	
 
 return (0);
}

/*******************************************************************************
 *
 *******************************************************************************/
void logprintSRB (PLOGGER pLogger, LPSRB pSRB)
{
 if (pSRB->SRB_Cmd == SC_EXEC_SCSI_CMD){
	PSRB_ExecSCSICmd pSRBSCSI = (PSRB_ExecSCSICmd)pSRB;
	LOGPRINT_LOW(pLogger, "SC_EXEC_SCSI_CMD %s (0x%02X)\r\n", srbGetSCSICommandDesc(pSRBSCSI->CDBByte[0]), pSRBSCSI->CDBByte[0]);
	loggerPrintHexDump (pLogger, LOGLEVEL_HIGH, (uint8_t*)pSRB, (uint32_t)sizeof(SRB_ExecSCSICmd), 0, 16);

	//extra data buffer
	if (pSRBSCSI->SRB_BufLen){
		LOGPRINT_HIGH(pLogger, "SRB_Buf data:\r\n");
		loggerPrintHexDump (pLogger, LOGLEVEL_HIGH, (uint8_t*)pSRBSCSI->SRB_BufPointer, (uint32_t)pSRBSCSI->SRB_BufLen, 0, 16);
	}

 } else {
	loggerPrintHexDump (pLogger, LOGLEVEL_HIGH, (uint8_t*)pSRB, (uint32_t)srbGetSize(pSRB,1), 0, 16);//TODO: srbGetSize isWin32 not correctly implemented !
 }
}