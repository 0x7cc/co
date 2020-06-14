//
// Created by vos on 2020/6/13.
//

#if __APPLE__

#include "co/co_private.h"

#define _GNU_SOURCE 1
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

typedef ssize_t (*sys_recv_t) (int sockfd, void* buf, size_t len, int flags);
typedef ssize_t (*sys_send_t) (int sockfd, const void* buf, size_t len, int flags);
typedef ssize_t (*sys_read_t) (int fd, void* buf, size_t count);
typedef ssize_t (*sys_write_t) (int fd, const void* buf, size_t count);

struct
{
  sys_recv_t  sys_recv;
  sys_send_t  sys_send;
  sys_read_t  sys_read;
  sys_write_t sys_write;
} hooks;

void co_thread_init_native (co_thread_context_t* ctx) {
}

void co_thread_cleanup_native (co_thread_context_t* ctx) {
}

#if CO_ENABLE_HOOKS

ssize_t recv (int sockfd, void* buf, size_t len, int flags) {
  {
    int flags = fcntl (sockfd, F_GETFL, 0);
    fcntl (sockfd, F_SETFL, flags | O_NONBLOCK);
  }
  register int ret = 0;
  while (1) {
    co_yield_ ();
    ret = hooks.sys_recv (sockfd, buf, len, flags);
    if (ret == -1 && errno == EAGAIN)
      continue;
    return ret;
  }
  return -1; // ???
}

ssize_t send (int sockfd, const void* buf, size_t len, int flags) {
  {
    int flags = fcntl (sockfd, F_GETFL, 0);
    fcntl (sockfd, F_SETFL, flags | O_NONBLOCK);
  }
  register int ret = 0;
  while (1) {
    co_yield_ ();
    ret = hooks.sys_send (sockfd, buf, len, flags);
    if (ret == -1 && errno == EAGAIN)
      continue;
    return ret;
  }
  return -1; // ???
}

ssize_t read (int fd, void* buf, size_t count) {
  {
    int flags = fcntl (fd, F_GETFL, 0);
    fcntl (fd, F_SETFL, flags | O_NONBLOCK);
  }

  register int ret = 0;
  while (1) {
    co_yield_ ();
    ret = hooks.sys_read (fd, buf, count);
    if (ret == -1 && errno == EAGAIN)
      continue;
    return ret;
  }
  return -1; // ???
}

ssize_t write (int fd, const void* buf, size_t count) {
  {
    int flags = fcntl (fd, F_GETFL, 0);
    fcntl (fd, F_SETFL, flags | O_NONBLOCK);
  }
  register int ret = 0;
  while (1) {
    co_yield_ ();
    ret = hooks.sys_write (fd, buf, count);
    if (ret == -1 && errno == EAGAIN)
      continue;
    return ret;
  }
  return -1; // ???
}

#endif // CO_ENABLE_HOOKS

void co_init_hooks () {
#if CO_ENABLE_HOOKS
  hooks.sys_recv  = (sys_recv_t)dlsym (RTLD_NEXT, "recv");
  hooks.sys_send  = (sys_send_t)dlsym (RTLD_NEXT, "send");
  hooks.sys_read  = (sys_read_t)dlsym (RTLD_NEXT, "read");
  hooks.sys_write = (sys_write_t)dlsym (RTLD_NEXT, "write");
#endif
}


#endif
