//
// Created by x7cc on 2020/5/15.
//

#include "co.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void coroutine_1 (co_thread_t* ctx, void* data)
{
  puts ("coroutine 1 begin");
  co_yield(ctx);
  puts ("coroutine 1 end");
  co_yield(ctx);
}

void coroutine_2 (co_thread_t* ctx, void* data)
{
  puts ("coroutine 2 begin");
  co_yield(ctx);
  puts ("coroutine 2 end");
}

thread_local int a;

void* work (void* a)
{
  co_thread_t ctx;
  co_setup (&ctx);
  co_add (&ctx, coroutine_1, NULL);
  co_add (&ctx, coroutine_2, NULL);
  co_wait (&ctx);
  return 0;
}

int main (int argc, char* argv[])
{
  pthread_t tid;

  pthread_create (&tid, NULL, work, 0);
  pthread_join (tid, nullptr);
  return 0;
}
