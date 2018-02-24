#ifndef PACKET_H
#define PACKET_H

#include "data.h"
#include "conf.h"

#include <stdlib.h>

typedef enum {
	PROBLEM,
	DELAYED,
	CREATED,
	QUEUED,
	SENT,
	DESTROYED
} packetStatus_t;

typedef struct packet {
	packetStatus_t status;
	agent_t agent;
	unsigned long long time; // ms
	class_t class;
	size_t size;
	void* data;
	char* message;
	size_t messageLength;
} packet_t;

#endif
