/* SYSC 4001 Assignment #2
 *
 * Author: Yuzhou Liu, 100853392
 *
 * File contains definitions of shared memory functions used by both producer and consumer
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>

#include "common_shm.h"

/* Initializes the Circular Buffer shared memory segment */
int init_shm_circularBuffer(int shm_key, int *shmid, void **shared_memory)
{
	*shmid = shmget((key_t)shm_key, sizeof(struct buffer_block)*NUM_BUFFERS, 0666 | IPC_CREAT);
	if (*shmid == -1) {
		fprintf(stderr, "shmget failed\n");
		return -1;
	}

	// Make the shared memory accessible by the program
	*shared_memory = shmat(*shmid, (void *)0, 0);
	if (*shared_memory == (void *)-1) {
		fprintf(stderr, "shmat failed\n");
		return -1;
	}
	return 0;
}

/* Initializes the Metadata shared memory segment */
int init_shm_metadata(int shm_key, int *shmid, void **shared_memory)
{
	*shmid = shmget((key_t)shm_key, sizeof(struct metadata), 0666 | IPC_CREAT);
	if (*shmid == -1) {
		fprintf(stderr, "shmget failed\n");
		return -1;
	}

	// Make the shared memory accessible by the program
	*shared_memory = shmat(*shmid, (void *)0, 0);
	if (*shared_memory == (void *)-1) {
		fprintf(stderr, "shmat failed\n");
		return -1;
	}
	return 0;
}