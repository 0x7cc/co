
#if __unix__ || __APPLE__

#include "co/co.h"
#include <pthread.h>

co_int co_thread_create (co_func func, void* data)
{
  pthread_t t;
  pthread_create (&t, NULL, (void* (*)(void*))func, data);
  return (co_int) t;
}

co_int co_thread_join (co_int tid)
{
  return pthread_join ((pthread_t) tid, nullptr);
}

#endif // __linux
