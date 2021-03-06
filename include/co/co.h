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

    #if __MINGW64__
      #define static_assert _Static_assert
    #endif

  #else

    #define static_assert _Static_assert

  #endif
#endif

#if defined(_WIN32) && defined(_WINDLL)
  #define CO_API CO_EXTERN __declspec(dllexport)
#else
  #define CO_API CO_EXTERN
#endif

typedef signed char        co_int8;
typedef signed short       co_int16;
typedef signed int         co_int32;
typedef signed long long   co_int64;
typedef unsigned char      co_uint8;
typedef unsigned short     co_uint16;
typedef unsigned int       co_uint32;
typedef unsigned long long co_uint64;

static_assert (sizeof (co_int8)  == 1, "");
static_assert (sizeof (co_int16) == 2, "");
static_assert (sizeof (co_int32) == 4, "");
static_assert (sizeof (co_int64) == 8, "");
static_assert (sizeof (co_uint8)  == 1, "");
static_assert (sizeof (co_uint16) == 2, "");
static_assert (sizeof (co_uint32) == 4, "");
static_assert (sizeof (co_uint64) == 8, "");

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

#define CO_RUN_DEFAULT 0
#define CO_RUN_ONCE    1

// clang-format on

#define co_max(a, b) (((a) > (b)) ? (a) : (b))
#define co_min(a, b) (((a) < (b)) ? (a) : (b))

typedef struct co_task_s co_task_t;

typedef void* (*co_func) (void* data);

/**
 * co_init
 * @return
 */
CO_API co_int co_init ();

/**
 * co_cleanup
 * @return
 */
CO_API void co_cleanup ();

/**
 * 让出执行权
 * @return
 */
CO_API void co_yield_ ();

/**
 * 等待所有协程执行完毕
 * @return
 */
CO_API void co_run ();

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
 * 在一个非co_thread_create函数创建的线程中启用协程功能
 * @return
 */
CO_API void co_thread_init ();

/**
 * 在一个非co_thread_create函数创建的线程中
 * @return
 */
CO_API void co_thread_cleanup ();

/**
 * 在当前线程中添加一个协程
 * @param func      协程的入口函数
 * @param data      func执行时的参数
 * @param stackSize 分配给该协程的栈大小
 * @return
 */
CO_API co_task_t* co_task_add (co_func func, void* data, co_uint stackSize);

/**
 * 等待一个协程结束，并获取返回值.
 * @param task      协程
 * @return
 */
CO_API void* co_task_await (co_task_t* task);

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

CO_API co_uint co_atomic_get (co_uint* mem);
CO_API void    co_atomic_set (co_uint* mem, co_uint value);
CO_API void    co_atomic_inc (co_uint* mem);
CO_API void    co_atomic_dec (co_uint* mem);
CO_API void    co_atomic_add (co_uint* mem, co_uint value);
CO_API void    co_atomic_sub (co_uint* mem, co_uint value);

CO_API void co_cond_init (co_int* key);
CO_API void co_cond_cleanup (co_int key);
CO_API void co_cond_wait (co_int key, co_int timeout);
CO_API void co_cond_notify_one (co_int key);
CO_API void co_cond_notify_all (co_int key);

CO_API void  co_tls_init (co_int* key);
CO_API void  co_tls_cleanup (co_int key);
CO_API void* co_tls_get (co_int key);
CO_API void  co_tls_set (co_int key, void* value);

CO_API co_uint64 co_timestamp_ms ();

#endif
