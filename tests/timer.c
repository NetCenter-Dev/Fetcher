#include "tests.h"

#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <timer.h>
#include <error.h>

#define TOLLERANCE (20*1000*1000) // 20ms
#define TEST_VALUE (1*1000*1000*1000) // 1s
#define STEPS 5

bool cont = false;
bool failed = false;

void handler() {
	static unsigned long long last = 0;
	static int step = 0;

	if (step++ >= STEPS)
		cont = true;

	unsigned long long tmp = getRelativeTime();
	long long diff = tmp - last - TEST_VALUE;
	if (last > 0) {
		if (llabs(diff) > TOLLERANCE) {
			printf("%s%sDifference too big (diff: %lld)\n", SUBSPACING, SUBSPACING, diff);
			failed = true;
		} else {
			printf("%s%sDifference okay (diff: %lld)\n", SUBSPACING, SUBSPACING, diff);
		}
	}
	last = tmp;
}

bool timer() {
	printf("%sTesting getRelativeTime (tollerance = %d)\n", SUBSPACING, TOLLERANCE);
	unsigned long long t1 = getRelativeTime();
	printf("%s%sGot %llu.\n", SUBSPACING, SUBSPACING, t1);
	printf("%s%sSleeping for %dns\n", SUBSPACING, SUBSPACING, TEST_VALUE);
	usleep(TEST_VALUE / 1000);
	unsigned long long t2 = getRelativeTime();
	printf("%s%sGot %llu.\n", SUBSPACING, SUBSPACING, t2);
	long long diff = t2 - t1 - TEST_VALUE;
	if (llabs(diff) > TOLLERANCE) {
		printf("%sFailed (diff = %lld)\n", SUBSPACING, diff);
		return false;
	} else {
		printf("%sOkay (diff = %lld)\n", SUBSPACING, diff);
	}

	printf("%sSetup interval timer (%d).\n", SUBSPACING, TEST_VALUE / 1000 / 1000);
	timerid_t timer = createTimer(&handler);
	if (timer == NO_TIMER) {
		printf("%s%sError: %s\n", SUBSPACING, SUBSPACING, error);
		return false;
	}

	if (startInterval(timer, TEST_VALUE / 1000 / 1000) < 0) {
		printf("%s%sError: %s\n", SUBSPACING, SUBSPACING, error);
		return false;
	}

	usleep(TEST_VALUE / 1000 * (STEPS + 2));

	if (!cont) {
		printf("%s%sError: handler took to long.\n", SUBSPACING, SUBSPACING);
		return false;
	}

	if (failed) {
		printf("%s%sError: %s\n", SUBSPACING, SUBSPACING, error);
		return false;
	}


	printf("%sStopping timer.\n", SUBSPACING);
	if (stopTimer(timer) < 0) {
		printf("%s%sError: %s\n", SUBSPACING, SUBSPACING, error);
		return false;
	}

	printf("%sDeleting timer.\n", SUBSPACING);
	if (deleteTimer(timer) < 0) {
		printf("%s%sError: %s\n", SUBSPACING, SUBSPACING, error);
		return false;
	}

	printf("%sTesting id check.\n", SUBSPACING);
	if (!(deleteTimer(timer) < 0)) {
		printf("%s%sError: startTimer with unused id should fail.\n", SUBSPACING, SUBSPACING);
		return false;
	}

	return true;
}
