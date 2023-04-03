/*******************************************************************************
* Functions related to endianness (convert LSB<->MSB, ...)
*
* functions to use in your code:
*  BIG-ENDIAN CONVERSION
*    Convert from host (eg little-endian) to TCP/IP network byte order (which is big-endian)
*      uint32_t htonl (uint32_t hostlong);
*      uint16_t htons (uint16_t hostshort);
*    Convert from TCP/IP network byte order to host byte order (which is little-endian on Intel processors)
*      uint32_t ntohl (uint32_t netlong);
*      uint16_t ntohs (uint16_t netshort);
*  LITTLE-ENDIAN CONVERSION
*    TODO maybe ? add functions to convert to little endian ?  ltohl ltohs htoll htols
*******************************************************************************/
#ifndef __UTILS_ENDIAN_H_DEFINED__
#define __UTILS_ENDIAN_H_DEFINED__

#include <stdint.h> /* for uint16_t, ...*/

#if defined( __linux__ ) 
	#include <arpa/inet.h>
	/*note: #include <endian.h> has similar functions:
	    be32toh	<-> ntohl
		be16toh	<-> ntohs
		htobe32	<-> htonl
		htobe16	<-> htons
		, but those do not exist on Windows 	*/

#else /* Windows _WIN32 _WIN64 */

	#include <winsock2.h> /* note: #define _WINSOCKAPI_  before the <windows.h> include to avoid duplicate definitions... and undefine it right after the windows.h include*/
	#pragma comment(lib,"ws2_32.lib") /*Winsock Library*/

#endif

#endif