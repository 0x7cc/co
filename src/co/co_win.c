
#if _WIN32

#include "co/co.h"
#include <windows.h>

typedef struct
{
  co_func func;
  void*   data;
} threadCtx;

static DWORD WINAPI thread_start_routine (LPVOID lpThreadParameter)
{
  threadCtx* ctx = (threadCtx*)lpThreadParameter;
  ctx->func (ctx->data);
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

#endif // WIN32
