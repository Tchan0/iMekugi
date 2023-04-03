/*******************************************************************************
* Helper functions to log things
*
*******************************************************************************/
#include "logger.h"

/*******************************************************************************
*
*******************************************************************************/
void loggerPrintf (PLOGGER pLogger, int loglevel, char *format, ...)
{
 va_list args;

 if (!pLogger || !pLogger->fout) return; 
 if (pLogger->logLevel < loglevel) return;
 if (pLogger->currentLogFileSize >= pLogger->logFileSizeMax) return;

 va_start(args, format);
 vfprintf(pLogger->fout, format, args);
 va_end(args);

 pLogger->currentLogFileSize = fseek(pLogger->fout, 0, SEEK_CUR);
}

/*******************************************************************************
* 
*******************************************************************************/
void loggerPrintHexDump (PLOGGER pLogger, int loglevel, uint8_t* pdata, uint32_t size_in_bytes, uint8_t swapmode, uint8_t colwidth)
{
 if (!pLogger || !pLogger->fout) return; 
 if (pLogger->logLevel < loglevel) return;
 if (pLogger->currentLogFileSize >= pLogger->logFileSizeMax) return;

 printHexDump (pLogger->fout, pdata, size_in_bytes, swapmode, colwidth);

 pLogger->currentLogFileSize = fseek(pLogger->fout, 0, SEEK_CUR);
}

/*******************************************************************************
* 
*******************************************************************************/
void loggerSetConfig (PLOGGER pLogger, int logLevel, int logFileSizeMax)
{
 if (!pLogger) return;

 pLogger->logFileSizeMax = logFileSizeMax;
 pLogger->logLevel       = logLevel;
}

/*******************************************************************************
* 
*******************************************************************************/
void loggerClose(PLOGGER pLogger, int bFreeMemory)
{
 if (!pLogger) return;

 if ((pLogger->fout != stdout) && pLogger->fout) fclose(pLogger->fout);

 if (bFreeMemory) free((void*)pLogger);
}

/*******************************************************************************
* 
*******************************************************************************/
PLOGGER loggerOpen (char* filename, int logLevel, int logFileSizeMax)
{
 PLOGGER ptmp;
 FILE*   fout;

 logLevel = logLevel > LOGLEVEL_MAX ? LOGLEVEL_MAX : logLevel;
 
 ptmp = (PLOGGER)calloc (1, sizeof(LOGGER_STRUCT));
 if (!ptmp) return ((PLOGGER)NULL);

 if (filename && filename[0]){
	fout = fopen(filename, "wb");
 	if (!fout){
  		loggerClose(ptmp, 1);
  		return ((PLOGGER)NULL);
 	}
 } else fout = stdout;

 ptmp->fout           = fout;
 ptmp->logLevel       = logLevel;
 ptmp->logFileSizeMax = logFileSizeMax;
 strncpy(ptmp->logfilename, filename, 1024);
 
 return ptmp;
}

/*******************************************************************************
* 
*******************************************************************************/
PLOGGER loggerFileOpen (PLOGGER pLogger)
{
 FILE* fout;
 
 if (!pLogger) return ((PLOGGER)NULL);

 if (pLogger->logfilename[0]){
	fout = fopen(pLogger->logfilename, "wb");
	if (!fout){
		fprintf(stdout, "Unable to open log file %s.\r\n", pLogger->logfilename);
		loggerClose(pLogger, 0);
		return ((PLOGGER)NULL);
	} else {
		fprintf(stdout, "log file %s opened.\r\n", pLogger->logfilename);
 	}
	pLogger->fout = fout;
 } else {
	pLogger->fout = stdout;
 }
  
 pLogger->logLevel = pLogger->logLevel > LOGLEVEL_MAX ? LOGLEVEL_MAX : pLogger->logLevel;

 return (pLogger);
}
