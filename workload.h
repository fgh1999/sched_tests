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

#define PRIME_NUMBERS_COUNT 10
unsigned long long PRIME_NUMBERS[PRIME_NUMBERS_COUNT] = {
    6638502468113671201,
    6347131540809704857,
    7492796293366244137,
    7288037931431975507,
    8068113935371568237,
    4246735778145906659,
    4294294486894962779,
    5833948629861456217,
    2314786123305588893,
    5262866060139588767,
};

void burn_cpu()
{
    for (size_t j = 0; j < 1e5; j++)
    {
        int ans = 0;
        for (size_t i = 0; i < 1e5; i++)
            ans += 1;
    }
}
#endif // _SCHED_WORKLOAD_H
