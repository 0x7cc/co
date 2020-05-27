
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
  struct co_task_s* prev;
  struct co_task_s* next;
  void*             stack;
  co_task_context_t ctx;
} co_task_t;

typedef struct co_thread_context_s
{
  /***
   * 循环链表的头节点
   */
  co_task_t* task_head;

  /**
   * 尾指针，方便快速添加新协程
   */
  co_task_t* task_last;

  /**
   * 当前运行的任务指针
   */
  co_task_t* task_current;
} co_thread_context_t;

/**
 * 这个变量在每个线程中的地址都应该不同.
 */
static thread_local co_thread_context_t threadCtx;

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

static void co_task_stack_push (co_task_t* task, co_int value)
{
  task->ctx.rsp -= sizeof (co_int);
  *((co_int*)task->ctx.rsp) = value;
}

co_int co_enable ()
{
  memset (&threadCtx, 0, sizeof (co_thread_context_t));
  threadCtx.task_head = (co_task_t*)co_calloc (sizeof (co_task_t));

  threadCtx.task_head->prev = threadCtx.task_head;
  threadCtx.task_head->next = threadCtx.task_head;
  threadCtx.task_last       = threadCtx.task_head;
  return 0;
}

static void co_del ()
{
  register co_task_t* task = threadCtx.task_current;

  if (threadCtx.task_last == task)
    threadCtx.task_last = task->prev;

  threadCtx.task_current = task->next;
  task->prev->next       = task->next;
  task->next->prev       = task->prev;

  co_free (task->stack);
  co_free (task);

  co_load_context (&(threadCtx.task_current->ctx));
}

co_int co_add (co_func func, void* data)
{
  register co_task_t* task       = (co_task_t*)co_calloc (sizeof (co_task_t));
  register co_task_t* last       = threadCtx.task_last;
  const size_t        stack_size = 1024 * 2048; // 每个协程给2MiB的栈空间

  task->prev      = last;
  task->next      = threadCtx.task_head;
  task->stack     = co_calloc (stack_size);
  task->ctx.argv0 = (co_uint)data;
  task->ctx.rip   = (co_uint)func;
  task->ctx.rsp = task->ctx.rbp = (co_uint) (task->stack) + stack_size;
  co_task_stack_push (task, (co_int)co_del);

  threadCtx.task_last = last->next = task;

  return 0;
}

co_int co_wait ()
{
  register co_task_t* task;

  co_store_context (&(threadCtx.task_head->ctx));

  if (threadCtx.task_head->next == threadCtx.task_head)
    return 0;

  task = threadCtx.task_current = threadCtx.task_head->next;
  co_load_context (&(threadCtx.task_head->next->ctx));

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
