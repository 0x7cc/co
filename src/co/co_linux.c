//
// Created by vos on 2020/6/13.
//

#if __linux__

#include "co/co.h"
#include "co/co_private.h"

#define _GNU_SOURCE 1
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
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
  ctx->epfd = epoll_create (10);
}

void co_thread_cleanup_native (co_thread_context_t* ctx) {
  close (ctx->epfd);
}

#if CO_ENABLE_HOOKS

ssize_t recv (int sockfd, void* buf, size_t len, int flags) {
  co_thread_context_t* ctx = co_get_context ();

  {
    int flags = fcntl (sockfd, F_GETFL, 0);
    fcntl (sockfd, F_SETFL, flags | O_NONBLOCK | O_NDELAY);
  }

  struct epoll_event ev;
  ev.events   = EPOLLIN;
  ev.data.ptr = ctx->task_current;
  epoll_ctl (ctx->epfd, EPOLL_CTL_ADD, sockfd, &ev);
  ctx->task_current->status = CO_TASK_STATUS_WAITING;
  co_yield_ ();
  epoll_ctl (ctx->epfd, EPOLL_CTL_DEL, sockfd, NULL);

  return hooks.sys_recv (sockfd, buf, len, flags);
}

ssize_t send (int sockfd, const void* buf, size_t len, int flags) {
  {
    int flags = fcntl (sockfd, F_GETFL, 0);
    fcntl (sockfd, F_SETFL, flags | O_NONBLOCK | O_NDELAY);
  }
  register int ret       = 0;
  register int sentbytes = 0;
  while (sentbytes != len) {
    co_yield_ ();
    ret = hooks.sys_send (sockfd, buf, len, flags);
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
    fcntl (fd, F_SETFL, flags | O_NONBLOCK | O_NDELAY);
  }

  co_thread_context_t* ctx = co_get_context ();
  struct epoll_event   ev;
  ev.events   = EPOLLIN;
  ev.data.ptr = &ctx;
  epoll_ctl (ctx->epfd, EPOLL_CTL_ADD, fd, &ev);
  ctx->task_current->status = CO_TASK_STATUS_WAITING;
  co_yield_ ();

  return hooks.sys_read (fd, buf, count);
}

ssize_t write (int fd, const void* buf, size_t count) {
  {
    int flags = fcntl (fd, F_GETFL, 0);
    fcntl (fd, F_SETFL, flags | O_NONBLOCK | O_NDELAY);
  }
  register int ret       = 0;
  register int sentbytes = 0;
  while (sentbytes != count) {
    co_yield_ ();
    ret = hooks.sys_write (fd, buf, count);
    if (ret == -1) {
      if (errno == EAGAIN)
        continue;
      return ret;
    }
    sentbytes += ret;
  }
  return sentbytes;
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
