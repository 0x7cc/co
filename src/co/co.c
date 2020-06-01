
#include "co/co.h"
#include <string.h>
#include <stdlib.h>

// clang-format off

#define elf64_fastcall_argv0 di
#define elf64_fastcall_argv1 si
#define elf64_fastcall_argv2 dx
#define elf64_fastcall_argv3 cx

#define win64_fastcall_argv0 cx
#define win64_fastcall_argv1 dx
#define win64_fastcall_argv2 r8
#define win64_fastcall_argv3 r9

#if __x86_64__ || _WIN64
  #if __linux__
    #define argv0 elf64_fastcall_argv0
    #define argv1 elf64_fastcall_argv1
    #define argv2 elf64_fastcall_argv2
    #define argv3 elf64_fastcall_argv3
  #elif __APPLE__
    #define argv0 elf64_fastcall_argv0
    #define argv1 elf64_fastcall_argv1
    #define argv2 elf64_fastcall_argv2
    #define argv3 elf64_fastcall_argv3
  #elif _WIN64
    #define argv0 win64_fastcall_argv0
    #define argv1 win64_fastcall_argv1
    #define argv2 win64_fastcall_argv2
    #define argv3 win64_fastcall_argv3
  #endif
#else
  #error "目前只支持64位格式的fastcall"
#endif

// clang-format on

#define CO_MINIMAL_STACK_SIZE 0x4000

typedef struct co_task_context_s
{
  co_uint ax;
  co_uint bx;
  co_uint cx;
  co_uint dx;
  co_uint si;
  co_uint di;
  co_uint sp;
  co_uint bp;
  co_uint ip;
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

/**
 * 携程执行结束返回时的处理函数
 */
static void co_exited ()
{
  threadCtx.task_current->status = -1;
  co_yield ();
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

co_int co_add (co_func func, void* data, co_uint stackSize)
{
  register co_task_t* task = (co_task_t*)co_calloc (sizeof (co_task_t));
  register co_task_t* last = threadCtx.task_last;

  stackSize = co_max (stackSize, CO_MINIMAL_STACK_SIZE) & 0xFFFFFFFFFFFFFFF0;

  task->prev      = last;
  task->next      = threadCtx.task_head;
  task->stack     = co_calloc (stackSize);
  task->ctx.argv0 = (co_uint)data;
  task->ctx.ip    = (co_uint)func;

  // Windows: 经测试，Windows平台需要16bytes栈底空间，否则会发生堆溢出问题，原因不详.
  // macOS: 根据苹果官方文档，这里理应是16-byte对齐，但我的切换context是用jmp做跳转，没有call的压栈操作，所以这里就要是8的单数倍.See: https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/LowLevelABI/130-IA-32_Function_Calling_Conventions/IA32.html
  task->ctx.sp             = ((((co_uint)task->stack) + stackSize - 16) & 0xFFFFFFFFFFFFFFF0) - 8;
  *((co_int*)task->ctx.sp) = (co_uint)co_exited;
  last->next->prev         = task;
  last->next               = task;
  threadCtx.task_last      = task;

  return 0;
}

co_int co_run ()
{
  threadCtx.task_current = threadCtx.task_head;

  while (threadCtx.task_last != threadCtx.task_head)
    co_yield ();

  co_free (threadCtx.task_head);

  return 0;
}

co_int co_yield ()
{
  {
    register co_task_t* task = threadCtx.task_current;
    threadCtx.task_current   = task->next;
    co_swap_context (&(task->ctx), &(task->next->ctx));
  }

  register co_task_t* prev = threadCtx.task_current->prev;
  if (prev->status == -1)
  {
    if (prev == threadCtx.task_last)
      threadCtx.task_last = prev->prev;

    prev->prev->next = prev->next;
    prev->next->prev = prev->prev;
    co_free (prev->stack);
    co_free (prev);
  }

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
