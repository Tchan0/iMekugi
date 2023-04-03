/*******************************************************************************
* SPTI-specific defines
* https://en.wikipedia.org/wiki/SCSI_Pass_Through_Interface
*******************************************************************************/
#ifndef SPTI_H_DEFINED__
#define SPTI_H_DEFINED__

#define _WINSOCKAPI_ // stops windows.h from including winsock.h, since we will include winsock2.h later via the TCP code if needed...
#include <windows.h>
#undef _WINSOCKAPI_  //to avoid warning later on that "winsock2.h should be included before windows.h"

#endif
