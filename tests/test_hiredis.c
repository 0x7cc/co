//
// Created by x7cc on 2020/5/15.
//

#include "co/co.h"
#include <stdio.h>
#include <hiredis/hiredis.h>

static void* test (void* data) {
  redisContext* ctx = redisConnect ("127.0.0.1", 6379);
  if (ctx->err) {
    puts (ctx->errstr);
    return nullptr;
  }

  {
    redisReply* reply = redisCommand (ctx, "AUTH 12345678");
    freeReplyObject (reply);
  }

  {
    //    redisReply* reply = redisCommand (ctx, "select 0");
    //    freeReplyObject (reply);
  }

  {
    redisReply* reply = redisCommand (ctx, "get 029c0abc-467d-4b6e-88c4-6fe02e775770");

    if (reply && reply->str)
      puts (reply->str);
    else
      puts ("NULL???");
    freeReplyObject (reply);
  }
  redisFree (ctx);
  return nullptr;
}

void* test_redis (void* data) {
  for (int i = 0; i < 1000; ++i) // redis default maxclients
  {
    // hiredis.c:redisBufferRead 需要16k以上的栈空间.
    co_task_add (test, 0, 0x8000);
    // test (NULL);
  }
  co_run ();
  return nullptr;
}

int main (int argc, char* argv[]) {
  co_init ();

  co_int ts[10];
  for (int i = 0; i < 10; ++i) {
    ts[i] = co_thread_create (test_redis, nullptr);
  }

  for (int j = 0; j < 10; ++j) {
    co_thread_join (ts[j]);
  }

  co_cleanup ();

  return 0;
}
