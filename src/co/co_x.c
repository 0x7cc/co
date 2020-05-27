
#if __unix

#include "co/co.h"
#include <pthread.h>

co_extern co_int co_thread_create (co_func func, void* data)
{
  pthread_t t;
  pthread_create (&t, NULL, func, data);
  return t;
}

co_extern co_int co_thread_join (co_int tid)
{
  return pthread_join (tid, nullptr);
}

#endif // __linux
