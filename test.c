//
// Created by x7cc on 2020/5/15.
//

#include "co.h"
#include <stdio.h>

void coroutine_1 (void* data)
{
  while (1)
  {
    printf ("coroutine 1 : %p\n", data);
    co_yield();
  }
}

void coroutine_2 (void* data)
{
  while (1)
  {
    printf ("coroutine 2 : %p\n", data);
    co_yield();
  }
}

//thread_local int a;

void* work (void* a)
{
  co_enable ();
  co_add (coroutine_1, 0x3489);
  co_add (coroutine_2, 0x07cc);
  co_wait ();
  return 0;
}

void* work2 (void* a)
{
  co_enable ();
  co_add (coroutine_1, 0x2222);
  co_add (coroutine_2, 0x1212);
  co_wait ();
  return 0;
}

int main (int argc, char* argv[])
{
  co_int tid[2];
  tid[0] = co_thread_create (work, 0);
  tid[1] = co_thread_create (work2, 0);

  co_thread_join (tid[0]);
  co_thread_join (tid[1]);
  return 0;
}
