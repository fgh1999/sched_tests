// SPDX-License-Identifier: MPL-2.0

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include "log.h"
#include "priority.h"
#include "workload.h"
#include "shm.h"

typedef void (*workload_t)(void);

typedef struct worker_t
{
    int pid;
    int nice; // normal priority value
    workload_t workload;
    time_t start_time; // The time when the workload is started.
    time_t end_time;   // The time when the workload finishes.
} worker_t;

typedef struct worker_collection_t
{
    worker_t **workers;
    int count;
} worker_collection_t;

void fork_normal_worker(int shmid_workers, size_t offset, workload_t workload, int nice)
{
    int pid = fork();
    if (pid != 0)
        return;

    worker_t *worker = (worker_t *)_shmat(shmid_workers, 0) + offset;
    bzero(worker, sizeof(worker_t));
    worker->pid = getpid();
    worker->workload = workload;
    if (set_normal_prio(PRIO_PROCESS, 0, nice) == 0)
        worker->nice = nice;
    else
        exit(EXIT_FAILURE);

    time(&(worker->start_time)); // TODO: in secs, not enough precision?
    log("starts at %ld with nice(%d)\n", worker->start_time, worker->nice);
    worker->workload();
    time(&(worker->end_time));
    log("ends at %ld with nice(%d)\n", worker->end_time, worker->nice);
    exit(EXIT_SUCCESS);
}

void wait_worker(worker_t *worker)
{
    int status;
    waitpid(worker->pid, &status, 0);
    if (!WIFEXITED(status))
        log("Worker %d exited abnormally\n", worker->pid);
}

void wait_workers(const worker_collection_t *collection)
{
    for (int i = 0; i < collection->count; i++)
        wait_worker(collection->workers[i]);
}

void free_workers(const int shmid_col, const int shmid_workers)
{
    void *shmaddr = _shmat(shmid_workers, 0);
    _shmdt(shmaddr);
    shmaddr = _shmat(shmid_col, 0);
    _shmdt(shmaddr);
    delshm(shmid_workers);
    delshm(shmid_col);
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
int valid_end_time_order(const worker_collection_t *workers)
{
    qsort(workers->workers, workers->count, sizeof(worker_t *), cmp_worker_by_nice);
    for (int i = 0; i < workers->count - 1; i++)
        if (difftime(workers->workers[i]->end_time, workers->workers[i + 1]->end_time) > 0)
            return 0;
    return 1;
}

void log_worker_end_times(const worker_collection_t *workers)
{
    for (int i = 0; i < workers->count; i++)
    {
        const worker_t *worker = workers->workers[i];
        log("Worker %d(nice:%d)'s end time: %ld\n", worker->pid, worker->nice, worker->end_time);
    }
}

/**
 * @brief Test the workers' behavior under different nice values and the same workload.
 *
 * @param worker_count The number of workers to be tested.
 * @param nice_targets The target nice values of the workers.
 * @param workload The workload to be tested.
 * @return 1 if the all workers' end time are valid , 0 otherwise.
 */
int test_case(const int worker_count, const int *nice_targets, workload_t workload)
{
    int shmid_col = getshm_for("./.sched_test_worker_collection", sizeof(worker_collection_t));
    worker_collection_t *worker_collection = _shmat(shmid_col, 0);
    bzero(worker_collection, sizeof(worker_collection_t));

    int shmid_workers = getshm_for("./.sched_test_workers", sizeof(worker_t) * worker_count);
    worker_t *workers = _shmat(shmid_workers, 0);
    bzero(workers, sizeof(worker_t) * worker_count);
    worker_collection->count = worker_count;
    worker_collection->workers = (worker_t **)malloc(sizeof(worker_t *) * worker_count);
    for (int i = 0; i < worker_count; i++)
        worker_collection->workers[i] = workers + i;
    log("shared memory is allocated\n");

    for (size_t i = 0; i < worker_collection->count; i++)
        fork_normal_worker(shmid_workers, i, workload, nice_targets[i]);

    wait_workers(worker_collection);
    int ret = valid_end_time_order(worker_collection);
    if (ret == 0)
        log("Some workers' end time are invalid\n");
    log_worker_end_times(worker_collection);
    free_workers(shmid_col, shmid_workers);
    return ret;
}

void io_bound_workload()
{
    sleep_for(3, 1);
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
    log("starts io_bound_test\n");
    return test_case(worker_count, nice_targets, io_bound_workload);
}

int cpu_bound_test(const int worker_count, const int *nice_targets)
{
    log("starts cpu_bound_test\n");
    return test_case(worker_count, nice_targets, burn_cpu);
}

int main()
{
    int worker_num = 3;
    int *nice_targets = (int *)malloc(sizeof(int) * worker_num);
    for (int i = 0; i < worker_num; i++)
        nice_targets[i] = i * 10;

    if (!io_bound_test(worker_num, nice_targets))
        log("io_bound_test failed\n");

    if (!cpu_bound_test(worker_num, nice_targets))
        log("cpu_bound_test failed\n");

    // 工作负载：
    // 计算的话比如素因子分解，IO就用“中间包含间断的sleep”模拟一下吧
    // 可以多试几种工作负载
    // 测试方法：
    // fork几个*工作负载相同的*非实时子进程，让他们每个都能跑个10s左右才结束。
    // 通过setpriority(stdlib)/set_normal_prio(priority.h)为每个子进程设置不同的优先级

    // 期望：（注意：优先级越高，其数值反而更低）
    // 优先级高的进程能够更多的占用CPU时间
    // 单核环境下预期能够观察到：优先级高的进程更早结束

    // 当前先在linux gcc环境下测试就行
    // 等#599、徐老师的amendment都合入后，可抢占的调度器实现才会用得到这个测试
    return 0;
}
