//
// Created by vos on 2020/6/13.
//

#if __linux__

#define _GNU_SOURCE 1
#include "co/co.h"
#include "co_.h"
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

typedef struct
{
  co_func func;
  void*   data;
} threadCtx;

typedef int (*sys_connect_t) (int sockfd, const struct sockaddr* addr, socklen_t addrlen);
typedef ssize_t (*sys_recv_t) (int sockfd, void* buf, size_t len, int flags);
typedef ssize_t (*sys_send_t) (int sockfd, const void* buf, size_t len, int flags);
typedef ssize_t (*sys_read_t) (int fd, void* buf, size_t count);
typedef ssize_t (*sys_write_t) (int fd, const void* buf, size_t count);

struct
{
  sys_connect_t sys_connect;
  sys_recv_t    sys_recv;
  sys_send_t    sys_send;
  sys_read_t    sys_read;
  sys_write_t   sys_write;
} hooks;

static void* thread_start_routine (void* data) {
  threadCtx* ctx = (threadCtx*)data;

  co_thread_init ();

  ctx->func (ctx->data);

  co_run ();

  co_thread_cleanup ();

  co_free (ctx);

  return 0;
}

co_int co_thread_create (co_func func, void* data) {
  pthread_t  t;
  threadCtx* ctx = (threadCtx*)co_alloc (sizeof (threadCtx));
  ctx->func      = func;
  ctx->data      = data;
  pthread_create (&t, NULL, thread_start_routine, ctx);
  return (co_int)t;
}

co_int co_thread_join (co_int tid) {
  return pthread_join ((pthread_t)tid, nullptr);
}

void co_thread_init_native (co_thread_context_t* ctx) {
  ctx->epfd = epoll_create (20480);
}

void co_thread_cleanup_native (co_thread_context_t* ctx) {
  close (ctx->epfd);
  ctx->epfd = 0;
}

void co_tls_init (co_int* key) {
  pthread_key_create ((pthread_key_t*)key, NULL);
}

void co_tls_cleanup (co_int key) {
  pthread_key_delete (key);
}

void* co_tls_get (co_int key) {
  return pthread_getspecific (key);
}

void co_tls_set (co_int key, void* value) {
  pthread_setspecific (key, value);
}

co_uint64 co_timestamp_ms () {
  struct timeval now = {0};
  gettimeofday (&now, NULL);
  unsigned long long u = now.tv_sec;
  u *= 1000;
  u += now.tv_usec / 1000;
  return u;
}

void co_run () {
  co_thread_context_t* ctx = co_get_context ();

  struct epoll_event evs[2048];
  while (ctx->num_of_active || ctx->num_of_pending) {
    register co_uint64 now = co_timestamp_ms ();
    ctx->now               = now;
    if (ctx->num_of_pending) {
      int nfds = epoll_wait (ctx->epfd, evs, 2048, ctx->num_of_active ? 0 : -1);
      for (register int i = 0; i < nfds; ++i) {
        register co_task_t* task = (co_task_t*)evs->data.ptr;
        task->status             = CO_TASK_STATUS_ACTIVE;
      }
    }
    co_yield_ ();
  }
}

#if CO_ENABLE_HOOKS

int connect (int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
  {
    int flags = fcntl (sockfd, F_GETFL, 0);
    fcntl (sockfd, F_SETFL, flags | O_NONBLOCK);
  }

  int ret = 0;
  while (1) {
    ret = hooks.sys_connect (sockfd, addr, addrlen);
    if (ret == -1) {
      if (errno == EINPROGRESS || errno == EALREADY) {
        co_yield_ ();
        continue;
      }
    }
    return ret;
  }
}

ssize_t recv (int sockfd, void* buf, size_t len, int flags) {
  co_thread_context_t* ctx = co_get_context ();
  ctx->active->timeout     = -1;
  ctx->active->status      = CO_TASK_STATUS_PENDING;
  struct epoll_event ev;
  ev.events   = EPOLLIN;
  ev.data.ptr = ctx->active;

  epoll_ctl (ctx->epfd, EPOLL_CTL_ADD, sockfd, &ev);
  ++ctx->num_of_pending;
  --ctx->num_of_active;
  co_yield_ ();
  ++ctx->num_of_active;
  --ctx->num_of_pending;
  epoll_ctl (ctx->epfd, EPOLL_CTL_DEL, sockfd, nullptr);

  return hooks.sys_recv (sockfd, buf, len, flags);
}

ssize_t send (int sockfd, const void* buf, size_t len, int flags) {
  {
    int flags = fcntl (sockfd, F_GETFL, 0);
    fcntl (sockfd, F_SETFL, flags | O_NONBLOCK);
  }
  register int ret       = 0;
  register int sentbytes = 0;
  while (sentbytes != len) {
    co_yield_ ();
    ret = hooks.sys_send (sockfd, ((const unsigned char*)buf) + sentbytes, len - sentbytes, flags);
    if (ret == -1) {
      if (errno == EAGAIN)
        continue;
      return ret;
    }
    sentbytes += ret;
  }
  return sentbytes;
}

ssize_t read (int fd, void* buf, size_t count) {
  co_thread_context_t* ctx = co_get_context ();
  ctx->active->timeout     = -1;
  ctx->active->status      = CO_TASK_STATUS_PENDING;
  struct epoll_event ev;
  ev.events   = EPOLLIN;
  ev.data.ptr = ctx->active;

  epoll_ctl (ctx->epfd, EPOLL_CTL_ADD, fd, &ev);
  ++ctx->num_of_pending;
  --ctx->num_of_active;
  co_yield_ ();
  ++ctx->num_of_active;
  --ctx->num_of_pending;
  epoll_ctl (ctx->epfd, EPOLL_CTL_DEL, fd, nullptr);

  register int ret = 0;
  while (1) {
    co_yield_ ();
    ret = hooks.sys_read (fd, buf, count);
    if (ret == -1) {
      if (errno == EAGAIN)
        continue;
    }
    return ret;
  }

  return hooks.sys_read (fd, buf, count);
}

ssize_t write (int fd, const void* buf, size_t count) {
  {
    int flags = fcntl (fd, F_GETFL, 0);
    fcntl (fd, F_SETFL, flags | O_NONBLOCK);
  }
  register int ret       = 0;
  register int sentbytes = 0;
  while (sentbytes != count) {
    co_yield_ ();
    ret = hooks.sys_write (fd, ((const unsigned char*)buf) + sentbytes, count - sentbytes);
    if (ret == -1) {
      if (errno == EAGAIN)
        continue;
      return ret;
    }
    sentbytes += ret;
  }
  return sentbytes;
}

#endif

void co_init_hooks () {
#if CO_ENABLE_HOOKS
  hooks.sys_connect = (sys_connect_t)dlsym (RTLD_NEXT, "connect");
  hooks.sys_recv    = (sys_recv_t)dlsym (RTLD_NEXT, "recv");
  hooks.sys_send    = (sys_send_t)dlsym (RTLD_NEXT, "send");
  hooks.sys_read    = (sys_read_t)dlsym (RTLD_NEXT, "read");
  hooks.sys_write   = (sys_write_t)dlsym (RTLD_NEXT, "write");
#endif
}

#endif
