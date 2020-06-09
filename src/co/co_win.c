
#if _WIN32

#include "co/co.h"
#include "co/co_private.h"
#include <windows.h>

static DWORD WINAPI thread_start_routine (LPVOID lpThreadParameter)
{
  threadCtx* ctx = (threadCtx*)lpThreadParameter;

  co_thread_init ();

  ctx->func (ctx->data);
  co_thread_run ();

  co_thread_cleanup ();

  co_free (ctx);

  return 0;
}

co_int co_thread_create (co_func func, void* data)
{
  threadCtx* ctx = (threadCtx*)co_alloc (sizeof (threadCtx));
  ctx->func      = func;
  ctx->data      = data;
  return (co_int)CreateThread (NULL, NULL, thread_start_routine, ctx, NULL, NULL);
}

co_int co_thread_join (co_int tid)
{
  return WaitForSingleObject ((HANDLE)tid, INFINITE);
}

void co_tls_init (co_int* key)
{
  *key = TlsAlloc ();
}

void co_tls_cleanup (co_int key)
{
  TlsFree (key);
}

void* co_tls_get (co_int key)
{
  return TlsGetValue (key);
}

void co_tls_set (co_int key, void* value)
{
  TlsSetValue (key, value);
}

void co_init_hooks ()
{
}

#endif // WIN32
