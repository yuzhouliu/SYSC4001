#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_CPU 4
#define NUM_TASKS_PER_CPU 5

void *producer_thread_function(void *arg);
void *consumer_thread_function(void *arg);

int consumer_finished = 0;

typedef struct {
	int pid;
	int static_prio;
	int prio;
	int expected_exec_time;
	
	int time_slice;
	int accu_time_slice;
	int last_cpu;
} task_struct;

typedef struct {
	int head;
	int tail;
	int num_process;
	task_struct tasks[NUM_TASKS_PER_CPU];
} run_queue;

run_queue global_run_queue[NUM_CPU];

int main() {
    int res;
	pthread_t producer_thread;
	pthread_t consumer_thread[NUM_CPU];
    void *thread_result;
    int thread_num;

	res = pthread_create(&producer_thread, NULL, producer_thread_function, NULL);
	if (res != 0) {
		perror("Producer thread creation failed\n");
		exit(EXIT_FAILURE);
	}
	sleep(1);		// Delay for producer to finish producing tasks
	
	for(thread_num = 0; thread_num < NUM_CPU; thread_num++) {
		res = pthread_create(&(consumer_thread[thread_num]), NULL , consumer_thread_function, (void *)&thread_num);
		if (res != 0) {
			perror("Consumer thread creation failed\n");
			exit(EXIT_FAILURE);
		}
		sleep(1);
	}
	printf("Waiting for threads to finish\n");
	for(thread_num = NUM_CPU-1; thread_num >= 0; thread_num--) {
		res = pthread_join(consumer_thread[thread_num], &thread_result);
		if(res != 0) {
			perror("pthread_join failed");
		}
	}
	consumer_finished = 1;
	
	res = pthread_join(producer_thread, &thread_result);
	if( res != 0) {
		perror("pthread_join failed");
	}
			
	printf("All done\n");
    exit(EXIT_SUCCESS);
}

void *consumer_thread_function(void *arg) {
	int i;
	int this_num = *(int *)arg;
	task_struct current_task;
	
    printf("This is consumer %d\n", this_num);
    for(i = 0; i < global_run_queue[this_num].num_process; i++) {
    	current_task = (global_run_queue[this_num]).tasks[i];
    	printf("consumed: %d\n", current_task.pid);
    }
    printf("Bye from %d\n", this_num);
    pthread_exit(NULL);
}

void *producer_thread_function(void *arg) {
	int i;
	int curr_cpu;
	int buffer_tail;
	task_struct *current_task;
	int temp_pid = 0;
	
	printf("This is producer\n");
	for(i = 0; i < NUM_CPU * NUM_TASKS_PER_CPU; i++) {
		curr_cpu = i % 4;
		buffer_tail = global_run_queue[curr_cpu].tail;
		
		current_task = &((global_run_queue[curr_cpu]).tasks[buffer_tail]);
		current_task->pid = temp_pid++;
		global_run_queue[curr_cpu].tail++;
		
		global_run_queue[curr_cpu].num_process++;
	}
	while(!consumer_finished) {}
	pthread_exit(NULL);
}


