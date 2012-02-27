// -*- c-basic-offset: 4; c-backslash-column: 79; indent-tabs-mode: nil -*-
// vim:sw=4 ts=4 sts=4 expandtab
/* Copyright 2012
 * This file is part of Fachoda.
 *
 * Fachoda is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fachoda is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fachoda.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GTIME_H_120126
#define GTIME_H_120126
#include <stdint.h>

#define ONE_SECOND 1000000ULL
#define ONE_MILLISECOND 1000ULL

// Game Time (which can be stoped, restarted, accelerated...
typedef uint_least64_t gtime;   // how many usec since beginning of the simulation

#define MIN_DT ((uint_least64_t) 25000ULL)  // below which gtime_next() sleeps
#define MAX_DT ((uint_least64_t)100000ULL) // above which gtime_next() returns only MAX_DT (and skips this time)
#define MIN_DT_SEC (MIN_DT/(double)ONE_SECOND)
#define MAX_DT_SEC (MAX_DT/(double)ONE_SECOND)

gtime gtime_now(void);  // return the current time (involve a syscall)
gtime gtime_last(void); // return the last gtime returned by gtime_now()
gtime gtime_age(gtime date);    // age relative to gtime_last()
void gtime_accel(gtime how_much);
void gtime_stop(void);
void gtime_start(void);
void gtime_toggle(void);
gtime gtime_next(void);
float gtime_next_sec(void);

#endif
