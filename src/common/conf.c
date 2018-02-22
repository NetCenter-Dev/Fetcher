#include "conf.h"
#include "error.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <inttypes.h>

static int parseLine(int, char*, char**, char**);

/*
# Example agent config

name = "Agent1" 	# strings in quotes
data = datavalue 	# none, message, datavalue, property
type = int 				# int, double, string
timing = interval # no other timing mode at the moment
timing.value = 60 # seconds

messages.warning.1 = "Value is too high (%v)." # %v for value, %m for output
messages.error.10 = "%m"
*/

int parseAgent(const char* config, agent_t* agent) {
	char* copy = malloc(strlen(config) + 1);
	if (copy == NULL) {
		error = strerror(errno);
		return -1;
	}

	(void) strcpy(copy, config);

	int last = 0;

	int line = 0;
	bool doNext = true;
	for (size_t i = 0; doNext; i++) {
		doNext = copy[i] != '\0';
		if (copy[i] == '\n' || copy[i] == '\0') {
			line++;
			copy[i] = '\0';

			char* key;
			char* value;

			int tmp = parseLine(line, copy + last, &key, &value);
			//printf("'%s' - '%s'\n", key, value);
			last = i + 1;
			if (tmp < 0)
				return -1;
			if (tmp > 0)
				continue;

			if (strcmp("name", key) == 0)
				agent->name = value;
			else if (strcmp("type", key) == 0) {
				if (strcmp(value, "void") == 0)
					agent->type = VOID;
				else if (strcmp(value, "int") == 0)
					agent->type = INT;
				else if (strcmp(value, "double") == 0)
					agent->type = DOUBLE;
				else if (strcmp(value, "string") == 0)
					agent->type = STRING;
				else {
					// TODO error
				}
			} else if (strcmp("data", key) == 0) {
				if (strcmp(value, "none") == 0)
					agent->data = NONE;
				else if (strcmp(value, "datavalue") == 0)
					agent->data = DATA_VALUE;
				else if (strcmp(value, "message") == 0)
					agent->data = MESSAGE;
				else if (strcmp(value, "property") == 0)
					agent->data = PROPERTY;
				else {
					printf("\n%s is an unknown value.\n", value);
					// TODO error
				}
			} else if (strcmp("timing", key) == 0) {
				if (strcmp(value, "interval") == 0)
					agent->timing.type = Interval;
				else {
					// TODO error
				}
			} else if (strcmp("timing.value", key) == 0) {
				char* tmp;
				agent->timing.value = strtol(value, &tmp, 10);
				if (*tmp != '\0') {
					// TODO error
				}
			} else if (strstr(key, "messages") == key) {
				char* tmp;
				int index = 0;
				char* parts[3];
				while((tmp = strsep(&key, ".")) != NULL) {
					if (index >= 3)
						break;
					parts[index++] = tmp;
				}
				if (tmp != NULL || index != 3 || (strcmp(parts[0], "messages") != 0)) {
					// TODO error
				}
				int code = strtol(parts[2], &tmp, 10);
				if (*tmp != '\0') {
					// TODO error
				}
				if (code < 0 || code > 255) {
					// TODO error
				}
				agent->messages[code].text = value;

				// meta is no message class
				if (strcmp(parts[1], "info") == 0)
					agent->messages[code].class = INFO;
				else if (strcmp(parts[1], "warning") == 0)
					agent->messages[code].class = WARNING;
				else if (strcmp(parts[1], "alarm") == 0)
					agent->messages[code].class = ALARM;
				else if (strcmp(parts[1], "error") == 0)
					agent->messages[code].class = ERROR;
				else if (strcmp(parts[1], "emergency") == 0)
					agent->messages[code].class = EMERGENCY;
				else {
					// TODO error
				}
			}

		}
	}
	return 0;
}

int parseLine(int linenr, char* string, char** key, char** value) {
	#define MAX_ERROR_LENGTH 1024
	static char* errmsg = NULL;
	if (errmsg == NULL)
		errmsg = malloc(MAX_ERROR_LENGTH);
	if (errmsg == NULL) {
		error = strerror(errno);
		return -1;
	}
	#define INIT 0
	#define KEY_START_FOUND 1
	#define KEY_STOP_FOUND 2
	#define SEPERATOR_FOUND 3
	#define VALUE_START_FOUND 4
	#define VALUE_STOP_FOUND 5
	int state = INIT;
	bool isEnquoted = false;
	bool isMasked = false;
	bool willMasked = false;
	char* sub = string;
	for (size_t i = 0; string[i] != '\0'; i++) {
		if ((string[i] == ' ' || string[i] == '\t') && !isEnquoted) {
			switch(state) {
				case INIT:
				case KEY_STOP_FOUND:
				case SEPERATOR_FOUND:
				case VALUE_STOP_FOUND:
					sub = string + i + 1;
					break;
				case KEY_START_FOUND:
					state = KEY_STOP_FOUND;
					string[i] = '\0';
					*key = sub;
					sub = string + i + 1;
					break;
				case VALUE_START_FOUND:
					state = VALUE_STOP_FOUND;
					string[i] = '\0';
					*value = sub;
					sub = string + i + 1;
					break;
				default:
					assert(false);
			}
		} else if (string[i] == '=' && !isEnquoted) {
			switch(state) {
				case INIT:
				case SEPERATOR_FOUND:
				case VALUE_START_FOUND:
				case VALUE_STOP_FOUND:
					if (snprintf(errmsg, MAX_ERROR_LENGTH, "Unexpected '=' on line %d:%lu.", linenr, i + 1) < 0) {
						error = strerror(errno);
						return -1;
					}
					error = errmsg;
					return -1;
				case KEY_STOP_FOUND:
					state = SEPERATOR_FOUND;
					break;
				case KEY_START_FOUND:
					string[i] = '\0';
					*key = sub;
					sub = string + i + 1;
					state = SEPERATOR_FOUND;
					break;
				default:
					assert(false);
			}
		} else if (string[i] == '#' && !isEnquoted) {
			break;
		} else if (string[i] == '"' && !isMasked && isEnquoted) {
			isEnquoted = false;
			switch (state) {
				case KEY_START_FOUND:
					string[i] = '\0';
					*key = sub;
					sub = string + i + 1;
					state = KEY_STOP_FOUND;
					break;
				case VALUE_START_FOUND:
					string[i] = '\0';
					*value = sub;
					sub = string + i + 1;
					state = VALUE_STOP_FOUND;
					break;
				default:
					case INIT:
					case KEY_STOP_FOUND:
					case VALUE_STOP_FOUND:
					case SEPERATOR_FOUND:
					assert(false);
			}
		} else if (string[i] == '"' && !isMasked) {
				isEnquoted = true;
				switch (state) {
					case INIT:
						sub = string + i + 1;
						state = KEY_START_FOUND;
						break;
					case SEPERATOR_FOUND:
						sub = string + i + 1;
						state = VALUE_START_FOUND;
						break;
					case KEY_STOP_FOUND:
					case VALUE_STOP_FOUND:
						if (snprintf(errmsg, MAX_ERROR_LENGTH, "Unexpected \" on line %d:%lu.", linenr, i + 1) < 0) {
							error = strerror(errno);
							return -1;
						}
						error = errmsg;
						return -1;
					case KEY_START_FOUND:
					case VALUE_START_FOUND:
					default: assert(false);
				}
		} else if (string[i] == '\\' && !isMasked) {
			willMasked = true;
			memmove(sub + 1, sub, string + i - sub);
			sub++;
		} else {
			switch (state) {
				case INIT:
					state = KEY_START_FOUND;
					break;
				case SEPERATOR_FOUND:
					state = VALUE_START_FOUND;
					break;
				case KEY_START_FOUND:
				case VALUE_START_FOUND:
					break;
				case KEY_STOP_FOUND:
				case VALUE_STOP_FOUND:
					if (snprintf(errmsg, MAX_ERROR_LENGTH, "Unexpected string on line %d:%lu.", linenr, i + 1) < 0) {
						error = strerror(errno);
						return -1;
					}
					error = errmsg;
					return -1;
				default:
					assert(false);
			}
		}
		isMasked = willMasked;
		willMasked = false;
	}
	if (isEnquoted) {
		if (snprintf(errmsg, MAX_ERROR_LENGTH, "Missing \" on line %d.", linenr) < 0) {
			error = strerror(errno);
			return -1;
		}
		error = errmsg;
		return -1;
	}
	if (state == INIT) {
		*key = NULL;
		*value = NULL;
		return 1;
	}
	if (state == KEY_START_FOUND || state == KEY_STOP_FOUND) {
		if (snprintf(errmsg, MAX_ERROR_LENGTH, "Missing = on line %d.", linenr) < 0) {
			error = strerror(errno);
			return -1;
		}
		error = errmsg;
		return -1;
	}
	if (state == SEPERATOR_FOUND) {
		if (snprintf(errmsg, MAX_ERROR_LENGTH, "Missing value on line %d.", linenr) < 0) {
			error = strerror(errno);
			return -1;
		}
		error = errmsg;
		return -1;
	}
	if (state == VALUE_START_FOUND) {
		*value = sub;
	}
	return 0;
}
