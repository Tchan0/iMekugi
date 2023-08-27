/*******************************************************************************
* Generic SCSI functions
*   
*******************************************************************************/
#include <stdio.h>

#include "scsi.h"

#if defined( __linux__ ) 
	#include "sgi.c" //SCSI generic (sg) driver (Linux)
#else //Windows _WIN32 _WIN64
	#include "spti.c" //SCSI Pass Through Interface via DeviceIoControl
#endif

/*******************************************************************************
* 
*******************************************************************************/
char* scsiGetCodeDesc (char code, SCSI_CODE_DESC* descArray, int max_entries, char* szDefaultDesc)
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
char* scsiGetSenseKeyDesc (char senseKey)
{
 return (scsiGetCodeDesc(senseKey, scsiSenseKeyDesc, SCSI_SENSE_KEY_MAX, "SCSI sense key unknown"));
}

/*******************************************************************************
* 
*******************************************************************************/
char* scsiGetStatusDesc (char status)
{
 return (scsiGetCodeDesc(status, scsiStatusCodeDesc, SCSI_STATUS_CODE_MAX, "SCSI status code unknown"));
}

/*******************************************************************************
* 
*******************************************************************************/
char* scsiGetCommandDesc (char command)
{
 return (scsiGetCodeDesc(command, scsiCommandCodeDesc, SCSI_COMMAND_CODE_MAX, "SCSI command code unknown"));
}

/*******************************************************************************
* 
*******************************************************************************/
void scsiDeviceClose (PMSDEV pDev, int isAppShutdown)
{
 if (scsiIsOpen (pDev)){
	scsiClose (pDev, isAppShutdown);
	loggerClose((PLOGGER)&pDev->logger, 0);
 }
}

/*******************************************************************************
* 
*******************************************************************************/
int scsiDeviceOpen (PMSDEV pDev)
{
 if (scsiIsOpen (pDev)) return 1;

 if (!scsiOpen (pDev)){
	fprintf(stdout, "Unable to open device %s (%d,%d,%d)\r\n",
		pDev->deviceOpenStr, pDev->real_hostId, pDev->real_targetId, pDev->real_lunId);
	return 0;
 } else {
	fprintf(stdout, "device opened: %s (%d,%d,%d)\r\n",
		pDev->deviceOpenStr, pDev->real_hostId, pDev->real_targetId, pDev->real_lunId);
 }

 if (pDev->logger.logLevel)
 	loggerFileOpen ((PLOGGER)&pDev->logger);
 
 return 1;
}

/*******************************************************************************
* 
*******************************************************************************/
PMSDEVLIST scsiDeviceMapAlloc (uint32_t num_devices)
{
 PMSDEVLIST pList;

 if (!num_devices) return NULL;

 pList = (PMSDEVLIST)calloc (1, sizeof(MAPPEDSCSIDEVICELIST) + ((num_devices - 1) * sizeof(MAPPEDSCSIDEVICE)));
 if (!pList) return NULL;

 pList->num_devices_allocated = num_devices;
}

/*******************************************************************************
* 
*******************************************************************************/
int scsiDeviceMapAdd (PMSDEVLIST pDevList, PMSDEV pDev)
{
 uint32_t i;

 if (!pDevList || !pDev) return (0);

 if (pDevList->num_devices >= pDevList->num_devices_allocated) return (0);
 
 i = pDevList->num_devices;
 memcpy ((void*)pDevList->dev[i].vendor,   (void*)pDev->vendor, 9);
 memcpy ((void*)pDevList->dev[i].model,    (void*)pDev->model, 17);
 memcpy ((void*)pDevList->dev[i].revision, (void*)pDev->revision, 5);
 pDevList->dev[i].type				= pDev->type;

 pDevList->dev[i].hostId			= pDev->hostId;
 pDevList->dev[i].targetId			= pDev->targetId;
 pDevList->dev[i].lunId				= pDev->lunId;
 pDevList->dev[i].timeout           = pDev->timeout;
 pDevList->dev[i].async_delay       = pDev->async_delay;

 pDevList->dev[i].real_hostId		= pDev->real_hostId;
 pDevList->dev[i].real_channelId	= pDev->real_channelId;
 pDevList->dev[i].real_targetId		= pDev->real_targetId;
 pDevList->dev[i].real_lunId		= pDev->real_lunId;

#if defined( __linux__ ) 
  pDevList->dev[i].fd              = pDev->fd;
#else
 pDevList->dev[i].hasDriveLetter    = pDev->hasDriveLetter;
 pDevList->dev[i].hand              = pDev->hand;
#endif

 memcpy ((void*)pDevList->dev[i].deviceOpenStr, (void*)pDev->deviceOpenStr, 1024);

 pDevList->dev[i].logger.logLevel   = pDev->logger.logLevel;
 pDevList->dev[i].logger.type		= pDev->logger.type;
 pDevList->dev[i].logger.logFileSizeMax = pDev->logger.logFileSizeMax;
 memcpy ((void*)pDevList->dev[i].logger.logfilename, (void*)pDev->logger.logfilename, 1024);
 
 pDevList->num_devices++;

 return (1);
}

/*******************************************************************************
* copy over all found devices to an allocated structure that the client app can use & free after usage 
*******************************************************************************/
PMSDEVLIST copyMappedSCSIDeviceToList (PMSDEV pDev, int numDevices)
{
 PMSDEVLIST pDevList = (PMSDEVLIST)NULL;

 //Allocate the list
 numDevices = numDevices > 128 ? 128 : numDevices;//extra security... probably not needed
 pDevList = scsiDeviceMapAlloc(numDevices);
 if (!pDevList) return ((PMSDEVLIST)NULL);

 for (int j = 0; j < numDevices; j++){
	if (!scsiDeviceMapAdd (pDevList, pDev)) break;
	pDev++;
 }

 return (pDevList);
}

/*******************************************************************************
* 
*******************************************************************************/
int scsiGetNextDevice (PMSDEVLIST pDevList, int* pId, PMSDEV* ppNextDev)
{
 if (!pDevList || !ppNextDev || !pId) return (0);

 *pId = *pId + 1;
 if (*pId + 1 > pDevList->num_devices) return (0);

 *ppNextDev = &pDevList->dev[*pId];

 return 1;
}

/*******************************************************************************
* 
*******************************************************************************/
PMSDEV scsiGetDeviceByVendorAndModel (PMSDEVLIST pDevList, PMSDEV pDev)
{
 int i;
 PMSDEV pTmpDev = NULL;

 i = -1;
 while (scsiGetNextDevice(pDevList, &i, &pTmpDev)){
	if (!strcmp((char*)pTmpDev->vendor, (char*)pDev->vendor)){
		if ((pDev->model[0] == '*') || (!strcmp((char*)pTmpDev->model, (char*)pDev->model)))
			return (pTmpDev);
	}
 }

 return (NULL);
}

/*******************************************************************************
* 
*******************************************************************************/
PMSDEV scsiGetDeviceByVendorAndModelStr (PMSDEVLIST pDevList, char* vendor, char* model)
{
 int i;
 PMSDEV pTmpDev = NULL;

 i = -1;
 while (scsiGetNextDevice(pDevList, &i, &pTmpDev)){
	if (!strcmp((char*)pTmpDev->vendor, (char*)vendor)){
		if ((model[0] == '*') || (!strcmp((char*)pTmpDev->model, (char*)model)))
			return (pTmpDev);
	}
 }

 return (NULL);
}

/*******************************************************************************
*
*******************************************************************************/
PMSDEV scsiGetDeviceByMappedHTL (PMSDEVLIST pDevList, uint32_t hostId, uint32_t targetId, uint32_t lunId, int bOpenDevice)
{
 int i;
 PMSDEV pTmpDev = NULL;

 i = -1;
 while (scsiGetNextDevice(pDevList, &i, &pTmpDev)){
	if ((pTmpDev->hostId 	== hostId) &&
		(pTmpDev->targetId 	== targetId) &&
		(pTmpDev->lunId		== lunId)){
			if (bOpenDevice && !scsiDeviceOpen(pTmpDev)) break;
			return (pTmpDev);
		}
 }

 return (NULL);
}
