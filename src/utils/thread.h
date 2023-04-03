/*******************************************************************************
* Helper methods to make child threads
* 
*******************************************************************************/
#ifndef __UTILS_THREAD_H_DEFINED__
#define __UTILS_THREAD_H_DEFINED__


#if defined( __linux__ ) 
  typedef void* (*PFNTHREADSTARTFUNC)(void*);
#else
  typedef DWORD WINAPI (*PFNTHREADSTARTFUNC)(LPVOID);
#endif

int threadCreate (PFNTHREADSTARTFUNC pfnStart, void* pData);

#endif