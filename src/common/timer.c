#include "timer.h"
#include "error.h"

#include <stdlib.h>
#include <stdio.h>

#include <time.h>

#define MAX_TIMERS 128

#ifdef __linux__

static void timerHandler(union sigval target) {
	((void (*)(void))(target.sival_ptr))();
}

timer_t timers[MAX_TIMERS] = {NO_TIMER};

static timerid_t findUnusedTimerId() {
	for (int i = 0; i < MAX_TIMERS; i++) {
		if (timers[i] == NO_TIMER)
			return i;
	}
	error = "There is no free timer.";
	return NO_TIMER;
}

timerid_t creatTimer(void (*handler)()) {
	timerid_t id = findUnusedTimerId();
	if (id == NO_TIMER)
		return NO_TIMER;

	struct sigevent sevp;
	sevp.sigev_notify = SIGEV_THREAD;
	sevp.sigev_notify_function = timerHandler;
	sevp.sigev_value.sival_ptr = handler;

	timer_t timer;

	if (timer_create(CLOCK_BOOTTIME, &sevp, &timer) < 0) {
		error = strerror(errno);
		return NO_TIMER;
	}

	timers[id] = timer;
	return id;
}

int startTimer(timerid_t id, unsigned long ms) {
	timer_t timer = timers[id];
	if (timer == NO_TIMER) {
		error = "No such timer.";
		return -1;
	}

	struct itimerspec time, old;

	time.it_value.tv_sec = ms / 1000;
	time.it_value.tv_nsec = ((ms % 1000) * 1000000);
	time.it_interval.tv_sec = 0;
	time.it_interval.tv_nsec = 0;

	if (timer_settime(timer, 0, &time, &old) < 0) {
		error = strerror(errno);
		return -1;
	}
	return 0;
}

int startInterval(timerid_t id, unsigned long ms) {
	timer_t timer = timers[id];
	if (timer == NO_TIMER) {
		error = "No such timer.";
		return -1;
	}

	struct itimerspec time, old;

	time.it_value.tv_sec = ms / 1000;
	time.it_value.tv_nsec = ((ms % 1000) * 1000000);
	time.it_interval.tv_sec = ms / 1000;
	time.it_interval.tv_nsec = ((ms % 1000) * 1000000);

	if (timer_settime(timer, 0, &time, &old) < 0) {
		error = strerror(errno);
		return -1;
	}
	return 0;
}

int stopTimer(timerid_t id) {
	timer_t timer = timers[id];
	if (timer == NO_TIMER) {
		error = "No such timer.";
		return -1;
	}

	struct itimerspec time, old;

	time.it_value.tv_sec = 0;
	time.it_value.tv_nsec = 0;
	time.it_interval.tv_sec = 0;
	time.it_interval.tv_nsec = 0;

	if (timer_settime(timer, 0, &time, &old) < 0) {
		error = strerror(errno);
		return -1;
	}
	return 0;
}

int deleteTimer(timerid_t id) {
	timer_t timer = timers[id];
	if (timer == NO_TIMER) {
		error = "No such timer.";
		return -1;
	}
	if (timer_delete(timer) < 0) {
		error = strerror(errno);
		return -1;
	}
	timers[id] = NO_TIMER;
	return 0;
}

#endif
#ifdef __MACH__
	#include <dispatch/dispatch.h>

	#define LEEWAY 20000000 // 20 ms

	dispatch_source_t timers[MAX_TIMERS] = {NULL};

	static timerid_t findUnusedTimerId() {
		for (int i = 0; i < MAX_TIMERS; i++) {
			if (timers[i] == NULL)
				return i;
		}
		error = "There is no free timer.";
		return NO_TIMER;
	}

	timerid_t createTimer(void (*handler)()) {
		timerid_t id = findUnusedTimerId();
		if (id == NO_TIMER)
			return NO_TIMER;

		//dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
		dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0));
		if (timer == NULL) {
			error = "dispatch_source_create failed.";
			return NO_TIMER;
		}
    dispatch_source_set_event_handler_f(timer, handler);
		//dispatch_source_set_event_handler(timer, ^{ fprintf(stderr, "HANDLER BLOCK\n"); });

		timers[id] = timer;
		return id;
	}

	int startTimer(timerid_t id, unsigned long ms) {
		error = "No single shot support for mach yet.";
		return -1;
	}

	int startInterval(timerid_t id, unsigned long ms) {
		dispatch_source_t timer = timers[id];
		if (timer == NULL) {
			error = "No such timer.";
			return -1;
		}
		//dispatch_source_set_timer(timer, dispatch_walltime(NULL, ms * 1000 * 1000), ms * 1000 * 1000 * 1000, LEEWAY);
		dispatch_source_set_timer(timer, dispatch_time(DISPATCH_TIME_NOW, ms * 1000 * 1000), ms * 1000 * 1000, LEEWAY);
		dispatch_resume(timer);
		return 0;
	}

	int stopTimer(timerid_t id) {
		dispatch_source_t timer = timers[id];
		if (timer == NULL) {
			error = "No such timer.";
			return -1;
		}
		dispatch_suspend(timer);
		return 0;
	}

	int deleteTimer(timerid_t id) {
		dispatch_source_t timer = timers[id];
		if (timer == NULL) {
			error = "No such timer.";
			return -1;
		}
		dispatch_source_set_event_handler_f(timer, NULL); // remove handler
		dispatch_resume(timer);
		dispatch_source_cancel(timer);
		//dispatch_release(timer);
		timers[id] = NULL;
		return 0;
	}

#endif

unsigned long long getRealTime() {
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	return time.tv_sec * 1000000000 + time.tv_nsec;
}

unsigned long long getRelativeTime() {
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	return time.tv_sec * 1000000000 + time.tv_nsec;
}

unsigned long long getProcessTime() {
	struct timespec time;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
	return time.tv_sec * 1000000000 + time.tv_nsec;
}

unsigned long long getThreadTime() {
	struct timespec time;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time);
	return time.tv_sec * 1000000000 + time.tv_nsec;
}
