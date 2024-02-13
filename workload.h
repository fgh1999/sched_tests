// SPDX-License-Identifier: MPL-2.0

#ifndef _SCHED_WORKLOAD_H
#define _SCHED_WORKLOAD_H

#include <stdio.h>
#include <unistd.h>

/**
 * @brief Sleep for a given number of seconds.
 *
 * @param total_seconds The total number of seconds to sleep.
 * @param interval The interval at which to have a break.
 *
 * @return The number of seconds remaining to sleep if the sleep was interrupted.
 */
unsigned int sleep_for(unsigned int total_seconds, unsigned int interval)
{
    unsigned int remaining_seconds = 0; // remaining seconds in a sleep call
    while (total_seconds > 0)
    {
        if (total_seconds < interval)
        {
            remaining_seconds = sleep(total_seconds);
            break;
        }
        total_seconds -= interval;
        remaining_seconds = sleep(interval);
    }
    return remaining_seconds + total_seconds;
}

#endif // _SCHED_WORKLOAD_H
