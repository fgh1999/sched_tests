// SPDX-License-Identifier: MPL-2.0

#include <stdlib.h>
#include "log.h"
#include "priority.h"

/**
 * @brief a test case of setpriority
 *
 * @param which PRIO_PROCESS, PRIO_PGRP, or PRIO_USER
 * @param who pid, pgrp, or uid
 * @param new_nice a new nice value from -20 to 19
 * @return int 0 if success, -1 if failed
 */
int test_set_prio(int which, int who, int new_nice)
{
    log("target new nice value = %d\n", new_nice);
    if (set_normal_prio(which, who, new_nice) == 0)
    {
        int prio = get_normal_prio(which, who);
        if (errno >= 0)
        {
            if (prio == new_nice)
                log("nice value is correctly set(%d)\n", prio);
            else if (prio == 19 && new_nice > 19)
                log("nice value is set to the maximum value(19)\n");
            else if (prio == -20 && new_nice < -20)
                log("nice value is set to the minimum value(-20)\n");
            else
            {
                log("Unknown error\n");
                return -1;
            }
        }
        else
        {
            err_log("failed to set nice value");
            return -1;
        }
        return 0;
    }
    return -1;
}

void test_get_self_prio()
{
    int prio = get_normal_prio(PRIO_PROCESS, 0);
    if (errno >= 0)
    {
        log("nice value = %d\n", prio);
    }
    else
        exit(-1);
}

int main()
{
    int who = 0; // self - the calling process
    int ret = 0;
    test_get_self_prio();

    ret += test_set_prio(PRIO_PROCESS, who, 10);
    ret += test_set_prio(PRIO_PROCESS, who, 20);
    int prio = get_normal_prio(PRIO_PROCESS, who);
    if (errno < 0 || prio != 19)
    {
        err_log("failed to set nice value to the upper bound(19)");
        exit(-1);
    }

    // lower the priority value == increase the priority
    // permission denied if not root
    ret += test_set_prio(PRIO_PROCESS, who, -10);
    return ret;
}
