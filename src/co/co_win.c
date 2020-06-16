
#if _WIN32

#include "co/co.h"
#include "co_.h"
#include <windows.h>
#include <sys/timeb.h>

typedef struct
{
  co_func func;
  void*   data;
} threadCtx;

static DWORD WINAPI thread_start_routine (LPVOID lpThreadParameter) {
  threadCtx* ctx = (threadCtx*)lpThreadParameter;

  co_thread_init ();

  ctx->func (ctx->data);

  co_run ();

  co_thread_cleanup ();

  co_free (ctx);

  return 0;
}

co_int co_thread_create (co_func func, void* data) {
  threadCtx* ctx = (threadCtx*)co_alloc (sizeof (threadCtx));
  ctx->func      = func;
  ctx->data      = data;
  return (co_int)CreateThread (NULL, NULL, thread_start_routine, ctx, NULL, NULL);
}

co_int co_thread_join (co_int tid) {
  return WaitForSingleObject ((HANDLE)tid, INFINITE);
}

void co_thread_init_native (co_thread_context_t* ctx) {}

void co_thread_cleanup_native (co_thread_context_t* ctx) {}

void co_tls_init (co_int* key) {
  *key = TlsAlloc ();
}

void co_tls_cleanup (co_int key) {
  TlsFree (key);
}

void* co_tls_get (co_int key) {
  return TlsGetValue (key);
}

void co_tls_set (co_int key, void* value) {
  TlsSetValue (key, value);
}

co_uint64 co_timestamp_ms () {
  struct timeb rawtime;
  ftime (&rawtime);
  return rawtime.time * 1000 + rawtime.millitm;
}

void co_run () {
  co_thread_context_t* ctx = co_get_context ();

  while (ctx->num_of_active)
    co_yield_ ();
}

void co_init_hooks () {
}

#endif // WIN32
