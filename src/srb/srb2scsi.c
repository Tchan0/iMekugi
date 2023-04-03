/*******************************************************************************
* Executes SRB requests via SCSI (eg. via sg or spti-DeviceIOControl)
*
*******************************************************************************/
#include "scsi/scsi.c"

#include "srb2tcp.h"
#include "srb2scsi.h"

//#include "devices/device.c"

PMSDEVLIST pDevList = NULL;
PMSDEVLIST pExposedDevList = NULL;

/*******************************************************************************
* SC_GETSET_TIMEOUTS
*******************************************************************************/
DWORD getSetTimeouts (PSRB_GetSetTimeouts pSRB)
{
 PMSDEV pTmpDev;
 uint32_t seconds;
 int numDevicesSet = 0;

 if (pSRB->SRB_Flags & SRB_DIR_IN){
	//getTimeouts(), no wildcards (0xFF) allowed
	pTmpDev = scsiGetDeviceByMappedHTL (pExposedDevList, pSRB->SRB_HaId, pSRB->SRB_Target, pSRB->SRB_Lun, 1);
 	if (!pTmpDev){
		pSRB->SRB_Status = SS_NO_DEVICE;
		return (pSRB->SRB_Status);//or SS_INVALID_HA
	}
 
 	if (!scsiGetTimeouts (pTmpDev, &seconds)){
		pSRB->SRB_Status = SS_INVALID_SRB;
		return (pSRB->SRB_Status);
	}
	pSRB->SRB_Timeout = seconds * 2;//we need to return half-seconds

 } else if (pSRB->SRB_Flags & SRB_DIR_OUT){ //08 00 FF 10 00 00 00 00 FF FF 0A 00 00 00    
	//setTimeouts, wildcards (0xFF =all) allowed in HaId, Target, Lun
	seconds = pSRB->SRB_Timeout / 2;//we receive half-seconds
	if (pExposedDevList){
		int id = -1;
		while (scsiGetNextDevice(pExposedDevList, &id, &pTmpDev)){
			if ((pSRB->SRB_HaId == pTmpDev->real_hostId) ||
				(pSRB->SRB_HaId == 0xFF)){
				if ((pSRB->SRB_Target == pTmpDev->real_targetId) ||
					(pSRB->SRB_Target == 0xFF)){
					if ((pSRB->SRB_Lun == pTmpDev->real_lunId) ||
						(pSRB->SRB_Lun == 0xFF)){
						if (!scsiSetTimeouts (pTmpDev, seconds)){
							printf("??? scsiSetTimeouts error \r\n");
							pSRB->SRB_Status = SS_INVALID_SRB;
							return (pSRB->SRB_Status);
						} else {
							numDevicesSet++;
						}
					}
				}
			}
		}
	}
 	if (!numDevicesSet){
		pSRB->SRB_Status = SS_NO_DEVICE;
		return (pSRB->SRB_Status);//or SS_INVALID_HA
	}

 } else {
	printf("??? invalid flag for SetGetTimeouts\r\n");
	pSRB->SRB_Status = SS_INVALID_SRB;
	return (pSRB->SRB_Status);
 }

 pSRB->SRB_Status = SS_COMP;
 return (pSRB->SRB_Status);
}

/*******************************************************************************
* SC_RESCAN_SCSI_BUS
*******************************************************************************/
DWORD rescanSCSIBus (PSRB_RescanPort pSRB)
{
 if (scsiRescan (pSRB->SRB_HaId)){
	pSRB->SRB_Status = SS_COMP;
	return (pSRB->SRB_Status);
 }

 pSRB->SRB_Status = SS_INVALID_HA;
 return (SS_INVALID_HA);
}

/*******************************************************************************
* SC_GET_DISK_INFO
*******************************************************************************/
DWORD getDiskInfo (PSRB_GetDiskInfo pSRB)
{
 PMSDEV pTmpDev;

 pTmpDev = scsiGetDeviceByMappedHTL (pExposedDevList, pSRB->SRB_HaId, pSRB->SRB_Target, pSRB->SRB_Lun, 0);
 if (!pTmpDev){
	pSRB->SRB_Status = SS_NO_DEVICE;
	return (pSRB->SRB_Status);//or SS_INVALID_HA
 } 

 //we don't need to support this I think:
 // - DOS will call TCP anyways
 // - Windows will either access it via spti, or via TCP
 pSRB->SRB_DriveFlags = DISK_NOT_INT13;

 pSRB->SRB_Status = SS_COMP;
 return (pSRB->SRB_Status);
}

/*******************************************************************************
* SC_SET_HA_PARMS
*******************************************************************************/
DWORD setHostAdapterParameters (PSRB_SetHostAdapterParams_DOS pSRB)
{
 pSRB->SRB_Status = SS_ERR;
 return (pSRB->SRB_Status);//TODO OPTIONAL SOMEDAY? not sure if this is implementable...
}

/*******************************************************************************
* SC_RESET_DEV: send a SCSI Bus Device reset to the specified target
*******************************************************************************/
DWORD resetDeviceCore (PSRB_BusDeviceReset pSRB, int isWin32Call)//this is the code that is executed in a separate thread
{//TODO: core functions should retun void, since they can be called from a thread <- cfr scsi execute cmd
 PMSDEV pTmpDev;
 BYTE	HaId, Target, Lun;

 if (isWin32Call){
	HaId   = pSRB->SRB_HaId;
	Target = pSRB->SRB_Target;
	Lun    = pSRB->SRB_Lun;
 } else {
	PSRB_BusDeviceReset_DOS pSRBDOS = (PSRB_BusDeviceReset_DOS)pSRB;
	HaId   = pSRBDOS->SRB_HaId;
	Target = pSRBDOS->SRB_Target;
	Lun    = pSRBDOS->SRB_Lun;
 }

 pTmpDev = scsiGetDeviceByMappedHTL (pExposedDevList, HaId, Target, Lun, 1);
 if (!pTmpDev){
	pSRB->SRB_Status = SS_NO_DEVICE;
	return (pSRB->SRB_Status);//or SS_INVALID_HA // TODO: set this on SRB, and signal somehow that the async is finished
 } 
 
 if (!scsiResetTarget (pTmpDev)){
	pSRB->SRB_Status = SS_ERR;
	return (pSRB->SRB_Status); // TODO: set this on SRB, and signal somehow that the async is finished
 } 

 //TODO: if not to be sent back via TCP: notifySRBCaller (pSRB);
 //      else: send TCPSRB via server->client connection
 pSRB->SRB_Status = SS_COMP;
 return (pSRB->SRB_Status); // TODO: set this on SRB, and signal somehow that the async is finished
}

DWORD resetDevice (PSRB_BusDeviceReset pSRB, int isWin32Call)
{//TODO:<- cfr scsi execute cmd, similar async processing
 if (!pSRB){
	pSRB->SRB_Status = SS_ERR;
	return (pSRB->SRB_Status);
 } 

 //asynchronuous call - only valid if command came from a DOS system TODO: incorporate "DOS" in TCPSRB ?
 //TODO: is the above correct ? wnaspi32 doc says SC_RESET_DEV is also async
 #ifdef ASYNC_CALLS_ACTIVE //TODO: not defined anywhere anymore... do we need this ?
 if (pSRB->SRB_Flags & SRB_POSTING){//there are no events in DOS, and Windows shouldn't use async for this cmd...
	if (!createSRBThread ((PFNSRBTHREADSTART)resetDeviceCore, (void*)pSRB))
		pSRB->SRB_Status = SS_ERR;
	else pSRB->SRB_Status = SS_PENDING;
 	return (pSRB->SRB_Status);
 }
 #endif

 //synchronuous call
 pSRB->SRB_Status = resetDeviceCore(pSRB, isWin32Call);
 return (pSRB->SRB_Status);
}

//TODO
// return SS_PENDING, otherwise might give problems with some bad programs
//    ASPI/WNASPI to TCP:         SERVER/WNASPI32							SEPARATE THREAD ("...CORE")
//      SYNC SEND		---->	  CREATE THREAD & PASS SRB			--->    IN THREAD, LAUNCHES A SYNC CMD
//                      <----     SYNC RESPOND: SS_PENDING
//                                ==================================        CMD RETURNS
//    treat extra msgs	<----	  RESPOND: eg.SS_COMP        		<---    send back SRB, and close thread
//      IF event: notify event
//      IF posting: create thread: callback with SRB
//      IF polling: just update original SRB (pointer passed into TCP struct, otherwise it's lost)

// if posting: user callback to be called in separate thread
// polling: 
/*******************************************************************************
* SC_ABORT_SRB
*******************************************************************************/
DWORD abortSRB (PSRB_Abort pSRB, int isWin32Call)
{
 //is this even possible ?  eh_abort_handler ? TODO OPTIONAL SOMEDAY
 // Note that there is a DOS structure, but it is compatible with the WIN32 structure, so we'll leave this like this for the moment
 pSRB->SRB_Status = SS_COMP;
 return (pSRB->SRB_Status);//SS_COMP always has to be returned. The real status will be in the actual SRB.
}

/*******************************************************************************
* SC_EXEC_SCSI_CMD
*******************************************************************************/
void executeSCSICommandCore (PSRB_ExecSCSICmd pSRB, int isWin32Call)//this is the code that is executed in a separate thread
{
 PMSDEV pTmpDev;
 int bStopProcessing = 0;
 BYTE  HaId, Target, Lun, CDBLen;
 BYTE* pCDBByte;
 DWORD BufLen;
 //TODO  pBufPtr;
 BYTE  SenseLen;
 BYTE* pSenseArea;

 //int isAsync = 0;

//#ifndef IS_SERVER_BULD
 //if ((pSRB->SRB_Flags & SRB_EVENT_NOTIFY) || (pSRB->SRB_Flags & SRB_POSTING)) isAsync = 1;
//#endif
 if (isWin32Call){
	HaId     = pSRB->SRB_HaId;
	Target   = pSRB->SRB_Target;
	Lun      = pSRB->SRB_Lun;
	CDBLen   = pSRB->SRB_CDBLen;
	pCDBByte = (BYTE*)&pSRB->CDBByte[0];
	BufLen   = pSRB->SRB_BufLen;
	//TODO pBufPtr  = pSRB->SRB_BufPointer;	
	SenseLen = pSRB->SRB_SenseLen;//TODO: max with SRB_SenseLenSENSE_LEN+2 ?
	pSenseArea = (BYTE*)&pSRB->SenseArea[0]; 
 } else {
	PSRB_ExecSCSICmd_DOS pSRBDOS = (PSRB_ExecSCSICmd_DOS)pSRB;
	HaId     = pSRBDOS->SRB_HaId;
	Target   = pSRBDOS->SRB_Target;
	Lun      = pSRBDOS->SRB_Lun;
	CDBLen   = pSRBDOS->SRB_CDBLen;
	pCDBByte = (BYTE*)&pSRBDOS->CDBAndSenseBytes[0];
	BufLen   = pSRBDOS->SRB_BufLen;
	//TODO pBufPtr  = pSRB->SRB_BufPointer;	
	SenseLen = pSRB->SRB_SenseLen;//TODO: is there a max ?
	pSenseArea = (BYTE*)&pSRBDOS->CDBAndSenseBytes[CDBLen];
 }

 pTmpDev = scsiGetDeviceByMappedHTL (pExposedDevList, HaId, Target, Lun, 1);
 if (!pTmpDev){
	LOGPRINT_CORE(papplogger, "SS_NO_DEVICE: No mapped device for (%u,%u,%u)\r\n", HaId, Target, Lun);
	pSRB->SRB_Status = SS_NO_DEVICE;
	bStopProcessing = 1;
 }
  
 if (!bStopProcessing){
    LOGPRINT_LOW(papplogger, "scsiSend %s: %s (0x%02X)\r\n", 
		pTmpDev->deviceOpenStr, scsiGetCommandDesc(pCDBByte[0]), pCDBByte[0]);
	//logDeviceSRB((PLOGGER)&pTmpDev->logger, (LPSRB)pSRB, 1);

	//TODO pBufPtr
	if (!scsiSend (pTmpDev, pCDBByte, CDBLen, pSRB->SRB_BufPointer, BufLen, SenseLen, pSenseArea)){
		pSRB->SRB_Status = SS_ERR;
		bStopProcessing = 1;
 	} else { //TODO: sense data is not returned
		LOGPRINT_LOW(papplogger, "scsiSend response: "); 
		logprintSRB (papplogger, (LPSRB)pSRB);
		//logDeviceSRB((PLOGGER)&pTmpDev->logger, (LPSRB)pSRB, 0);
	}
 }

 if (!bStopProcessing) pSRB->SRB_Status = SS_COMP;
 
 //TODO ZZZ: if we're in a thread on the server, do a server -> client(async listener) call
}

DWORD executeSCSICommand (PSRB_ExecSCSICmd pSRB, int isWin32Call)
{
 if (!pSRB){
	pSRB->SRB_Status = SS_ERR;
	return (pSRB->SRB_Status);
 } 

 //asynchronuous call
 #ifdef ASYNC_CALLS_ACTIVE //TODO: not defined anywhere anymore... do we need this ?
 if ((pSRB->SRB_Flags & SRB_EVENT_NOTIFY) || (pSRB->SRB_Flags & SRB_POSTING)){
	if (!createSRBThread ((PFNSRBTHREADSTART)executeSCSICommandCore, (void*)pSRB))
  		pSRB->SRB_Status = SS_ERR;
		else pSRB->SRB_Status = SS_PENDING;//the real response will come later via the separate thread
	return (pSRB->SRB_Status);
 }
 #endif

 //synchronuous call
 executeSCSICommandCore (pSRB, isWin32Call);
 return (pSRB->SRB_Status);
}

/*******************************************************************************
* SC_GET_DEV_TYPE
*******************************************************************************/
DWORD getDeviceType (PSRB_GDEVBlock pSRB, int isWin32Call)
{ //Note: DOS version exists, but no need for a different handling here
 PMSDEV pTmpDev;

 pTmpDev = scsiGetDeviceByMappedHTL (pExposedDevList, pSRB->SRB_HaId, pSRB->SRB_Target, pSRB->SRB_Lun, 0);
 if (!pTmpDev){
	pSRB->SRB_Status = SS_NO_DEVICE;
	return (pSRB->SRB_Status);//or SS_INVALID_HA
 } 

 //output
 pSRB->SRB_DeviceType = pTmpDev->type; //as defined by the SCSI specification (eg 0x03 = processor device)
 pSRB->SRB_Status = SS_COMP;
 return (pSRB->SRB_Status);
}

/*******************************************************************************
* SC_HA_INQUIRY
*******************************************************************************/
DWORD hostAdapterInquiry (PSRB_HAInquiry pSRB)
{
 if (pSRB->SRB_HaId > HOST_ADAPTER_COUNT){
	pSRB->SRB_Status = SS_INVALID_HA;
	return (SS_INVALID_HA);
 }

 pSRB->HA_Count   = 1;//Note: MekugiAspi < 1.0 returned 8 (like GetASPI32SupportInfo ? TODO - check this)
 pSRB->HA_SCSI_ID = HOST_ADAPTER_SCSI_ID;
 memset ((void*)&pSRB->HA_Identifier[12], 0x20 , 4);//to pad with spaces
 sprintf ((char*)pSRB->HA_Identifier, "Host Adapter %d", pSRB->SRB_HaId);//TODO: check what to do with trailing 0x00

#ifdef TARGET_DOS   //TODO: probably not correct to define ifdef DOS, since this runs on Linux or Windows...
 //TODO ? SRB_Ext_Req_Sig & SRB_Ext_Buf_Len
 sprintf ((char*)pSRB->HA_ManagerId, "DOS SCSIMGR 1.36");  //TODO: change this into our stuff ? (taken from viewing ASPI8DOS.SYS in a hex editor - probably this string is returned)
 memset ((void*)pSRB->HA_Unique, 0x00 , 16);  
 pSRB->HA_Sup_Ext = 0x0000; /* TODO               Bit 15-4  Reserved
                      Bit 3     0 = Not a Wide SCSI 32-bit host adapter
                                1 = Wide SCSI 32-bit host adapter
                      Bit 2     0 = Not a Wide SCSI 16-bit host adapter
                                1 = Wide SCSI 16-bit host adapter
                      Bit 1     0 = Residual byte length not reported
                                1 = Residual byte length reported. See
                                    section on Residual Byte Length below.
                      Bit 0     Reserved*/
#else
 memcpy ((void*)pSRB->HA_ManagerId, (void*)"ASPI for Win32  ", 16);
 memset ((void*)pSRB->HA_Unique, 0x00 , 16);  //TODO: real implementation
 //pSRB->HA_Unique[0] = buffer alignment mask
 //pSRB->HA_Unique[2] = residual byte count supported
 pSRB->HA_Unique[3] = 8;//the maximum number of targets (SCSI IDs) the adapter supports (if not 8 or 16, the app needs to assume 8)
 //pSRB->HA_Unique[4-7]:Maximum transfer length supported by the HA. if < 64KB, the app needs to assume 64KB
 pSRB->HA_Sup_Ext = 0x0000;
#endif

 pSRB->SRB_Status = SS_COMP;
 return (SS_COMP);
}

/*******************************************************************************
* This function is doing the real SCSI calls to the underlying OS driver 
*	- for a real Windows dll, this will call the SPTI commands,
*	- for a scsi server, this will call the Windows SPTI commands, or the Linux SG commands
* This is not used by DOS, but in case of the scsi server, this might receive pSRB's in DOS format
*******************************************************************************/
DWORD dispatchSRB2SCSI (LPSRB pSRB, int isWin32Call)
{
 DWORD ret = SS_INVALID_CMD;

 if (pSRB) {
	switch (pSRB->SRB_Cmd) {
		case SC_HA_INQUIRY:			//0x00
			ret = hostAdapterInquiry((PSRB_HAInquiry)pSRB);
			break;
		case SC_GET_DEV_TYPE:		//0x01
			ret = getDeviceType((PSRB_GDEVBlock)pSRB, isWin32Call);
			break;
		case SC_EXEC_SCSI_CMD:		//0x02
			ret = executeSCSICommand((PSRB_ExecSCSICmd)pSRB, isWin32Call);
			break;
		case SC_ABORT_SRB:			//0x03
			ret = abortSRB((PSRB_Abort)pSRB, isWin32Call);
			break;
		case SC_RESET_DEV:			//0x04
			ret = resetDevice((PSRB_BusDeviceReset)pSRB, isWin32Call);
			break;
		case SC_SET_HA_PARMS:		//0x05 - only in aspidos, NOT in aspi32 !
			ret = setHostAdapterParameters((PSRB_SetHostAdapterParams_DOS)pSRB);
			break;
		case SC_GET_DISK_INFO:		//0x06
			ret = getDiskInfo((PSRB_GetDiskInfo)pSRB);
			break;
		case SC_RESCAN_SCSI_BUS:	//0x07 - NOT in aspidos - this cmd code was "reserved for future expansion"
			ret = rescanSCSIBus((PSRB_RescanPort)pSRB);
			break;
		case SC_GETSET_TIMEOUTS:	//0x08 - NOT in aspidos - this cmd code was "reserved for future expansion"
			ret = getSetTimeouts((PSRB_GetSetTimeouts)pSRB);
			break;
		default:
			break;
 	}//TODO everywhere: check that SRB_Status is also filled in ? or keep the aspi error separate from the SRB error ? 
 }

 //Note: don't put a raise notification (event) here, since we don't know if this part of the code is running on the server 
 // or not. Put it in the client code (cfr wnaspi32.c or scsiserv.c).
 /*For async calls: TODO
  - the TCP server will return the real info from the individual Threads, via event notification.
  - the standalone client will raise an event/call the callback here at the latest point, right before the wnaspi call finishes*/

 return (ret);
}
