#ifndef TIMER_GETRUSAGE_H
#  define TIMER_GETRUSAGE_H
#  include <sys/time.h>         // for getrusage()
#  include <sys/resource.h>     // for getrusage()
#  include <unistd.h>           // for getrusage()
namespace stapl
{
  typedef struct rusage timer;
  timer start_timer(void);
  double stop_timer(const timer time1);
};
#endif
