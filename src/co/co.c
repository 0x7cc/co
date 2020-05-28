
#include "co/co.h"
#include <string.h>
#include <stdlib.h>

// clang-format off

#define elf64_fastcall_argv0 rdi
#define elf64_fastcall_argv1 rsi
#define elf64_fastcall_argv2 rdx
#define elf64_fastcall_argv3 rcx

#define win64_fastcall_argv0 rcx
#define win64_fastcall_argv1 rdx
#define win64_fastcall_argv2 r8
#define win64_fastcall_argv3 r9

#if __linux__ && __x86_64__
  #define argv0 elf64_fastcall_argv0
  #define argv1 elf64_fastcall_argv1
  #define argv2 elf64_fastcall_argv2
  #define argv3 elf64_fastcall_argv3
#elif _WIN64
  #define argv0 win64_fastcall_argv0
  #define argv1 win64_fastcall_argv1
  #define argv2 win64_fastcall_argv2
  #define argv3 win64_fastcall_argv3
#else
  #error "目前只支持64位格式的fastcall"
#endif

// clang-format on

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
  co_int            status;
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
 * 保存上下文.(由汇编实现)
 * @return
 */
extern co_int co_store_context (co_task_context_t* ctx);

/**
 * 载入上下文, 执行过程中将会切换到其他协程.(由汇编实现)
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

static void co_exited ()
{
  threadCtx.task_current->status = -1;
  co_load_context (&(threadCtx.task_head->ctx));
}

co_int co_add (co_func func, void* data, co_int stackSize)
{
  register co_task_t* task = (co_task_t*)co_calloc (sizeof (co_task_t));
  register co_task_t* last = threadCtx.task_last;
  stackSize &= 0xFFFFFFFFFFFFFFF8;

  task->prev      = last;
  task->next      = threadCtx.task_head;
  task->stack     = co_alloc (stackSize);
  task->ctx.argv0 = (co_uint)data;
  task->ctx.rip   = (co_uint)func;
  task->ctx.rsp = task->ctx.rbp = (co_uint) (task->stack) + stackSize - sizeof (co_uintptr);
  co_task_stack_push (task, (co_int)co_exited);

  threadCtx.task_last = last->next = task;

  return 0;
}

co_int co_wait ()
{
  register co_task_t* task = threadCtx.task_head->next;

  while (threadCtx.task_last != threadCtx.task_head)
  {
    threadCtx.task_current = task;
    co_swap_context (&(threadCtx.task_head->ctx), &(task->ctx));

    register co_task_t* next = task->next;
    if (next == threadCtx.task_head)
      next = next->next;

    if (task->status == -1)
    {
      if (task == threadCtx.task_last)
        threadCtx.task_last = task->prev;

      task->prev->next = task->next;
      task->next->prev = task->prev;
      co_free (task->stack);
      co_free (task);
    }

    task = next;
  }

  return 0;
}

co_int co_yield ()
{
  co_swap_context (&(threadCtx.task_current->ctx), &(threadCtx.task_head->ctx));
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
