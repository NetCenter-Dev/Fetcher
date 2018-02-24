#include "packet.h"
#include "data.h"
#include "conf.h"
#include "error.h"
#include "timer.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#ifdef __linux__
	#include <endian.h>
#endif
#ifdef __MACH__
	#include <machine/endian.h>

	#define htobe16(x) htons(x)
	#define htobe32(x) htonl(x)
	#define htobe64(x) htonll(x)
#endif

#define MAX_PACKET_QUEUE_LENGTH 1024

#define HEARTBEAT_PREAMBLE "hb:"
#define HEARTBEAT_POSTAMBLE ":hb"

#define NEXT_POINTER(p) p = ((p + 1) % MAX_PACKET_QUEUE_LENGTH)

packet_t packets[MAX_PACKET_QUEUE_LENGTH] = {{}};
int startPointer = 0;
int endPointer = 0;

int getQueueLength() {
	int result = 0;
	if (startPointer > endPointer) {
		result += endPointer;
		result += MAX_PACKET_QUEUE_LENGTH - startPointer;
	} else {
		result = endPointer - startPointer;
	}
	return result;
}

packet_t newPacket(agent_t agent, void* data, class_t class, const char* message) {
	packet_t packet;
	packet.agent = agent;
	packet.class = class;
	packet.time = getRealTime() / (1*1000*1000); // we want ms
	packet.message = NULL;
	packet.data = NULL;
	packet.status = CREATED;
	packet.size = 0;
	packet.messageLength = 0;
	if (message != NULL) {
		packet.message = strdup(message);
		if (packet.message == NULL) {
			packet.status = PROBLEM;
			packet.message = strerror(errno);
		}
		packet.messageLength = strlen(message) + 1;
	}
	packet.data = NULL;
	if (data != NULL) {
		switch(agent.type) {
			case VOID:
				packet.status = PROBLEM;
				packet.message = "Void data type, but non-null given.";
				break;
			case INT:
				packet.size = sizeof(int);
				break;
			case DOUBLE:
				packet.size = sizeof(double);
				break;
			case STRING:
				packet.size = strlen(data) + 1;
				break;
			default:
				assert(false);
		}
		if (packet.size != 0) {
			packet.data = malloc(packet.size);
			if (packet.data == NULL) {
				packet.status = PROBLEM;
				packet.message = strerror(errno);
			} else
				memcpy(packet.data, data, packet.size);
		}
	} else {
		if (agent.type != VOID) {
			packet.status = PROBLEM;
			packet.message = "Non-void data type, but NULL given.";
		}
	}
	return packet;
}

bool pushPacket(packet_t packet) {
	switch(packet.status) {
		case PROBLEM:
			fail("There was an error with the packet: %s", packet.message);
			return false;
		case CREATED:
			break;
		case DELAYED:
			break;
		case QUEUED:
			error = "Packet already queued.";
			return false;
		case SENT:
			error = "Packet already sent.";
			return false;
		case DESTROYED:
			error = "Cannot push destroyed packet.";
			return false;
		default:
			assert(false);
	}
	if (getQueueLength() >= MAX_PACKET_QUEUE_LENGTH) {
		error = "The queue is full.";
		return false;
	}
	packet.status = QUEUED;
	packets[endPointer] = packet;
	NEXT_POINTER(endPointer);
	return true;
}

bool peakPacket(packet_t* packet) {
	if (getQueueLength() < 1) {
		error = "No packets on queue.";
		return false;
	}
	packet = &(packets[startPointer]);
	return true;
}

void shiftPacket() {
	if (getQueueLength() >= 1) {
		NEXT_POINTER(startPointer);
	}
}

bool popPacket(packet_t* packet) {
	if (!peakPacket(packet))
		return false;
	NEXT_POINTER(startPointer);
	return true;
}

void destroyPacket(packet_t packet) {
	if (packet.status == DESTROYED)
		return;
	if (packet.message != NULL)
		free(packet.message);
	if (packet.data != NULL)
		free(packet.data);
	packet.message = NULL;
	packet.status = DESTROYED;
}

size_t getBufferFromPacket(packet_t packet, char** buffer) {
	size_t size = 0;

	size_t nameLength = strlen(packet.agent.name) + 1;

	size += sizeof(size_t); // agent name
	size += nameLength;
	size += sizeof(data_t); // data
	size += sizeof(type_t); // type
	size += sizeof(uint64_t); // time
	size += sizeof(uint64_t); // size of the data
	size += sizeof(uint64_t); // size of the message
	size += packet.size;
	size += packet.messageLength;

	*buffer = malloc(size);
	if (*buffer == NULL) {
		error = strerror(errno);
		return -1;
	}

	uint64_t tmp;
	size_t position = 0;
	tmp = htobe64(nameLength);
	memcpy(buffer + position, &tmp, sizeof(uint64_t));
	position += sizeof(size_t);
	memcpy(buffer + position, packet.agent.name, nameLength);
	position += nameLength;
	// data and type should be 8 bit -> no endian convertion
	memcpy(buffer + position, &(packet.agent.data), sizeof(data_t));
	position += sizeof(data_t);
	memcpy(buffer + position, &(packet.agent.type), sizeof(type_t));
	position += sizeof(type_t);
	tmp = htobe64(packet.time);
	memcpy(buffer + position, &tmp, sizeof(uint64_t));
	position += sizeof(unsigned long long);
	tmp = htobe64(packet.size);
	memcpy(buffer + position, &tmp, sizeof(uint64_t));
	position += sizeof(size_t);
	tmp = htobe64(packet.messageLength);
	memcpy(buffer + position, &tmp, sizeof(uint64_t));
	position += sizeof(size_t);
	memcpy(buffer + position, packet.data, packet.size);
	position += packet.size;
	memcpy(buffer + position, packet.message, packet.messageLength);
	position += packet.messageLength;

	return size;
}

void sendHeartbeat(int fd) {

}
