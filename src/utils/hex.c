/*******************************************************************************
* Functions related to Hexadecimal
*
*******************************************************************************/
#include "hex.h"


/*******************************************************************************
* TODO
*******************************************************************************/
/* cfr gdb - rsp.c:
const static char hextab[] = "0123456789ABCDEF";//move to /utils/hex.h  BYTE2HEXASCII(c)  (hextab[(c>>4) & 0x000F]...)
#define ASCIIHEX2INT(x) ((x >= 0x30) && (x<= 0x39) ? (x - 0x30) : ((x >= 0x41) && (x <= 0x5A)) ? (x - 0x41 + 10): (x >= 0x61) && (x <= 0x7A) ? (x - 0x61 +10 ) : 0x00)
uint32_t convertAsciiHex2Binary (char* outBuf, char* srcBuf, uint32_t length)
//also: strol, gdbPrint2DigitHexResponse


*/