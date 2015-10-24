/*
*	SYSC 4001 Operating Systems
*	Assignment 1
*	
*	Yuzhou Liu
*	100853392
*/

#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>

#include "common.h"

#define FIFO_NAME "/tmp/yuzhou_fifo"

void print_received_data(struct msg_structure *data);

int main(int argc, char *argv[])
{
	printf("Cloud process started.\n");

	int running = 1;

	int pipe_fd;
	int res;
	int open_mode = O_RDONLY;
	//struct msg_structure received_msg;
	char received_msg[MAX_TEXT];

	memset((void *)&received_msg, '\0', sizeof(received_msg));

	// Try to access FIFO. If it is not there, create one
	if(access(FIFO_NAME, F_OK) == -1) {
		res = mkfifo(FIFO_NAME, 0777);
		if (res != 0) {
			fprintf(stderr, "Could not create fifo %s\n", FIFO_NAME);
			exit(EXIT_FAILURE);
		}
	}

	pipe_fd = open(FIFO_NAME, open_mode);
	if (pipe_fd == -1) {
		printf("Opening the FIFO failed. Exiting\n");
		exit(EXIT_FAILURE);
	}

	while(running)
	{
		res = read(pipe_fd, (void *)&received_msg, MAX_TEXT);
		if(res == -1) {
			fprintf(stderr, "Read error on pipe\n");
			exit(EXIT_FAILURE);
		}

		if (strncmp(received_msg, "end", MAX_TEXT) == 0)
		{
			running = 0;
		}
		printf("\n[Cloud] Received:\n");
		printf("%s\n", received_msg);
	}
	
	exit(EXIT_SUCCESS);
}
