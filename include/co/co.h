#ifndef co_H
#define co_H

// clang-format off

#if __cplusplus
  #define CO_EXTERN extern "C"
#else
  #define CO_EXTERN

  #ifndef nullptr
    #define nullptr 0
  #endif

  #if _WIN32

    #define thread_local  __declspec(thread)

    #if __MINGW64__
      #define static_assert _Static_assert
    #endif

  #else

    #define static_assert _Static_assert
    #define thread_local  __thread

  #endif
#endif

#if defined(_WIN32) && defined(_WINDLL)
  #define CO_API CO_EXTERN __declspec(dllexport)
#else
  #define CO_API CO_EXTERN
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

#ifndef NULL
  #define NULL 0
#endif

// clang-format on

typedef void (*co_func) (void* data);

/**
 * 在一个线程中启用协程功能
 * @return
 */
CO_API co_int co_enable ();

/**
 * 在当前线程中添加一个协程
 * @param func      协程的入口函数
 * @param data      func执行时的参数
 * @param stackSize 分配给该协程的栈大小
 * @return
 */
CO_API co_int co_add (co_func func, void* data, co_int stackSize);

/**
 * 等待所有协程执行完毕
 * @return
 */
CO_API co_int co_wait ();

/**
 * 让出执行权
 * @return
 */
CO_API co_int co_yield ();

/**
 * 创建线程
 * @param func
 * @param data
 * @return thread id
 */
CO_API co_int co_thread_create (co_func func, void* data);

/**
 * 等待线程
 * @param tid
 * @return
 */
CO_API co_int co_thread_join (co_int tid);

/**
 * malloc
 * @param size
 * @return
 */
CO_API void* co_alloc (co_uint size);

/**
 * calloc
 * @param size
 * @return
 */
CO_API void* co_calloc (co_uint size);

/**
 * free
 * @param ptr
 */
CO_API void co_free (void* ptr);

#endif
