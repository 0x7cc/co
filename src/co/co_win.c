
#if WIN32

#include "co/co.h"
#include <windows.h>

co_int co_thread_create (co_func func, void* data)
{
  return CreateThread (NULL, NULL, func, data, NULL, NULL);
}

co_int co_thread_join (co_int tid)
{
  return WaitForSingleObject (tid, INFINITE);
}

#endif // WIN32
