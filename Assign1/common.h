/*
*	SYSC 4001 Operating Systems
*	Assignment 1
*	
*	Yuzhou Liu
*	100853392
*
*	Common structures and defines used throughout the sensor, actuator, controller
*	and cloud.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <signal.h>
#include <sys/ipc.h>

#define MSG_KEY_PATHNAME "/tmp/yuzhou_msg_queue"
#define MSG_KEY_ID 1
#define DEVICE_MSG 1
#define ACTUATOR_REPLY 2
#define CHILD_TO_PARENT 3
#define REQUEST_STOP 4

#define ACKNOWLEDGE "ack"
#define STOP "stop"

#define MAX_NAME 20
#define MAX_TEXT 200
#define MAX_SENSOR_NUM 5
#define MAX_ACTUATOR_NUM 5

enum device_type {
	sensor,
	actuator
};

// message structure for sending from device to controller
// includes info_structure and device_type from above
struct msg_structure {
	long int msg_type;
	struct info_structure {
		enum device_type this_device;
		pid_t device_pid;
		bool first_setup;
		char device_name[MAX_NAME];
		int threshold;
		int sensing_data;
	} info;
};

// message structure for sending acknowledge/stop from controller to device
struct ack_structure {
	long int msg_type;
	char message[MAX_TEXT];
};

// message structure for sending instruction to actuator
struct actuator_structure {
	long int msg_type;
	struct actuator_instruct_struct {
		int sequence_num;
		bool actuator_active;
	} actuator_instruct;
};

// message structure for sending reply from actuator to server
struct actuator_reply_structure {
	long int msg_type;
	struct reply_info_struct {
		int sequence_num;
		char actuator_reply[MAX_TEXT];
	} reply_info;	
};
