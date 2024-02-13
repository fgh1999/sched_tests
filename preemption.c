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
    int nice;           // normal priority value
    workload_t workload;
    time_t start_time;  // The time when the workload is started.
    time_t end_time;    // The time when the workload finishes.
} worker_t;

typedef struct worker_collection_t
{
    worker_t **workers;
    int count;
} worker_collection_t;

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
    if (!WIFEXITED(status))
        log("Worker %d exited abnormally\n", worker->pid);
}

void wait_workers(const worker_collection_t collection)
{
    int count = collection.count;
    while (count-- > 0)
    {
        wait_worker(collection.workers[count]);
    }
}

void free_workers(const worker_collection_t collection)
{
    for (int i = 0; i < collection.count; i++)
    {
        free(collection.workers[i]);
    }
}

/**
 * @brief Compare two workers by their nice values ascendingly.
 */
int cmp_worker_by_nice(const void *a, const void *b)
{
    return ((worker_t *)a)->nice - ((worker_t *)b)->nice;
}

/**
 * @brief Check the workers' end times according to their nice values.
 *
 * @param workers The workers to be checked.
 * @return 1 if the all workers' end time are valid , 0 otherwise.
 */
int valid_end_time_order(worker_collection_t workers)
{
    qsort(workers.workers, workers.count, sizeof(worker_t *), cmp_worker_by_nice);
    for (int i = 0; i < workers.count - 1; i++)
    {
        if (difftime(workers.workers[i]->end_time, workers.workers[i + 1]->end_time) > 0)
        {
            return 0;
        }
    }
    return 1;
}

void io_bound_workload()
{
    sleep_for(5, 1);
}

/**
 * @brief Test the workers' behavior under different nice values.
 *
 * @param worker_count The number of workers to be tested.
 * @param nice_targets The target nice values of the workers.
 * @return 1 if the all workers' end time are valid , 0 otherwise.
 */
int io_bound_test(const int worker_count, const int *nice_targets)
{
    worker_collection_t worker_collection = {
        .workers = malloc(sizeof(worker_t *) * worker_count),
        .count = worker_count,
    };

    for (int i = 0; i < worker_collection.count; i++)
        worker_collection.workers[i] = fork_normal_worker(io_bound_workload, nice_targets[i]);

    wait_workers(worker_collection);
    int ret = valid_end_time_order(worker_collection);
    if (ret == 0)
        log("Some workers' end time are invalid\n");
    free_workers(worker_collection);
    return ret;
}

int main()
{
    if (!io_bound_test(2, (int[]){1, 2}))
        log("io_bound_test failed\n");
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
