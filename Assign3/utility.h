/*  SYSC 4001, Assignment 3
 *	Student: Yuzhou Liu 100853392
 */

#ifndef UTILITY_H
#define UTILITY_H

 /* ============================================================================
 * Contains utility functions used by producer
 * =============================================================================
 */

#include "common_structs.h"

/* Prints the run queue of each processor into a table format
 */
void print_task_list(cpu_run_queue *cpu_run_queues, int num_cpu);

/* Prints the details of individual tasks
 */
void print_task_detail(int rq_num, task_struct *task);

/* Prints task action's header
 */
void print_task_action_header(void);

/* Prints info on the action of task
 */
void print_task_action(int cpu_num, int rq_num, task_struct *task, int s_time);

/* Get the next task to run by finding highest priority in a queue
 */
task_struct * get_task_to_run(cpu_run_queue *current_run_queue, int *rq_num, int *buf_index);

/* Delete the finished task from the circular buffer
 */
void delete_task_from_queue(cpu_run_queue *current_run_queue, int rq_num, int buf_index);

/* Move task from src queue to destination queue
 */
task_struct * move_task_from_rqSrc_to_rqDest(int rq_num_dest, int rq_num_src, cpu_run_queue *current_run_queue, int buf_index);

/* Calculates the quantum size for execution (in ms)
 */
int get_quantum_size(task_struct *task);

int max(int a, int b);

int min(int a, int b);

void update_turnaround_time( cpu_run_queue *current_run_queue, int time_slice );

#endif /* UTILITY_H */