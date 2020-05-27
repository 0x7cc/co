
#include "co.h"
#include <string.h>
#include <stdlib.h>

static void co_task_stack_push (co_task_t* task, co_int value)
{
  task->ctx.rsp -= sizeof (co_int);
  *((co_int*)task->ctx.rsp) = value;
}

co_int co_setup (co_thread_t* thread)
{
  memset (thread, 0, sizeof (co_thread_t));
  thread->task_head = (co_task_t*)calloc (1, sizeof (co_task_t));

  thread->task_head->next = thread->task_head;
  thread->task_end        = thread->task_head;
  return 0;
}

static co_int co_task_finished (co_task_t* task)
{
  return 0;
}

co_int co_add (co_thread_t* thread, co_func func, void* data)
{
  co_task_t*   task       = (co_task_t*)calloc (1, sizeof (co_task_t));
  co_task_t*   node       = thread->task_end;
  const size_t stack_size = 1024 * 2048; // 每个协程给2MiB的栈空间

  task->next      = thread->task_head;
  task->stack     = calloc (1, stack_size);
  task->ctx.argv0 = (co_uint)thread;
  task->ctx.argv1 = (co_uint)data;
  task->ctx.rip   = (co_uint)func;
  task->ctx.rsp = task->ctx.rbp = (co_uint) (task->stack) + stack_size;
  co_task_stack_push (task, (co_int)co_task_finished);

  thread->task_end = node->next = task;

  return 0;
}

co_int co_wait (co_thread_t* thread)
{
  register co_task_t* L;
  register co_task_t* R;
  register co_task_t* task;
  if (thread->task_head->next == nullptr)
    return 0;

  L    = thread->task_head;
  task = L->next;

  while (task)
  {
    if (task == thread->task_head)
      return 0;

    R = task->next;

    thread->task_current = task;
    co_exec (&(task->ctx));

    // 协程运行结束返回了,释放内存.
    L->next = R;
    free (task);

    task = R;
  }
  return 0;
}

co_int co_yield(co_thread_t* thread)
{
  co_task_t* current = thread->task_current;
  co_task_t* next    = current->next;
  if (next == thread->task_head)
    next = next->next;

  if (next == current)
    return 0; // 只有当前协程了,没必要切换.

  thread->task_current = next;
  co_swap_context (&(current->ctx), &(next->ctx));
  return 0;
}
