//
// Created by vos on 2020/6/9.
//

#ifndef CO_PRIVATE_H
#define CO_PRIVATE_H
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

#define CO_TASK_STATUS_READY       ((co_uint)1 << 0)
#define CO_TASK_STATUS_INTERRUPTED ((co_uint)1 << 1)
#define CO_TASK_STATUS_COMPLETED   ((co_uint)1 << 2)

#define CO_MINIMAL_STACK_SIZE 0x4000

// clang-format on

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
  void*             result;
  co_uint           status;
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

  /**
   * 协程数
   */
  co_int num_of_coroutines;
} co_thread_context_t;

typedef struct
{
  co_func func;
  void*   data;
} threadCtx;

extern co_int tls_key_thread_ctx;

/**
 * 切换上下文(由汇编实现)
 * @param store    协程的context
 * @param load     协程的context
 * @return
 */
extern void co_swap_context (co_task_context_t* store, co_task_context_t* load);

/**
 * 保存上下文.(由汇编实现)
 * @return
 */
extern void co_store_context (co_task_context_t* ctx);

/**
 * 载入上下文, 执行过程中将会切换到其他协程.(由汇编实现)
 * @return
 */
extern void co_load_context (co_task_context_t* ctx);

extern void co_exited_asm ();

extern void co_init_hooks ();

#endif //CO_PRIVATE_H
