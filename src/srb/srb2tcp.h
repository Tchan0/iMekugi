/*******************************************************************************
* Used to send SRB requests over TCP
*
*******************************************************************************/
#ifndef SRB2TCP_H_DEFINED__
#define SRB2TCP_H_DEFINED__

#if (defined _MSC_VER) || (defined __MINGW32__)
	typedef unsigned __int32 uint32_t;//for older VS versions, including stdint.h doesn't work...
	typedef unsigned __int8  uint8_t;//for older VS versions, including stdint.h doesn't work...
#else
	#include <stdint.h>
#endif

#include "srb.h"

#define TCPSRB_STRUCT_HEADERSIZE	12 //we need at least the magicId, size & ret before doing any analysis of completeness of the SRB packet
#define TCPSRB_MAGICID				"SRBT"
#define TCPSRB_STRUCT_EXTRA_EXPANSIONSIZE 20 //since we work with a 32/64/... pointer in SRB_ExecSCSICmd, the structure has a variable size

//#pragma pack(1)
typedef struct __attribute__ ((__packed__)) {
	uint32_t	magicId;	// TCPSRB_MAGICID
	uint32_t	size;		// length in bytes of the TCPSRB packet, including the full header (ie, the bytes stored in data[] + the 12 bytes of the header)
	uint32_t	ret;		// return code of the SRB dispatcher
	uint32_t    pointerSize; //size of a pointer = 4 bytes (32bits), 8 bytes (64bits), 16 (128bits),...
	uint8_t		data[1];	// can be any length
} TCPSRB, *PTCPSRB;
//#pragma pack()

#endif
