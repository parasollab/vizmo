#ifndef TIMER_CLOCK_GETTIME_H
#  define TIMER_CLOCK_GETTIME_H
#  include <time.h>             // for clock_gettime()
namespace stapl
{
  typedef struct timespec timer;
  timer start_timer();
  double stop_timer(const stapl::timer start);
}
#endif
