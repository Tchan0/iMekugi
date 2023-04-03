/*******************************************************************************
* Helper functions to log things
*
*******************************************************************************/
#ifndef __UTILS_LOGGER_H_DEFINED__
#define __UTILS_LOGGER_H_DEFINED__

#include <stdio.h>
#include <stdarg.h> /*for va_start, va_end*/
#include <stdint.h> /*for uint8_t, ...*/
#include <stdlib.h> /*for free()*/
#include <string.h>

#include "print.h"

#define LOGLEVEL_NONE 0 /*no logging*/
#define LOGLEVEL_CORE 1
#define LOGLEVEL_LOW  2
#define LOGLEVEL_HIGH 3
#define LOGLEVEL_MAX  LOGLEVEL_HIGH

#define LOGPRINT_CORE(logger, ...) loggerPrintf(logger, LOGLEVEL_CORE, __VA_ARGS__)
#define LOGPRINT_LOW(logger, ...)  loggerPrintf(logger, LOGLEVEL_LOW,  __VA_ARGS__)
#define LOGPRINT_HIGH(logger, ...) loggerPrintf(logger, LOGLEVEL_HIGH, __VA_ARGS__)

typedef struct LOGGER_STRUCT {
	FILE* fout;
	int   currentLogFileSize;
	int   logFileSizeMax;
	int   logLevel;
	uint32_t	type;
	char		logfilename[1024];
} LOGGER_STRUCT, *PLOGGER;

PLOGGER loggerOpen (char* filename, int logLevel, int logFileSizeMax);
PLOGGER loggerFileOpen (PLOGGER pLogger);
void loggerClose(PLOGGER pLogger, int bFreeMemory);
void loggerSetConfig (PLOGGER pLogger, int logLevel, int logFileSizeMax);
void loggerPrintf(PLOGGER pLogger, int loglevel, char *format, ...);
void loggerPrintHexDump (PLOGGER pLogger, int loglevel, uint8_t* pdata, uint32_t size_in_bytes, uint8_t swapmode, uint8_t colwidth);

#endif