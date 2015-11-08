/* SYSC 4001 Assignment #2
 *
 * Author: Yuzhou Liu, 100853392
 *
 * This is the consumer process that reads from input buffer into shared memory (circular buffer)
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
#include <sys/stat.h>

#include "common_shm.h"
#include "common_sem.h"

#define FILE_IN "_text_input.txt"

int write_to_shared_memory(struct buffer_block *shared_buffer_head, int *buffer_index, char *input_buffer, int ncopied, int nread);

int main(void)
{
	// Shared memory. shm1 used for circular buffer, shm2 used for metadata
	int shmid, shmid2;
	void *shared_memory1 = (void *)0;
	void *shared_memory2 = (void *)0;
 	struct buffer_block *shared_buffer_head;
 	struct metadata *shared_metadata;
 	int buffer_index;  

  // Semaphores. S: mutual exclusion. N: synch producer before consumer. E: buffer space
	int sem_S_id, sem_N_id, sem_E_id;

	// Input file for reading
	int file_in;
	int file_size;
	struct stat st;  
  
  // Buffer for storing contents read from file
  char input_buffer[BUFSIZ];
  // Num bytes read from file, and num bytes written to shared memory
  int nRead, nWrite_shm, nWrite_shmTot;


  /* Creating shared memory (circular buffer) */
 	if (init_shm_circularBuffer(SHM_KEY, &shmid, &shared_memory1) == -1) {
		exit(EXIT_FAILURE);
	}
	printf("[P] Shared Memory (circular buffer) attached at: %p. shmid: %d\n", (int*)shared_memory1, shmid);	
	shared_buffer_head = (struct buffer_block*) shared_memory1;		// Assigns shared_memory segment to shared_buffer_head
	buffer_index = 0;

	/* Creating shared memory (metadata) */
	if (init_shm_metadata(SHM2_KEY, &shmid2, &shared_memory2) == -1) {
		exit(EXIT_FAILURE);
	}
	printf("[P] Shared Memory (metadata) attached at: %p. shmid: %d\n", (int*)shared_memory2, shmid2);
	shared_metadata = (struct metadata*) shared_memory2;					// Assigns shared_memory segment to shared_metadata

	printf(" - - - - - - - - - - - - - - - - - - - - - - \n");
	
	/* Creating and initializing semaphores */
	sem_S_id = semget((key_t)SEM_S_KEY, 1, 0666 | IPC_CREAT);
	sem_N_id = semget((key_t)SEM_N_KEY, 1, 0666 | IPC_CREAT);
	sem_E_id = semget((key_t)SEM_E_KEY, 1, 0666 | IPC_CREAT);

	if (!set_semvalue(sem_S_id, 1) || !set_semvalue(sem_N_id, 0) || !set_semvalue(sem_E_id, NUM_BUFFERS)) {
    fprintf(stderr, "[P] Failed to initialize semaphore\n");
    exit(EXIT_FAILURE);
  }

	/* Get size of file in bytes and store to shared_metadata */
	stat(FILE_IN, &st);
	file_size = st.st_size;
	shared_metadata->file_byte_count = file_size;
	printf("[P] Input File size in bytes: %d\n", shared_metadata->file_byte_count);
	semaphore_signal(sem_N_id);

	/* Open the file as read only */
	file_in = open(FILE_IN, O_RDONLY);
	if (file_in == -1) {
		fprintf(stderr, "[P] file open failed with errno: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	/* Read the file into input_buffer in chunks of BUFSIZ. After each read store contents into shared memory
	 * in sizes of STRING_SIZE.
	 * Repeat until the whole file has been read and stored into shared memory
	 */
	nWrite_shmTot = 0;
	nWrite_shm = 0;
	printf("[P] Reading from input file '%s' and storing into shared memory...\n", FILE_IN);
	while (nWrite_shmTot < file_size) {
		nRead = read(file_in, input_buffer, BUFSIZ);
		if (nRead == -1) {
			fprintf(stderr, "[P] read failed with errno: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		else if (nRead == 0) {
			break;		//EOF reached
		}

		while (nWrite_shm < nRead) {
			semaphore_wait(sem_E_id);
			semaphore_wait(sem_S_id);
			// Write from input_buffer into shared memory in sizes of STRING_SIZE
			nWrite_shm += write_to_shared_memory(shared_buffer_head, &buffer_index, input_buffer, nWrite_shm, nRead);
			semaphore_signal(sem_S_id);
			semaphore_signal(sem_N_id);			
		}
		nWrite_shmTot += nWrite_shm;
		printf("[P]	Bytes written to shared memory: %d\n", nWrite_shmTot);
		nWrite_shm = 0;
	}
	
	printf(" - - - - - - - - - - - - - - - - - - - - - - \n");
	printf("[P] Total Bytes written from '%s' to shared memory: %d\n", FILE_IN, nWrite_shmTot);
	sleep(2);		// Wait for consumer to finish so semaphores can be released

	/* Cleanup */
	// Close file
	if (close(file_in) == -1) {
		fprintf(stderr, "[P] File close failed\n");
	}
	// Shared memory is detached
  if (shmdt(shared_memory1) == -1) {
    fprintf(stderr, "[P] shmdt failed\n");
    exit(EXIT_FAILURE);
  }
  if (shmdt(shared_memory2) == -1) {
    fprintf(stderr, "[P] shmdt failed\n");
    exit(EXIT_FAILURE);
  }

	exit(EXIT_SUCCESS);
}


/* Writes from the input buffer into shared memory. 
 * Divides input_buffer into chunks of STRING_SIZE to be stored into shared_buffer_head[*buffer_index]
 * Returns bytes written
 */
int write_to_shared_memory(struct buffer_block *shared_buffer_head, int *buffer_index, char *input_buffer, int ncopied, int nread)
{
	int bytes_to_write = 0;				// Num of bytes written for the current buffer block
	
	// If bytes left is greater than a single buffer can hold, then write as much as STRING_SIZE
	if ((nread - ncopied) > STRING_SIZE)
		bytes_to_write = STRING_SIZE;
	else
		bytes_to_write = nread-ncopied;
	//printf("[P] bytes to write: %d\n", bytes_to_write);

	// Write to shared memory
	shared_buffer_head[*buffer_index].bytes_to_count = bytes_to_write;
	strncpy(shared_buffer_head[*buffer_index].string_data, input_buffer+ncopied, STRING_SIZE);

	// Update to the next buffer block index to write from in circular buffer
	*buffer_index = ((*buffer_index)+1)%NUM_BUFFERS;

	return bytes_to_write;
}
