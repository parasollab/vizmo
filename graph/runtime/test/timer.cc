#ifdef STAPL_SELF_TEST

#  include <unistd.h>           // for sleep()
#  include <stdio.h>            // for printf()
#  include <stdlib.h>           // for exit()

// Notes:  1) use g++, not gcc... 2) if using clock_gettime(), link against -lrt on Linux 

#  define _PRINT_TIMER

#  include "runtime.h"

void stapl_main(int argc, char **argv)
{
  stapl::timer starttime;
  starttime = stapl::start_timer();
  sleep(5);                     // a trivial example
  double stoptime = stapl::stop_timer(starttime);
  printf("%f\n", stoptime);
}
#endif
