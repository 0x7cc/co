#ifndef co_H
#define co_H

// clang-format off
#if __cplusplus
  #define co_extern extern "C"
#else
  #define co_extern

  #if _WIN32
    #define thread_local  __declspec(thread)
  #else
    #define static_assert _Static_assert
    #define thread_local  __thread
  #endif
#endif

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

typedef signed char        co_int8;
typedef short              co_int16;
typedef int                co_int32;
typedef long long          co_int64;
typedef unsigned char      co_uint8;
typedef unsigned short     co_uint16;
typedef unsigned int       co_uint32;
typedef unsigned long long co_uint64;

static_assert (sizeof (co_int8)  == 1, "");
static_assert (sizeof (co_int16) == 2, "");
static_assert (sizeof (co_int32) == 4, "");
static_assert (sizeof (co_int64) == 8, "");
static_assert (sizeof (co_int8)  == 1, "");
static_assert (sizeof (co_int16) == 2, "");
static_assert (sizeof (co_int32) == 4, "");
static_assert (sizeof (co_int64) == 8, "");

#if __x86_64__ || _WIN64
  typedef co_int64  co_int;
  typedef co_uint64 co_uint;
  typedef co_int64  co_intptr;
  typedef co_uint64 co_uintptr;
#elif __i386__ || _WIN32
  typedef co_int32  co_int;
  typedef co_uint32 co_uint;
  typedef co_int32  co_intptr;
  typedef co_uint32 co_uintptr;
#else
  #error "wtf???"
#endif

#ifndef nullptr
  #define nullptr 0
#endif

#ifndef NULL
  #define NULL 0
#endif
// clang-format on
typedef struct co_context_s
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
} co_context_t;

typedef struct co_thread_s co_thread_t;
typedef void (*co_func) (co_thread_t* ctx, void* data);
typedef struct co_task_s
{
  struct co_task_s* next;
  void*             stack;
  co_context_t      ctx;
} co_task_t;

typedef struct co_thread_s
{
  /***
   * 循环链表
   */
  co_task_t* task_head;
  co_task_t* task_end;
  co_task_t* task_current;
} co_thread_t;

/**
 * 在一个线程中安装
 * @param thread
 * @return
 */
co_extern co_int co_setup (co_thread_t* thread);

/**
 * 在当前任务中添加一个协程
 * @param thread
 * @param func
 * @param data
 * @return
 */
co_extern co_int co_add (co_thread_t* thread, co_func func, void* data);

/**
 * 等待所有协程执行完毕
 * @param thread
 * @return
 */
co_extern co_int co_wait (co_thread_t* thread);

/**
 * 让出执行权
 * @param thread
 * @return
 */
co_extern co_int co_yield(co_thread_t* thread);

/**
 * 交换协程上下文, 执行后将会执行协程.
 * @param ctx     协程的context
 * @return
 */
co_extern co_int co_exec (co_context_t* ctx);

/**
 * 切换上下文
 * @param store    协程的context
 * @param load     协程的context
 * @return
 */
co_extern co_int co_swap_context (co_context_t* store, co_context_t* load);

#endif
