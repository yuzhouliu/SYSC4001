/* SYSC 4001 Assignment #2
 *
 * Author: Yuzhou Liu, 100853392
 *
 * Header file for declaring shared memory: keys, sizes, structs, and functions 
 */

#ifndef COMMON_SHM_H_
#define COMMON_SHM_H_

#define SHM_KEY 1740
#define SHM2_KEY 1741

#define NUM_BUFFERS 100
#define STRING_SIZE 128

/* Struct used for a single buffer block in the circular buffer
 * bytes_to_count stores the number of elements in string_data
 * string_data is a non-null terminated string of the input data
*/
struct buffer_block {
 	int bytes_to_count;
 	char string_data[STRING_SIZE];
};

/* Struct used for passing metadata between producer and consumer
 */
struct metadata {
	int file_byte_count;
};

int init_shm_circularBuffer(int shm_key, int *shmid, void **shared_memory);
int init_shm_metadata(int shm_key, int *shmid, void **shared_memory);

#endif