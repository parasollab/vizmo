// This is a top level include file, and should be what is included by
// user programs.

#ifndef _TIMING_H
#define _TIMING_H

// This #define allows compilation on SGI with POSIX thread support
#define _BSD_COMPAT

#include <sys/time.h>


namespace stapl {
/**
 * @addtogroup timer
 * The STAPL timer is a simple, platform independent group of timer functions.
 * The timer can only be started and stopped, meaning only one section of code
 * can be benchmarked at a time.  The actual resolution of the timer varies
 * with the underlying platform, and is generally based on \c gettimeofday.
 * However, it is generally on the order of microseconds.
 *
 * \include
 * <timing.h>
 * @{
 **/

struct timeval __first, __end;
//struct timezone __tzp;

/**
 * Reset the timer and start the clock.  The clock is stopped using
 * stop_timer().
 **/
void start_timer() {
  gettimeofday(&__first,NULL);
}

/**
 * Stop the timer and return the number of seconds that have passed on the
 * clock.  The clock is started using start_timer().
 *
 * \return the number of seconds passed since start_timer() was called
 **/
float stop_timer() {
  gettimeofday(&__end,NULL);
  return ( (float)(__end.tv_sec-__first.tv_sec)*1000000 +
	   (float)(__end.tv_usec-__first.tv_usec) ) / 1000000;
}

/** @} **/
} //end namespace

#endif
