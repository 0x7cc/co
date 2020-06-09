//
// Created by x7cc on 2020/5/28.
//

#include "co/co.h"
#include <stdio.h>

void* coroutine_3 (void* data)
{
  for (int i = 0; i < 3; i++)
  {
    printf ("coroutine_3 : %d\n", i);
    co_yield_ ();
  }
  return nullptr;
}

void* coroutine_1 (void* data)
{
  for (int i = 0; i < 10000; ++i)
  {
    printf ("coroutine_1 : i = %d, data = %p\n", i, data);
    co_task_add (coroutine_3, nullptr, 0);
  }
  // co_yield ();
  return nullptr;
}

void* coroutine_2 (void* data)
{
  for (int i = 0; i < 3; ++i)
  {
    printf ("coroutine_2 : i = %d, data = %p\n", i, data);
    co_yield_ ();
  }
  return nullptr;
}

void* work (void* a)
{
  co_task_add (coroutine_1, (void*)0x1111, 0);
  co_task_add (coroutine_2, (void*)0x2222, 0);
  return nullptr;
}

void* work2 (void* a)
{
  co_task_add (coroutine_1, (void*)0x3333, 0);
  co_task_add (coroutine_2, (void*)0x4444, 0);
  return nullptr;
}

int main (int argc, char* argv[])
{
  co_int tid[2];

  co_init ();

  tid[0] = co_thread_create (work, nullptr);
  tid[1] = co_thread_create (work2, nullptr);

  co_thread_join (tid[0]);
  co_thread_join (tid[1]);

  co_cleanup ();

  return 0;
}
