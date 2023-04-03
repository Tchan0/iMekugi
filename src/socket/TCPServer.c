/*******************************************************************************
* TCPServer
*   helper methods to make a TCP server receive/send data
*******************************************************************************/
#ifndef __TCSERVER_C_DEFINED__
#define __TCPSERVER_C_DEFINED__

#define _XOPEN_SOURCE 700 /*removes warnings 'incomplete type is not allowed for sigaction' https://stackoverflow.com/questions/6491019/struct-sigaction-incomplete-error*/
#include <signal.h> /*https://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c*/

#include "TCPServer.h"
#include "TCPBase.c"

#include "utils/thread.c"

static volatile int keepRunning = 1;

/*******************************************************************************
* 
*******************************************************************************/
int TCPSend (int sock, char* SendBuf, int bytesToSend)
{
 ssize_t num_actual_write = 0;
 
 if (bytesToSend)
 	num_actual_write = send (sock, SendBuf, bytesToSend, 0);
 
 return (num_actual_write);
}

/*******************************************************************************
* 
*******************************************************************************/
int TCPServerDummyProcessXBytes (char* recvBuf, int recvBufSize)
{
 return (recvBufSize);
}

/*******************************************************************************
* 
*******************************************************************************/
void TCPServerRecvSendLoop (HTCPSERVER hServer,/*int sock, int recvBufMaxSize, int sendBufMaxSize,*/
									  PFNISCOMPLETEPACKET pfnIsCompletePacket,
									  PFNSERVERDATA pfnCmdProcessingCallback,
 									  PFNSERVER     pfnServerRecvSendNewLoop)
{
 ssize_t num_actual_read;
 ssize_t num_actual_write;
 int numBytesToSend = 0;
 int bContinue = 1;
 int validFrameSize;
 char* curRecvBuf;
 char* curSendBuf;
 int recvBufDataSize = 0; 
 int sendBufDataSize = 0;
 unsigned int recvBufMaxSize;
 unsigned int sendBufMaxSize;
 /*PFNSERVERPROCESSXBYTES pfnServerGetNextValidFrameSize;*/
#if (!defined _MSC_VER) && (!defined __MINGW32__)
 int rc;
 int recvlowat, recvlowatsize;
#endif

 if (!hServer || !hServer->socketfd || !pfnCmdProcessingCallback) return;
 if (!pfnIsCompletePacket) 
 	pfnIsCompletePacket = (PFNISCOMPLETEPACKET)TCPServerDummyProcessXBytes;
 
 /*fprintf(stdout, "entered TCP Server loop...\n");*/

 /*Allocate the receiving & sending buffers that will contain the data that pass thru the TCP server*/
 recvBufMaxSize = hServer->recvBufMaxSize;
 sendBufMaxSize = hServer->sendBufMaxSize;
 if (!TCPAllocRSBuffers (&curRecvBuf, &recvBufMaxSize, &curSendBuf, &sendBufMaxSize)){
	fprintf(stderr, "error trying to malloc() the recv/send buffers for the TCP server\n");
	return;
 }

 /*TODO: move this somewhere else*/
#if (!defined _MSC_VER) && (!defined __MINGW32__)
 rc = getsockopt (hServer->socketfd, SOL_SOCKET, SO_RCVLOWAT, (char *)&recvlowat, (socklen_t*)&recvlowatsize);
 if(rc < 0) { fprintf(stderr, "error getsockopt(SO_RCVLOWAT)...\n"); return; }/*TODO free() the buffers*/
 /*fprintf(stdout, "recvlowat = %d, recvlowatsize = %d...\n", recvlowat, recvlowatsize);*/
 if (recvlowat != 1){
	fprintf(stdout, "recvlowat = %d, I want this set to 1... trying...\n", recvlowat);
	recvlowat = 1;
 	rc = setsockopt (hServer->socketfd, SOL_SOCKET, SO_RCVLOWAT, (const void *)&recvlowat, recvlowatsize);
	if(rc < 0) { fprintf(stderr, "error setsockopt(SO_RCVLOWAT)...\n"); return; }/*TODO free() the buffers*/
	rc = getsockopt (hServer->socketfd, SOL_SOCKET, SO_RCVLOWAT, (char *)&recvlowat, (socklen_t*)&recvlowatsize);
 	if(rc < 0) { fprintf(stderr, "error getsockopt(SO_RCVLOWAT)...\n"); }
 	fprintf(stdout, "recvlowat = %d, recvlowatsize = %d...\n", recvlowat, recvlowatsize);
 }
#endif

 /*fprintf(stdout, "starting the endless loop...\n");*/
 while (bContinue) {
	/*callback to process whatever has to happen at every loop*/
	if (pfnServerRecvSendNewLoop) pfnServerRecvSendNewLoop(hServer);/*todo: also use a dummy fallback ?*/

	/*fprintf(stdout, "loop+1 ...\n");*/

	/*read IN*/
 	num_actual_read = recv (hServer->socketfd, &curRecvBuf[recvBufDataSize], recvBufMaxSize - recvBufDataSize - 1, 0);/*TODO: check if don't go negative*/
	/*fprintf(stdout, "loop+1 recv done... %ld bytes read\n", num_actual_read);*/

 	if (!num_actual_read){/*When a stream socket peer has performed an orderly shutdown, the return value will be 0 (the traditional "end-of-file" return).*/
		bContinue = 0;
		fprintf (stderr, "client closed its socket (recv() returned zero)... stopping child server (recvBufMaxSize= %d, recvBufDataSize= %d)\n", recvBufMaxSize, recvBufDataSize);
		break; 
	} else if (num_actual_read > 0){/*we got new data !*/
		recvBufDataSize += num_actual_read;
		
		validFrameSize = 1;
		while (validFrameSize){
			validFrameSize = pfnIsCompletePacket (curRecvBuf, recvBufDataSize);
			/*fprintf(stdout, "pfnIsCompletePacket() called with %d bytes... validFrameSize = %d...\n", recvBufDataSize, validFrameSize);*/

			if (validFrameSize > 0){
				/*fprintf(stdout, "RECV validframesize found: %d bytes\n", validFrameSize);*/
				if (validFrameSize > recvBufDataSize) validFrameSize = recvBufDataSize;/*just to be sure, a client app could mess up...*/
				numBytesToSend += pfnCmdProcessingCallback (hServer, curRecvBuf, validFrameSize, &curSendBuf[sendBufDataSize], sendBufMaxSize - sendBufDataSize - 1);
				/*don't loose any extra recv data we migh have beyond the valid frame*/
				recvBufDataSize = recvBufDataSize - validFrameSize;
				if (recvBufDataSize < 0) recvBufDataSize = 0;/*just to be sure, a client app could mess up...*/
				memmove ((void*)&curRecvBuf[0], (void*)&curRecvBuf[validFrameSize], recvBufDataSize);
			} else if (validFrameSize < 0){/*invalid data, discard all data*/
				/*fprintf(stdout, "RECV negative framesize received, discarding %d bytes\n", recvBufDataSize);*/
				recvBufDataSize = 0;
				continue;
			} else {/*not a complete frame, store this data for later & continue*/
				/*fprintf(stdout, "RECV %ld extra bytes received, but not yet a valid frame size\n", num_actual_read);*/
				/*recvBufDataSize += num_actual_read;//already done
				//TODO: break if we reach our buf limits ? */
			}
		}
	/*TODO: negative value: check errno - check for EAGAIN or EWOULDBLOCK ?*/
	} 
 	/*if negative, error: TODO*/
    if (!numBytesToSend && !sendBufDataSize) continue;/*we want to continue receiving data*/

 	/*buf[num_actual_read] = 0x00;//just to be safe...*/
	num_actual_write = send (hServer->socketfd, curSendBuf, sendBufDataSize + numBytesToSend, 0);
 	if (num_actual_write < 0) { fprintf (stderr, "Negative actual write\n"); bContinue = 0; break; }/*connection closed or error writing*/
	else if (num_actual_write) {
		sendBufDataSize = sendBufDataSize + numBytesToSend - num_actual_write;
		if (sendBufDataSize < 0) sendBufDataSize = 0;
		memmove ((void*)&curSendBuf[0], (void*)&curSendBuf[num_actual_write], sendBufDataSize);
		//numBytesToSend = sendBufDataSize; WRONG ! is already contained in numBytesToSend
		numBytesToSend = 0;
	}//todo: what if num_actual_write = 0 ?
 }

 fprintf (stdout, "exiting the %s server loop...\n", hServer->isChild ? "child" : "server");
}

/*******************************************************************************
* 
*******************************************************************************/
#if defined( __linux__ ) 
  void* startChildServer (void* pData)
#else
  DWORD WINAPI startChildServer (LPVOID pData)
#endif
{
 HTCPSERVER hChild = (HTCPSERVER)pData;
 char tmpbuf[32];

 if (hChild){
	if (hChild->pfnAcceptCallback)
		hChild->pfnAcceptCallback(hChild);
	
	u32add2str(hChild->addr, tmpbuf);
	fprintf (stdout, "connection from %s:%d closed... exiting child thread...\n", tmpbuf, htons(hChild->port));
	
	TCPServerShutdown (hChild);
 };
 
#if defined( __linux__ ) 
  return NULL;
#else
  return 0;
#endif
}

/*******************************************************************************
* 
*******************************************************************************/
int TCPServerCreateChild (HTCPSERVER hServer, int childfd, uint32_t addr, uint16_t port, PFNSERVER pfnAcceptCallback)
{
 char tmpbuf[32];

 HTCPSERVER hChild = TCPServerInit ();
 hChild->socketfd = childfd;
 hChild->isChild = 1;
 hChild->recvBufMaxSize = hServer->recvBufMaxSize;
 hChild->sendBufMaxSize = hServer->sendBufMaxSize;
 hChild->addr = addr;
 hChild->port = port;
 hChild->pfnAcceptCallback = pfnAcceptCallback;
 
 u32add2str(hChild->addr, tmpbuf);
 fprintf (stdout, "connection accepted from %s:%d\n", tmpbuf, htons(hChild->port));

 if (!threadCreate ((PFNTHREADSTARTFUNC)startChildServer, (void*)hChild)){
	TCPServerShutdown(hChild);
	return 0;
 }

 return 1;
}

int serverfd;/*TODO: not really happy about having a global variable...*/
/*******************************************************************************
* 
*******************************************************************************/
void TCPServerSignalHandler (int dummy)
{
 keepRunning = 0;
 close (serverfd);/*we will probably be in accept() mode, so just changing keepRunning might do nothing...*/
}

/*******************************************************************************
* handle CTRL+C in a clean way
*  cfr https://stackoverflow.com/questions/27621866/catch-ctrlc-in-c-program 
*      https://man7.org/linux/man-pages/man2/sigaction.2.html
*      based on https://www.gnu.org/software/libc/manual/html_node/Sigaction-Function-Example.html
*******************************************************************************/
void TCPServerSetSignalHandler (void)
{
#if (defined _MSC_VER) || (defined __MINGW32__)
 signal (SIGINT, TCPServerSignalHandler);
#else
 struct sigaction new_action, old_action;

 /* Set up the structure to specify the new action.*/
 new_action.sa_handler = TCPServerSignalHandler;
 sigemptyset (&new_action.sa_mask);
 new_action.sa_flags = 0;

 /*note: SIG_IGN specifies that the signal should be ignored. 
 //sigaction() seems to be better than signal()*/
 sigaction (SIGINT, NULL, &old_action); /*Interrupt from keyboard*/
 if (old_action.sa_handler != SIG_IGN)  sigaction (SIGINT, &new_action, NULL);
 sigaction (SIGHUP, NULL, &old_action); /*Hangup detected on controlling terminal or death of controlling process*/
 if (old_action.sa_handler != SIG_IGN)  sigaction (SIGHUP, &new_action, NULL);
 sigaction (SIGTERM, NULL, &old_action);/*Termination signal*/
 if (old_action.sa_handler != SIG_IGN)  sigaction (SIGTERM, &new_action, NULL);
#endif
}

/*******************************************************************************
* 
*******************************************************************************/
int TCPServerConnect (HTCPSERVER hServer, uint16_t port, char* hostname, PFNSERVER pfnAcceptCallback)
{
 int childfd, pid;
 int rc;
 int client_addr_length;
 struct sockaddr_in server_addr, client_addr;

 if (!hServer || !pfnAcceptCallback) return (0);
 
 TCPServerSetSignalHandler ();

#ifdef WIN32 /*initialize winsock*/
 { 
  WSADATA wsaData;
  int iResult;
  iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (iResult != 0) {
    fprintf(stderr, "WSAStartup failed with error: %d\n", iResult);
	return (0);	
  }
 }
#endif

 /*Create a new socket of type TYPE in domain DOMAIN, using protocol PROTOCOL.*/
 serverfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
 if (serverfd < 0) {
	fprintf (stderr, "error trying to get a file descriptor for a stream socket\n");
#ifdef WIN32
	rc = WSAGetLastError();
	fprintf (stderr, "WSAGetLastError: %d\n",rc);
#endif
	return (0);
 }

 hServer->socketfd = serverfd;
 TCPServerSetRecvBufferSize(hServer, hServer->recvBufMaxSize);
 TCPServerSetSendBufferSize(hServer, hServer->sendBufMaxSize);
 //TCPServerDisableNagleAlgorithm (hServer, hServer->disableNagle);

 /*set the address family struct*/
 memset ((void*)&server_addr, 0x00, sizeof(server_addr));
 server_addr.sin_family = AF_INET;
 /*server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); TODO: specify only local or not*/
 server_addr.sin_addr.s_addr = INADDR_ANY; /*TODO: for a specific address only*/
 server_addr.sin_port = htons(port);      /*port in network byte order*/

 /* Give the socket FD the server address*/
 rc =  bind (serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
 if (rc < 0){
	fprintf (stderr, "error trying to bind the socket to port %d\n", port);
	close (serverfd);
	hServer->socketfd = 0;
	return (0);
 }

 /*set the socket in listen mode & enqueue max x clients, after which we refuse requests*/
 rc = listen (serverfd, hServer->numQueuedConnections);
 if (rc < 0){
	fprintf (stderr, "error setting up listen()\n");
	close (serverfd);/*TODO ? do we need to do more than a close, since there was a bind ?*/
	hServer->socketfd = 0;
	return (0);
 }

 while (keepRunning) {
	/*Accept a connection with the accept() system call. This call typically blocks until a client connects with the server.*/
	client_addr_length = sizeof(client_addr);/* the caller must initialize it to contain the size (in bytes) of the structure pointed to by addr; on return it will contain the actual size of the peer address. */
	childfd = accept (serverfd, (struct sockaddr *)&client_addr, &client_addr_length);
	if (childfd < 0) {
        if (errno == 0x16){ /*invalid argument*/
			char tmpbuf[32];
			u32add2str((uint32_t)client_addr.sin_addr.s_addr, tmpbuf);
			fprintf (stderr, "accept() ignored from %s:%d - %s (0x%04x)\n", tmpbuf, htons(client_addr.sin_port), strerror(errno), errno);
			continue;
		} else {/*TODO: check other reasons*/
			fprintf (stderr, "accept() stopped with reason %s (0x%04x)\n", strerror(errno), errno);
			break;
		}
    }
    //Create child process
	if (!TCPServerCreateChild (hServer, childfd, client_addr.sin_addr.s_addr, client_addr.sin_port, pfnAcceptCallback)){
		fprintf (stderr, "error trying to create a thread. Exiting...\n");
		close (childfd);//TODO: do more than just a close ?
		break;
	}
	//TODO? set a max limit of nr of child processes
 }

 TCPServerDisconnect (hServer);

 return (1);
}

/*******************************************************************************
* for the server, merge disconnect & close socket
*******************************************************************************/
void TCPServerDisconnect (HTCPSERVER hServer)
{
 if (!hServer || !hServer->socketfd) return;

 fprintf (stdout, "disconnecting the %s socket...\n", hServer->isChild ? "child" : "server");

 /*disconnect gracefully*/
#if (defined _MSC_VER) || (defined __MINGW32__)
 shutdown (hServer->socketfd, SD_SEND);/*SHUT_WR*/
 shutdown (hServer->socketfd, SD_RECEIVE);/*SHUT_RD*/
#else
 shutdown (hServer->socketfd, SHUT_WR);
 shutdown (hServer->socketfd, SHUT_RD);
#endif
 
 /*close the socket*/
 fprintf (stdout, "closing the %s socket...\n", hServer->isChild ? "child" : "server");
 close (hServer->socketfd);
 hServer->socketfd = 0;
}

/*******************************************************************************
* set the size of buffer allocated for outgoing data
*******************************************************************************/
void TCPServerSetNumQueuedConnections (HTCPSERVER hServer, unsigned int numQueuedConnections)
{
 if (!hServer) return;
 
 hServer->numQueuedConnections = numQueuedConnections;
}

/*******************************************************************************
* set the size of buffer allocated for outgoing data
*******************************************************************************/
void TCPServerSetRecvBufferSize (HTCPSERVER hServer, unsigned int newSize)
{
 int rcvBuffSize, res;
 socklen_t optionLen;
 
 if (!hServer) return;
 
 if (newSize < TCPSERVER_RECVBUFFER_MINIMUM_SIZE) newSize = TCPSERVER_RECVBUFFER_MINIMUM_SIZE;
 
 if (hServer->socketfd){
	//Get the current SO_RCVBUF
 	optionLen = sizeof(rcvBuffSize);
 	res = getsockopt(hServer->socketfd, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBuffSize, &optionLen);
 	if(res == -1) printf("Error getting SO_RCVBUF\n");
 			else  printf("send buffer size (SO_RCVBUF)= %d\n", rcvBuffSize);

	//Change the SO_RCVBUF
	/*rcvBuffSize = newSize;
	res = setsockopt(hServer->socketfd, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBuffSize, sizeof(rcvBuffSize));
	if(res == -1) printf("Error trying to set SO_RCVBUF to %d\n", rcvBuffSize);
			else  printf("SO_RCVBUF set to %d\n", rcvBuffSize);*/
 }

 hServer->recvBufMaxSize = newSize;
}

/*******************************************************************************
* set the size of buffer allocated for outgoing data
*******************************************************************************/
void TCPServerSetSendBufferSize (HTCPSERVER hServer, unsigned int newSize)
{
 int sendBuffSize, res;
 socklen_t optionLen;

 if (!hServer) return;

 if (newSize < TCPSERVER_SENDBUFFER_MINIMUM_SIZE) newSize = TCPSERVER_SENDBUFFER_MINIMUM_SIZE;

 if (hServer->socketfd){
	//Get the current SO_SNDBUF
 	optionLen = sizeof(sendBuffSize);
 	res = getsockopt(hServer->socketfd, SOL_SOCKET, SO_SNDBUF, (char*)&sendBuffSize, &optionLen);
 	if(res == -1) printf("Error getting SO_SNDBUF\n");
 			else  printf("send buffer size (SO_SNDBUF)= %d\n", sendBuffSize);

 	//Change the SO_SNDBUF
 	/*sendBuffSize = newSize;
 	res = setsockopt(hServer->socketfd, SOL_SOCKET, SO_SNDBUF, (char*)&sendBuffSize, sizeof(sendBuffSize));
 	if(res == -1) printf("Error trying to set SO_SNDBUF to %d\n", sendBuffSize);
 			else  printf("SO_SNDBUF set to %d\n", sendBuffSize);*/
 }

 hServer->sendBufMaxSize = newSize;
}

/*******************************************************************************
* Disables the Nagle algorithm (that is, the potential buffering of small requests befor sending)
*******************************************************************************/
void TCPServerDisableNagleAlgorithm (HTCPSERVER hServer, int disableNagle)
{
 if (!hServer || (hServer->disableNagle == disableNagle)) return;

 if (hServer->socketfd){
	TCPDisableNagleAlgorithm (hServer->socketfd, disableNagle);
 }

 hServer->disableNagle = disableNagle;
}

/*******************************************************************************
* 
*******************************************************************************/
void TCPServerShutdown (HTCPSERVER hServer)
{
 if (!hServer) return;
 
 /*disconnect & close the socket*/
 TCPServerDisconnect (hServer);

 /*deallocate the recv & send buffers*/
 if (hServer->pRecvBuf) free (hServer->pRecvBuf);
 if (hServer->pSendBuf) free (hServer->pSendBuf);

#ifdef WIN32
	WSACleanup();
#endif

 free(hServer);
}

/*******************************************************************************
* allocate the handle to the TCP Server & init the defaults
*******************************************************************************/
HTCPSERVER TCPServerInit (void)
{
 HTCPSERVER hServer;

 hServer = (HTCPSERVER)calloc (1, sizeof(TCPSERVER_STRUCT));
 if (!hServer) return ((HTCPSERVER)NULL);
 
 /*init of the Receive & Send buffers*/
 hServer->recvBufMaxSize = TCPSERVER_RECVBUFFER_MINIMUM_SIZE;
 hServer->sendBufMaxSize = TCPSERVER_SENDBUFFER_MINIMUM_SIZE;
 hServer->numQueuedConnections = TCPSERVER_DEFAULT_NUM_QUEUED_CONNECTIONS;

 return (hServer);
}

#endif