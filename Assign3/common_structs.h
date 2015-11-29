/*  SYSC 4001, Assignment 3
 *	Student: Yuzhou Liu 100853392
 */
#ifndef COMMON_STRUCTS_H
#define COMMON_STRUCTS_H

/* ============================================================================
 * This header file defines the multilevel run queue structure for each CPU as:
 *												CPU
 *												|
 *												-----> RQ0 -> FIFO and RR tasks [t0, t1, t2, ...]
 *												|
 *												-----> RQ1 -> NORMAL tasks prio < 130 [t0, t1, t2, ...]
 *												|
 *												-----> RQ2 -> NORMAL tasks prio >= 130 [t0, t1, t2, ...]
 * =============================================================================
 */

#define MAX_QUEUE_SIZE 10		// Max processor queue size

/* Defines that specify schedule type */
#define MY_SCHED_FIFO 0
#define MY_SCHED_RR 1
#define MY_SCHED_NORMAL 2

/* Structure to store Process information */
typedef struct {
	int pid;									// Process ID
	int sched_type;						// Schedule type (ex: SCHED_FIFO, SCHED_RR, SCHED_NORMAL)
	int static_prio;					// Static priority
	int prio;									// Dynamic priority
	int expected_exec_time;		// Expected Execution time
	
	int time_slice;						// Time slice
	int accu_time_slice;			// Accumulated time slice
	int last_cpu;							// Last CPU the process was executed on
} task_struct;

/* Circular buffer to store Processes/tasks */
typedef struct {
	int head;
	int tail;
	int count;
	task_struct tasks[MAX_QUEUE_SIZE];
} circular_buffer;

/* The multi-level queue for each CPU. 
 * Contains 3 queues:
 *  RQ0 -> real-time applications SCHED_FIFO and SCHED_RR, priority range 0 <= static_prio < 100
 *  RQ1 -> SCHED_NORMAL with dynamic priority of 100 <= prio < 130
 *  RQ2 -> SCHED_NORMAL with dynamic priority of 130 <= prio
 */
typedef struct {
	circular_buffer rq0;
	circular_buffer rq1;
	circular_buffer rq2;
} cpu_run_queue;

#endif /* COMMON_STRUCTS_H */