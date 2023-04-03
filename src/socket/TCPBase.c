/*******************************************************************************
* Base functions for TCP clients & servers
* 
*******************************************************************************/
#ifndef __TCPBASE_C_DEFINED__
#define __TCPBASE_C_DEFINED__

#include "TCPBase.h"

#include <stdio.h>  /*for stderr*/
#include <stdlib.h> /*for malloc*/

/*******************************************************************************
* 
*******************************************************************************/
void u32add2str (uint32_t u32addr, char* pbuf)
{
 sprintf(pbuf, "%d.%d.%d.%d", u32addr & 0xFF, (u32addr>>8 & 0xFF), (u32addr>>16 & 0xFF), (u32addr>>24 & 0xFF));  
}

/*******************************************************************************
* 
*******************************************************************************/
int TCPAllocRSBuffers (char** ppRecvBuf, int* pRecvBufSize, char** ppSendBuf, int* pSendBufSize)
{
 void* pbuf1;
 void* pbuf2;

 if (!ppRecvBuf || !ppSendBuf || !pRecvBufSize || !pSendBufSize) return 0;

 /*we need at least some byte space in the buffers...*/
 if (*pRecvBufSize < TCP_MINIMUM_BUFSIZE) *pRecvBufSize = TCP_MINIMUM_BUFSIZE;
 if (*pSendBufSize < TCP_MINIMUM_BUFSIZE) *pSendBufSize = TCP_MINIMUM_BUFSIZE;

 /*alloc the recv buffer*/
 pbuf1 = malloc (*pRecvBufSize);
 if (!pbuf1){
	return 0;
 }

 /*alloc the send buffer*/
 pbuf2 = malloc (*pSendBufSize);
 if (!pbuf2){
	free (pbuf1);
	return 0;
 }

 *ppRecvBuf = pbuf1;
 *ppSendBuf = pbuf2;

 return (1);
}

/*******************************************************************************
* Disables the Nagle algorithm (that is, the potential buffering of small requests befor sending)
*******************************************************************************/
void TCPDisableNagleAlgorithm (int socketfd, int disableNagle)
{
 int rc;
 int noDelayActive;

 rc = getsockopt (socketfd, SOL_SOCKET, TCP_NODELAY, (void *)&noDelayActive, (socklen_t*)&noDelayActive);
 if(rc < 0) { fprintf(stderr, "error getsockopt(TCP_NODELAY)...\n"); return; }

 if (noDelayActive != disableNagle){
	fprintf(stdout, "TCP_NODELAY = %d, I want this set to %d... trying...\n", noDelayActive, disableNagle);
	noDelayActive = disableNagle;
	rc = setsockopt (socketfd, SOL_SOCKET, TCP_NODELAY, (const void *)&noDelayActive, (socklen_t)sizeof(noDelayActive));
	if(rc < 0) { fprintf(stderr, "error setsockopt(TCP_NODELAY)...\n"); return; }
	rc = getsockopt (socketfd, SOL_SOCKET, TCP_NODELAY, (void *)&noDelayActive, (socklen_t*)&noDelayActive);
	if(rc < 0) { fprintf(stderr, "error getsockopt(TCP_NODELAY)...\n"); }
	fprintf(stdout, "TCP_NODELAY = %d...\n", noDelayActive);
 }
}

#endif