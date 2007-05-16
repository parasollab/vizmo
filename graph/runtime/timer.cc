
#include "timer.h"

#ifndef STAPL_SELF_TEST

#  ifdef STAPL_TIMER_CLOCK_GETTIME
#    include "timer_clock_gettime.cc"
#  endif

#  ifdef STAPL_TIMER_GETTIMEOFDAY
#    include "timer_gettimeofday.cc"
#  endif

#  ifdef STAPL_TIMER_GETRUSAGE
#    include "timer_getrusage.cc"
#  endif

#  ifdef STAPL_TIMER_PAPI
#    include "timer_papi.cc"
#  endif

#else                           // build test to link against timer.o generated from above

#  include <unistd.h>           // for sleep()

// Notes:  1) use g++, not gcc... 2) if using clock_gettime(), link against -lrt on Linux 

#  define _PRINT_TIMER

#  include "Defines.h"
#  include "runtime.h"

void stapl_main(int argc, char **argv)
{
  int i;
  stapl::timer starttime = stapl::start_timer();
  double stoptime = stapl::stop_timer(starttime);
  cout << "initial start/stop call took " << stoptime << endl;
  for (i = 0; i < 10; i++) {
    starttime = stapl::start_timer();
    stoptime = stapl::stop_timer(starttime);
    cout << "Iteration " << i << ": start/stop call took " << stoptime <<
        endl;
  }
  starttime = stapl::start_timer();
  sleep(1);
  stoptime = stapl::stop_timer(starttime);
  cout << "sleep(1) took " << stoptime << endl;
  starttime = stapl::start_timer();
  sleep(5);
  stoptime = stapl::stop_timer(starttime);
  cout << "sleep(5) took " << stoptime << endl;
}
#endif
