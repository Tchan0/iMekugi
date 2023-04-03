/*******************************************************************************
* SCSI Pass Through Interface (spti) functions - Windows specific
*
* https://en.wikipedia.org/wiki/SCSI_Pass_Through_Interface
* https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddscsi/ni-ntddscsi-ioctl_scsi_pass_through
* https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddscsi/ni-ntddscsi-ioctl_scsi_pass_through_direct
* IOCTL_SCSI_PASS_THROUGH 
* IOCTL_SCSI_PASS_THROUGH_DIRECT
*
* Note that these functions need to be run as administrator
*******************************************************************************/
#include <ntddscsi.h> //for IOCTL_SCSI_PASS_THROUGH_DIRECT

#include "spti.h"

#define SPTI_MAX_HOSTADAPTERS 32
HANDLE	hostAdapterHandles[SPTI_MAX_HOSTADAPTERS]={0};/*with spti, we open host adapters via CreateFile(), not individual targets*/

/*******************************************************************************
*
*******************************************************************************/
HANDLE sptiGetHostAdapterHandle (PMSDEV pDev)
{
 if (!pDev || (pDev->real_hostId >= SPTI_MAX_HOSTADAPTERS)) return ((HANDLE)NULL);

 return (hostAdapterHandles[pDev->real_hostId]);
}

/*******************************************************************************
*
*******************************************************************************/
HANDLE sptiGetDeviceHandle (PMSDEV pDev)
{
 if (!pDev) return ((HANDLE)NULL);

 if (pDev->hasDriveLetter){//harddisks & co need to be accessed via their driveletter, not via their host adapter
	return (pDev->hand);
 }
 
 return sptiGetHostAdapterHandle (pDev);
}


/*******************************************************************************
*
*******************************************************************************/
void sptiFillDriveLetterInDeviceList (char driveletter, PMSDEVLIST pDevList)
{
 HANDLE hand;
 BOOL  status;
 DWORD dw, bytesReturned;
 SCSI_ADDRESS scsiaddr;
 char drivePath[32];

 wsprintf ((LPTSTR)drivePath, (LPTSTR)"\\\\.\\%c:", driveletter);

 hand = CreateFile ((LPTSTR)drivePath,
 					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE, 
					NULL, OPEN_EXISTING, 0 , NULL);
 if (hand == INVALID_HANDLE_VALUE){
	//dw = GetLastError();
	//fprintf (stderr, "(SPTI) CreateFile() for IOCTL_SCSI_GET_ADDRESS: %s: error: (0x%04x)\r\n", drivePath, dw);
	return;
 } 

 status = DeviceIoControl (hand, IOCTL_SCSI_GET_ADDRESS,
		NULL, 0, &scsiaddr, sizeof(scsiaddr), &bytesReturned, NULL );
 if (!status){
	//dw = GetLastError();
	//fprintf (stderr, "(SPTI) IOCTL_SCSI_GET_ADDRESS(): %s: error: (0x%04x)\r\n", drivePath, dw);
 } else {
	//fprintf (stdout, "(SPTI) IOCTL_SCSI_GET_ADDRESS(): %s: (%d,%d,%d,%d)\r\n", drivePath,
	//	);
	PMSDEV pDev;
	pDev = scsiGetDeviceByMappedHTL(pDevList, scsiaddr.PortNumber, scsiaddr.TargetId, scsiaddr.Lun, 0);
	if (pDev){
		fprintf (stdout, "(SPTI) found driveletter: %s for (%d,%d,%d,%d)\r\n", drivePath,
			scsiaddr.PortNumber, scsiaddr.PathId, scsiaddr.TargetId, scsiaddr.Lun);
		//TODO: modify pDev with driveOpenStr && a flag
		wsprintf ((LPTSTR)pDev->deviceOpenStr, (LPTSTR)"%s", drivePath);
		pDev->hasDriveLetter = 1;
	}
 }
 CloseHandle (hand);
}

/*******************************************************************************
*
*******************************************************************************/
void sptiGetCorrespondingDriveLetters (PMSDEVLIST pDevList)
{
 for (char driveletter='C'; driveletter <= 'Z'; driveletter++){
	sptiFillDriveLetterInDeviceList (driveletter, pDevList);
 }
}

/*******************************************************************************
*
*******************************************************************************/
int sptiFindSCSIDevices (PMSDEV pDev, int maxNumDevices)
{
 char szDeviceName[64];
 int devices_found = 0;
 BOOL  status;
 DWORD bytesReturned;
 uint8_t buf[4096];
 PSCSI_ADAPTER_BUS_INFO pbusInfo;
 MAPPEDSCSIDEVICE tempDevice;
 HANDLE hand;
 //IO_SCSI_CAPABILITIES ioscsicaps;
 //TODO: use maxNumDevices

 //Try to open the first 8 Host Adapters
 for (int haId=0; haId <= 7; haId++){
	tempDevice.timeout = 20000;

	wsprintf ((LPTSTR)szDeviceName, (LPTSTR)"\\\\.\\Scsi%u:", haId);
	wsprintf ((LPTSTR)tempDevice.deviceOpenStr, (LPTSTR)"\\\\.\\Scsi%u:", haId);
	tempDevice.real_hostId = haId;
	if (!scsiOpen (&tempDevice)) continue; //no host adapter here -> go to the next one
	hand = sptiGetHostAdapterHandle(&tempDevice);
	//fprintf(stdout, "SPTI: Host adapter Scsi%u opened successfully:\n", HaId);

	//get the capabilities / limitations of the SCSI HBA.
	//IOCTL_SCSI_PASS_THROUGH and IOCTL_SCSI_PASS_THROUGH_DIRECT are required to honor these limitations (MaximumTransferLength and AlignmentMask)
	/*
	status = DeviceIoControl (hand, IOCTL_SCSI_GET_CAPABILITIES,
				(LPVOID)NULL, 0, (LPVOID)&ioscsicaps, sizeof(ioscsicaps), (LPDWORD)&bytesReturned, NULL);
	if (!status){
		DWORD dw = GetLastError();
		fprintf (stderr, "(SPTI) IOCTL_SCSI_GET_CAPABILITIES: Ha %d, error: (0x%04x)\r\n", haId, dw);
	} else {
		fprintf (stdout, "(SPTI) IOCTL_SCSI_GET_CAPABILITIES: Ha %d: Alignment: %u, MaxLength: %u, MaxPages: %u\r\n",
			haId, ioscsicaps.AlignmentMask, ioscsicaps.MaximumTransferLength, ioscsicaps.MaximumPhysicalPages);
	}*/

	pbusInfo = (PSCSI_ADAPTER_BUS_INFO)&buf[0];
	status = DeviceIoControl (hand, IOCTL_SCSI_GET_INQUIRY_DATA,
				(LPVOID)NULL, 0, (LPVOID)pbusInfo, 4096, (LPDWORD)&bytesReturned, NULL);
	if (!status){
		DWORD dw = GetLastError();
		fprintf (stderr, "(SPTI) IOCTL_SCSI_GET_INQUIRY_DATA: Ha %d, error: (0x%04x)\r\n", haId, dw);
	} else {
		//Early SCSI buses were limited to 36 targets (rather than the current limit of 128), so some vendors manufactured HBAs with several buses, in order to increase the maximum number of targets
		for (UCHAR i=0; i < pbusInfo->NumberOfBuses; i++){
			PSCSI_BUS_DATA pBusData = (PSCSI_BUS_DATA)&pbusInfo->BusData[i];
			ULONG inquiryDataOffset = pBusData->InquiryDataOffset;
	    	while (inquiryDataOffset) {
				PSCSI_INQUIRY_DATA pInquiryData;
       			pInquiryData = (PSCSI_INQUIRY_DATA) (((uint8_t*)pbusInfo) + inquiryDataOffset);

				//a device was found, retrieve all info
				strncpy ((char*)pDev->vendor,  (char*)&pInquiryData->InquiryData[8],   8);
				strncpy ((char*)pDev->model,   (char*)&pInquiryData->InquiryData[16], 16);
				strncpy ((char*)pDev->revision,(char*)&pInquiryData->InquiryData[32],  4);
				pDev->type           = pInquiryData->InquiryData[0] & 0x1F;//bits 4-0
				pDev->real_hostId    = haId;
				pDev->hostId         = haId;
				pDev->real_channelId = i;//channel = bus
				//pDev->channelId    = i;//channel = bus
				pDev->real_targetId  = pInquiryData->TargetId;
				pDev->targetId       = pInquiryData->TargetId;
				pDev->real_lunId     = pInquiryData->Lun;
				pDev->lunId          = pInquiryData->Lun;
				pDev->timeout        = 20000; //in millisecs

				wsprintf ((LPTSTR)&pDev->deviceOpenStr[0], (LPTSTR)"\\\\.\\Scsi%u:", haId);
				fprintf(stdout, "Real device detected (%d,%d,%d,%d): %.8s, %.16s, %.4s\r\n",
					haId, i, pInquiryData->TargetId, pInquiryData->Lun,
					&pInquiryData->InquiryData[8],	&pInquiryData->InquiryData[16],	&pInquiryData->InquiryData[32]);
				devices_found++;
				pDev++;

				inquiryDataOffset = pInquiryData->NextInquiryDataOffset;
			}
		}


	}

	scsiClose (&tempDevice, 1);//close the adapter
 }	

 return (devices_found);
}

/*******************************************************************************
* 
*******************************************************************************/
PMSDEVLIST scsiGetDeviceList (void)
{
 MAPPEDSCSIDEVICE devs[128];
 PMSDEVLIST pDevList;
 int devices_found = 0;

 //get the list of all scsi devices via SPTI
 devices_found = sptiFindSCSIDevices((PMSDEV)&devs[0], 128);

 //copy over all found devices to an allocated structure that the client app can use & free after usage
 pDevList = copyMappedSCSIDeviceToList ((PMSDEV)&devs[0], devices_found);

 //correction: harddisks cannot be accessed via the host adapter, but need an openstring like "\\.\C:"
 sptiGetCorrespondingDriveLetters(pDevList);

 return (pDevList);
}

/*******************************************************************************
* get time-out value
*******************************************************************************/
int scsiGetTimeouts (PMSDEV pDev, uint32_t* pseconds)
{
 if (!pDev || !pseconds) return (0);

 *pseconds = pDev->timeout / 1000;//no real action needed, since the timeout is passed with every deviceio command

 return 1;
}

/*******************************************************************************
* set time-out value
*******************************************************************************/
int scsiSetTimeouts (PMSDEV pDev, uint32_t seconds)
{
 if (!pDev) return 0;

 pDev->timeout = seconds * 1000;//no real action needed, since the timeout is passed with every deviceio command

 return 1;
}

/*******************************************************************************
* 
*******************************************************************************/
int scsiRescan (uint32_t HaId)
{
 return (0);//TODO: real action
}

/*******************************************************************************
* 
*******************************************************************************/
int scsiResetTarget (PMSDEV pDev)
{
 return (0);//TODO: real action // a scsi miniport driver must have a HwScsiResetBus routine, so we have to find a way to call this....
}

/*******************************************************************************
* todo: align parameters with sg function
*******************************************************************************/
int scsiSend (PMSDEV pDev, unsigned char* pCDB, unsigned char cdbLength, void* pResultBuf, unsigned int resultBufLength, uint8_t senseLen, uint8_t* pSenseArea)
{
 PSCSI_PASS_THROUGH_DIRECT psptd;
 DWORD bufferSize;
 BOOL  status;
 DWORD bytesReturned;
 int   i, ret;
 unsigned char* pSense_buffer;
 HANDLE hand;

 hand = sptiGetDeviceHandle(pDev);//SPTI opens the Host Adapters, not targets...
 if (!hand) return 0;

 //allocate memory big enough to contain the sptd structure + extra data
 bufferSize = sizeof(SCSI_PASS_THROUGH_DIRECT) + senseLen;
 psptd = (PSCSI_PASS_THROUGH_DIRECT)GlobalAlloc (GPTR, bufferSize);//GPTR Combines GMEM_FIXED and GMEM_ZEROINIT
 if (!psptd) return (0);

 //Copy the scsi command to the sptd structure
 psptd->Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
 //(psptd->ScsiStatus is an output value)
 psptd->PathId       = pDev->real_channelId;
 psptd->TargetId     = pDev->real_targetId;
 psptd->Lun          = pDev->real_lunId;
 psptd->TimeOutValue = (ULONG)(pDev->timeout/1000);
 //CDB
 cdbLength = cdbLength > 16 ? 16 : cdbLength;//shouldn't happen, but you never know... psptd has a fixed CDB of 16
 psptd->CdbLength = cdbLength;
 for (i=0; i < cdbLength; i++) psptd->Cdb[i] = pCDB[i];
 //Space for Sense Info should be at the end of the structure
 psptd->SenseInfoLength = senseLen;
 psptd->SenseInfoOffset = (ULONG)sizeof(SCSI_PASS_THROUGH_DIRECT);
 pSense_buffer = ((unsigned char*)psptd) + (unsigned char)psptd->SenseInfoOffset;
 //extra data to be exchanged
 psptd->DataIn = (pCDB[0] == SCSI_CMD_WRITE_BUFFER) ? SCSI_IOCTL_DATA_OUT : SCSI_IOCTL_DATA_IN;//TODO TEMP HACK -> make generic srb function ? also used in sg.c
 psptd->DataBuffer         = (PVOID)pResultBuf;/*TODO ? not a PVOID with w64 ?*/
 psptd->DataTransferLength = (ULONG)resultBufLength; /*TODO:  Many devices transfer chunks of data of predefined length. The value in DataTransferLength must be an integral multiple of this predefined, minimum length that is specified by the device.*/
									/*see https://stackoverflow.com/questions/30970541/windows-scsi-readcapacity16-in-d*/
 
 /*LOGPRINT_HIGH((PLOGGER)&pDev->logger, "(SPTI) hand: %d, psptd: %p, bufferSize: %d \r\n", hand, psptd, bufferSize);
 LOGPRINT_HIGH((PLOGGER)&pDev->logger, "(SPTI) length: %d, (%d,%d,%d), Timeout: %lu\r\n",
	psptd->Length, psptd->PathId, psptd->TargetId, psptd->Lun, psptd->TimeOutValue);
 LOGPRINT_HIGH((PLOGGER)&pDev->logger, "(SPTI) CDB: %d\r\n",	psptd->CdbLength);
 //for (i=0; i < cdbLength; i++) psptd->Cdb[i] = pCDB[i];
 LOGPRINT_HIGH((PLOGGER)&pDev->logger, "(SPTI) Sense: length: %d, offset: %d, buf: %p\r\n",	
 	psptd->SenseInfoLength, psptd->SenseInfoOffset, pSense_buffer );
 LOGPRINT_HIGH((PLOGGER)&pDev->logger, "(SPTI) Data: length: %d, IN_OUT: %d,buf: %p\r\n",	
 	psptd->DataTransferLength, psptd->DataIn, psptd->DataBuffer );*/

 status = DeviceIoControl (hand, IOCTL_SCSI_PASS_THROUGH_DIRECT,
                           (LPVOID)psptd, bufferSize, //input
                           (LPVOID)psptd, bufferSize, //output
                           (LPDWORD)&bytesReturned, NULL);
 if (!status){
	DWORD dw = GetLastError();
	//TODO: remove this line ? better: base yourself on pDev to turn loggin on/off
	LOGPRINT_CORE((PLOGGER)&pDev->logger, "(SPTI) %s (0x%02x) to (%d,%d,%d,%d): DeviceIoControl() error: (0x%04x)\r\n", 
	 	scsiGetCommandDesc(pCDB[0]), (unsigned char)pCDB[0],
		pDev->real_hostId, psptd->PathId, psptd->TargetId, psptd->Lun, dw);
	ret = 0;
 } else {//copy over returned values to source buffers
	ret = psptd->ScsiStatus == SCSI_STATUSCODE_GOOD ? 1 : 0;

	LOGPRINT_HIGH((PLOGGER)&pDev->logger, "(SPTI) command %s (0x%02x): %s\r\n",
 		scsiGetCommandDesc(pCDB[0]), (unsigned char)pCDB[0], scsiGetStatusDesc(psptd->ScsiStatus));

	if (psptd->ScsiStatus != SCSI_STATUSCODE_GOOD){
		//probably already ok: pscsicmd->buf_len = psptd->DataTransferLength;
		//TODO for (i=0; i<pscsicmd->SenseLen; i++) pscsicmd->SenseArea[0] = *(UINT8*)(((UINT8*)psptd) + psptd->SenseInfoOffset);//copy back sense data
		//SENSE DATA AVAILABLE  //TODO: return SENSE DATA TO CALLER
		if (psptd->SenseInfoLength){
        	LOGPRINT_CORE((PLOGGER)&pDev->logger, "\t(SPTI) SENSE data: %s", scsiGetSenseKeyDesc ((pSense_buffer[2]) & 0x0F));
        	for (int k = 0; k < psptd->SenseInfoLength; k++) {
            	if (0 == (k % 10)) LOGPRINT_CORE((PLOGGER)&pDev->logger, "\r\n\t");
            	LOGPRINT_CORE((PLOGGER)&pDev->logger, "0x%02x ", pSense_buffer[k]);
				//copy over sense data to the requestor
				if (k < senseLen) pSenseArea[k] = pSense_buffer[k];
        	}
        	LOGPRINT_CORE((PLOGGER)&pDev->logger, "\r\n");	
		}
	}
	for (i=0; i < cdbLength; i++) pCDB[i] = psptd->Cdb[i];
 }

 GlobalFree ((HGLOBAL)psptd);
 return (ret);
}

/*******************************************************************************
* 
*******************************************************************************/
void scsiClose (PMSDEV pDev, int isAppShutdown)
{
 HANDLE hand;
 
 hand = sptiGetDeviceHandle(pDev);
 if (!hand) return;

 if (isAppShutdown){
	CloseHandle (hand);
	hostAdapterHandles[pDev->real_hostId] = NULL;
 }//TODO maybe someday: closing when not doing a full app shutdown...
 //    in that case, we might leave the HA open for other target devices
}

/*******************************************************************************
* 
*******************************************************************************/
int scsiIsOpen (PMSDEV pDev)
{
 HANDLE hand;
 
 hand = sptiGetDeviceHandle(pDev);
 
 return (hand ? 1 : 0);
}

/*******************************************************************************
* 
*******************************************************************************/
int scsiOpen (PMSDEV pDev)
{
 HANDLE hand;
 DWORD  dwAccess;
 DWORD  dwFlagsAndAttributes;
 
 if (!pDev || (pDev->real_hostId >= SPTI_MAX_HOSTADAPTERS)) return 0;
 hand = sptiGetDeviceHandle(pDev);//SPTI opens the Host Adapters, not targets...so it might already been open
 if (hand) return 1;

 dwAccess             = GENERIC_READ | GENERIC_WRITE;
 dwFlagsAndAttributes = 0; //old standalone mekugiaspi was 0, but check if we need FILE_ATTRIBUTE_NORMAL ? TODO
 
 hand = CreateFile ((LPTSTR)pDev->deviceOpenStr,
                        dwAccess,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, //FILE_SHARE_WRITE needed for a drive
                        NULL,   
                        OPEN_EXISTING, //always needed for a drive
                        dwFlagsAndAttributes,
                        NULL);

 if (pDev->hasDriveLetter){
	pDev->hand = (hand == INVALID_HANDLE_VALUE ? NULL : hand);
 } else {
	hostAdapterHandles[pDev->real_hostId] = (hand == INVALID_HANDLE_VALUE ? NULL : hand);
 }
 
 return (hand == INVALID_HANDLE_VALUE ? 0 : 1);
}