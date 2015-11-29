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
void print_task_detail(task_struct *task);

#endif /* UTILITY_H */