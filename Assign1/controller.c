/*
*	SYSC 4001 Operating Systems
*	Assignment 1
*	
*	Yuzhou Liu
*	100853392
*
*	Upon init controller forks into child and parent processes
*	Child is responsible in taking in data from sensor and setting 
*	actuator active.
*	Parent is responsible in talking to the cloud and and receiving 
*	threshold crossed messages from Child via signal and message queue.
*/

#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "common.h"

#define FIFO_NAME "/tmp/yuzhou_fifo"

void child_process_handler();
void parent_process_handler();
void retrieve_msg(int signum);
void print_received_data(struct msg_structure *data);
void device_setup(int msgid, struct msg_structure *data);
void receive_sensing_data(int msgid, struct msg_structure *data);
int search_sensor_list_for_pid(pid_t pid);
void activate_actuator(int msgid, int actuator_num);
void send_msg_to_parent(int msgid, struct msg_structure *data, int actuator_num);
void sigint_handler(int signum);	

static pid_t sensor_list[MAX_SENSOR_NUM];
static pid_t actuator_list[MAX_ACTUATOR_NUM];
static int num_sensors = 0;
static int num_actuators = 0;

static sig_atomic_t retrieve_msg_flag = 0;
static sig_atomic_t kill_process = 0;


int main(int argc, char *argv[])
{
	pid_t pid;
	int msgid;
		
	// Handle SIGINT signal (CTRL+C)
	struct sigaction act;
	act.sa_handler = sigint_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, 0);	

	// Creates and flushes a message queue
	msgid = msgget(ftok(MSG_KEY_PATHNAME, MSG_KEY_ID), 0666 | IPC_CREAT);
	if (msgid == -1)
	{
		printf("msgget failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	if (msgctl(msgid, IPC_RMID, 0) == -1) {
		printf("msgctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}

	pid = fork();

	switch(pid)
	{
		case -1:
			printf("fork failed\n");
			exit(EXIT_FAILURE);
		case 0:
			child_process_handler();
			break;
		default:
			parent_process_handler();
			break;
	}

	exit(EXIT_SUCCESS);
}

void child_process_handler()
{
	int msgid;
	struct msg_structure data;
	struct ack_structure end_devices;
	int i;

	int running = 1;

	printf("Child process started with PID %d\n", getpid());

	msgid = msgget(ftok(MSG_KEY_PATHNAME, MSG_KEY_ID), 0666 | IPC_CREAT);
	if (msgid == -1)
	{
		printf("msgget failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	while(running) 
	{
		// Retrieve messages from the message queue
		if (msgrcv(msgid, (void *)&data, BUFSIZ, DEVICE_MSG, 0) == -1) {
			if (errno == EINTR) {
				printf("Caught (CTRL+C) signal while blocked on msgrc. Cleaning up and ending\n");
				
				// STOP devices
				end_devices.msg_type = REQUEST_STOP;
				strncpy(end_devices.message, STOP, MAX_TEXT);
				for(i = 0; i < (num_sensors+num_actuators); i++)
				{
					if (msgsnd(msgid, (void *)&end_devices, sizeof(end_devices.message), 0) == -1) {
						printf("msgsnd from server to sensor failed\n");
						exit(EXIT_FAILURE);
					}
				}

				// Delete the message queue
				if (msgctl(msgid, IPC_RMID, 0) == -1) {
					fprintf(stderr, "msgctl(IPC_RMID) failed\n");
					exit(EXIT_FAILURE);
				}
				exit(EXIT_SUCCESS);	
			}
			fprintf(stderr, "msgrcv failed with error: %d\n", errno);
			exit(EXIT_FAILURE);		
		}

		// If this is first msg from device, perform device setup
		// Otherwise, take in the sensing data
		if (data.info.first_setup) {
			printf("\n[CHILD]\n");
			device_setup(msgid, &data);
		}
		else if (data.info.this_device == sensor) {
			receive_sensing_data(msgid, &data);
		}
	}
	if (msgctl(msgid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "msgctl(IPC_RMID) failed\n");
					exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}

void parent_process_handler()
{
	int msgid;
	int running = 1;
	//struct msg_structure msg_received;
	struct ack_structure msg_received2;
	char end_cloud[MAX_TEXT];

	// For FIFO pipe
	int pipe_fd;
	int res;
	int open_mode = O_WRONLY;

	struct sigaction sa;
	memset((void *)&sa, 0, sizeof(sa));
	sa.sa_handler = &retrieve_msg;
	sigaction(SIGUSR1, &sa, 0);

	printf("Parent process started with PID %d\n", getpid());

	msgid = msgget(ftok(MSG_KEY_PATHNAME, MSG_KEY_ID), 0666 | IPC_CREAT);
	if (msgid == -1)
	{
		printf("msgget failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	// Try to access FIFO. If it is not there, create one
	if(access(FIFO_NAME, F_OK) == -1) {
		res = mkfifo(FIFO_NAME, 0777);
		if (res != 0) {
			fprintf(stderr, "Could not create fifo %s\n", FIFO_NAME);
			exit(EXIT_FAILURE);
		}
	}
	printf("Parent process opening FIFO O_WRONLY\n");
	pipe_fd = open(FIFO_NAME, open_mode);
	if (pipe_fd == -1)
	{
		fprintf(stderr, "Error opening fifo\n");
		exit(EXIT_FAILURE);
	}
	
	while(running)
	{

		if (kill_process)
		{
			strncpy(end_cloud, "end", MAX_TEXT);
			res = write(pipe_fd, (void *)&end_cloud, sizeof(end_cloud));
			if(res == -1) {
				fprintf(stderr, "Write error on pipe\n");
				exit(EXIT_FAILURE);
			}
			close(pipe_fd);
			exit(EXIT_SUCCESS);	
		}

		if(retrieve_msg_flag)
		{
			if (msgrcv(msgid, (void *)&msg_received2, MAX_TEXT, CHILD_TO_PARENT, 0) == -1) {
				printf("debug msgrcv failed with error: %d\n", errno);
				exit(EXIT_FAILURE);		
			}
			//printf("\n[PARENT] %s\n", msg_received2.message);

			printf("[PARENT] Passing message to the cloud\n");

			res = write(pipe_fd, (void *)&msg_received2.message, sizeof(msg_received2.message));
			if(res == -1) {
				fprintf(stderr, "Write error on pipe\n");
				exit(EXIT_FAILURE);
			}

			retrieve_msg_flag = 0;
		}

	}

	close(pipe_fd);
	exit(EXIT_SUCCESS);
}

// Signal handler for SIGUSR1
void retrieve_msg(int signum)
{
	retrieve_msg_flag = 1;
}

void print_received_data(struct msg_structure *data)
{
	if (data->info.this_device == sensor)
		printf("  device type: sensor\n");
	else
		printf("  device type: actuator\n");
	printf("  pid: %d, name: %s, threshold: %d, sensing_data: %d\n", data->info.device_pid, data->info.device_name, data->info.threshold, data->info.sensing_data);
}

// First instance of device contact with server. Server responds with acknowledge
void device_setup(int msgid, struct msg_structure *data)
{
	struct ack_structure server_reply;
	
	printf("Device first setup\n");
	print_received_data(data);

	if (data->info.this_device == sensor) {
		sensor_list[num_sensors++] = data->info.device_pid;
	}
	else if (data->info.this_device == actuator) {
		actuator_list[num_actuators++] = data->info.device_pid;
	}

	server_reply.msg_type = data->info.device_pid;
	strncpy(server_reply.message, ACKNOWLEDGE, sizeof(ACKNOWLEDGE));

	if (msgsnd(msgid, (void *)&server_reply, sizeof(server_reply.message), 0) == -1) {
		printf("msgsnd from server to device failed\n");
		exit(EXIT_FAILURE);
	}

	printf("number of sensors: %d\n", num_sensors);
	printf("number of actuators: %d\n", num_actuators);
}

void receive_sensing_data(int msgid, struct msg_structure *data)
{
	//supress the print according to prof
	//print_received_data(data);
	int sensor_num, actuator_num;

	if(data->info.sensing_data > data->info.threshold) {
		printf("\n[CHILD] Sensor [%s]. Sensor value %d is over threshold %d\n", data->info.device_name, data->info.sensing_data, data->info.threshold);

		// Activate actuator1 for sensor1, actuator2 for sensor2, etc
		sensor_num = search_sensor_list_for_pid(data->info.device_pid);
		actuator_num = sensor_num;
		
		send_msg_to_parent(msgid, data, actuator_num);

		if ( num_actuators <= actuator_num ) {
			printf("Actuator does not exist yet.\n");
			return;
		}		
		activate_actuator(msgid, actuator_num);		
	}
}

int search_sensor_list_for_pid(pid_t pid)
{
	int i;
	for(i = 0; i < MAX_SENSOR_NUM; i++)
	{
		if(sensor_list[i] == pid)
			return i;
	}
	return -1;
}

void activate_actuator(int msgid, int actuator_num)
{
	static int sequence_number = 0;
	struct actuator_structure actuator_instruction;
	struct actuator_reply_structure actuator_reply;
	struct ack_structure end_devices;
	int i;

	printf("Activating actuator: [%d]\n", actuator_num);

	actuator_instruction.msg_type = actuator_list[actuator_num];
	actuator_instruction.actuator_instruct.actuator_active = true;
	actuator_instruction.actuator_instruct.sequence_num = sequence_number;

	if (msgsnd(msgid, (void *)&actuator_instruction, sizeof(actuator_instruction.actuator_instruct), 0) == -1) {
		printf("msgsnd from server to device failed\n");
		exit(EXIT_FAILURE);
	}

	if (msgrcv(msgid, (void *)&actuator_reply, BUFSIZ, ACTUATOR_REPLY, 0) == -1) {
		if (errno == EINTR) {
			printf("Caught (CTRL+C) signal while blocked on msgrc. Cleaning up and ending\n");
				
			// STOP devices
			end_devices.msg_type = REQUEST_STOP;
			strncpy(end_devices.message, STOP, MAX_TEXT);
			for(i = 0; i < (num_sensors+num_actuators); i++)
			{
				if (msgsnd(msgid, (void *)&end_devices, sizeof(end_devices.message), 0) == -1) {
					printf("msgsnd from server to sensor failed\n");
					exit(EXIT_FAILURE);
				}
			}

			// Delete the message queue
			if (msgctl(msgid, IPC_RMID, 0) == -1) {
				fprintf(stderr, "msgctl(IPC_RMID) failed\n");
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);	
		}
		printf("msgrcv failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	printf("Sequence num [%d], [%s]\n", actuator_reply.reply_info.sequence_num, actuator_reply.reply_info.actuator_reply);

	sequence_number++;
}

void send_msg_to_parent(int msgid, struct msg_structure *data, int actuator_num)
{	
	struct ack_structure msg_to_parent2;
	
	msg_to_parent2.msg_type = CHILD_TO_PARENT;
	snprintf(msg_to_parent2.message, MAX_TEXT, "Sensor %s is sensing %d with threshold %d. Activating actuator %d", data->info.device_name, data->info.sensing_data, data->info.threshold, actuator_num);
	if (msgsnd(msgid, (void *)&msg_to_parent2, MAX_TEXT, 0) == -1) {
		printf("msgsnd from child to parent failed\n");
		exit(EXIT_FAILURE);
	}

	kill(getppid(), SIGUSR1);
}

void sigint_handler(int signum)
{
	kill_process = 1;
}
