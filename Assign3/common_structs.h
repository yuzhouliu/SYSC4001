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

#define NUM_RQ 3						// Number of RQs

#define MAX_QUEUE_SIZE 10		// Max processor queue size

/* Defines that specify schedule type */
#define MY_SCHED_FIFO 0
#define MY_SCHED_RR 1
#define MY_SCHED_NORMAL 2

/* Defines that specify state of task */
#define READY 0
#define RUNNING 1
#define FINISHED 2

/* Define for the max sleep avg of 10ms */
#define MAX_SLEEP_AVG 10

/* Structure to store Process information */
typedef struct {
	int pid;									// Process ID
	int state;								// State: running, ready, or finished
	int turnaround_started;		// Task was ran but now queued
	int sched_type;						// Schedule type (ex: SCHED_FIFO, SCHED_RR, SCHED_NORMAL)
	int priority;							// Priority (Static for FIFO and RR, Dynamic for NORMAL)
	int expected_exec_time;		// Expected Total Service Time
	
	int sleep_avg;						// Average sleep time
	int time_slice;						// Time slice
	int accu_time_slice;			// Accumulated time slice
	int remaining_exec_time;	// Remaining execution time
	int last_cpu;							// Last CPU the process was executed on
	int turnaround_time;			// Turn around time
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
	circular_buffer rq[NUM_RQ];
} cpu_run_queue;

#endif /* COMMON_STRUCTS_H */