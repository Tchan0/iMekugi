/*******************************************************************************
* Used to send SRB requests over TCP
*
*******************************************************************************/
#include "socket/TCPClient.c"

#ifdef IS_SERVER_BUILD
 #include "socket/TCPServer.c"
#endif

#include "srb2tcp.h"

HTCPCLIENT hClient;

/*******************************************************************************
*
*******************************************************************************/
void freeTCPSRB(PTCPSRB pTCPSRB)
{
 if (!pTCPSRB) return;
 
 free (pTCPSRB);//TODO: more than this is needed I think...
}

/*******************************************************************************
*
*******************************************************************************/
int  copyTCPSRBToSRB (PTCPSRB pTCPSRB, LPSRB pSRB)
{
 DWORD SRBSize;
  
 if (!pTCPSRB || !pSRB) return (0);

 SRBSize = srbGetSize (pSRB,1);//TODO: srbGetSize isWin32 not correctly implemented !
 if (!SRBSize) return (0);

 //copy over the data part
 if (pSRB->SRB_Cmd == SC_EXEC_SCSI_CMD){
	PSRB_ExecSCSICmd pdest = (PSRB_ExecSCSICmd)pSRB;
	PSRB_ExecSCSICmd psrc  = (PSRB_ExecSCSICmd)&pTCPSRB->data[0];
	//SC_EXEC_SCSI_CMD has a memory pointer to the data buffer, so we need to restore the original
 	// pointer so that the requestor recognizes/can access it
	LPBYTE original_ptr  = pdest->SRB_BufPointer;//To avoid overwriting it with memcpy
	LPVOID original_ptr2 = pdest->SRB_PostProc;//To avoid overwriting it with memcpy

	pdest->SRB_Cmd      = psrc->SRB_Cmd;
	pdest->SRB_Status   = psrc->SRB_Status;
	pdest->SRB_HaId     = psrc->SRB_HaId;
	pdest->SRB_Flags    = psrc->SRB_Flags;
	pdest->SRB_Hdr_Rsvd = psrc->SRB_Hdr_Rsvd;
	pdest->SRB_Target   = psrc->SRB_Target;
	pdest->SRB_Lun      = psrc->SRB_Lun;
	pdest->SRB_Rsvd1    = psrc->SRB_Rsvd1;
	pdest->SRB_BufLen   = psrc->SRB_BufLen;

	uint32_t shift_offset1 = pTCPSRB->pointerSize - (uint32_t)sizeof(LPBYTE);//SRB_BufPointer
	uint32_t shift_offset2 = pTCPSRB->pointerSize - (uint32_t)sizeof(LPVOID);//SRB_PostProc
	if (shift_offset1){//or shift_offset2, doesn't matter
		PSRB_ExecSCSICmd pShifted1 = (PSRB_ExecSCSICmd)&pTCPSRB->data[shift_offset1];//valid beyond SRB_BufPointer
		PSRB_ExecSCSICmd pShifted2 = (PSRB_ExecSCSICmd)&pTCPSRB->data[shift_offset1 + shift_offset2];//valid beyond SRB_PostProc

		//shift everything beyond SRB_BufPointer but before SRB_PostProc
		pdest->SRB_SenseLen = pShifted1->SRB_SenseLen;
		pdest->SRB_CDBLen   = pShifted1->SRB_CDBLen;
		pdest->SRB_HaStat   = pShifted1->SRB_HaStat;
		pdest->SRB_TargStat = pShifted1->SRB_TargStat;		

		//shift everything beyond SRB_PostProc
		memmove((void*)&pdest->SRB_Rsvd2[0],(void*)&pShifted2->SRB_Rsvd2[0],20+16+SENSE_LEN+2);
	} else {
		//copy everything beyond SRB_BufPointer but before SRB_PostProc
		pdest->SRB_SenseLen = psrc->SRB_SenseLen;
		pdest->SRB_CDBLen   = psrc->SRB_CDBLen;
		pdest->SRB_HaStat   = psrc->SRB_HaStat;
		pdest->SRB_TargStat = psrc->SRB_TargStat;		

		//copy everything beyond SRB_PostProc
		memmove((void*)&pdest->SRB_Rsvd2[0],(void*)&psrc->SRB_Rsvd2[0],20+16+SENSE_LEN+2);
	}

	//restore the original pointers
	pdest->SRB_BufPointer = original_ptr;
	pdest->SRB_PostProc   = original_ptr2;


	//There might be a shifting inside the EXEC SCSI CMD struct, due to a pointer inside the struct:
	//  eg we except ptr size 4 in the win32 dll, but we received a package from a 64-bit system
	/*uint32_t shift_offset = pTCPSRB->pointerSize - sizeof(LPBYTE);
	if (shift_offset){
		char* pBegin1 = (char*)psrc;
		uint32_t block1size = (uint32_t)((char*)&psrc->SRB_BufLen + sizeof(DWORD) - (char*)psrc);
		char* pBegin2 = (char*)&psrc->SRB_SenseLen + shift_offset;//offset added because the buffer pointer is right before this
		uint32_t block2size = 
			(((char*)psrc) + sizeof(SRB_ExecSCSICmd) + shift_offset) //size of the scsi struct inside the TCPSRB 
			- (char*)&psrc->SRB_SenseLen;

		//copy from begin to SRB_BufLen
		memcpy ((void*)pdest, (void*)pBegin1, block1size);
		//copy from SRB_SenseLen to end of struct
		memcpy ((void*)&pdest->SRB_SenseLen, (void*)pBegin2, block2size);

	} else {
		memcpy ((void*)pdest, (void*)&pTCPSRB->data[0], sizeof(SRB_ExecSCSICmd));
	}*/
	
	//copy over the buffer data from the end of the TCP struct to the requestor
	if (pdest->SRB_BufLen){
		uint8_t* pbufferdata = (uint8_t*)pTCPSRB->data + sizeof(SRB_ExecSCSICmd) + shift_offset1 + shift_offset2;
		memcpy((void*)pdest->SRB_BufPointer, (void*)pbufferdata, pdest->SRB_BufLen);
	}	
 } else {
	memcpy ((void*)pSRB, (void*)pTCPSRB->data, SRBSize);
 }

 return (1);
}

/*******************************************************************************
*
*******************************************************************************/
PTCPSRB copySRBToTCPSRB (LPSRB pSRB)
{
 DWORD		SRBSize;
 PTCPSRB	pTCPSRB;
 
 if (!pSRB) return (NULL);
 SRBSize = srbGetSize (pSRB,1);//TODO: srbGetSize isWin32 not correctly implemented !
 if (!SRBSize) return (NULL);

 pTCPSRB = (PTCPSRB)calloc (1, TCPSRB_STRUCT_HEADERSIZE + SRBSize + TCPSRB_STRUCT_EXTRA_EXPANSIONSIZE);
 if (!pTCPSRB) return (NULL);

 //fill in the header
 memcpy ((void*)pTCPSRB, TCPSRB_MAGICID, sizeof(TCPSRB_MAGICID));
 pTCPSRB->pointerSize = (uint32_t)sizeof(LPBYTE);
 pTCPSRB->size = TCPSRB_STRUCT_HEADERSIZE + SRBSize + TCPSRB_STRUCT_EXTRA_EXPANSIONSIZE;

 //copy over the data part
 if (pSRB->SRB_Cmd == SC_EXEC_SCSI_CMD){
	PSRB_ExecSCSICmd pSRBSCSI = (PSRB_ExecSCSICmd)pSRB;
	
	memcpy ((void*)pTCPSRB->data, (void*)pSRBSCSI, sizeof(SRB_ExecSCSICmd));
	
	//SC_EXEC_SCSI_CMD potentially has a memory pointer to a data buffer, so we need to copy 
 	// this over to be useable by the target. This is the only command with such a thing
	//pTCPSRB->originalBufPointer = (uint32_t)pSRBSCSI->SRB_BufPointer;
	if (pSRBSCSI->SRB_BufLen){
		PSRB_ExecSCSICmd pdest = (PSRB_ExecSCSICmd)pTCPSRB->data;
		uint8_t* pbufferdata = (uint8_t*)pdest + sizeof(SRB_ExecSCSICmd);
		memcpy((void*)pbufferdata, (void*)pSRBSCSI->SRB_BufPointer, pSRBSCSI->SRB_BufLen);
	}	
 } else {
	memcpy ((void*)pTCPSRB->data, (void*)pSRB, SRBSize);
 }

 return (pTCPSRB);
}

/*******************************************************************************
*
*******************************************************************************/
int isCompleteTCPSRB (char* pData, unsigned int dataSizeInBytes)
{
 PTCPSRB pTCPSRB;

 //fprintf(stdout, "isCompleteTCPSRB: bytes: %d\n", dataSizeInBytes);
 if (dataSizeInBytes < TCPSRB_STRUCT_HEADERSIZE){
	//fprintf(stdout, "isCompleteTCPSRB: dataSizeInBytes smaller than tcpsrb hdrsize(%d)\n", TCPSRB_STRUCT_HEADERSIZE);
	return 0; //we need more data
 } 
 if (strncmp(pData, TCPSRB_MAGICID, 4)){
	//fprintf(stdout, "isCompleteTCPSRB: TCPSRB_MAGICID NOT OK\n");
	return (-1); //something is wrong, packets should start with the magic id
 } 
 //fprintf(stdout, "isCompleteTCPSRB: TCPSRB_MAGICID OK\n");

 pTCPSRB = (PTCPSRB)pData;
 if (pTCPSRB->size > dataSizeInBytes){
	//fprintf(stdout, "isCompleteTCPSRB: pTCPSRB->size bigger than dataSizeInBytes - not enough data\n");
	return (0);//we need more data
 } 
 
 return (pTCPSRB->size);// we have our valid packet, only process this size
}

/*******************************************************************************
* SCSI I/O requests are done via a SRB (SCSI Request Block), which is passed to
*  this dispatcher function:
*	- on Windows: via the function SendASPI32Command(), exposed by wnaspi32.dll
*	- this function is not called by DOS, it has its own function
*******************************************************************************/
DWORD dispatchSRB2TCP (LPSRB pSRB, int isWin32Call)
{//Note: since this is not called by DOS, we will not use isWin32Call, and hence we will always use non-DOS structures
 PTCPSRB pTCPSRB;
 DWORD ret;
 char inBuf[128 * 1024];
 int rc;

 //convert the SRB to a "TCP SRB"
 pTCPSRB = copySRBToTCPSRB (pSRB);
 if (!pTCPSRB) return (SS_INSUFFICIENT_RESOURCES);

 //send the SRB via TCP
 // Even if an async call was requested, we need to wait for the server to say it has taken the request (SS_PENDING)
 // Async calls will return via another client-server connection, otherwise we migh block them during another sync call
 rc = TCPClientSendData (hClient, (char*)pTCPSRB, pTCPSRB->size);
 if (rc <= 0){
	freeTCPSRB(pTCPSRB);
	return (SS_ERR);
 } 
 
 //Wait for a complete TCP SRB packet before continuing...
 rc = TCPClientWaitForReceive (hClient, inBuf, 128 * 1024, (PFNISCOMPLETEPACKET)isCompleteTCPSRB, 5000);
 if (!rc){
	freeTCPSRB(pTCPSRB);
	return (SS_ERR);
 }

 //Copy over the data into src
 memcpy ((void*)pTCPSRB, (void*)inBuf, rc > pTCPSRB->size ? pTCPSRB->size : rc);
 if (!copyTCPSRBToSRB (pTCPSRB, pSRB)){
	freeTCPSRB(pTCPSRB);
	return (SS_ERR);
 }
 
 ret = pTCPSRB->ret;
 freeTCPSRB(pTCPSRB);

 return (ret);
}

/*******************************************************************************
*
*******************************************************************************/
void closeTCP(void)
{
 /*OPTIONAL before a TCPClientShutdown(), MANDATORY whenever you want to close a connection 
   started by TCPClientConnect(), to avoid keeping the connection open for too long.
 TCPClientDisconnect (hClient);*/

 /*MANDATORY*/
 TCPClientShutdown (hClient);
}

/*******************************************************************************
*
*******************************************************************************/
#ifdef IS_SERVER_BUILD
int runTCPServer(uint16_t port, PFNSERVER acceptCallback)
{
 HTCPSERVER hServer;
 int rc;

 hServer = TCPServerInit();
 if (!hServer){
	fprintf(stderr, "Unable to init the TCP Server\n");
	return 0;
 } 
 
 fprintf(stdout, "Starting TCP Server on port %d...\n", port);

 TCPServerSetRecvBufferSize(hServer, 128 * 1024);
 TCPServerSetSendBufferSize(hServer, 128 * 1024);
 //TCPServerDisableNagleAlgorithm  (hServer, 1);

 rc = TCPServerConnect(hServer, port, "localhost", (PFNSERVER)acceptCallback);
 if (!rc){
	fprintf(stderr, "Unable to connect the TCP server to the port %d\n", port);
	TCPServerShutdown(hServer);
	return 0;
 };

 fprintf(stdout, "TCP Server shutdown...\n");
 TCPServerShutdown(hServer);
 return 1;
}
#endif

/*******************************************************************************
*
*******************************************************************************/
#ifndef IS_SERVER_BUILD
int initTCPClient(char* hostname, uint16_t port)
{
 int rc;

 hClient = TCPClientInit ();
 if (!hClient){
	fprintf(stderr, "Unable to init the TCP Client\n");
	return 0;
 } 

 TCPClientSetRecvBufferSize (hClient, 128 * 1024); /* the buffer allocated for incoming data*/
 TCPClientSetSendBufferSize (hClient, 128 * 1024); /* the buffer allocated for outgoing data*/
 //TCPClientDisableNagleAlgorithm   (hClient, 1);

 rc = TCPClientConnect (hClient, port, hostname);
 if (!rc){
	fprintf(stderr, "Unable to connect TCP Client\n");//TODO: display IP & port
	TCPClientShutdown (hClient);
	return 0;
 } else fprintf (stdout, "client connected to %s:%u\n", hostname, port);

 return 1;
}
#endif