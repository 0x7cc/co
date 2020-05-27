//
// Created by x7cc on 2020/5/15.
//

#include "co/co.h"
#include <stdio.h>

void coroutine_3 (void* data)
{
  for (int i = 0; i < 2; i++)
  {
    printf ("coroutine_3 : %d\n", i);
    co_yield();
  }
}

void coroutine_1 (void* data)
{
  for (int i = 0; i < 3; ++i)
  {
    printf ("coroutine_1 : i = %d, data = %p\n", i, data);
    co_add (coroutine_3, 0);
    co_yield();
  }
}

void coroutine_2 (void* data)
{
  for (register int i = 0; i < 3; ++i)
  {
    printf ("coroutine_2 : i = %d, data = %p\n", i, data);
    co_yield();
  }
}

void work (void* a)
{
  co_enable ();
  co_add (coroutine_1, (void*)0x1111);
  co_add (coroutine_2, (void*)0x2222);
  co_wait ();
}

void work2 (void* a)
{
  co_enable ();
  co_add (coroutine_1, (void*)0x3333);
  co_add (coroutine_2, (void*)0x4444);
  co_wait ();
}

int main (int argc, char* argv[])
{
  co_int tid[2];
  tid[0] = co_thread_create (work, 0);
  // tid[1] = co_thread_create (work2, 0);

  co_thread_join (tid[0]);
  // co_thread_join (tid[1]);
  return 0;
}
