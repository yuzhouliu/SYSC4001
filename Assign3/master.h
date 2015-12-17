/*  SYSC 4001, Assignment 3
 *	Student: Yuzhou Liu 100853392
 */

#ifndef MASTER_H
#define MASTER_H

#include <pthread.h>

#define NUM_CPU 4
#define Q_SIZE_MULTIPLIER 3 		// Used to give more q_size to slow down output

extern int num_processes;											// Num of processes as user input
extern cpu_run_queue cpu_run_queues[NUM_CPU];
extern pthread_mutex_t queue_mutex[NUM_CPU];  // Protects cpu_run_queues

void *producer_thread_function(void *arg);
void *consumer_thread_function(void *arg);
void *balance_run_queue_thread_function(void *arg);

#endif /* MASTER_H */