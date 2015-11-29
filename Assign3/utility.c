/*  SYSC 4001, Assignment 3
 *	Student: Yuzhou Liu 100853392
 */

 /* ============================================================================
 * Contains utility functions used by producer
 * =============================================================================
 */

#include <stdio.h>

#include "utility.h"

/* Prints the run queue of each processor into a table format
 */
void print_task_list(cpu_run_queue *cpu_run_queues, int num_cpu)
{
	int i, j;
	task_struct *curr_task;

	// Print header
	
	printf("   PID | sched  | static_prio | dynamic_prio | exec_time\n");
	printf("------------------------------------------------------------------\n");
	// Print individual info from each queue
	for(i = 0; i < num_cpu; i++) {
		printf(">CPU: %d\n", i);

		// Print tasks in RQ0
		printf(" -RQ0:\n");
		for(j = cpu_run_queues[i].rq0.head; j < cpu_run_queues[i].rq0.tail; j++) {
			curr_task = &(cpu_run_queues[i].rq0.tasks[j]);
			print_task_detail(curr_task);
		}

		// Print tasks in RQ1
		printf(" -RQ1:\n");
		for(j = cpu_run_queues[i].rq1.head; j < cpu_run_queues[i].rq1.tail; j++) {
			curr_task = &(cpu_run_queues[i].rq1.tasks[j]);
			print_task_detail(curr_task);
		}

		// Print tasks in RQ2
		printf(" -RQ2:\n");
		for(j = cpu_run_queues[i].rq2.head; j < cpu_run_queues[i].rq2.tail; j++) {
			curr_task = &(cpu_run_queues[i].rq2.tasks[j]);
			print_task_detail(curr_task);
		}
	}
	printf("==============================================================\n");
}

/* Prints the details of individual tasks
 */
void print_task_detail(task_struct *task)
{
	printf("   %02d  |", task->pid);

	switch(task->sched_type) {
		case MY_SCHED_FIFO:
			printf(" FIFO   |");
			break;
		case MY_SCHED_RR:
			printf(" RR     |");
			break;
		case MY_SCHED_NORMAL:
			printf(" NORMAL |");
			break;
		default:
			fprintf(stderr, "print_task_detail - unknown sched_type");
	}

	printf(" %03d\t      | %03d\t     | %02d\n", task->static_prio, task->prio, task->expected_exec_time);
}
