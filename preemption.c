// SPDX-License-Identifier: MPL-2.0

#include <stdlib.h>
#include "log.h"
#include "priority.h"

int main()
{
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
