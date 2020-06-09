
# co

一个基于NASM+C语言编写的协程调度库，运行在AMD64架构的Windows、Linux、MacOS平台上。

为了尽可能的减少在频繁调用的函数中执行的代码量，所以我并没有将非法参数判断加入其中，只注重实现功能，请按照建议流程调用API。

## BUILD

```shell
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCO_ENABLE_HOOKS=1
make
```

## APIs

### co_int co_init()

初始化协程库，请在所有其他co APIs之前调用此函数。

建议：在主线程中调用

### void co_cleanup() 

cleanup

### co_int co_thread_create()

创建一个线程

注意：如果你需要在创建的线程中使用协程，你就需要使用这个接口创建线程，否则你就需要手动调用扩展API来启用协程调度。

### co_int co_thread_join

等待一个线程结束

### co_task_t* co_task_add (co_func func, void* data, co_uint stackSize)

添加一个协程任务，任务将在`co_run()`时被调度。

### void co_yield_ ()

让出CPU执行权，将在调用**过程中**切换至其他协程。

### void co_run ()

开始协程调度工作，所有协程结束后返回



