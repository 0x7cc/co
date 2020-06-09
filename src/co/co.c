
#include "co/co.h"
#include "co/co_private.h"
#include <string.h>
#include <stdlib.h>

co_int tls_key_thread_ctx;

/**
 * 携程执行结束返回时的处理函数
 */
static void co_exited (void* result)
{
  co_thread_context_t* threadCtx  = co_tls_get (tls_key_thread_ctx);
  threadCtx->task_current->status = CO_TASK_STATUS_COMPLETED;
  threadCtx->task_current->result = result;
  co_yield_ ();
}

co_int co_init ()
{
  co_tls_init (&tls_key_thread_ctx);
  co_init_hooks ();
  return 0;
}

void co_cleanup ()
{
  co_tls_cleanup (tls_key_thread_ctx);
  tls_key_thread_ctx = 0;
}

void co_thread_init ()
{
  co_thread_context_t* threadCtx = co_calloc (sizeof (co_thread_context_t));
  co_tls_set (tls_key_thread_ctx, threadCtx);

  threadCtx->task_head = (co_task_t*)co_calloc (sizeof (co_task_t));

  threadCtx->task_head->prev   = threadCtx->task_head;
  threadCtx->task_head->next   = threadCtx->task_head;
  threadCtx->task_last         = threadCtx->task_head;
  threadCtx->task_current      = threadCtx->task_head;
  threadCtx->num_of_coroutines = 0;
}

void co_thread_cleanup ()
{
  co_thread_context_t* threadCtx = co_tls_get (tls_key_thread_ctx);
  co_free (threadCtx->task_head);
  co_free (threadCtx);
  co_tls_set (tls_key_thread_ctx, NULL);
}

co_task_t* co_task_add (co_func func, void* data, co_uint stackSize)
{
  co_thread_context_t* threadCtx = co_tls_get (tls_key_thread_ctx);
  register co_task_t*  task      = (co_task_t*)co_calloc (sizeof (co_task_t));
  register co_task_t*  last      = threadCtx->task_last;

  stackSize = co_max (stackSize, CO_MINIMAL_STACK_SIZE) & 0xFFFFFFFFFFFFFFF0;

  task->prev      = last;
  task->next      = threadCtx->task_head;
  task->stack     = co_calloc (stackSize);
  task->ctx.argv0 = (co_uint)data;
  task->ctx.ip    = (co_uint)func;
  task->status    = CO_TASK_STATUS_READY;

  // Windows: 经测试，Windows平台需要32-byte栈底空间，否则会发生堆溢出问题，原因不详.
  // macOS: 根据苹果官方文档，这里理应是16-byte对齐，但我的切换context是用jmp做跳转，没有call的压栈操作，所以这里就要是8的单数倍.See: https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/LowLevelABI/130-IA-32_Function_Calling_Conventions/IA32.html
  task->ctx.sp                     = ((((co_uint)task->stack) + stackSize - 32) & 0xFFFFFFFFFFFFFFF0) - 8;
  *((co_uint*)(task->ctx.sp + 16)) = (co_uint)co_exited; // 玛德智障MSVC，函数中的参数居然会保存到当前函数栈之外(rsp + xxx).由于我只使用了一个参数,所以只被占用了rsp+8 位置的8个字节.
  *((co_uint*)(task->ctx.sp))      = (co_uint)co_exited_asm;
  last->next->prev                 = task;
  last->next                       = task;
  threadCtx->task_last             = task;

  ++threadCtx->num_of_coroutines;

  return task;
}

void co_task_del (co_task_t* task)
{
  task->status |= CO_TASK_STATUS_INTERRUPTED;
}

void* co_task_await (co_task_t* task)
{
  while ((task->status & (CO_TASK_STATUS_COMPLETED | CO_TASK_STATUS_INTERRUPTED)) == 0)
    co_yield_ ();

  if (task->status & CO_TASK_STATUS_COMPLETED)
    return (void*)task->result;

  return nullptr;
}

void co_run ()
{
  co_thread_context_t* threadCtx = co_tls_get (tls_key_thread_ctx);

  while (threadCtx->num_of_coroutines)
    co_yield_ ();
}

void co_yield_ ()
{
  co_thread_context_t*      threadCtx = co_tls_get (tls_key_thread_ctx);
  register co_task_t* const task      = threadCtx->task_current;
  register co_task_t*       next      = task->next;

  while (next->status & (CO_TASK_STATUS_INTERRUPTED | CO_TASK_STATUS_COMPLETED))
  {
    co_task_t* t = next;
    next         = next->next;

    if (t == threadCtx->task_last)
      threadCtx->task_last = t->prev;

    t->prev->next = t->next;
    t->next->prev = t->prev;
    co_free (t->stack);
    co_free (t);

    --threadCtx->num_of_coroutines;
  }

  threadCtx->task_current = next;

  co_swap_context (&(task->ctx), &(next->ctx));
}

void* co_alloc (co_uint size)
{
  return malloc (size);
}

void* co_calloc (co_uint size)
{
  return calloc (1, size);
}

void co_free (void* ptr)
{
  free (ptr);
}
