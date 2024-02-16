// SPDX-License-Identifier: MPL-2.0

#ifndef _SCHED_SHM_H
#define _SCHED_SHM_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>

/// The project id for the shared memory.
const int proj_id = 233;

int getshm_for(const char *pathname, size_t buf_size)
{
    FILE *file = fopen(pathname, "a+");
    if (!file) {
        err_log("fopen");
        exit(EXIT_FAILURE);
    }
    fclose(file);

    key_t key = ftok(pathname, proj_id);
    if (key == -1)
        err_log("ftok");

    int shmid = shmget(key, buf_size, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        err_log("shmget");
        exit(EXIT_FAILURE);
    }
    return shmid;
}

void* _shmat(const int shmid, const int shmflg) {
    void *shmaddr = shmat(shmid, NULL, shmflg);
    if (shmaddr == (void *)-1) {
        err_log("shmat");
        exit(EXIT_FAILURE);
    }
    return shmaddr;
}

void _shmdt(const void* shmaddr) {
    if (shmdt(shmaddr) < 0) {
        err_log("shmdt");
        exit(EXIT_FAILURE);
    }
}

void delshm(const int shmid) {
    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
        err_log("shmctl");
        exit(EXIT_FAILURE);
    }
}

#endif // _SCHED_SHM_H
