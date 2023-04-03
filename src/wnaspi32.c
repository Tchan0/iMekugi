/*******************************************************************************
 * Main file for iMekugi's wnaspi32.dll replacement
 *
 * Important note: use wnaspi32.def in your project to have the correct ordinals
 *   for the exposed functions! (Some programs fail if the correct ordinals are not used)
 *******************************************************************************/
#define _WINSOCKAPI_ // stops windows.h from including winsock.h, since we will include winsock2.h later via the TCP code if needed...
#include <windows.h>
#undef _WINSOCKAPI_ // to avoid warning later on that "winsock2.h should be included before windows.h"

//Logging
#include "utils/print.c"  //printf utilities (color, hex dump, ...)
#include "utils/logger.c"
#define LOGFILENAME "wnaspi32.log"
PLOGGER papplogger = (PLOGGER)NULL;

//#define ASYNC_CALLS_ACTIVE //TEMP TODO - uncomment this to have async scsi calls

#include "srb/srb.c"
#include "srb/srb2tcp.c"
#include "srb/srb2scsi.c"
PFNSRBDISPATCH pfnSRBDispatch = (PFNSRBDISPATCH)NULL;

#include "wnasync.c"

// DLL main configuration file - enable/disable TCP, set the target IP/PORT, activate the logging
#define DLLCONFIGFILENAME "wnaspi32.cfg"
uint32_t tcp_enabled = 0;
char target_hostname[32];
uint32_t target_port, async_listener_port;

// DLL targets configuration file - async (events/callback) response delay (msecs)
#define DLLTARGETSFILENAME "wnaspi32.tgt"
PMSDEVLIST pSCSITgtDevList;

/*******************************************************************************
 * Returns the number of host adapters installed and ensures that the
 *  ASPI manager is initialized properly.
 * This must be called once at initialization time by the client,
 *  before SendASPI32Command is accessed.
 *******************************************************************************/
__declspec(dllexport) DWORD GetASPI32SupportInfo(void)
{
 SRB_HAInquiry srb;
 DWORD ret;

 LOGPRINT_LOW(papplogger, "\r\nCLIENT called: GetASPI32SupportInfo():\r\n");

 // Similar to doing a SRB call SC_HA_INQUIRY to HaId=0
 memset((void *)&srb, 0x00, sizeof(srb));
 srb.SRB_Cmd = SC_HA_INQUIRY;

 ret = pfnSRBDispatch((LPSRB)&srb, 1);

 if (ret == SS_COMP) {
	ret = (SS_COMP << 8) | srb.HA_Count;
 } else	{ // SS_INVALID_HA or others
	// TODO SOMEDAY OPTIONAL: other possibilities: SS_ILLEGAL_MODE, SS_NO_ASPI,SS_MISMATCHED_COMPONENTS, SS_INSUFFICIENT_RESOURCES, SS_FAILED_INIT.
	ret = (SS_NO_ADAPTERS << 8);
 }

 LOGPRINT_LOW(papplogger, "<- DLL answer: 0x%08X\r\n", ret);
 return (ret);
}

/*******************************************************************************
* get the async delay, ie the delay after which we send the real status back
*******************************************************************************/
uint32_t getTgtDeviceAsyncDelay (BYTE hostId, BYTE targetId, BYTE lunId)
{
 uint32_t async_delay = 0;
 PMSDEV   pMatchedDev;

 pMatchedDev = scsiGetDeviceByMappedHTL (pSCSITgtDevList, hostId, targetId, lunId, 0);
 if (pMatchedDev) async_delay = pMatchedDev->async_delay;

 return (async_delay);
}

/*******************************************************************************
* 
*******************************************************************************/
void configureTargetDelay (PMSDEVLIST pDevList, PSRB_ExecSCSICmd pSCSIsrb)
{
 MAPPEDSCSIDEVICE srcDevice;
 PMSDEV			  pMatchedDev;

 if (!pDevList || !pSCSIsrb) return;
 
 if (pSCSIsrb->SRB_BufLen < 32) return;
 strncpy ((char*)srcDevice.vendor, (char*)&pSCSIsrb->SRB_BufPointer[8],   8);
 strncpy ((char*)srcDevice.model,  (char*)&pSCSIsrb->SRB_BufPointer[16], 16);

 pMatchedDev = scsiGetDeviceByVendorAndModel (pDevList, &srcDevice);
 if (pMatchedDev) {
	pMatchedDev->hostId   = pSCSIsrb->SRB_HaId;
	pMatchedDev->targetId = pSCSIsrb->SRB_Target;
	pMatchedDev->lunId    = pSCSIsrb->SRB_Lun;
	LOGPRINT_LOW(papplogger, "\r\nTarget Delay of %u msecs set for %s %s (%u,%u,%u):\r\n",
		pMatchedDev->async_delay, 
		pMatchedDev->vendor, pMatchedDev->model,
		pMatchedDev->hostId, pMatchedDev->targetId, pMatchedDev->lunId);
 }

}

/*******************************************************************************
* SCSI I/O requests are done via a SRB (SCSI Request Block), which is passed to
*  this dispatcher function
*******************************************************************************/
__declspec(dllexport) DWORD SendASPI32Command(LPSRB psrb)
{
 DWORD ret;
 int asyncHandlingDone = 0;
 uint32_t async_delay = 0;

 LOGPRINT_LOW(papplogger,  "\r\nCLIENT called: SendASPI32Command (%s):\r\n", srbGetCommandDesc(psrb->SRB_Cmd));
 logprintSRB(papplogger, psrb);

 // TODO ZZZ: if it's an async request, be sure we have a listener thread for the async reponses
 //  Note: since this cannot block the main dll doing extra calls, this needs to be a tcp server
 //     on another port, or better: a second client that continuously waits
 ret = pfnSRBDispatch(psrb, 1);

 // Intercept SCSI_CMD_INQUIRY commands to identify the targets that need a delay when sending the 
 //  completion event/calling the provided callback
 if ((ret == SS_COMP) && (psrb->SRB_Cmd == SC_EXEC_SCSI_CMD)){
	PSRB_ExecSCSICmd pSCSIsrb = (PSRB_ExecSCSICmd)psrb;
	if (pSCSIsrb->CDBByte[0] == SCSI_CMD_INQUIRY){
		configureTargetDelay (pSCSITgtDevList, pSCSIsrb);
	}
 }

 //some targets absolutely need delayed async notification:
 if ((psrb->SRB_Cmd == SC_EXEC_SCSI_CMD) || (psrb->SRB_Cmd == SC_RESET_DEV)){
	PSRB_ExecSCSICmd pSCSIsrb = (PSRB_ExecSCSICmd)psrb;//haId, TaId and LunId on the same place in SRB_BusDeviceReset, so either struct will do
	async_delay = getTgtDeviceAsyncDelay(pSCSIsrb->SRB_HaId, pSCSIsrb->SRB_Target, pSCSIsrb->SRB_Lun);	
 }

 asyncHandlingDone = answerAsyncSRB(psrb, ret, async_delay);
 if (asyncHandlingDone)	{
	//"hack" for programs that always expect a SS_PENDING response when they request event notification/callback calling,
	// because they did not implement the case where the final response would be send without delay... 
	// (against Adaptec's example in chapter "Waiting for Completion")
	LOGPRINT_LOW(papplogger,  "<- DLL answer: SS_PENDING, but returning real srb_status %s (0x%02X) later via event/callback\r\n",
		srbGetStatusDesc(ret), ret);
	psrb->SRB_Status = SS_PENDING;
	logprintSRB(papplogger, psrb);
	return SS_PENDING;
 }

 LOGPRINT_LOW(papplogger, "<- DLL answer: %s (0x%08X)\r\n", srbGetStatusDesc(ret), ret);
 if (ret == SS_COMP) logprintSRB(papplogger, psrb);

 return (ret);
}

/*******************************************************************************
 * TODO SOMEDAY OPTIONAL: describe & review this function - where does it come from ?
 *******************************************************************************/
__declspec(dllexport) DWORD GetASPI32DLLVersion(void)
{
 LOGPRINT_LOW(papplogger, "\r\nCLIENT called: GetASPI32DLLVersion():\r\n");
 LOGPRINT_LOW(papplogger, "<- DLL answer: 0x%08X\r\n", 1);
 return (1);
}

/*******************************************************************************
 * Provides translation between Windows 95 DEVNODEs and ASPI
 *  HA/ID/LUN triples (or vice versa). (cfr WM_DEVICECHANGE & DBT_DEVTYP_DEVNODE)
 *******************************************************************************/
__declspec(dllexport) BOOL TranslateASPI32Address(PDWORD pdw1, PDWORD pdw2)
{
 LOGPRINT_CORE(papplogger, "\r\nCLIENT called: TranslateASPI32Address():\r\n");
 LOGPRINT_CORE(papplogger, "   ERROR - TranslateASPI32Address called, but this is not implemented\r\n");
 LOGPRINT_CORE(papplogger, "<- DLL answer: 0x%08X\r\n", 0);
 return (FALSE); // TODO SOMEDAY MAYBE: implement it ? requires Windows DEVNODE IDs...
}

/*******************************************************************************
 * Allocates blocks of memory (up to 512KB) which are safe for use in client applications
 *******************************************************************************/
__declspec(dllexport) BOOL GetASPI32Buffer(PASPI32BUFF pbuf)
{
 DWORD ret;

 LOGPRINT_LOW(papplogger,  "\r\nCLIENT called: GetASPI32Buffer():\r\n");
 if (!pbuf || (pbuf->AB_BufLen > 512 * 1024)){ // must be <= 512KB
	LOGPRINT_CORE(papplogger,  "   ERROR - GetASPI32Buffer(): no buffer or buffer request size too big\r\n");
	return (FALSE);
 }

 if (pbuf->AB_ZeroFill)	pbuf->AB_BufPointer = (LPBYTE)calloc(1, (size_t)pbuf->AB_BufLen);
				else	pbuf->AB_BufPointer = (LPBYTE)malloc((size_t)pbuf->AB_BufLen);
 ret = pbuf->AB_BufPointer ? TRUE : FALSE;

 LOGPRINT_LOW(papplogger, "<- DLL answer: 0x%08X\r\n", ret);
 return (ret);
}

/*******************************************************************************
 * Releases memory previously allocated by a successful call go GetASPI32Buffer
 *******************************************************************************/
__declspec(dllexport) BOOL FreeASPI32Buffer(PASPI32BUFF pbuf)
{
 LOGPRINT_LOW(papplogger,  "\r\nCLIENT called: FreeASPI32Buffer():\r\n");
 if (!pbuf || !pbuf->AB_BufPointer || !pbuf->AB_BufLen)	{
	LOGPRINT_CORE(papplogger,  "   ERROR - FreeASPI32Buffer(): no buffer or buffer size specified\r\n");
	return (FALSE);
 }

 // TODO SOMEDAY OPTIONAL: check if we allocated it with GetASPI32Buffer
 free((void *)pbuf->AB_BufPointer);

 LOGPRINT_LOW(papplogger, "<- DLL answer: 0x%08X\r\n", TRUE);
 return (TRUE);
}

/*******************************************************************************
 *
 *******************************************************************************/
int createTargetsDLLConfigFile(void)
{
 FILE *fout;

 fout = fopen(DLLTARGETSFILENAME, "wb");
 if (!fout)	{
	LOGPRINT_CORE(papplogger,  "Unable to create %s file ???\r\n", DLLTARGETSFILENAME);
	return 0;
 }

 fprintf(fout, "# wnaspi32 targets configuration\r\n");
 fprintf(fout, "#\r\n");
 fprintf(fout, "# Only put an entry here if the device needs an async delay, ie\r\n");
 fprintf(fout, "#    the dll returns SS_PENDING, waits for async_delay milliseconds, and raises the event/calls the callback\r\n");
 fprintf(fout, "# If async_delay = 0 (default), the dll will return the real response immediately, and\r\n");
 fprintf(fout, "#    raise an event/call the callback immediately (even before the response actually)\r\n");
 fprintf(fout, "# (Most devices shouldn't need this if they followed Adaptec's recommendations...)\r\n");
 fprintf(fout, "#\r\n");
 fprintf(fout, "# Vendor, Model, async_delay (msecs)\r\n");
 fprintf(fout, "#\r\n"); 
 fprintf(fout, "# ! ADAPT the next line to your setup !\r\n");
 fprintf(fout, "#\r\n");
 fprintf(fout, "SEGA OA ,Saturn CartDev  ,100\r\n"); // Saturn CartDev Rev.B programs (eg coffload) really need this
 
 fclose(fout);
 return 1;
}

/*******************************************************************************
 *
 *******************************************************************************/
int createDLLConfigFile(void)
{
 FILE *fout;

 fout = fopen(DLLCONFIGFILENAME, "wb");
 if (!fout)	{
	LOGPRINT_CORE(papplogger, "Unable to create %s file ???\r\n", DLLCONFIGFILENAME);
	return 0;
 }

 fprintf(fout, "# wnaspi32 TCP destination configuration\r\n");
 fprintf(fout, "#\r\n");
 fprintf(fout, "# tcp_enabled:target_ip:target_port:async_events_listener_port:enable_logging (1=core, 2=low, 3=full detail):logging_file_max_MB_size\r\n");
 fprintf(fout, "#\r\n");
 fprintf(fout, "# ! ADAPT the next line to your setup !\r\n");
 fprintf(fout, "#\r\n");
 fprintf(fout, "0:192.168.1.221:7032:7050:1:50\r\n");

 fclose(fout);
 return 1;
}

/*******************************************************************************
 *
 *******************************************************************************/
int loadOrCreateConfigFiles(void)
{
 FILE *fin;
 char buf[1024];
 int i;
 uint32_t dllloggingLevel = LOGLEVEL_CORE;
 int loggingfileMaxMB = 0;
 int bStopNeeded = 0;
 MAPPEDSCSIDEVICE scsiDevice;
/* char vendor[32];
 char model[32];
 uint32_t async_delay = 0;*/

 // DLL main configuration file - enable/disable TCP, set the target IP/PORT, activate the logging
 fin = fopen(DLLCONFIGFILENAME, "rb");
 if (!fin) {
	LOGPRINT_CORE(papplogger, "Unable to find %s, so creating it...\r\n", DLLCONFIGFILENAME);
	LOGPRINT_CORE(papplogger, "Please open %s, and ADAPT the file to your setup !\r\n", DLLCONFIGFILENAME);
	createDLLConfigFile();
	bStopNeeded = 1;
 } else {
	while (fgets((char *)buf, 1024, fin)) {
		if (buf[0] == '#') continue;
		i = sscanf((char *)buf, "%u:%[^:]:%u:%u:%u:%d\r\n", &tcp_enabled, target_hostname, &target_port, &async_listener_port, &dllloggingLevel, &loggingfileMaxMB);
		if (i != 6)	LOGPRINT_CORE(papplogger, "ERROR: incomplete line found in cfg file %s: sscanf returned %d\r\n", DLLCONFIGFILENAME, i);
			else	LOGPRINT_LOW(papplogger, "OK: cfg file loaded, TCP: %s, host: %s, port: %u, logging active: %u, logging file max MB size: %u\r\n",
						 (tcp_enabled ? "enabled" : "disabled"),
						 target_hostname, target_port, // TODO: display listener port... if I still use that...
						 dllloggingLevel, loggingfileMaxMB);
 	}
 	bStopNeeded = (i == 6 ? 0 : 1);
	loggerSetConfig (papplogger, dllloggingLevel, loggingfileMaxMB * 1024 * 1024);
 	fclose(fin);	
 }

 // DLL targets configuration file - async (events/callback) response delay (msecs)
 fin = fopen(DLLTARGETSFILENAME, "rb");
 if (!fin) {
	LOGPRINT_CORE(papplogger, "Unable to find %s, so creating it...\r\n", DLLTARGETSFILENAME);
	LOGPRINT_CORE(papplogger, "Please open %s, and ADAPT the file to your setup !\r\n", DLLTARGETSFILENAME);
	createTargetsDLLConfigFile();
	bStopNeeded = 1;
 } else {
	pSCSITgtDevList = scsiDeviceMapAlloc(128);

	while (fgets((char *)buf, 1024, fin)) {
		if (buf[0] == '#') continue;
		i = sscanf((char *)buf, "%[^,],%[^,],%u\r\n", scsiDevice.vendor, scsiDevice.model, &scsiDevice.async_delay);
		if (i != 3)	LOGPRINT_CORE(papplogger, "ERROR: incomplete line found in cfg file %s: sscanf returned %d\r\n", DLLTARGETSFILENAME, i);
		else {
			LOGPRINT_LOW(papplogger, "OK: Device in .tgt file: Vendor: %s, Model: %s, Async delay: %u\r\n", scsiDevice.vendor, scsiDevice.model, scsiDevice.async_delay);
			scsiDeviceMapAdd(pSCSITgtDevList, &scsiDevice);
		}	

 	}
 	bStopNeeded = (i == 3 ? 0 : 1);

	fclose(fin);
 }

 return (bStopNeeded ? 0 : 1);
}

/*******************************************************************************
 *
 *******************************************************************************/
void closeApp(void)
{
 int id;
 PMSDEV pDev;

 if (tcp_enabled) {
	LOGPRINT_LOW(papplogger, "-- DLL: closing TCP Client...\r\n");
	closeTCP(); // if TCP was used by the DLL, it will be closed
 }

 if (pExposedDevList) {
	id = -1;
	while (scsiGetNextDevice(pExposedDevList, &id, &pDev)){
		if (scsiIsOpen (pDev)){
			LOGPRINT_LOW(papplogger, "-- DLL: closing SCSI device %s, %s (%u,%u,%u,%u)...\r\n",
			pDev->vendor, pDev->model,
			pDev->real_hostId, pDev->real_channelId, pDev->real_targetId, pDev->real_lunId);
			scsiDeviceClose (pDev, 1);
		}
	} 
	free(pExposedDevList);
	pExposedDevList = NULL;
 }

 loggerClose(papplogger, 1);
}

/*******************************************************************************
 *
 *******************************************************************************/
int initApp(void)
{
 if (!loadOrCreateConfigFiles()) return 0;

 // DLL in TCP mode, no need to search for devices on this system
 if (tcp_enabled) {
	if (!initTCPClient(target_hostname, target_port)) {
		closeApp();
		return 0;
	};
	LOGPRINT_LOW(papplogger, "-- DLL: TCP Client started\r\n");
	pfnSRBDispatch = (PFNSRBDISPATCH)dispatchSRB2TCP;
 } else	{ // DLL in regular mode, search for devices on this system
 	PMSDEV pDev;
	int id, numDevicesFound;

	LOGPRINT_LOW(papplogger, "-- DLL: non-TCP mode detected\r\n");
	LOGPRINT_LOW(papplogger, "-- DLL: trying to find SCSI devices:\r\n");
	pExposedDevList = scsiGetDeviceList();
	id = -1; numDevicesFound = 0;
	while (scsiGetNextDevice(pExposedDevList, &id, &pDev)){
		LOGPRINT_LOW(papplogger, "-- DLL: found device %s, %s (%d,%d,%d,%d)\r\n",
			pDev->vendor, pDev->model,
			pDev->real_hostId, pDev->real_channelId, pDev->real_targetId, pDev->real_lunId);
		numDevicesFound++;
	}
	if (!numDevicesFound) LOGPRINT_CORE(papplogger, "-- DLL: no SCSI devices found ???\r\n");

	pfnSRBDispatch = (PFNSRBDISPATCH)dispatchSRB2SCSI;
 }

 return 1;
}

/*******************************************************************************
 *
 *******************************************************************************/
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
 switch (fdwReason)	{
	case DLL_PROCESS_ATTACH: // LoadLibrary executed
		papplogger = loggerOpen (LOGFILENAME, LOGLEVEL_CORE, 50*1024*1024);
		if (!papplogger) return FALSE;
		if (!initApp())	{ closeApp(); return FALSE; }
		LOGPRINT_LOW(papplogger, "-- DLL: DLL_PROCESS_ATTACH called\r\n");
		return TRUE;
	case DLL_PROCESS_DETACH:
		LOGPRINT_LOW(papplogger, "-- DLL: DLL_PROCESS_DETACH called\r\n");
		closeApp();
		break;
	case DLL_THREAD_ATTACH:
		//no logging here on purpose - annoying when sub-threads are created for the async part
		break;
	case DLL_THREAD_DETACH:
		//no logging here on purpose - annoying when sub-threads are created for the async part
		break;
	default:
		break;
 };

 return 0;
}
