/*******************************************************************************
* Main file for iMekugi's SCSI server that accepts calls from the TCP wnaspi32.dll
*
*******************************************************************************/
#ifdef WIN32  
 #define _WINSOCKAPI_ // stops windows.h from including winsock.h, since we will include winsock2.h later via the TCP code if needed...
 #include <windows.h>
 #undef _WINSOCKAPI_  //to avoid warning later on  that winsock2.h should be included before windows.h
#endif

#include "socket/TCPServer.h"

//Logging
#include "utils/print.c"  //printf utilities (color, hex dump, ...)
#include "utils/logger.c"
#define LOGFILENAME "scsiserv.log"
PLOGGER papplogger = (PLOGGER)NULL;
int loglevel = LOGLEVEL_CORE;
int logmaxsizeMB = 50;

//#define ASYNC_CALLS_ACTIVE //TEMP - uncomment this to have async scsi calls

#define IS_SERVER_BUILD

#define DEFAULTSYNCPORTNR 7032

#include "srb/srb.c"
#include "srb/srb2tcp.c"
#include "srb/srb2scsi.c"

/*******************************************************************************
 *
 *******************************************************************************/
void closeApp(void)
{
 int id;
 PMSDEV pDev;

 if (pDevList) free(pDevList);

 if (pExposedDevList){
	id = -1;
	while (scsiGetNextDevice(pExposedDevList, &id, &pDev))
		scsiDeviceClose (pDev, 1);
	free(pExposedDevList);
	pExposedDevList = NULL;
 }

 loggerClose(papplogger, 1);
}

/*******************************************************************************
 *
 *******************************************************************************/
int dispatchTCPSRB(HTCPSERVER hServer, char *pSrc, int srcSize, char *pDst, int dstSize)
{ // we only receive valid & complete SRB packets here... only structurally speaking of course
 PTCPSRB pTCPSRB = (PTCPSRB)pSrc;
 LPSRB   pSRB    = (LPSRB)&pTCPSRB->data[0];
 int isWin32Call;

 isWin32Call = (pTCPSRB->pointerSize < 4 ? 0 : 1); 

 if (pSRB->SRB_Cmd == SC_EXEC_SCSI_CMD){
	PSRB_ExecSCSICmd pExecSCSI = (PSRB_ExecSCSICmd)pSRB;
	//This SRB has 2 pointers, so we might need to move everything behind those pointers to accomodate
	// for differences between the client (normally 32bit) & server arch (32 or 64 bit)
	uint32_t shift_offset1 = (uint32_t)sizeof(LPBYTE) - pTCPSRB->pointerSize;//SRB_BufPointer
	uint32_t shift_offset2 = (uint32_t)sizeof(LPVOID) - pTCPSRB->pointerSize;//SRB_PostProc
	if (shift_offset1){//or shift_offset2, doesn't matter
		PSRB_ExecSCSICmd pShifted1 = (PSRB_ExecSCSICmd)((uint8_t*)pExecSCSI - shift_offset1);
		PSRB_ExecSCSICmd pShifted2 = (PSRB_ExecSCSICmd)((uint8_t*)pExecSCSI - shift_offset1 - shift_offset2);

		// move the extra data to the new end of the structure
		uint8_t* pOriginalEnd = (uint8_t*)&pShifted2->SenseArea[SENSE_LEN+2];
		uint8_t* pNewEnd      = (uint8_t*)&pExecSCSI->SenseArea[SENSE_LEN+2];
		memmove ((void*)pNewEnd, (void*)pOriginalEnd, pExecSCSI->SRB_BufLen);

		//shift everything beyond SRB_PostProc
		memmove((void*)&pExecSCSI->SRB_Rsvd2[0],(void*)&pShifted2->SRB_Rsvd2[0],20+16+SENSE_LEN+2);
		//shift everything beyond SRB_BufPointer but before SRB_PostProc
		pExecSCSI->SRB_TargStat = pShifted1->SRB_TargStat;
		pExecSCSI->SRB_HaStat   = pShifted1->SRB_HaStat;
		pExecSCSI->SRB_CDBLen   = pShifted1->SRB_CDBLen;
		pExecSCSI->SRB_SenseLen = pShifted1->SRB_SenseLen;

		pTCPSRB->pointerSize = (uint32_t)sizeof(LPVOID);
	}
 
	//also, we need to make sure we point at the end of the struct, where we moved the buffer data,
	// so that it would be 1 block to send via TCP
	pExecSCSI->SRB_BufPointer = (LPBYTE)pExecSCSI + (int)sizeof(SRB_ExecSCSICmd);

	LOGPRINT_LOW(papplogger, "\r\nCLIENT called: %s-%s for (%u,%u,%u)\r\n", srbGetCommandDesc(pSRB->SRB_Cmd), 
		scsiGetCommandDesc (pExecSCSI->CDBByte[0]), pSRB->SRB_HaId, pExecSCSI->SRB_Target, pExecSCSI->SRB_Lun);
	logprintSRB (papplogger, pSRB);
 } else {
	switch (pSRB->SRB_Cmd){
		case SC_ABORT_SRB:
			LOGPRINT_LOW(papplogger, "\r\nCLIENT called: %s\r\n", srbGetCommandDesc(pSRB->SRB_Cmd));
			break;
		case SC_HA_INQUIRY:
		case SC_SET_HA_PARMS:
		case SC_RESCAN_SCSI_BUS:
			LOGPRINT_LOW(papplogger, "\r\nCLIENT called: %s for Ha: %u\r\n", srbGetCommandDesc(pSRB->SRB_Cmd), pSRB->SRB_HaId);
			break;
		case SC_GET_DEV_TYPE:
		case SC_RESET_DEV:
		case SC_GET_DISK_INFO:
		case SC_GETSET_TIMEOUTS:
			{
				PSRB_GDEVBlock ptr = (PSRB_GDEVBlock)pSRB;//hack to get the SRB_Target and SRB_Lun
				LOGPRINT_LOW(papplogger, "\r\nCLIENT called: %s for (%u,%u,%u)\r\n", srbGetCommandDesc(pSRB->SRB_Cmd), pSRB->SRB_HaId, ptr->SRB_Target, ptr->SRB_Lun);
			}
			break;
 	}
 }

 pTCPSRB->ret = dispatchSRB2SCSI(pSRB, isWin32Call);

 if (dstSize < pTCPSRB->size) pTCPSRB->size = dstSize;
 memcpy((void *)pDst, (void *)pTCPSRB, pTCPSRB->size);
 // verify if ok for ASYNC calls

  return (pTCPSRB->size); // tell the server how many bytes we need to send back
}

/*******************************************************************************
 *
 *******************************************************************************/
void myAcceptCallback(HTCPSERVER hServer)
{
 fprintf(stdout, "Accept OK\n");
 TCPServerRecvSendLoop(hServer,
	(PFNISCOMPLETEPACKET)isCompleteTCPSRB, (PFNSERVERDATA)dispatchTCPSRB, (PFNSERVER)NULL);
}

#define CONFIGFILENAME "scsiserv.cfg"
/*******************************************************************************
 *
 *******************************************************************************/
int createConfigFile(void)
{
 FILE *fout;
 int id;
 PMSDEV pDev;

 fout = fopen(CONFIGFILENAME, "wb");
 if (!fout)	return 0;

 fprintf(fout, "# SCSI Devices that will be exposed via TCP\r\n");
 fprintf(fout, "#\r\n");
 fprintf(fout, "# ! DELETE the lines for the devices you don't want to expose via TCP !\r\n");
 fprintf(fout, "# ! MODIFY the Mapped (HA id,TA id,LUN id) - they are all defaulted to (0,0,0) !\r\n");
 fprintf(fout, "#\r\n");
 fprintf(fout, "# Vendor, Model, Real (HA id,TA id,LUN id), Mapped (HA id,TA id,LUN id), enable_logging, logfilename, device logger type, max size logfile in MB\r\n");
 fprintf(fout, "#\r\n");
 fprintf(fout, "# If the Real ids contain a *, the device will be exposed as long as\r\n");
 fprintf(fout, "#   Vendor & Model match, regardless of its real SCSI address\r\n");
 fprintf(fout, "#\r\n");
 fprintf(fout, "# enable_logging: 1=core, 2=low, 3=full detail\r\n");
 fprintf(fout, "# device logger type: 1 (KATANA_DA), 2 (KATANA_GDM), ... (cfr device.h)\r\n");
 fprintf(fout, "#\r\n");
 fprintf(fout, "# Example 1: If this device is physically connected to (1,2,3), it will be exposed via TCP as (4,5,6):\r\n");
 fprintf(fout, "#  CPL,KATANA DA,1,2,3,4,5,6,0,katanada.log,1,50\r\n");
 fprintf(fout, "# Example 2: Regardless of which id this device is physically connected to, it will be exposed via TCP as (4,5,6):\r\n");
 fprintf(fout, "#  CPL,KATANA DA,*,*,*,4,5,6,0,katanada.log,1,50\r\n");
 fprintf(fout, "#\r\n");
 fprintf(fout, "#SEGA OA ,Saturn CartDev  ,*,*,*,0,6,0,0,saturncartdev.log,7,50\r\n");// Saturn CartDev Rev.B
 fprintf(fout, "#CROSPROD,MIRAGE EMULATOR ,*,*,*,0,3,0,0,saturnmirage.log,8,50\r\n"); // Saturn Cross Products Mirage
 fprintf(fout, "#CPL     ,KATANA DA       ,*,*,*,0,3,0,0,katanada.log,1,50\r\n");     // HKT-01 Dreamcast Dev.Box (Set4/5) - Debug Adapter
 fprintf(fout, "#CPL     ,GD-M            ,*,*,*,0,4,0,0,katanagdm.log,2,50\r\n");    // HKT-01 Dreamcast Dev.Box (Set4/5) - GD-M
 fprintf(fout, "#KATANA  ,DEVELOP-BOARD   ,*,*,*,0,5,0,0,katanasbx.log,3,50\r\n");    // HKT-03 Sound Box (only scsi id 4 or 5 selectable on real hardware)
 fprintf(fout, "#SEGA    ,GD-R1997        ,*,*,*,0,5,0,0,katanagdr.log,5,50\r\n");    // HKT-04 Katana GD-Writer
 fprintf(fout, "#CPL     ,GD-X            ,*,*,*,0,1,0,0,segagdx.log,6,50\r\n");      // HKT-05 GD-X GD-ROM Duplicator
 fprintf(fout, "#SEGA    ,DEVCAST NAZUNA  ,*,*,*,0,2,0,0,katanadevcas.log,4,50\r\n"); // HKT-11 Dev.Cas ? (Set7)
 fprintf(fout, "#\r\n");

 if (!pDevList)	pDevList = scsiGetDeviceList();
 id = -1;
 while (scsiGetNextDevice(pDevList, &id, &pDev))
	fprintf(fout, "%s,%s,%d,%d,%d,%d,%d,%d,0,logger%03d.log,0,50\r\n", pDev->vendor, pDev->model,
		pDev->real_hostId, pDev->real_targetId, pDev->real_lunId,
		pDev->hostId, pDev->targetId, pDev->lunId, id);

 fclose(fout);
 return 1;
}

#define NO_VALUE 4684697
/*******************************************************************************
 *
 *******************************************************************************/
int loadConfigFile(int port)
{
 FILE *fin;
 MAPPEDSCSIDEVICE dev;
 PMSDEV pTmpDev;
 char buf[1024];
 char real_hostIdStr[32];
 char real_targetIdStr[32];
 char real_lunIdStr[32];
 uint32_t real_hostId, real_targetId, real_lunId;
 int i, id;
 int numDevicesMapped = 0;

 fin = fopen(CONFIGFILENAME, "r");
 if (!fin) return 0;

 pDevList = scsiGetDeviceList(); // gets the devices that are physically present
 if (!pDevList || (pDevList->num_devices == 0))	{
	fclose(fin);
	return -1;
 }

 // we'll store the physically present && exposed devices in this new list
 pExposedDevList = scsiDeviceMapAlloc(pDevList->num_devices);

 // Try to map the devices that are physically present, with the ones we are allowed to expose according to the cfg file
 while (fgets((char *)buf, 1024, fin)) {
	if (buf[0] == '#') continue;
	dev.vendor[0] = dev.model[0] = dev.revision[0] = 0x00;
	dev.hostId = dev.targetId = dev.lunId = dev.real_hostId = dev.real_targetId = dev.real_lunId = NO_VALUE;

	i = sscanf((char *)buf, "%[^,],%[^,],%[^,],%[^,],%[^,],%u,%u,%u,%u,%[^,],%u,%u\r\n", dev.vendor, dev.model,
		   real_hostIdStr, real_targetIdStr, real_lunIdStr,
		   &dev.hostId, &dev.targetId, &dev.lunId, &dev.logger.logLevel, dev.logger.logfilename, &dev.logger.type, &dev.logger.logFileSizeMax);
	if (i == 12) {
		fprintf(stdout, "Device in cfg file: %s,%s,%s,%s,%s,%u,%u,%u,%u,%s,%u,%u\r\n", dev.vendor, dev.model,
			real_hostIdStr, real_targetIdStr, real_lunIdStr,
			dev.hostId, dev.targetId, dev.lunId, dev.logger.logLevel, dev.logger.logfilename, dev.logger.type, dev.logger.logFileSizeMax);
		// Search for this entry in the list of physical available devices, and if it's there, expose it to the mapped devices list
		pTmpDev = scsiGetDeviceByVendorAndModel(pDevList, &dev); // TODO: not enough, since I can have 2 Samsung SSD 850 -> crash (this method returns the 1st match)
		if (pTmpDev) {
			if (real_hostIdStr[0] == '*') real_hostId = NO_VALUE;
									else  real_hostId = atoi((char *)real_hostIdStr);
			if (real_targetIdStr[0] == '*')	real_targetId = NO_VALUE;
									else	real_targetId = atoi((char *)real_targetIdStr);
			if (real_lunIdStr[0] == '*')	real_lunId = NO_VALUE;
									else	real_lunId = atoi((char *)real_lunIdStr);
			if ((real_hostId != NO_VALUE) && (pTmpDev->real_hostId != real_hostId))	pTmpDev = NULL;
			if (pTmpDev && (real_targetId != NO_VALUE) && (pTmpDev->real_targetId != real_targetId)) pTmpDev = NULL;
			if (pTmpDev && (real_lunId != NO_VALUE) && (pTmpDev->real_lunId != real_lunId))	pTmpDev = NULL;
			if (pTmpDev) {
				pTmpDev->hostId				= dev.hostId;
				pTmpDev->targetId			= dev.targetId;
				pTmpDev->lunId				= dev.lunId;
				pTmpDev->logger.logLevel	= dev.logger.logLevel;
				memcpy ((void*)pTmpDev->logger.logfilename, (void*)dev.logger.logfilename, 1024);
				pTmpDev->logger.type		= dev.logger.type;
				pTmpDev->logger.logFileSizeMax = dev.logger.logFileSizeMax * 1024 * 1024;
				if (scsiDeviceMapAdd(pExposedDevList, pTmpDev))	numDevicesMapped++;
			}
		}
	} 
	//else fprintf(stderr, "ERR: incomplete line found in scsiserv.cfg file: sscanf returned %d\r\n", i);
 }

 id = -1;
 while (scsiGetNextDevice(pExposedDevList, &id, &pTmpDev))
	fprintf(stdout, "Exposing Device %s %s (%d,%d,%d) to ethernet (TCP:%d) as (%d,%d,%d), loglevel=%d, logfile=%s, logtype=%d, logMaxSiz=%d\n",
		(char *)pTmpDev->vendor, (char *)pTmpDev->model,
		pTmpDev->real_hostId, pTmpDev->real_targetId, pTmpDev->real_lunId,
		port,
		pTmpDev->hostId, pTmpDev->targetId, pTmpDev->lunId,
		pTmpDev->logger.logLevel, pTmpDev->logger.logfilename, pTmpDev->logger.type, pTmpDev->logger.logFileSizeMax);

 fclose(fin);
 return (numDevicesMapped ? 1 : -1);
}

/*******************************************************************************
 *
 *******************************************************************************/
int main(int argc, char *argv[])
{
 int i, rc;
 int port = DEFAULTSYNCPORTNR;

 //command line arguments
 if (argc > 1){
	i = atoi((char *)argv[1]);
	if (i){
		port = i;
	} else {
		fprintf(stdout, "invalid port nr as argument - defaulting to port %d...\n", port);
	}
	if (argc > 2){
		loglevel = atoi((char *)argv[2]);
		loglevel = loglevel > LOGLEVEL_MAX ? LOGLEVEL_MAX : loglevel;
	}
	if (argc > 3){
		logmaxsizeMB = atoi((char *)argv[3]);
	}	
 } else {
	fprintf(stdout, "scsiserv <TCP listening port> <loglevel:1,2 or 3> <max_size_log_file_in_MB>\n");
 }
 fprintf(stdout, "logging to %s: level %d, max size %d MB\n", LOGFILENAME, loglevel, logmaxsizeMB);
 
 switch (loadConfigFile(port)){
	case -1:
		fprintf(stderr, "No devices to be mapped found, so there is no point in starting the server...\n");
		fprintf(stderr, "Aborting...\n");
		closeApp();
		return 1;
	case 0:
		if (createConfigFile())
		{
			fprintf(stderr, "Unable to find scsiserv.cfg, so creating it...\n");
			fprintf(stderr, "Please:\n");
			fprintf(stderr, "- open scsiserv.cfg,\n");
			fprintf(stderr, "- DELETE the SCSI devices you don't want to expose via TCP !\n");
			fprintf(stderr, "- MODIFY the mapped (HA id,TA id,LUN id) ! (they are all defaulted to 0,0,0 !)\n");
			fprintf(stderr, "- and then relaunch this program.\n");
		}
		else
			fprintf(stderr, "Unable to create scsiserv.cfg file.\n");
		closeApp();
		return 2;
	default:
		break;
 }

 papplogger = loggerOpen (LOGFILENAME, loglevel, logmaxsizeMB * 1024 * 1024);

 rc = runTCPServer(port, (PFNSERVER)myAcceptCallback);
 if (!rc){
	fprintf(stderr, "Unable to connect the TCP server to the port %d\n", port);
	closeApp();
	return 4;
 };

 closeApp();
 return 0;
}
