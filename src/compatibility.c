#include "compatibility.h"

static struct timeval start, end;
static long mtime;
static long seconds;
static long useconds;

/* Simplifies use of timers */
void milliSleep(int millisec) {
	struct timespec req = {.tv_sec = 0, .tv_nsec = millisec * 1000000L};
	nanosleep(&req, (struct timespec *)NULL);	
}

void microSleep(int millisec) {
	struct timespec req = {.tv_sec = 0, .tv_nsec = millisec * 1000L};
	nanosleep(&req, (struct timespec *)NULL);	
}

void secSleep(int sec){
    struct timespec req = {.tv_sec = sec, .tv_nsec = 0};
    nanosleep(&req, (struct timespec *)NULL);   
}

/**
 * This function is added in order to simulate arduino millis() function
 */
void start_timer() {
	gettimeofday(&start, NULL);
}

long millis() {
	gettimeofday(&end, NULL);
    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;	
	return mtime;
}