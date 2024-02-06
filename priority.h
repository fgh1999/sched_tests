// SPDX-License-Identifier: MPL-2.0

#ifndef _SCHED_PRIO_H
#define _SCHED_PRIO_H

#include <stdio.h>
#include <errno.h>
#include <sys/resource.h>
#include <unistd.h>
#include "log.h"

int get_normal_prio(int which, int who)
{
    errno = 0;
    int prio = getpriority(which, who);
    if (prio == -1 && errno)
    {
        err_log("get_normal_prio(%d, %d)", which, who);
        return errno;
    }
    return prio;
}

int set_normal_prio(int which, int who, int prio)
{
    errno = 0;
    if (setpriority(which, who, prio))
    {
        err_log("set_normal_prio(%d, %d, %d)", which, who, prio);
        return errno;
    }
    return 0;
}

#endif // _SCHED_PRIO_H
