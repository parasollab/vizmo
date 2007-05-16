
#include "timer_gettimeofday.h"

namespace stapl
{
  typedef struct timeval timer;

/**
 * Create and return a handle to a new timer.
 **/
  timer start_timer()
  {
    timer time;
    struct timezone zone;
#ifdef _PRINT_TIMER
     cout << "start_timer: using gettimeofday()" << endl;
#endif
     timer_assert(gettimeofday(&time, &zone));
     return time;
  }
/**
 * Given a timer handle, determine the elapsed time in seconds.
 **/
  double stop_timer(const timer start)
  {
    timer stop;
    struct timezone zone;
    timer_assert(gettimeofday(&stop, &zone));
    return ((double) (stop.tv_sec - start.tv_sec) * 1000000 +
            (double) (stop.tv_usec - start.tv_usec)) / 1000000.0;
  }
}                               // end namespace

// EOF 
