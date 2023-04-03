/*******************************************************************************
* Helper methods to make child threads
* 
*******************************************************************************/
#include "thread.h"

/*******************************************************************************
* Linux thread creation
*
* Compile and link with   -lpthread
*
* https://www.thegeekstuff.com/2012/04/create-threads-in-linux/
*******************************************************************************/
#if defined( __linux__ ) 

#include <pthread.h>

int threadCreate (PFNTHREADSTARTFUNC pfnStart, void* pData)
{
 pthread_t pThreadId;
 int ret;

 if (!pfnStart) return 0;
 
 ret = pthread_create (&pThreadId, NULL, pfnStart, pData);
 
 return (ret ? 0 : 1);//On success, pthread_create() returns 0
}
#endif

/*******************************************************************************
* Windows thread creation
*
*******************************************************************************/
#if defined( WIN32 ) 
int threadCreate (PFNTHREADSTARTFUNC pfnStart, void* pData)
{
 HANDLE hret;
 
 if (!pfnStart) return 0;

 hret = CreateThread ((LPSECURITY_ATTRIBUTES)NULL, 0, 
 	(LPTHREAD_START_ROUTINE)pfnStart, (LPVOID)pData, 0, (LPDWORD)NULL);

 return (hret ? 1 : 0 );//If the function fails, the return value is NULL.
}
#endif
