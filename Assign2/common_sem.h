/* SYSC 4001 Assignment #2
 *
 * Author: Yuzhou Liu, 100853392
 *
 * Header file for declaring semaphore: union, keys, and functions 
 */

#ifndef COMMON_SEM_H_
#define COMMON_SEM_H_

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
    /* union semun is defined by including <sys/sem.h> */
#else
    /* according to X/OPEN we have to define it ourselves */
    union semun {
        int val;                    /* value for SETVAL */
        struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
        unsigned short int *array;  /* array for GETALL, SETALL */
        struct seminfo *__buf;      /* buffer for IPC_INFO */
    };
#endif

#define SEM_S_KEY 1742
#define SEM_N_KEY 1743
#define SEM_E_KEY 1744

int set_semvalue(int sem_id, int val);
void del_semvalue(int sem_id);
int semaphore_wait(int sem_id);
int semaphore_signal(int sem_id);

#endif