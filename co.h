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

typedef void (*co_func) (void* data);

/**
 * 在一个线程中启用协程
 * @return
 */
co_extern co_int co_enable ();

/**
 * 在当前线程中添加一个协程
 * @param func
 * @param data
 * @return
 */
co_extern co_int co_add (co_func func, void* data);

/**
 * 等待所有协程执行完毕
 * @return
 */
co_extern co_int co_wait ();

/**
 * 让出执行权
 * @return
 */
co_extern co_int co_yield();

/**
 *
 * @param func
 * @param data
 * @return thread id
 */
co_extern co_int co_thread_create (co_func func, void* data);

/**
 *
 * @param tid
 * @return
 */
co_extern co_int co_thread_join (co_int tid);

co_extern void* co_alloc (co_uint size);

co_extern void* co_calloc (co_uint size);

co_extern void co_free (void* ptr);

#endif
