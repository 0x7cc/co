
#if __unix__ || __linux__ || __APPLE__

#include "co/co.h"
#include "co/co_private.h"
#include <pthread.h>
#include <sys/time.h>

typedef struct
{
  co_func func;
  void*   data;
} threadCtx;

static void* thread_start_routine (void* data) {
  threadCtx* ctx = (threadCtx*)data;

  co_thread_init ();

  ctx->func (ctx->data);

  co_run ();

  co_thread_cleanup ();

  co_free (ctx);

  return 0;
}

co_int co_thread_create (co_func func, void* data) {
  pthread_t  t;
  threadCtx* ctx = (threadCtx*)co_alloc (sizeof (threadCtx));
  ctx->func      = func;
  ctx->data      = data;
  pthread_create (&t, NULL, thread_start_routine, ctx);
  return (co_int)t;
}

co_int co_thread_join (co_int tid) {
  return pthread_join ((pthread_t)tid, nullptr);
}

void co_tls_init (co_int* key) {
  pthread_key_create ((pthread_key_t*)key, NULL);
}

void co_tls_cleanup (co_int key) {
  pthread_key_delete (key);
}

void* co_tls_get (co_int key) {
  return pthread_getspecific (key);
}

void co_tls_set (co_int key, void* value) {
  pthread_setspecific (key, value);
}

co_uint64 co_timestamp_ms () {
  struct timeval now = {0};
  gettimeofday (&now, NULL);
  unsigned long long u = now.tv_sec;
  u *= 1000;
  u += now.tv_usec / 1000;
  return u;
}

#endif // __linux
