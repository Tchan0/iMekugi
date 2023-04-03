/*******************************************************************************
* TCPServer
*   helper methods to make a TCP server receive/send data
*
* https://www.tutorialspoint.com/unix_sockets/socket_server_example.htm
*******************************************************************************/
#ifndef __TCPSERVER_H_DEFINED__
#define __TCPSERVER_H_DEFINED__

#include "TCPBase.h"

//typedef struct TCPSERVER_STRUCT TCPSERVER_STRUCT_T;
typedef struct TCPSERVER_STRUCT {
	int          socketfd;
	unsigned int numQueuedConnections;
	int          isChild;
	char*        pRecvBuf;      /*the buffer that receives incoming data*/
	unsigned int recvBufCurSize;/*the current size of the buffer*/
	unsigned int recvBufMaxSize;/*the size that will be applied at the next connect()*/

	char*        pSendBuf;      /*the buffer that receives outgoing data*/
	unsigned int sendBufCurSize;/*the current size of the buffer*/
	unsigned int sendBufMaxSize;/*the size that will be applied at the next connect()*/
	int          disableNagle;  /*disable Nagle's algoithm*/

	uint32_t     addr;          /*corresponds to client_addr.sin_addr.s_addr*/
	uint16_t     port;          /*corresponds to client_addr.sin_port*/

	//PFNSERVER    pfnAcceptCallback;
	void (*pfnAcceptCallback)(struct TCPSERVER_STRUCT* hServer);
} TCPSERVER_STRUCT;
typedef TCPSERVER_STRUCT* HTCPSERVER;

/*generic callback provided by the customer application*/
typedef void (*PFNSERVER)(TCPSERVER_STRUCT*);

#define TCPSERVER_RECVBUFFER_MINIMUM_SIZE         2048
#define TCPSERVER_SENDBUFFER_MINIMUM_SIZE         2048
#define TCPSERVER_MAX_TIMEOUT_MSECS              60000 /*1 minute*/
#define TCPSERVER_DEFAULT_NUM_QUEUED_CONNECTIONS     5

/*MANDATORY - first get a handle to a TCP client*/
HTCPSERVER TCPServerInit (void); /*allocate the handle & init the defaults*/
void TCPServerShutdown (HTCPSERVER hServer); /*close the logger, free the handle*/

/*OPTIONAL - if needed, override the defaults before calling TCPServerConnect()*/
void TCPServerSetRecvBufferSize (HTCPSERVER hServer, unsigned int newSize); /* the buffer allocated for incoming data*/
void TCPServerSetSendBufferSize (HTCPSERVER hServer, unsigned int newSize); /* the buffer allocated for outgoing data*/
void TCPServerDisableNagleAlgorithm (HTCPSERVER hServer, int disableNagle); /*Disables the Nagle algorithm (that is, the potential buffering of small requests befor sending)*/
void TCPServerSetNumQueuedConnections (HTCPSERVER hServer, unsigned int numQueuedConnections); /*the "backlog" of listen (socket, backlog)*/
/*TCPServerSetLogger (hClient, type); (by default no logger, unless specifically asked via this function)
//TCPErrorLogger (hClient), TCPLogger(); 
//Logging helpers void TCPServerStdoutLoggerInit (void); void TCPServerFileLoggerInit (char* szFileName); void TCPServerFileLoggerShutdown ();
//TCPServerSetLogger (hServer); (by default no logger, unless specifically asked via this function)
//TCPServerSetMaxConnections (hServer);*/

/*MANDATORY*/
int TCPServerConnect (HTCPSERVER hServer, uint16_t port, char* hostname, PFNSERVER pfnAcceptCallback);/*bind, listen & accept*/
void TCPServerDisconnect (HTCPSERVER hServer);

/*MANDATORY - Server Data Processing Callback*/
/* incoming data is sent to this callback, and data to send out can be passed back:
//  - return 0 as long as we want to continue receiving TCP data (and thus, we go back into the TCP Server loop)
//  - return x if we want to send x bytes from SendBuf into the TCP connection*/
typedef int (*PFNSERVERDATA)(HTCPSERVER hServer, char* recvBuf, int recvBufSize, char* sendBuf, int sendBufSize); 

/*To be called from within your pfnAcceptCallback:*/
void TCPServerRecvSendLoop (HTCPSERVER hServer, 
							PFNISCOMPLETEPACKET pfnIsCompletePacket,
							PFNSERVERDATA pfnCmdProcessingCallback, 
							PFNSERVER pfnServerRecvSendNewLoop);

#endif