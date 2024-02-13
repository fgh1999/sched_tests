// SPDX-License-Identifier: MPL-2.0

#include <stdlib.h>
#include "log.h"
#include "priority.h"
#include "workload.h"
#include <time.h>
#include <string.h>
#include <sys/wait.h>

typedef void (*workload_t)(void);

typedef struct worker_t
{
    int pid;
    int nice;
    workload_t workload;
    time_t start_time;
    time_t end_time;
} worker_t;

worker_t *fork_normal_worker(workload_t workload, int nice)
{
    worker_t *worker = malloc(sizeof(worker_t));
    memset(worker, 0, sizeof(worker_t));

    int pid = fork();
    if (pid == 0)
    {
        worker->workload = workload;
        if (set_normal_prio(PRIO_PROCESS, 0, nice) == 0)
            worker->nice = nice;
        else
            exit(EXIT_FAILURE);

        worker->start_time = time(NULL);
        worker->workload();
        worker->end_time = time(NULL);
        exit(EXIT_SUCCESS);
    }

    worker->pid = pid;
    return worker;
}

void wait_worker(worker_t *worker)
{
    int status;
    waitpid(worker->pid, &status, 0);
    if (WIFEXITED(status))
    {
        log("Worker %d exited with status %d\n", worker->pid, WEXITSTATUS(status));
    }
    else
    {
        log("Worker %d exited abnormally\n", worker->pid);
    }
}

void wait_workers(worker_t **workers, int count)
{
    while (count-- > 0)
    {
        wait_worker(workers[count]);
    }
}

void free_workers(worker_t **workers, int count)
{
    while (count-- > 0)
    {
        free(workers[count]);
    }
}

void io_bound_workload()
{
    sleep_for(5, 1);
}

void io_bound_test(const int worker_count, const int *nice_targets)
{
    worker_t **workers = malloc(sizeof(worker_t *) * worker_count);

    for (int i = 0; i < worker_count; i++)
    {
        workers[i] = fork_normal_worker(io_bound_workload, nice_targets[i]);
    }

    wait_workers(workers, worker_count);
    free_workers(workers, worker_count);
}

int main()
{

    io_bound_test(2, (int[]){1,2});
    // 工作负载：
    // 计算的话比如素因子分解，IO就用“中间包含间断的sleep”模拟一下吧
    // 可以多试几种工作负载
    // 测试方法：
    // fork几个*工作负载相同的*非实时子进程，让他们每个都能跑个10s左右才结束。
    // 通过setpriority(stdlib)/set_normal_prio(priority.h)为每个子进程设置不同的优先级

    // 期望：（注意：优先级越高，其数值反而更低）
    // 优先级高的进程能够更多的占用CPU时间
    // 预期能够观察到：优先级高的进程更早结束

    // 当前先在linux gcc环境下测试就行
    // 等#599、徐老师的amendment都合入后，可抢占的调度器实现才会用得到这个测试
    return 0;
}
