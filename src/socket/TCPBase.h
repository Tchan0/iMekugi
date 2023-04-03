/*******************************************************************************
* Base defines for TCP clients & servers
* 
*******************************************************************************/
#ifndef __TCPBASE_H_DEFINED__
#define __TCPBASE_H_DEFINED__

#include <stdio.h>  /*for fprintf & stderr*/
#include <string.h> /*for memset*/
#include <stdlib.h> /*for calloc*/
#include <stdint.h> /*for uint16_t, ...*/
#include <errno.h>  /*to read the error returned by accept(), recv(), ...*/

#if (defined _MSC_VER) || (defined __MINGW32__)
	#include <winsock2.h> /* note: #define _WINSOCKAPI_  before the <windows.h> include to avoid duplicate definitions... and undefine it right after the windows.h include*/
	#pragma comment(lib,"ws2_32.lib") /*Winsock Library*/
	typedef int socklen_t; //#include <ws2tcpip.h> /* for socklen_t */
	#define close closesocket /*winsock does not define/use close() for sockets*/
#elif defined( __linux__ )
	#include <sys/socket.h>	 /*for socket, connect*/
	#include <netinet/in.h>	 /*for struct sockaddr_in, struct sockaddr*/
	#include <unistd.h>      /*for read, write, close*/
	#include <netinet/tcp.h> /*for TCP_NODELAY*/
#endif

#define TCP_MINIMUM_BUFSIZE 2048
/*#define TCP_MAXIMUM_BUFSIZE 2048*/

/*your app specifies what is a complete packet (ie, set of data). 
  This function is called whenever data is received by the server, and can also be used by a client if desired.
  - return  0 if you want to continue buffering the data
  - return >0(x) if you want to send the first x bytes from pData to your pfnCmdProcessingCallback
  - return <0 if you want to discard all the data received until now
 if not provided, whatever is received will be send to pfnCmdProcessingCallback*/
typedef int (*PFNISCOMPLETEPACKET)(char* pData, unsigned int dataSizeInBytes);

void TCPDisableNagleAlgorithm (int socketfd, int disableNagle);

#endif