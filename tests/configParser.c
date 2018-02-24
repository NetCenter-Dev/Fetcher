#include "tests.h"
#include <conf.h>
#include <error.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMBER_OF_TESTCASES 15
struct testcase {
	const char* config;
	int success;
	agent_t result;
} testcases[NUMBER_OF_TESTCASES];

bool compare (agent_t a1, agent_t a2, char** error) {
	if ((a1.name == NULL) != (a2.name == NULL)) {
		*error = "names null";
		return false;
	}
	if ((a1.name != a2.name) && strcmp(a1.name, a2.name) != 0) {
		*error = "names";
		return false;
	}
	if ((a1.script == NULL) != (a2.script == NULL)) {
		*error = "scripts null";
		return false;
	}
	if ((a1.script != a2.script) && strcmp(a1.script, a2.script) != 0) {
		*error = "scripts";
		return false;
	}
	if (a1.data != a2.data) {
		*error = "data";
		return false;
	}
	//printf("%d vs %d\n", a1.type, a2.type);
	if (a1.type != a2.type) {
		*error = "types";
		return false;
	}
	if (a1.lastValue != a2.lastValue) {
		*error = "last value";
		return false;
	}
	if (a1.timing.type != a2.timing.type) {
		*error = "timing type";
		return false;
	}
	if (a1.timing.value != a2.timing.value) {
		*error = "timing value";
		return false;
	}
	if (a1.timing.last != a2.timing.last) {
		*error = "timing last";
		return false;
	}
	for (int i = 0; i < MAX_MESSAGES; i++) {
		//printf("%s vs %s\n", a1.messages[i].text, a2.messages[i].text);
		if ((a1.messages[i].text == NULL) != (a2.messages[i].text == NULL)) {
			*error = "message text null";
			return false;
		}
		if ((a1.messages[i].text != a2.messages[i].text) && strcmp(a1.messages[i].text, a2.messages[i].text) != 0) {
			*error = "message text";
			return false;
		}
		if (a1.messages[i].class != a2.messages[i].class) {
			*error = "message class";
			return false;
		}
	}
	return true;
}

bool configParser() {
	memset(&(testcases[0]), 0, sizeof(struct testcase) * NUMBER_OF_TESTCASES);
	testcases[0] = (struct testcase) {
		.config = "name=hi # comment\n",
		.success = 0,
		.result = {.name = "hi"}
	};
	testcases[1] = (struct testcase) {
		.config = "name=\"hello world\"\n",
		.success = 0,
		.result = {.name = "hello world"}
	};
	testcases[2] = (struct testcase) {
		.config = "name=\"\\\"\"\n",
		.success = 0,
		.result = {.name = "\""}
	};
	testcases[3] = (struct testcase) {
		.config = "name = hi\n data  = datavalue\ntype = int",
		.success = 0,
		.result = {
			.name = "hi",
			.data = DATA_VALUE,
			.type = INT
		}
	};
	testcases[4] = (struct testcase) {
		.config = "timing = interval \ntiming.value = 60",
		.success = 0,
		.result = {
			.timing = {
				.value = 60,
				.type = Interval
			}
		}
	};
	testcases[5] = (struct testcase) {
		.config = "messages.warning.1 = hi",
		.success = 0,
		.result = {
			.messages = {
				[1] = {
					.class = WARNING,
					.text = "hi"
				}
			}
		}
	};
	testcases[6] = (struct testcase) {
		.config = "name = = hi",
		.success = -1,
		.result = {}
	};
	testcases[7] = (struct testcase) {
		.config = "messages.warning.x = hi",
		.success = -1,
		.result = {}
	};
	testcases[8] = (struct testcase) {
		.config = "name type = hi",
		.success = -1,
		.result = {}
	};
	testcases[9] = (struct testcase) {
		.config = "name = hello world",
		.success = -1,
		.result = {}
	};
	testcases[10] = (struct testcase) {
		.config = "type=error",
		.success = -1,
		.result = {}
	};
	testcases[11] = (struct testcase) {
		.config = "timing.value=hi",
		.success = -1,
		.result = {}
	};
	testcases[12] = (struct testcase) {
		.config = "messagesblabla.warning.1 = hi",
		.success = -1,
		.result = {}
	};
	testcases[13] = (struct testcase) {
		.config = "messages.warning.300",
		.success = -1,
		.result = {}
	};
	testcases[14] = (struct testcase) {
		.config = "messages.nothing.1",
		.success = -1,
		.result = {}
	};


	bool result = true;
	for (int i = 0; i < NUMBER_OF_TESTCASES; i++) {
		printf("%sTestcase %d... ", SUBSPACING, i);
		agent_t agent;
		memset(&agent, 0, sizeof(agent_t));
		int tmp = parseAgent(testcases[i].config, &agent);

		char* errmsg;
		if (tmp != testcases[i].success) {
			result = false;
			printf("failed.\n");
		} else if (tmp == 0 && !compare(agent, testcases[i].result, &errmsg)) {
			result = false;
			printf("failed (%s).\n", errmsg);
		} else {
			printf("okay.\n");
		}
		if (tmp != 0)
			printf("%s%sError: %s\n", SUBSPACING, SUBSPACING, error);
	}
	return result;
}
