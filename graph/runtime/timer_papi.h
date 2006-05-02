#ifndef TIMER_PAPI_H
#  define TIMER_PAPI_H
#  include <papi.h>
namespace stapl
{
  typedef long long int timer;
  timer start_timer(void);
  double stop_timer(timer time1);
};
#endif
