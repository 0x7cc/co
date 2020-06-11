//
// Created by vos on 2020/6/11.
//

#include <co/co.h>
#include <stdio.h>

static void* co1 (void* data) {
  co_int* cond = data;
  while (1) {
    puts ("aaa");
    co_cond_wait (cond, 2000);
    puts ("bbb");
  }
  return nullptr;
}

static void* co2 (void* data) {
  co_int* cond = data;
  while (1) {
    co_cond_notify_all (cond);
  }
  return nullptr;
}

int main (int argc, char* argv[]) {
  co_init ();
  co_thread_init ();

  co_int cond;
  co_cond_init (&cond);
  co_task_add (co1, &cond, 0);

  co_run ();

  co_thread_cleanup ();
  co_cleanup ();
  return 0;
}
