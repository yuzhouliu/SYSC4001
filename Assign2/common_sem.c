/* SYSC 4001 Assignment #2
 *
 * Author: Yuzhou Liu, 100853392
 *
 * File contains definitions of semaphor functions used by both producer and consumer 
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>

#include "common_sem.h"

/* The function set_semvalue initializes the semaphore using the SETVAL command in a
 semctl call. We need to do this before we can use the semaphore. */
int set_semvalue(int sem_id, int val)
{
    union semun sem_union;

    sem_union.val = val;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);
    return(1);
}

/* The del_semvalue uses the command IPC_RMID to remove the semaphore's ID. */
void del_semvalue(int sem_id)
{
    union semun sem_union;
    
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore\n");
}

/* semaphore_wait changes the semaphore by -1, Wait. */
int semaphore_wait(int sem_id)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* wait */
    sem_b.sem_flg = 0;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_wait failed\n");
        return(0);
    }
    return(1);
}

/* semaphore_signal changes the semaphore by +1, Signal */
int semaphore_signal(int sem_id)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* signal */
    sem_b.sem_flg = 0;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_signal failed\n");
        return(0);
    }
    return(1);
}
