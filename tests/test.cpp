//
// Created by x7cc on 2020/5/28.
//

#include "co/co.h"
#include <iostream>
#include <thread>

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
    printf ("coroutine_1 : i = %d, data = 0x%llx\n", i, (co_uintptr)data);
    co_task_add (coroutine_3, nullptr, 0);
  }
  // co_yield ();
  return nullptr;
}

void* coroutine_2 (void* data)
{
  for (int i = 0; i < 3; ++i)
  {
    printf ("coroutine_2 : i = %d, data = 0x%llx\n", i, (co_uintptr)data);
    co_yield_ ();
  }
  return nullptr;
}

void* work (void* a)
{
  co_thread_init ();
  co_task_add (coroutine_1, (void*)0x1111, 0);
  co_task_add (coroutine_2, (void*)0x2222, 0);
  co_run ();
  co_thread_cleanup ();
  return nullptr;
}

void* work2 (void* a)
{
  co_thread_init ();
  co_task_add (coroutine_1, (void*)0x3333, 0);
  co_task_add (coroutine_2, (void*)0x4444, 0);
  co_run ();
  co_thread_cleanup ();
  return nullptr;
}

int main (int argc, char* argv[])
{
  co_init ();
  co_thread_init ();

  std::thread t1 (work, nullptr);
  std::thread t2 (work2, nullptr);

  t1.join ();
  t2.join ();

  co_run ();
  co_thread_cleanup ();
  co_cleanup ();

  return 0;
}
