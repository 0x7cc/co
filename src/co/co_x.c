
#if __unix__ || __APPLE__

#include "co/co.h"
#include "co/co_private.h"
#define _GNU_SOURCE 1
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>

typedef int (*sys_puts_t) (const char* s);
typedef ssize_t (*sys_recv_t) (int sockfd, void* buf, size_t len, int flags);
typedef ssize_t (*sys_send_t) (int sockfd, const void* buf, size_t len, int flags);

struct
{
  sys_puts_t sys_puts;
  sys_recv_t sys_recv;
  sys_send_t sys_send;
} hooks;

static void* thread_start_routine (void* data)
{
  threadCtx* ctx = (threadCtx*)data;

  co_thread_init ();

  ctx->func (ctx->data);

  co_run ();

  co_thread_cleanup ();

  co_free (ctx);

  return 0;
}

co_int co_thread_create (co_func func, void* data)
{
  pthread_t  t;
  threadCtx* ctx = (threadCtx*)co_alloc (sizeof (threadCtx));
  ctx->func      = func;
  ctx->data      = data;
  pthread_create (&t, NULL, thread_start_routine, ctx);
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

#if CO_ENABLE_HOOKS

ssize_t recv (int sockfd, void* buf, size_t len, int flags)
{
  co_yield_ ();
  return hooks.sys_recv (sockfd, buf, len, flags);
}

ssize_t send (int sockfd, const void* buf, size_t len, int flags)
{
  co_yield_ ();
  return hooks.sys_send (sockfd, buf, len, flags);
}

int puts (const char* s)
{
  co_yield_ ();
  return hooks.sys_puts (s);
}

#endif

void co_init_hooks ()
{
#if CO_ENABLE_HOOKS
  hooks.sys_puts = (sys_puts_t)dlsym (RTLD_NEXT, "puts");
  hooks.sys_recv = (sys_recv_t)dlsym (RTLD_NEXT, "recv");
  hooks.sys_send = (sys_send_t)dlsym (RTLD_NEXT, "send");
#endif
}

#endif // __linux
