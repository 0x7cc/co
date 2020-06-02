
#if __unix__ || __APPLE__

#include "co/co.h"
#include <pthread.h>

co_int co_thread_create (co_func func, void* data)
{
  pthread_t t;
  pthread_create (&t, NULL, (void* (*)(void*))func, data);
  return (co_int)t;
}

co_int co_thread_join (co_int tid)
{
  return pthread_join ((pthread_t)tid, nullptr);
}

void co_tls_init (co_int* key)
{
  pthread_key_create ((pthread_key_t*)key, NULL);
}

void co_tls_cleanup (co_int key)
{
  pthread_key_delete (key);
}

void* co_tls_get (co_int key)
{
  return pthread_getspecific (key);
}

void co_tls_set (co_int key, void* value)
{
  pthread_setspecific (key, value);
}

#endif // __linux
