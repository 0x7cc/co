//
// Created by vos on 2020/6/13.
//

#if __linux__

#include "co/co.h"
#include "co_.h"
#define _GNU_SOURCE 1
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

typedef struct
{
  co_func func;
  void*   data;
} co_thread_create_info_t;

typedef int (*sys_accept_t) (int sockfd, struct sockaddr* addr, socklen_t* addrlen);
typedef int (*sys_connect_t) (int sockfd, const struct sockaddr* addr, socklen_t addrlen);
typedef ssize_t (*sys_recv_t) (int sockfd, void* buf, size_t len, int flags);
typedef ssize_t (*sys_send_t) (int sockfd, const void* buf, size_t len, int flags);
typedef ssize_t (*sys_read_t) (int fd, void* buf, size_t count);
typedef ssize_t (*sys_write_t) (int fd, const void* buf, size_t count);

struct
{
  sys_accept_t  sys_accept;
  sys_connect_t sys_connect;
  sys_recv_t    sys_recv;
  sys_send_t    sys_send;
  sys_read_t    sys_read;
  sys_write_t   sys_write;
} hooks;

static void* thread_start_routine (void* data) {
  co_thread_create_info_t* ctx = (co_thread_create_info_t*)data;

  co_thread_init ();

  ctx->func (ctx->data);

  co_run ();

  co_thread_cleanup ();

  co_free (ctx);

  return 0;
}

co_int co_thread_create (co_func func, void* data) {
  pthread_t                t;
  co_thread_create_info_t* ctx = (co_thread_create_info_t*)co_alloc (sizeof (co_thread_create_info_t));
  ctx->func                    = func;
  ctx->data                    = data;
  pthread_create (&t, NULL, thread_start_routine, ctx);
  return (co_int)t;
}

co_int co_thread_join (co_int tid) {
  return pthread_join ((pthread_t)tid, nullptr);
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

#if CO_ENABLE_HOOKS

int accept (int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
  int fd = hooks.sys_accept (sockfd, addr, addrlen);

  int flags = fcntl (sockfd, F_GETFL, 0);
  fcntl (fd, F_SETFL, flags | O_NONBLOCK | O_NDELAY);

  return fd;
}

int connect (int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
  {
    int flags = fcntl (sockfd, F_GETFL, 0);
    fcntl (sockfd, F_SETFL, flags | O_NONBLOCK | O_NDELAY);
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
  {
    int flags = fcntl (sockfd, F_GETFL, 0);
    assert (flags | O_NONBLOCK);
    assert (flags | O_NDELAY);
    // fcntl (sockfd, F_SETFL, flags | O_NONBLOCK | O_NDELAY);
  }
  register int ret = 0;
  while (1) {
    co_yield_ ();
    ret = hooks.sys_recv (sockfd, buf, len, flags);
    if (ret == -1) {
      if (errno == EAGAIN)
        continue;
      return ret;
    }
    return ret;
  }
  return -1; // ???
}

ssize_t send (int sockfd, const void* buf, size_t len, int flags) {
  {
    int flags = fcntl (sockfd, F_GETFL, 0);
    assert (flags | O_NONBLOCK);
    assert (flags | O_NDELAY);
    // fcntl (sockfd, F_SETFL, flags | O_NONBLOCK);
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
  {
    int flags = fcntl (fd, F_GETFL, 0);
    // fcntl (fd, F_SETFL, flags | O_NONBLOCK | O_NDELAY);
    assert (flags | O_NONBLOCK);
    assert (flags | O_NDELAY);
  }

  register int ret = 0;
  while (1) {
    co_yield_ ();
    ret = hooks.sys_read (fd, buf, count);
    if (ret == -1) {
      if (errno == EAGAIN)
        continue;
      return ret;
    }
    return ret;
  }
}

ssize_t write (int fd, const void* buf, size_t count) {
  {
    int flags = fcntl (fd, F_GETFL, 0);
    // fcntl (fd, F_SETFL, flags | O_NONBLOCK);
    assert (flags | O_NONBLOCK);
    assert (flags | O_NDELAY);
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
  hooks.sys_accept  = (sys_accept_t)dlsym (RTLD_NEXT, "accept");
  hooks.sys_connect = (sys_connect_t)dlsym (RTLD_NEXT, "connect");
  hooks.sys_recv    = (sys_recv_t)dlsym (RTLD_NEXT, "recv");
  hooks.sys_send    = (sys_send_t)dlsym (RTLD_NEXT, "send");
  hooks.sys_read    = (sys_read_t)dlsym (RTLD_NEXT, "read");
  hooks.sys_write   = (sys_write_t)dlsym (RTLD_NEXT, "write");
#endif
}

#endif
