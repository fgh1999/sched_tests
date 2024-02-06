// SPDX-License-Identifier: MPL-2.0

#ifndef _SCHED_LOG_H
#define _SCHED_LOG_H

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

char proc_tag[32];
char *process_tag()
{
    sprintf(proc_tag, "[Process %d]", getpid());
    return proc_tag;
}
#define log(format, ...) printf("%s " format, process_tag(), ##__VA_ARGS__)

char err_msg[64];
#define err_log(format, ...)                                          \
    do                                                                \
    {                                                                 \
        sprintf(err_msg, "%s " format, process_tag(), ##__VA_ARGS__); \
        perror(err_msg);                                              \
    } while (0)

#endif // _SCHED_LOG_H
