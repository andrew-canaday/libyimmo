/*=============================================================================
 * libyimmo: Lightweight socket server framework
 *
 * ymo_benchmark.h: Kludge benchmarking header for libyimmo tests.
 *
 *  Copyright (c) 2014 Andrew Canaday
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *===========================================================================*/

#ifndef YMO_BENCHMARK_H
#define YMO_BENCHMARK_H
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define USEC_PER_SEC 1000000

static struct timeval start_time;
static struct timeval stop_time;
static struct timeval pause_time;

/* Store the difference between to struct timevals: */
void timeval_subtract(
        struct timeval* result, struct timeval* start, struct timeval* end)
{
    result->tv_sec = end->tv_sec - start->tv_sec;
    if( end->tv_usec >= start->tv_usec ) {
        result->tv_usec = end->tv_usec - start->tv_usec;
    } else {
        result->tv_sec -= 1;
        result->tv_usec = (USEC_PER_SEC - start->tv_usec) + end->tv_usec;
    }
    return;
}


void timeval_increment(struct timeval* out, struct timeval* diff)
{
    out->tv_sec += diff->tv_sec;
    long tv_usec = out->tv_usec + diff->tv_usec;
    if( tv_usec >= USEC_PER_SEC ) {
        ++out->tv_sec;
        out->tv_usec = USEC_PER_SEC - tv_usec;
    } else {
        out->tv_usec = tv_usec;
    }
    return;
}


/* Start timing an algorithm: */
static inline void benchmark_start()
{
    gettimeofday(&start_time, NULL);
    return;
}


/* Pause the timer, don't count any time elapsed between pause and unpause: */
static inline void benchmark_pause()
{
    gettimeofday(&pause_time, NULL);
    return;
}


/* Unpause the timer: */
static inline void benchmark_unpause()
{
    static struct timeval unpause_time;
    static struct timeval total_pause;
    gettimeofday(&unpause_time, NULL);
    timeval_subtract(&total_pause, &pause_time, &unpause_time);
    timeval_increment(&start_time, &total_pause);
    return;
}


/* Stop the timer and spit out the total elapsed time: */
static inline struct timeval benchmark_stop()
{
    static struct timeval total_time;
    gettimeofday(&stop_time, NULL);
    timeval_subtract(&total_time, &start_time, &stop_time);
    return total_time;
}


#endif /* YMO_BENCHMARK_H */


