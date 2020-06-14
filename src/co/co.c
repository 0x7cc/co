
#include "co/co.h"
#include "co/co_private.h"
#include <string.h>
#include <stdlib.h>

co_int tls_key_thread_ctx;

/**
 * 携程执行结束返回时的处理函数
 */
static void co_exited (void* result) {
  co_thread_context_t* threadCtx  = co_get_context ();
  threadCtx->task_current->status = CO_TASK_STATUS_COMPLETED;
  threadCtx->task_current->result = result;
  co_yield_ ();
}

co_int co_init () {
  co_tls_init (&tls_key_thread_ctx);
  co_init_hooks ();
  return 0;
}

void co_cleanup () {
  co_tls_cleanup (tls_key_thread_ctx);
  tls_key_thread_ctx = 0;
}

void co_thread_init () {
  co_thread_context_t* threadCtx = co_calloc (sizeof (co_thread_context_t));
  co_tls_set (tls_key_thread_ctx, threadCtx);

  threadCtx->list_active = (co_task_t*)co_calloc (sizeof (co_task_t));

  threadCtx->list_active->prev = threadCtx->list_active;
  threadCtx->list_active->next = threadCtx->list_active;
  threadCtx->list_active_last  = threadCtx->list_active;
  threadCtx->task_current      = threadCtx->list_active;
  threadCtx->num_of_active     = 0;

  co_thread_init_native (threadCtx);
}

void co_thread_cleanup () {
  co_thread_context_t* threadCtx = co_get_context ();
  co_thread_cleanup_native (threadCtx);

  co_free (threadCtx->list_active);
  co_free (threadCtx);
  co_tls_set (tls_key_thread_ctx, NULL);
}

co_task_t* co_task_add (co_func func, void* data, co_uint stackSize) {
  co_thread_context_t* threadCtx = co_get_context ();
  register co_task_t*  task      = (co_task_t*)co_calloc (sizeof (co_task_t));
  register co_task_t*  last      = threadCtx->list_active_last;

  stackSize = co_max (stackSize, CO_MINIMAL_STACK_SIZE) & 0xFFFFFFFFFFFFFFF0;

  task->stack     = co_calloc (stackSize);
  task->ctx.argv0 = (co_uint)data;
  task->ctx.ip    = (co_uint)func;
  task->status    = CO_TASK_STATUS_READY;

  // Windows: 经测试，Windows平台需要32-byte栈底空间，否则会发生堆溢出问题，原因不详.
  // macOS: 根据苹果官方文档，这里理应是16-byte对齐，但我的切换context是用jmp做跳转，没有call的压栈操作，所以这里就要是8的单数倍.See: https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/LowLevelABI/130-IA-32_Function_Calling_Conventions/IA32.html
  task->ctx.sp                     = ((((co_uint)task->stack) + stackSize - 32) & 0xFFFFFFFFFFFFFFF0) - 8;
  *((co_uint*)(task->ctx.sp + 16)) = (co_uint)co_exited; // 玛德智障MSVC，函数中的参数居然会保存到当前函数栈之外(rsp + xxx).由于我只使用了一个参数,所以只被占用了rsp+8 位置的8个字节.
  *((co_uint*)(task->ctx.sp))      = (co_uint)co_exited_asm;

  co_add_task_to_active (task);

  return task;
}

void co_task_del (co_task_t* task) {
  task->status |= CO_TASK_STATUS_INTERRUPTED;
}

void* co_task_await (co_task_t* task) {
  while ((task->status & (CO_TASK_STATUS_COMPLETED | CO_TASK_STATUS_INTERRUPTED)) == 0)
    co_yield_ ();

  if (task->status & CO_TASK_STATUS_COMPLETED)
    return (void*)task->result;

  return nullptr;
}

void co_run () {
  co_thread_context_t* threadCtx = co_get_context ();

  while (threadCtx->num_of_active || threadCtx->num_of_waiting) {
    co_yield_ ();
  }
}

void co_yield_ () {
  co_thread_context_t* ctx = co_get_context ();

  register co_task_t* const task = ctx->task_current;
  register co_task_t*       active = task->next;
  register co_task_t*       waiting = ctx->list_active;
  register co_uint64        now  = co_timestamp_ms ();

  while (1) {
  repeat:
    switch (active->status) {
      case CO_TASK_STATUS_INTERRUPTED:
      case CO_TASK_STATUS_COMPLETED: {
        co_task_t* t = active;
        active       = active->next;

        co_remove_task_from_active (t);

        co_free (t->stack);
        co_free (t);
      }
        goto repeat;
      case CO_TASK_STATUS_WAITING: {
        co_task_t* t = active;
        active       = active->next;
        co_remove_task_from_active (t);
        co_add_task_to_waiting (t);
      }
        goto repeat;
      default:
        break;
    }
    break;
  }

  ctx->task_current = active;

  co_swap_context (&(task->ctx), &(active->ctx));
}

void* co_alloc (co_uint size) {
  return malloc (size);
}

void* co_calloc (co_uint size) {
  return calloc (1, size);
}

void co_free (void* ptr) {
  free (ptr);
}

void co_cond_init (co_int* key) {
}

void co_cond_cleanup (co_int key) {
}

void co_cond_wait (co_int key, co_int timeout) {
  co_thread_context_t* ctx   = co_get_context ();
  ctx->task_current->timeout = co_timestamp_ms () + timeout;
  ctx->task_current->status |= CO_TASK_STATUS_WAITING;
  co_yield_ ();
  ctx->task_current->status &= ~CO_TASK_STATUS_WAITING;
}

void co_cond_notify_one (co_int key) {
}

void co_cond_notify_all (co_int key) {
}
