#ifndef GTIME_H_120126
#define GTIME_H_120126
#include <stdint.h>

#define ONE_SECOND 1000000ULL
#define ONE_MILLISECOND 1000ULL

// Game Time (which can be stoped, restarted, accelerated...
typedef uint_least64_t gtime;	// how many usec since beginning of the simulation

gtime gtime_now(void);
gtime gtime_age(gtime date);
void gtime_accel(gtime how_much);
void gtime_stop(void);
void gtime_start(void);
void gtime_toggle(void);
gtime gtime_next(void);
float gtime_next_sec(void);

#endif
