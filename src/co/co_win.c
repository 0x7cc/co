
#if _WIN32

#include "co/co.h"
#include <windows.h>

co_int co_thread_create (co_func func, void* data)
{
  return (co_int)CreateThread (NULL, NULL, (LPTHREAD_START_ROUTINE)func, data, NULL, NULL);
}

co_int co_thread_join (co_int tid)
{
  return WaitForSingleObject ((HANDLE)tid, INFINITE);
}

#endif // WIN32
