/*******************************************************************************
* Completion Handling under Windows - Event & Posting
*
*******************************************************************************/
#define _WINSOCKAPI_ // stops windows.h from including winsock.h, since we will include winsock2.h later via the TCP code if needed...
#include <windows.h>
#undef _WINSOCKAPI_  //to avoid warning later on that "winsock2.h should be included before windows.h"

#include "srb/srb.h"

#include "utils/time.h"
#include "utils/thread.c"

typedef struct {
 LPSRB   pSRB;
 BYTE    real_SRB_status;
 LPVOID  postProc;
 uint32_t async_delay;
} SRB_Delayed_Status, *LPSRBDELAY;

/*******************************************************************************
* 
*******************************************************************************/
DWORD WINAPI notifyRealSRBStatusWithDelay (LPVOID pData)
{
 LPSRBDELAY pSRBDelay = (LPSRBDELAY)pData;
 LPSRB pSRB;
 
 if (!pSRBDelay || !pSRBDelay->pSRB || !pSRBDelay->postProc) return 0;
 pSRB = pSRBDelay->pSRB;

 SleepMsecs(pSRBDelay->async_delay);//delay xxx milliseconds (based on config file)

 //Event notification - SRB_PostProc contains an event handle
 if (pSRB->SRB_Flags & SRB_EVENT_NOTIFY){
	pSRB->SRB_Status = pSRBDelay->real_SRB_status;//restore the real completion status
	SetEvent((HANDLE)pSRBDelay->postProc);// !! Note: as soon as we raise the event, we SHALL NOT use pSRB anymore !! (the client might already start other things with it)
	free ((void*)pSRBDelay);
	return (0);
 }

 //Posting - SRB_PostProc contains a pointer to a function
 if (pSRB->SRB_Flags & SRB_POSTING){
	PFNSRBCALLBACK pCallback = (PFNSRBCALLBACK)pSRBDelay->postProc;
	pSRB->SRB_Status = pSRBDelay->real_SRB_status;//restore the real completion status
	/*logprint(LOGLEVEL_LOW, "?? ASYNC POSTING ??: %s (0x%08X)\r\n", srbGetStatusDesc(pSRB->SRB_Status), pSRB->SRB_Status);
	logprint(LOGLEVEL_LOW, "real SRB status=%u, SRB flags=0x%08X, callback ptr=%p, async delay=%u\r\n", 
		pSRBDelay->real_SRB_status, pSRB->SRB_Flags, pCallback, pSRBDelay->async_delay);
	if (pSRBDelay->real_SRB_status == SS_COMP) logprintSRB(pSRB);*/
	pCallback(pSRB);// !! Note: as soon as we call the callback, we SHALL NOT use pSRB anymore !! (the client might already start other things with it)
	free ((void*)pSRBDelay);
	return (0);
 }

 free ((void*)pSRBDelay);
 return (0);
}

/*******************************************************************************
* Sending the real srb_status after a delay of XX milliseconds
*  Mainly done for cartdev programs that MUST receive a SS_PENDING when events are on
*******************************************************************************/
int delayRealAnswer (LPSRB pSRB, BYTE real_SRB_status, LPVOID postProc, uint32_t async_delay)
{
 LPSRBDELAY pSRBDelay;
 
 pSRBDelay = (LPSRBDELAY) malloc (sizeof(SRB_Delayed_Status));
 if (!pSRBDelay) return 0;
 pSRBDelay->pSRB            = pSRB;
 pSRBDelay->real_SRB_status = real_SRB_status;
 pSRBDelay->postProc        = postProc;
 pSRBDelay->async_delay     = async_delay;

 if (!threadCreate ((PFNTHREADSTARTFUNC)notifyRealSRBStatusWithDelay, (void*)pSRBDelay)){
	free ((void*)pSRBDelay);
	return 0;
 }

 return 1;
} 

/*******************************************************************************
* If the status code is not SS_PENDING then the SRB is complete 
*   and it is safe to look at its status codes, etc. 
*******************************************************************************/
int handleCompletionAction (LPSRB pSRB, BYTE flags, LPVOID postProc, DWORD ret, uint32_t async_delay)
{
 //If SS_PENDING is returned then the SRB is still under the control of ASPI, and the
 //  caller needs to wait for the SRB to complete before doing anything else with that SRB.
 if (ret == SS_PENDING) return 0;

 //Event notification - SRB_PostProc contains an event handle
 if (flags & SRB_EVENT_NOTIFY){
	HANDLE hEvent = (HANDLE)postProc;
	if (hEvent){
		if (!async_delay) SetEvent(hEvent);
					else  return delayRealAnswer (pSRB, pSRB->SRB_Status, postProc, async_delay);//SetEvent(hEvent);
	} 
	return 0;
 }

 //Posting - SRB_PostProc contains a pointer to a function
 if (flags & SRB_POSTING){
	PFNSRBCALLBACK pCallback = (PFNSRBCALLBACK)postProc;
	if (pCallback){
		if (!async_delay) pCallback(pSRB);
					else  return delayRealAnswer (pSRB, pSRB->SRB_Status, postProc, async_delay);//pCallback(pSRB);
	} 
	return 0;
 }

 //Polling
 // Nothing to do - this is the client app continuously looping until the status of the SRB changes
 return 0;
}

/*******************************************************************************
* only async calls need special handling (event notification or calling a callback)
*******************************************************************************/
int answerAsyncSRB (LPSRB pSRB, DWORD ret, uint32_t async_delay)
{
 int asyncHandlingDone = 0;
 
 if (!pSRB) return 0;
 
 switch (pSRB->SRB_Cmd){
	case SC_EXEC_SCSI_CMD: {
		PSRB_ExecSCSICmd pSRBSCSI = (PSRB_ExecSCSICmd)pSRB;
		asyncHandlingDone = handleCompletionAction(pSRB, pSRBSCSI->SRB_Flags, pSRBSCSI->SRB_PostProc, ret, async_delay);	
		}
		break;
	case SC_RESET_DEV: {
		PSRB_BusDeviceReset pSRBDevReset = (PSRB_BusDeviceReset)pSRB;
		asyncHandlingDone = handleCompletionAction(pSRB, pSRBDevReset->SRB_Flags, pSRBDevReset->SRB_PostProc, ret, async_delay);
		}
		break;
	default:
		break;
 }

 return asyncHandlingDone;
}
