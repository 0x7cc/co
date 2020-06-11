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
    redisReply* reply = redisCommand (ctx, "select 0");
    freeReplyObject (reply);
  }

  {
    redisReply* reply = redisCommand (ctx, "get fddbe044-072e-487d-9581-4b61854e7a48");

    if (reply)
      puts (reply->str);
    else
      puts ("NULL???");
    freeReplyObject (reply);
  }
  redisFree (ctx);
  return nullptr;
}

int main (int argc, char* argv[]) {
  co_init ();
  co_thread_init ();

  for (int i = 0; i < 10000; ++i) // redis default maxclients
  {
    // hiredis.c:redisBufferRead 需要16k以上的栈空间.
    co_task_add (test, 0, 0x8000);
    // test (NULL);
  }

  co_run ();
  co_thread_cleanup ();
  co_cleanup ();

  return 0;
}
