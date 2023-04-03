/*******************************************************************************
* TCPClient 
*   helper methods to make a TCP client connection & receive/send data
*******************************************************************************/
#ifndef __TCPCLIENT_C_DEFINED__
#define __TCPCLIENT_C_DEFINED__

#if defined( __linux__ ) 
	#include <netdb.h>      /*for gethostbyname*/
	#include <sys/time.h>   /*needed for timeval*/
	//#include <fcntl.h>      /*for fcntl() - set socket non blocking mode*/
#endif

#include "TCPClient.h"
#include "TCPBase.c"

/*******************************************************************************
* 
*******************************************************************************/
int TCPClientSendData (HTCPCLIENT hClient, char* dataToSend, unsigned int siz)
{
 int num_actual_write;
 
 if (!hClient || !hClient->socketfd || !siz) return 0;
 
 num_actual_write = send (hClient->socketfd, dataToSend, siz, 0);

 return num_actual_write;
}

/*******************************************************************************
* 
*******************************************************************************/
int TCPClientSend (HTCPCLIENT hClient, char* strToSend)
{
 int num_actual_write;
 
 if (!hClient || !hClient->socketfd || !strlen(strToSend)) return 0;
 
 num_actual_write = send (hClient->socketfd, strToSend, strlen(strToSend), 0);

 return num_actual_write;
}

/*******************************************************************************
* 
*******************************************************************************/
int TCPClientWaitForReceive (HTCPCLIENT hClient, char* inBuf, unsigned int inBufSize, PFNISCOMPLETEPACKET pfnIsCompletePacket, unsigned int timeoutMsecs)
{
 int num_actual_read;
 int bContinue = 1;
 char* ptr;
 unsigned int cursize;
 int ret;

 if (!hClient || !hClient->socketfd || !inBuf || !inBufSize) return 0;
 if (timeoutMsecs > TCPCLIENT_MAX_TIMEOUT_MSECS) timeoutMsecs = TCPCLIENT_MAX_TIMEOUT_MSECS;

 ptr = inBuf;
 cursize = 0;

 while (bContinue){
	num_actual_read = recv (hClient->socketfd, ptr, inBufSize - cursize, 0);
	
 	if (!num_actual_read){ /*When a stream socket peer has performed an orderly shutdown, the return value will be 0 (the traditional "end-of-file" return).*/
		/*fprintf(stdout, "num_actual_read = 0 after a recv, stopping...\n");*/
		bContinue = 0;
		break; 
	} else if (num_actual_read > 0){/*we got new data !*/
		cursize += num_actual_read;
		ptr		+= num_actual_read;
		if (pfnIsCompletePacket){
			ret = pfnIsCompletePacket(inBuf, cursize);
			switch (ret){
				case -1://discard everything until now
						ptr = inBuf;
						cursize = 0;
						continue;
				case 0://not enough data, continue
						continue;
				default://enough data, process it.
						return (cursize);
			}
		} else return (cursize);//just process everything immediately
	} else {
		/*TODO: negative value: check the error to see if we need to continue*/
#if (defined _MSC_VER) || (defined __MINGW32__)
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK){
			/*fprintf (stderr, "recv error= %d\n", err);*/
#elif defined ( __linux__ )
		if (errno == EWOULDBLOCK){
/*			fprintf (stderr, "recv error= %s\n", strerror(errno));*/
#endif
			return (0);
		} else if (errno == EAGAIN) {
			/*do nothing continue until there is data available*/
		} 
	}

	/*TODO; check timeout*/
 }

 return 0;
}

/*******************************************************************************
* 
*******************************************************************************/
void TCPClientDisconnect (HTCPCLIENT hClient)
{
 if (!hClient) return;

 /*fprintf (stdout, "disconnecting the socket...\n");*/

 /*disconnect gracefully*/
#if (defined _MSC_VER) || (defined __MINGW32__)
 shutdown (hClient->socketfd, SD_SEND);/*SHUT_WR*/
 shutdown (hClient->socketfd, SD_RECEIVE);/*SHUT_RD*/
#else
 shutdown (hClient->socketfd, SHUT_WR);
 shutdown (hClient->socketfd, SHUT_RD);
#endif

}

/*******************************************************************************
* 
*******************************************************************************/
void TCPClientCloseSocket (HTCPCLIENT hClient)
{
 if (!hClient) return;

 TCPClientDisconnect (hClient);
 
 /*fprintf (stdout, "closing the socket...\n");*/
 close (hClient->socketfd);
 hClient->socketfd = 0;
}

/*******************************************************************************
* 
*******************************************************************************/
int TCPClientConnect (HTCPCLIENT hClient, uint16_t port, char* hostname)
{
 int fd;
 int rc;
 struct sockaddr_in addr;
 struct hostent* phost;
 struct timeval tv;
 /*int tv;*/
#if (defined _MSC_VER) || (defined __MINGW32__)/*initialize winsock*/
 WSADATA wsaData;
 int iResult;

 iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
 if (iResult != 0) {
    /*fprintf(stderr, "WSAStartup failed with error: %d\n", iResult);*/
	return (0);	
 }
#endif

 if (!hClient) return (0);
 
 /*disconnect & close any previous connections*/
 if (hClient->socketfd) TCPClientCloseSocket (hClient);

 /*Create a new socket of type TYPE in domain DOMAIN, using protocol PROTOCOL.*/
 fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
 if (fd < 0) {
	/*fprintf (stderr, "error trying to get a file descriptor for a stream socket\n");*/
	return (0);
 }

 hClient->socketfd = fd;
 TCPClientSetRecvBufferSize(hClient, hClient->recvBufMaxSize);
 TCPClientSetSendBufferSize(hClient, hClient->sendBufMaxSize);
 //TCPClientDisableNagleAlgorithm  (hClient, hClient->disableNagle);

 /*set the address family struct*/
 memset ((void*)&addr, 0x00, sizeof(addr));
 addr.sin_family = AF_INET;
 phost = gethostbyname(hostname);
 if (!phost){
	/*fprintf (stderr, "gethostbyname() error\n");*/
	close (fd);
	hClient->socketfd = 0;
	return (0);
 }
 memcpy (&addr.sin_addr.s_addr, phost->h_addr_list[0], phost->h_length);
 addr.sin_port = htons(port);      /*port in network byte order*/

 /* Give the socket FD the local address ADDR (which is LEN bytes long)*/
 rc = connect (fd, (struct sockaddr *)&addr, sizeof(addr));
 if (rc < 0){
	/*fprintf (stderr, "error trying to connect the client socket to port %d\n", port);*/
	close (fd);
	hClient->socketfd = 0;
	return (0);
 }

 /* Put the socket in non-blocking mode:*/
 /*if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) < 0) {
	fprintf (stderr, "not possible to put socket in non-blocking mode... aborting\n");
	close (fd);
	return (0);
 }*/

 /*set the socket timeout*/
 tv.tv_sec = 10;
 tv.tv_usec = 0;
 /*tv = 1000;*/
 rc = setsockopt (fd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&tv, sizeof(tv));
 /*rc = setsockopt (fd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&tv, sizeof(tv));
 //setsockopt (fd, SO_SNDTIMEO -> default is zero, a send operation should not time out*/

 return (1);
}
     
/*******************************************************************************
* set the size of buffer allocated for outgoing data
*******************************************************************************/
void TCPClientSetRecvBufferSize (HTCPCLIENT hClient, unsigned int newSize)
{
 int rcvBuffSize, res;
 socklen_t optionLen;

 if (!hClient) return;
 
 if (newSize < TCPCLIENT_RECVBUFFER_MINIMUM_SIZE) newSize = TCPCLIENT_RECVBUFFER_MINIMUM_SIZE;

 if (hClient->socketfd){
	//Get the current SO_RCVBUF
 	optionLen = sizeof(rcvBuffSize);
 	res = getsockopt(hClient->socketfd, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBuffSize, &optionLen);
 	if(res == -1) printf("Error getting SO_RCVBUF\n");
 			else  printf("send buffer size (SO_RCVBUF)= %d\n", rcvBuffSize);

	//Change the SO_RCVBUF
	/*rcvBuffSize = newSize;
	res = setsockopt(hClient->socketfd, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBuffSize, sizeof(rcvBuffSize));
	if(res == -1) printf("Error trying to set SO_RCVBUF to %d\n", rcvBuffSize);
			else  printf("SO_RCVBUF set to %d\n", rcvBuffSize);*/
 }

 hClient->recvBufMaxSize = newSize;
}

/*******************************************************************************
* set the size of buffer allocated for outgoing data
*******************************************************************************/
void TCPClientSetSendBufferSize (HTCPCLIENT hClient, unsigned int newSize)
{
 int sendBuffSize, res;
 socklen_t optionLen;

 if (!hClient) return;

 if (newSize < TCPCLIENT_SENDBUFFER_MINIMUM_SIZE) newSize = TCPCLIENT_SENDBUFFER_MINIMUM_SIZE;

 if (hClient->socketfd){
	//Get the current SO_SNDBUF
	optionLen = sizeof(sendBuffSize);
	res = getsockopt(hClient->socketfd, SOL_SOCKET, SO_SNDBUF, (char*)&sendBuffSize, &optionLen);
	if(res == -1) printf("Error getting SO_SNDBUF\n");
			else  printf("send buffer size (SO_SNDBUF)= %d\n", sendBuffSize);

	//Change the SO_SNDBUF
	/*sendBuffSize = newSize;
	res = setsockopt(hClient->socketfd, SOL_SOCKET, SO_SNDBUF, (char*)&sendBuffSize, sizeof(sendBuffSize));
	if(res == -1) printf("Error trying to set SO_SNDBUF to %d\n", sendBuffSize);
 			else  printf("SO_SNDBUF set to %d\n", sendBuffSize);*/
 }

 hClient->sendBufMaxSize = newSize;
}

/*******************************************************************************
* Disables the Nagle algorithm (that is, the potential buffering of small requests befor sending)
*******************************************************************************/
void TCPClientDisableNagleAlgorithm (HTCPCLIENT hClient, int disableNagle)
{
 if (!hClient || (hClient->disableNagle == disableNagle)) return;

 if (hClient->socketfd){
	TCPDisableNagleAlgorithm (hClient->socketfd, disableNagle);
 }

 hClient->disableNagle = disableNagle;
}

/*******************************************************************************
* 
*******************************************************************************/
void TCPClientShutdown (HTCPCLIENT hClient)
{
 if (!hClient) return;
 
 /*disconnect & close the socket*/
 TCPClientCloseSocket (hClient);

 /*deallocate the recv & send buffers*/
 if (hClient->pRecvBuf) free (hClient->pRecvBuf);
 if (hClient->pSendBuf) free (hClient->pSendBuf);

#ifdef WIN32
	WSACleanup();
#endif

 free(hClient);
}

/*******************************************************************************
* allocate the handle to the TCP Client & init the defaults
*******************************************************************************/
HTCPCLIENT TCPClientInit (void)
{
 HTCPCLIENT hClient;

 hClient = (HTCPCLIENT)calloc (1, sizeof(TCPCLIENT_STRUCT));
 if (!hClient) return ((HTCPCLIENT)NULL);
 
 /*init of the Receive & Send buffers*/
 hClient->recvBufMaxSize = TCPCLIENT_RECVBUFFER_MINIMUM_SIZE;
 hClient->sendBufMaxSize = TCPCLIENT_SENDBUFFER_MINIMUM_SIZE;
 
 return (hClient);
}

#endif