// Note: STAPL_TIMER_CLOCK_GETTIME is both a #define at compile time and an 
// environment variable at runtime.   You can override the default clock_gettime()
// for the system you are on (if available) by setting it... e.g.
//
//     export STAPL_TIMER_CLOCK_GETTIME=1 
//
// will use the CLOCK_MONOTONIC on Linux... see time.h on your system for
// appropriate values.

//#include <stdlib.h>
//#include <stdio.h>
//#include <errno.h>
//#include <assert.h>

#include "timer_clock_gettime.h"

namespace stapl
{
  typedef struct timespec timer;
  static int which_timer = (-1);        // CLOCK_REALTIME, CLOCK_SGI_CYCLE or user-override

/* Linux
#   define CLOCK_REALTIME               0
#   define CLOCK_MONOTONIC              1
#   define CLOCK_PROCESS_CPUTIME_ID     2
#   define CLOCK_THREAD_CPUTIME_ID      3
*/

  void print_timer(void)
  {
    char *msg;

#ifndef _PRINT_TIMER
     return;                    // bail quickly
#endif
    switch (which_timer) {
#ifdef CLOCK_REALTIME
    case CLOCK_REALTIME:
      msg = "realtime clock";
      break;
#endif
#ifdef CLOCK_SGI_CYCLE
      case CLOCK_SGI_CYCLE:msg = "SGI hi-res timer";
      break;
#endif
      default:msg = "unknown timer";
    }
    cout << "start_timer: using clock_gettime( " << msg << " )" << endl;
  }

  int select_timer(void)
  {
    int which = CLOCK_REALTIME; // default
    char *env_timer;            // for runtime specification
    // if SGI timer avail... use it..
#ifdef CLOCK_SGI_CYCLE
    which = CLOCK_SGI_CYCLE;
#endif
    env_timer = getenv("STAPL_TIMER_CLOCK_GETTIME");
    if (env_timer)
      which = atoi(env_timer);
    return which;
  }

  timer start_timer()
  {
    timer time;
    if (which_timer == -1)
      which_timer = select_timer();
    print_timer();
    timer_assert(clock_gettime(which_timer, &time));
    return time;
  }

  double stop_timer(const stapl::timer start)
  {
    timer stop;
    assert(which_timer != -1);  // should call start before stop
    timer_assert(clock_gettime(which_timer, &stop));
    return ((double) (stop.tv_sec - start.tv_sec) * 1000000000 +
            (double) (stop.tv_nsec - start.tv_nsec)) / 1000000000.0;
  }

}                               // end namespace
