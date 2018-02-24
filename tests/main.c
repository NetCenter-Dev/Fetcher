#include <stdio.h>
#include <stdbool.h>
#include <conf.h>
#include <error.h>
#include <stdio.h>

#include "tests.h"

void test(const char* name, bool (*f)()) {
	printf("Testing %s...\n", name);
	bool tmp = f();
	if (tmp) {
		printf("Test \033[32msuccess\033[0m\n");
	} else {
		printf("Test \033[31mfailed\033[0m\n");
	}
}

int main(int argc, char** argv) {
	errorInit();

	test("config parser", configParser);
	test("timer", timer);

	return 0;
}
