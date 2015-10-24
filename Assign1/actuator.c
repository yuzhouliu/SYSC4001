/*
*	SYSC 4001 Operating Systems
*	Assignment 1
*	
*	Yuzhou Liu
*	100853392
*
*	Upon init, actuator registers with controller via message queue.
*	Thereafter it checks message queue every 1 second to see if controller
* 	has requested for it to become active.
*/

#include <sys/time.h>
#include <time.h>

#include "common.h"

void first_setup(int msgid, char *actuator_name);
void check_actuator_action(int msgid, bool *actuator_running, char *actuator_name);
void check_stop_signal(int msgid);

int main(int argc,char *argv[])
{
	int running = 1;

	char actuator_name[MAX_NAME];
	bool actuator_running = false;
	struct timeval current_timestamp, previous_timestamp;

	int msgid;

	// Get the command line arguments
	if(argc != 2)
	{
		printf("Invalid number of arguments entered. Exiting\n");
		exit(0);
	}	
	strncpy(actuator_name, argv[1], MAX_NAME);
	printf("actuator_name: %s\n", actuator_name);

	// Init msg queue
	msgid = msgget(ftok(MSG_KEY_PATHNAME, MSG_KEY_ID), 0666 | IPC_CREAT);
	if (msgid == -1)
	{
		printf("msgget failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	first_setup(msgid, actuator_name);

	gettimeofday(&previous_timestamp, NULL);

	while(running) {
		check_stop_signal(msgid);
		gettimeofday(&current_timestamp, NULL);

		// Check for actuator action every second
		if(current_timestamp.tv_sec > (previous_timestamp.tv_sec+1))
		{
			check_actuator_action(msgid, &actuator_running, actuator_name);
			previous_timestamp = current_timestamp;
		}
	}

	exit(EXIT_SUCCESS);
}

// First setup. Sends device info to controller. Waits for controller acknowledge
void first_setup(int msgid, char *actuator_name)
{
	struct msg_structure device_data;
	struct ack_structure server_reply;
	int server_reply_type = getpid();

	// First setup
	memset((void *)&device_data, 0, sizeof(device_data));
	device_data.msg_type = 1;
	device_data.info.device_pid = getpid();
	device_data.info.this_device = actuator;
	device_data.info.first_setup = true;
	strncpy(device_data.info.device_name, actuator_name, MAX_NAME);

	if (msgsnd(msgid, (void *)&device_data, sizeof(device_data.info), 0) == -1) {
		printf("msgsnd from actuator to server failed\n");
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

void check_actuator_action(int msgid, bool *actuator_running, char *actuator_name)
{
	struct actuator_structure actuate_instruction;
	int actuate_instruction_type = getpid();

	// Check if there a message from server for actuator
	if (msgrcv(msgid, (void *)&actuate_instruction, BUFSIZ, actuate_instruction_type, IPC_NOWAIT) == -1) {
		if(errno == ENOMSG)	{
			//No instructions for actuator yet, just return
			return;
		}
		else {
			printf("msgrcv for device failed with error: %d\n", errno);
			exit(EXIT_FAILURE);
		}

	}

	// If server sends actuate signal, then activate actuator	
	if(actuate_instruction.actuator_instruct.actuator_active)
	{
		printf("Actuator [%s]. Turning on! Sequence num [%d]\n", actuator_name,  actuate_instruction.actuator_instruct.sequence_num);
		*actuator_running = true;

		//reply back to server that actuator is active
		struct actuator_reply_structure actuator_reply;
		char buffer[MAX_TEXT];
		snprintf(buffer, MAX_TEXT, "Actuator: %s is active.", actuator_name);
		actuator_reply.msg_type = 2;
		actuator_reply.reply_info.sequence_num = actuate_instruction.actuator_instruct.sequence_num;
		strncpy(actuator_reply.reply_info.actuator_reply, buffer, MAX_TEXT);

		if (msgsnd(msgid, (void *)&actuator_reply, sizeof(actuator_reply.reply_info), 0) == -1) {
			printf("msgsnd from actuator to server failed\n");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		*actuator_running = false;
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
