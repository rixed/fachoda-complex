#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/time.h>
#include "gtime.h"

static bool running;
static gtime last;
static gtime last_start_time;	// when we started running (if running)
static gtime accelerated;	// how many usecs were skipped due to acceleration since last start
static gtime leaked;	// the usecs of wall clock we skipped because we were late
static gtime prev_gtime;	// gtime_now() value last time gtime_next() was called

static gtime tv_2_gtime(struct timeval const *tv)
{
	return tv->tv_sec * 1000000 + tv->tv_usec;
}

static gtime getgtimeofday(void)
{
	struct timeval tv;
	if (0 != gettimeofday(&tv, NULL)) {
		fprintf(stderr, "Cannot gettimeofday(): %s\n", strerror(errno));
		abort();
	}
	return tv_2_gtime(&tv);
}

gtime gtime_now(void)
{
	if (! running) return last;

	last = getgtimeofday() - last_start_time + accelerated - leaked;
	return last;
}

gtime gtime_age(gtime date)
{
	gtime now = gtime_now();
	return now - date;
}

void gtime_stop(void)
{
	if (! running) return;
	(void)gtime_now();	// update 'last'
	running = false;
}

void gtime_start(void)
{
	running = true;
	last_start_time = getgtimeofday();
	prev_gtime = gtime_now();
	accelerated = 0;
}

void gtime_toggle(void)
{
	if (running) {
		gtime_stop();
	} else {
		gtime_start();
	}
}

void gtime_accel(gtime t)
{
	if (! running) return;
	accelerated += t;
}

gtime gtime_next(void)
{
	gtime now, dt;
	while (1) {
		now = gtime_now();
		dt = now - prev_gtime + 1;
#		define MIN_DT (uint_least64_t)25000ULL	// below which we sleep
#		define MAX_DT (uint_least64_t)500000ULL	// above which we return only MAX_DT (and skip this time)
		if (dt > MAX_DT) {
			printf("gtime_next(): leak %"PRIuLEAST64"usecs\n", dt - MAX_DT);
			leaked += dt - MAX_DT;
			dt = MAX_DT;
		}
		if (dt >= MIN_DT) break;
		usleep(MIN_DT - dt); // give excess CPU to the system
		dt = MIN_DT;
		if (lapause) break;
	}
	prev_gtime = now;
	return dt;
}

float gtime_next_sec(void)
{
	gtime const dt = gtime_next();
	return dt / (float)ONE_SECOND;
}

