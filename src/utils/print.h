/*******************************************************************************
* Helper functions to print things
*
*******************************************************************************/
#ifndef __UTILS_PRINT_H_DEFINED__
#define __UTILS_PRINT_H_DEFINED__

#include <stdio.h>
#include <stdarg.h> /*for va_start, va_end*/
#include <stdint.h> /*for uint8_t, ...*/

void printHexDump (FILE* fout, uint8_t* pdata, uint32_t size_in_bytes, uint8_t swapmode, uint8_t colwidth);

#endif