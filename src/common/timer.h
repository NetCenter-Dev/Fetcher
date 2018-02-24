#ifndef TIMER_H
#define TIMER_H

typedef int timerid_t;
#define NO_TIMER (-1)

timerid_t createTimer(void (*)(void));
int startTimer(timerid_t, unsigned long);
int startInterval(timerid_t, unsigned long);
int stopTimer(timerid_t);
int deleteTimer(timerid_t);

unsigned long long getRealTime(void);
unsigned long long getRelativeTime(void);
unsigned long long getProcessTime(void);
unsigned long long getThreadTime(void);

#endif
