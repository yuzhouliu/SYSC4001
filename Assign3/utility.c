/*  SYSC 4001, Assignment 3
 *	Student: Yuzhou Liu 100853392
 */

 /* ============================================================================
 * Contains utility functions used by producer
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "utility.h"
#include "master.h"

/* Prints the run queue of each processor into a table format
 */
void print_task_list(cpu_run_queue *cpu_run_queues, int num_cpu)
{
	int cpu_num, rq_num, task_num;
	circular_buffer *curr_buffer;
	task_struct *curr_task;

	// Print header	
	printf("   RQ# | PID | sched_type | priority | expected_exec_t (ms)\n");
	printf("----------------------------------------------------------------\n");
	// Print individual info from each queue
	for(cpu_num = 0; cpu_num < num_cpu; cpu_num++) {
		printf(" >CPU %d:\n", cpu_num);

		for(rq_num = 0; rq_num < NUM_RQ; rq_num++) {
			curr_buffer = &(cpu_run_queues[cpu_num].rq[rq_num]);

			for(task_num = curr_buffer->head; task_num < curr_buffer->tail; task_num++) {
				curr_task = &(curr_buffer->tasks[task_num]);
				print_task_detail(rq_num, curr_task);
			}
		}
	}
	printf("============================================================\n");
}

/* Prints the details of individual tasks
 */
void print_task_detail(int rq_num, task_struct *task)
{
	printf("   RQ%d |", rq_num);
	printf(" %2d  |", task->pid);

	switch(task->sched_type) {
		case MY_SCHED_FIFO:
			printf("  FIFO\t  |");
			break;
		case MY_SCHED_RR:
			printf("  RR\t  |");
			break;
		case MY_SCHED_NORMAL:
			printf("  NORMAL\t  |");
			break;
		default:
			fprintf(stderr, "print_task_detail - unknown sched_type");
	}

	printf(" %3d\t     | %02d\n", task->priority, task->expected_exec_time);
}

/* Prints task action's header
 */
void print_task_action_header(void)
{
	printf("      |     |     |       |             |          | Expected  | Service | Accum    | Turn     \n");
	printf(" CPU# | RQ# | PID | SCHED | State       | Priority | Exec Time | Time    | Service  | Around   \n");
	printf("      |     |     |       |             |          |  (ms)     |  (ms)   | Time(ms) | Time (ms)\n");
	printf(" ----------------------------------------------------------------------------------------------\n");
}

/* Prints info on the action of task
 */
void print_task_action(int cpu_num, int rq_num, task_struct *task, int s_time)
{
	int print_service_t = 0;
	int print_turnaround_t = 0;

	printf(" CPU%d | RQ%d | %02d  |", cpu_num, rq_num, task->pid);

	// Print sched_type
	switch(task->sched_type) {
		case MY_SCHED_FIFO:
			printf(" FIFO  |");
			break;
		case MY_SCHED_RR:
			printf(" RR    |");
			break;
		case MY_SCHED_NORMAL:
			printf(" NORM  |");
			break;
		default:
			fprintf(stderr, "print_task_action - unknown sched_t_ype");
	}

	// Print task state
	switch(task->state) {
		case READY:
			printf(" RUN->READY  |");
			break;
		case RUNNING:
			printf(" READY->RUN  |");
			print_service_t = 1;
			break;
		case FINISHED:
			printf(" RUN->FINISH |");
			print_turnaround_t = 1;
			break;
		default:
			fprintf(stderr, "print_task_action - unknown state");
	}

	// Print priority and expected execution time
	printf(" %3d      | %4d      |", task->priority, task->expected_exec_time);						

	// Print Service Time
	if (print_service_t)
		printf(" %4d    |", s_time);
	else
		printf("   -     |");

	// Print Accumulated Time Slice
	printf(" %4d     |", task->accu_time_slice);

	// Print Turn Around Time
	if (print_turnaround_t)
		printf(" %4d", task->turnaround_time);
	else
		printf("   - ");

	printf("\n");
}

/* Get the next task to run by finding highest priority in a queue
 */
task_struct * get_task_to_run(cpu_run_queue *current_run_queue, int *rq_num, int *buf_index)
{
	int i, j;
	circular_buffer *curr_buffer;

	int temp_priority = 140;
	task_struct *task_to_run = NULL;

	// While the task to run has not been found, go through the RQ in order from 0->1->2
	for(i = 0; (i < NUM_RQ) && (task_to_run == NULL); i++) {
		curr_buffer = &(current_run_queue->rq[i]);

		// If the queue is not empty 
		if( curr_buffer->count != 0 ) {

			// Search for highest priority task in the queue
			for(j = curr_buffer->head; j < curr_buffer->tail; j++) {

				if( (curr_buffer->tasks[j]).priority < temp_priority ) {
					temp_priority = (curr_buffer->tasks[j]).priority;
					task_to_run = &(curr_buffer->tasks[j]);
					*buf_index = j;
					*rq_num = i;
				}				
			}
		}
	}

	return task_to_run;
}

/* Delete the finished task from the circular buffer
 */
void delete_task_from_queue(cpu_run_queue *current_run_queue, int rq_num, int buf_index)
{
	int i;
	circular_buffer *curr_buffer;
	task_struct *task_hole;

	curr_buffer = &(current_run_queue->rq[rq_num]);

	// Shift the tasks forward to replace the deleted task in the buffer
	for(i = buf_index+1; i < curr_buffer->tail; i++) {
		task_hole = &(curr_buffer->tasks[i-1]);
		*task_hole = curr_buffer->tasks[i];
	}
	curr_buffer->tail--;
	curr_buffer->count--;
}

task_struct * move_task_from_rqSrc_to_rqDest(int rq_num_dest, int rq_num_src, cpu_run_queue *current_run_queue, int buf_index)
{
	circular_buffer *dest_circ_buffer, *src_circ_buffer;

	src_circ_buffer = &(current_run_queue->rq[rq_num_src]);
	dest_circ_buffer = &(current_run_queue->rq[rq_num_dest]);

	dest_circ_buffer->tasks[dest_circ_buffer->tail] = src_circ_buffer->tasks[buf_index];	// Transfer task over from src q to dest q
	dest_circ_buffer->tail++;
	dest_circ_buffer->count++;

	delete_task_from_queue(current_run_queue, rq_num_src, buf_index);

	return &( dest_circ_buffer->tasks[dest_circ_buffer->tail-1] );
}

/* Calculates the quantum size for execution
 */
int get_quantum_size(task_struct *task)
{
	int q_size;

	// If task is FIFO then the task runs until completion
	if(task->sched_type == MY_SCHED_FIFO) {
		return task->remaining_exec_time;
	}
	else {
		if(task->priority < 120) {
			q_size = (140 - task->priority)*20;
		}
		else {
			q_size = (140 - task->priority)*5;
		}
	}

	q_size *= Q_SIZE_MULTIPLIER;		// Use a multiplier to slow down output

	if ( q_size > task->remaining_exec_time ) {
		q_size = task->remaining_exec_time;
	}
	return q_size;			
}

int max(int a, int b)
{
	if (a > b)
		return a;
	else
		return b;
}

int min(int a, int b)
{
	if (a < b)
		return a;
	else
		return b;
}

void update_turnaround_time( cpu_run_queue *current_run_queue, int time_slice )
{
	int i, j;
	circular_buffer *curr_buffer;
	task_struct *temp_task;

	// Go through the RQ in order from 0->1->2
	for(i = 0; i < NUM_RQ; i++) {
		curr_buffer = &(current_run_queue->rq[i]);

		// If the queue is not empty 
		if( curr_buffer->count != 0 ) {

			// Search for highest priority task in the queue
			for(j = curr_buffer->head; j < curr_buffer->tail; j++) {

				if( (curr_buffer->tasks[j]).turnaround_started ) {
					temp_task = &(curr_buffer->tasks[j]);
					temp_task->turnaround_time += time_slice;
				}				
			}
		}
	}

}
