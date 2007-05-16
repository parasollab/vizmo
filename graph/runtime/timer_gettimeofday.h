#ifndef TIMER_GETTIMEOFDAY_H
#  define TIMER_GETTIMEOFDAY_H
#  include <sys/time.h>         // for gettimeofday()
namespace stapl
{
  typedef struct timeval timer;
  timer start_timer(void);
  double stop_timer(const timer start);
};
#endif
