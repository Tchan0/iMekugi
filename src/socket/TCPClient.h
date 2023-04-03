/*******************************************************************************
* TCPClient 
*   helper methods to make a TCP client connection & receive/send data
*******************************************************************************/
#ifndef __TCPCLIENT_H_DEFINED__
#define __TCPCLIENT_H_DEFINED__

#include "TCPBase.h"

typedef struct TCPCLIENT_STRUCT {
	int          socketfd;
	
	char*        pRecvBuf;      /*the buffer that receives incoming data*/
	unsigned int recvBufCurSize;/*the current size of the buffer*/
	unsigned int recvBufMaxSize;/*the size that will be applied at the next connect()*/

	char*        pSendBuf;      /*the buffer that receives outgoing data*/
	unsigned int sendBufCurSize;/*the current size of the buffer*/
	unsigned int sendBufMaxSize;/*the size that will be applied at the next connect()*/
	int			 disableNagle;  /*disable Nagle's algoithm*/
} TCPCLIENT_STRUCT;
typedef TCPCLIENT_STRUCT* HTCPCLIENT;

#define TCPCLIENT_RECVBUFFER_MINIMUM_SIZE 2048
#define TCPCLIENT_SENDBUFFER_MINIMUM_SIZE 2048
#define TCPCLIENT_MAX_TIMEOUT_MSECS      60000 /*1 minute*/

/*typedef int (*PFNCLIENTDATAPROCESSING)(int sock, char* RecvBuf, int RecvBufSize, char* SendBuf, int SendBufSize); */

/*MANDATORY - first get a handle to a TCP client*/
HTCPCLIENT TCPClientInit (void); /*allocate the handle & init the defaults*/
void TCPClientShutdown (HTCPCLIENT hClient); /*close the logger, free the handle*/

/*OPTIONAL - if needed, override the defaults*/
void TCPClientSetRecvBufferSize (HTCPCLIENT hClient, unsigned int newSize); /* the buffer allocated for incoming data*/
void TCPClientSetSendBufferSize (HTCPCLIENT hClient, unsigned int newSize); /* the buffer allocated for outgoing data*/
void TCPClientDisableNagleAlgorithm (HTCPCLIENT hClient, int disableNagle); /* Disables the Nagle algorithm (that is, the potential buffering of small requests befor sending)*/
/*TCPClientSetLogger (hClient, type); (by default no logger, unless specifically asked via this function)*/
/*TCPErrorLogger (hClient), TCPLogger(); */

/*MANDATORY*/
int TCPClientConnect (HTCPCLIENT hClient, uint16_t port, char* hostname);
/*void TCPClientRecvSendLoop (HTCPCLIENT hClient, PFNCLIENTDATAPROCESSING pfnCmdProcessingCallback);//optional, also done by TCPClientShutdown()*/
int TCPClientSend (HTCPCLIENT hClient, char* strToSend);
int TCPClientSendData (HTCPCLIENT hClient, char* dataToSend, unsigned int siz);
int TCPClientWaitForReceive (HTCPCLIENT hClient, char* pInBuf, unsigned int inBufSize, PFNISCOMPLETEPACKET pfnIsCompletePacket, unsigned int timeoutMsecs);
void TCPClientDisconnect (HTCPCLIENT hClient); 

#endif