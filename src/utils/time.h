/*******************************************************************************
* Helper functions - Time-related
*
*******************************************************************************/
#ifndef __UTILS_TIME_H_DEFINED__
#define __UTILS_TIME_H_DEFINED__


#if defined( __linux__ ) 
  #include <unistd.h>
  #define SleepMsecs(x)  usleep(x * 1000)
#else
  #define SleepMsecs(x)  Sleep(x)
#endif

#endif