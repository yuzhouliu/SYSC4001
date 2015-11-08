/* SYSC 4001 Assignment #2
 *
 * Author: Yuzhou Liu, 100853392
 *
 * This is the consumer process that reads from shared memory (circular buffer) and stores into output file
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

/* for bonus */
#include <time.h>
#include <sys/time.h>
#define MICRO_SEC_IN_SEC 1000000

#include "common_shm.h"
#include "common_sem.h"

#define FILE_OUT "_text_output.txt"

void open_file_write(int *file_out);
int copy_to_file(int file_out, struct buffer_block *shared_buffer, int *buffer_index);

int main(void)
{
	// Shared memory. shm1 used for circular buffer, shm2 used for metadata
	int shmid, shmid2;
	void *shared_memory1 = (void *)0;
	void *shared_memory2 = (void *)0;
 	struct buffer_block *shared_buffer;
 	struct metadata *shared_metadata;
 	int buffer_index;
  
  // Semaphores. S: mutual exclusion. N: synch producer before consumer. E: buffer space
  int sem_S_id, sem_N_id, sem_E_id;

  // Output file
  int file_out;
  int nWrite_file; 	// Num bytes written to file

	
	/* Creating shared memory (circular buffer) */
	if (init_shm_circularBuffer(SHM_KEY, &shmid, &shared_memory1) == -1) {
		exit(EXIT_FAILURE);
	}
	printf("[C] Shared Memory (circular buffer) attached at: %p. shmid: %d\n", (int*)shared_memory1, shmid);	
	shared_buffer = (struct buffer_block*) shared_memory1;			// Assigns shared_memory segment to shared_buffer
	buffer_index = 0;

	/* Creating shared memory (metadata) */
	if (init_shm_metadata(SHM2_KEY, &shmid2, &shared_memory2) == -1) {
		exit(EXIT_FAILURE);
	}
	printf("[C] Shared Memory (metadata) attached at %p, shmid: %d\n", (int*)shared_memory2, shmid2);
	shared_metadata = (struct metadata*) shared_memory2;						// Assigns shared_memory2 segment to shared_metadata

	printf(" - - - - - - - - - - - - - - - - - - - - - - \n");
	
	/* Creating semaphores */
	sem_S_id = semget((key_t)SEM_S_KEY, 1, 0666 | IPC_CREAT);
	sem_N_id = semget((key_t)SEM_N_KEY, 1, 0666 | IPC_CREAT);
	sem_E_id = semget((key_t)SEM_E_KEY, 1, 0666 | IPC_CREAT);

  // Open file for writing
  open_file_write(&file_out);
  // Wait for producer first so that the number of bytes read from file is known
  printf("[C] Waiting for producer to get input file size...\n");
  semaphore_wait(sem_N_id);
  printf("[C] File size information from producer: %d bytes\n", shared_metadata->file_byte_count);

  /* Start time difference. For Bonus */
  struct timeval start_time, end_time;
  gettimeofday(&start_time, NULL);


  /* Copy contents from shared memory to the output file */
  printf("[C] Reading from shared memory and writing to file...\n");
  nWrite_file = 0;
	while (nWrite_file < shared_metadata->file_byte_count) {
		semaphore_wait(sem_N_id);
		semaphore_wait(sem_S_id);
		nWrite_file += copy_to_file(file_out, shared_buffer, &buffer_index);
		semaphore_signal(sem_S_id);
		semaphore_signal(sem_E_id);
		printf("[C]	Bytes written to file: %d\n", nWrite_file);
	}

	printf(" - - - - - - - - - - - - - - - - - - - - - - \n");
	printf("[C] Total Bytes written to '%s': %d\n", FILE_OUT, nWrite_file);

	/* Calculate time difference. For Bonus */
	gettimeofday(&end_time, NULL);
	printf(">> %ld microseconds\n", ((end_time.tv_sec * MICRO_SEC_IN_SEC + end_time.tv_usec) - (start_time.tv_sec * MICRO_SEC_IN_SEC + start_time.tv_usec)));

	/* Cleanup */
	// Close file
	if (close(file_out) == -1) {
		fprintf(stderr, "[C] File close failed\n");
	}
	// Shared memory is detached and deleted
  if (shmdt(shared_memory1) == -1) {
    fprintf(stderr, "[C] shmdt failed\n");
    exit(EXIT_FAILURE);
  }
  if (shmctl(shmid, IPC_RMID, 0) == -1) {
    fprintf(stderr, "[C] shmctl(IPC_RMID) failed\n");
    exit(EXIT_FAILURE);
  }
  if (shmdt(shared_memory2) == -1) {
    fprintf(stderr, "[C] shmdt failed\n");
    exit(EXIT_FAILURE);
  }
  if (shmctl(shmid2, IPC_RMID, 0) == -1) {
    fprintf(stderr, "[C] shmctl(IPC_RMID) failed\n");
    exit(EXIT_FAILURE);
  }

  // Delete Semaphores
  del_semvalue(sem_S_id);
  del_semvalue(sem_N_id);
  del_semvalue(sem_E_id);

	exit(EXIT_SUCCESS);
}

/* Opens file as write only */
void open_file_write(int *file_out)
{
	// Delete file if exists
	*file_out = unlink(FILE_OUT);
	if (*file_out == -1) {
		// Ignore file doesn't exist error
		if (errno != ENOENT) {
			fprintf(stderr, "[C] file delete failed with errno: %d\n", errno);
			exit(EXIT_FAILURE);
		}
	}

	// Open the file as write only
	*file_out = open(FILE_OUT, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
	if (*file_out == -1) {
		fprintf(stderr, "[C] file open failed with errno: %d\n", errno);
		exit(EXIT_FAILURE);
	}
}

/* Copies contents from shared memory into file */
int copy_to_file(int file_out, struct buffer_block *shared_buffer, int *buffer_index)
{
	int nWrite = 0;

	nWrite = write(file_out, shared_buffer[*buffer_index].string_data, shared_buffer[*buffer_index].bytes_to_count);	
	if (nWrite == -1) {
		fprintf(stderr, "[C] file write failed with errno: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	if (nWrite != shared_buffer[*buffer_index].bytes_to_count) {
		fprintf(stderr, "[C] Bytes written to file does not match bytes stored in buffer\n");
	}
	*buffer_index = ((*buffer_index)+1)%NUM_BUFFERS;

	return nWrite;
}
