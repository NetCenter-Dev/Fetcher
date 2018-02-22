#ifndef CONF_H
#define CONF_H

#include "data.h"

typedef struct {
	const char* text; // %v for datavalue, %m for message
	class_t class;
} message_t;

typedef enum {
	Interval
} timingType_t;

typedef unsigned long long int timestamp_t;

typedef struct {
	timingType_t type;
	timestamp_t value;
	timestamp_t last;
} timing_t;

#define MAX_MESSAGES 255

typedef struct {
	const char* name;
	const char* script;
	data_t data;
	type_t type;
	void* lastValue;
	timing_t timing;
	message_t messages[MAX_MESSAGES];
} agent_t;

int parseAgent(const char*, agent_t*);

#endif
