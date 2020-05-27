
#include "co.h"
#include <string.h>
#include <stdlib.h>

typedef struct co_task_context_s
{
  co_uint rax;
  co_uint rbx;
  co_uint rcx;
  co_uint rdx;
  co_uint rsi;
  co_uint rdi;
  co_uint rsp;
  co_uint rbp;
  co_uint rip;
#if __x86_64__ || _WIN64
  co_uint64 r8;
  co_uint64 r9;
  co_uint64 r12;
  co_uint64 r13;
  co_uint64 r14;
  co_uint64 r15;
#endif
} co_task_context_t;

typedef struct co_task_s
{
  struct co_task_s* next;
  void*             stack;
  co_task_context_t ctx;
} co_task_t;

typedef struct co_thread_context_s
{
  /***
   * 循环链表
   */
  co_task_t* task_head;
  co_task_t* task_end;
  co_task_t* task_current;
} co_thread_context_t;

static thread_local co_thread_context_t threadCtx;

static void co_task_stack_push (co_task_t* task, co_int value)
{
  task->ctx.rsp -= sizeof (co_int);
  *((co_int*)task->ctx.rsp) = value;
}

/**
 * 切换上下文(由汇编实现)
 * @param store    协程的context
 * @param load     协程的context
 * @return
 */
extern co_int co_swap_context (co_task_context_t* store, co_task_context_t* load);

/**
 * 保存上下文, 执行后将会执行协程.(由汇编实现)
 * @return
 */
extern co_int co_store_context (co_task_context_t* ctx);

/**
 * 加载上下文, 执行后将会执行协程.(由汇编实现)
 * @return
 */
extern co_int co_load_context (co_task_context_t* ctx);

co_int co_enable ()
{
  memset (&threadCtx, 0, sizeof (co_thread_context_t));
  threadCtx.task_head = (co_task_t*)calloc (1, sizeof (co_task_t));

  threadCtx.task_head->next = threadCtx.task_head;
  threadCtx.task_end        = threadCtx.task_head;
  return 0;
}

static co_int co_task_finished (co_task_t* task)
{
  return 0;
}

co_int co_add (co_func func, void* data)
{
  co_task_t*   task       = (co_task_t*)calloc (1, sizeof (co_task_t));
  co_task_t*   node       = threadCtx.task_end;
  const size_t stack_size = 1024 * 2048; // 每个协程给2MiB的栈空间

  task->next      = threadCtx.task_head;
  task->stack     = calloc (1, stack_size);
  task->ctx.argv0 = (co_uint)data;
  task->ctx.rip   = (co_uint)func;
  task->ctx.rsp = task->ctx.rbp = (co_uint) (task->stack) + stack_size;
  co_task_stack_push (task, (co_int)co_task_finished);

  threadCtx.task_end = node->next = task;

  return 0;
}

co_int co_wait ()
{
  register co_task_t* L;
  register co_task_t* R;
  register co_task_t* task;

  co_store_context (&(threadCtx.task_head->ctx));

  if (threadCtx.task_head->next == nullptr)
    return 0;

  L    = threadCtx.task_head;
  task = L->next;

  while (task)
  {
    if (task == threadCtx.task_head)
      return 0;

    R = task->next;

    threadCtx.task_current = task;
    co_load_context (&(task->ctx));

    // 协程运行结束返回了,释放内存.
    L->next = R;
    free (task);

    task = R;
  }
  return 0;
}

co_int co_yield()
{
  co_task_t* current = threadCtx.task_current;
  co_task_t* next    = current->next;
  if (next == threadCtx.task_head)
    next = next->next;

  if (next == current)
    return 0; // 只有当前协程了,没必要切换.

  threadCtx.task_current = next;
  co_swap_context (&(current->ctx), &(next->ctx));
  return 0;
}
