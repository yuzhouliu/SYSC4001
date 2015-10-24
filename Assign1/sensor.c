/*
*	SYSC 4001 Operating Systems
*	Assignment 1
*	
*	Yuzhou Liu
*	100853392
*
*	Upon init, sensor registers with controller via message queue.
*	Thereafter it generates random data every 2 seconds and sends
*	to controller via message queue.
*/

#include <time.h>
#include <sys/time.h>

#include "common.h"

void first_setup(int msgid, char *sensor_name, int threshold);
void send_sensing_data(int msgid, char *sensor_name, int threshold);
void check_stop_signal(int msgid);

int main(int argc,char *argv[])
{
	int running = 1;
	char sensor_name[MAX_NAME];
	int threshold;
	struct timeval current_timestamp, previous_timestamp;

	int msgid;

	// For generating random numbers
	srand(time(NULL));

	// Get the command line arguments
	if(argc != 3)
	{
		printf("Invalid number of arguments entered. Exiting\n");
		exit(0);
	}	
	strncpy(sensor_name, argv[1], MAX_NAME);
	threshold = atoi(argv[2]);
	printf("sensor_name: %s\n", sensor_name);
	printf("threshold: %d\n", threshold);

	// Init msg queue
	msgid = msgget(ftok(MSG_KEY_PATHNAME, MSG_KEY_ID), 0666 | IPC_CREAT);
	if (msgid == -1)
	{
		printf("msgget failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	first_setup(msgid, sensor_name, threshold);

	gettimeofday(&previous_timestamp, NULL);

	while(running) {
		check_stop_signal(msgid);
		gettimeofday(&current_timestamp, NULL);

		//Generate random numbers every 2 seconds
		if(current_timestamp.tv_sec > (previous_timestamp.tv_sec+2))
		{
			send_sensing_data(msgid, sensor_name, threshold);
			previous_timestamp = current_timestamp;
		}		
	}

	exit(EXIT_SUCCESS);
}

// First setup. Sends device info to controller. Waits for controller acknowledge
void first_setup(int msgid, char *sensor_name, int threshold)
{
	struct msg_structure device_data;
	struct ack_structure server_reply;
	int server_reply_type = getpid();

	// First setup
	memset((void *)&device_data, 0, sizeof(device_data));
	device_data.msg_type = 1;
	device_data.info.device_pid = getpid();
	device_data.info.this_device = sensor;
	device_data.info.first_setup = true;
	strncpy(device_data.info.device_name, sensor_name, MAX_NAME);
	device_data.info.threshold = threshold;

	if (msgsnd(msgid, (void *)&device_data, sizeof(device_data.info), 0) == -1) {
		printf("msgsnd from sensor to server failed\n");
		exit(EXIT_FAILURE);
	}

	// Wait for acknowledge signal from server
	if (msgrcv(msgid, (void *)&server_reply, BUFSIZ, server_reply_type, 0) == -1) {
		printf("msgrcv for device failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	// If ack signal, then continue operation
	// If stop signal, then exit immediately
	if(strncmp(server_reply.message, ACKNOWLEDGE, sizeof(ACKNOWLEDGE)) == 0)
	{
		printf("Server acknowledged device. Continue operation\n");
		return;
	}
	else if (strncmp(server_reply.message, STOP, sizeof(STOP)) == 0)
	{
		printf("Server ordered stop for device. Exiting\n");
		exit(EXIT_SUCCESS);
	}
}

void send_sensing_data(int msgid, char *sensor_name, int threshold)
{
	int ran_data = rand() % 11;
	struct msg_structure device_data;

	printf("Sensor [%s]. Sensing data: %d\n", sensor_name, ran_data);
	if (ran_data > threshold)
		printf("Warning! Sensing Data %d over threshold %d\n", ran_data, threshold);

	device_data.msg_type = 1;
	device_data.info.device_pid = getpid();
	device_data.info.this_device = sensor;
	device_data.info.first_setup = false;
	strncpy(device_data.info.device_name, sensor_name, MAX_NAME);
	device_data.info.threshold = threshold;
	device_data.info.sensing_data = ran_data; 

	if (msgsnd(msgid, (void *)&device_data, sizeof(device_data.info), 0) == -1) {
		printf("msgsnd from sensor to server failed\n");
		exit(EXIT_FAILURE);
	}

}

void check_stop_signal(int msgid)
{
	struct ack_structure server_reply;

	// Check to see if server sent stop signal
	if (msgrcv(msgid, (void *)&server_reply, MAX_TEXT, REQUEST_STOP, IPC_NOWAIT) == -1) {
		if(errno == ENOMSG)	{
			//No instructions for actuator yet, just return
			return;
		}
		printf("msgrcv for device failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	if (strncmp(server_reply.message, STOP, sizeof(STOP)) == 0)
	{
		printf("Server ordered stop for device. Exiting\n");
		exit(EXIT_SUCCESS);
	}
}
