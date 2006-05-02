#ifndef TIMER_H
#  define TIMER_H

#  include "Defines.h"
#  include <limits.h>           // pick up _POSIX_CLOCKRES_MIN
#  include <time.h>             // pick up CLOCK_REALTIME
#  include <sys/time.h>
#  include <errno.h>            // for errno

#  define timer_assert(rtn) { if( rtn != 0 ) { fprintf( stderr, \
  "STAPL ASSERT: timer failed with errno %d (file: %s, line: %d)\n", \
  errno, __FILE__, __LINE__ ); abort(); } }

// make sure one, AND ONLY ONE, timer defined (if any)

// if PAPI was specified, disable others
#  ifdef STAPL_TIMER_PAPI
#    ifdef STAPL_TIMER_GETRUSAGE
#      undef STAPL_TIMER_GETRUSAGE
#    endif
#    ifdef STAPL_TIMER_CLOCK_GETTIME
#      undef STAPL_TIMER_CLOCK_GETTIME
#    endif
#    ifdef STAPL_TIMER_GETTIMEOFDAY
#      undef STAPL_TIMER_GETTIMEOFDAY
#    endif
#  endif

// else if clock_gettime() was specified, disable others
#  ifdef STAPL_TIMER_CLOCK_GETTIME
#    ifdef STAPL_TIMER_GETRUSAGE
#      undef STAPL_TIMER_GETRUSAGE
#    endif
#    ifdef STAPL_TIMER_GETTIMEOFDAY
#      undef STAPL_TIMER_GETTIMEOFDAY
#    endif
#  endif

// else if gettimeofday() was specified, disable getrusage()
#  ifdef STAPL_TIMER_GETTIMEOFDAY
#    ifdef STAPL_TIMER_GETRUSAGE
#      undef STAPL_TIMER_GETRUSAGE
#    endif
#  endif

// if no timer specified, provide a default for platform

#  if !defined(STAPL_TIMER_GETTIMEOFDAY) && ! defined(STAPL_TIMER_CLOCK_GETTIME) && ! defined(STAPL_TIMER_GETRUSAGE) && ! defined(STAPL_TIMER_PAPI)
  // The IEEE 1003.1 Standard (POSIX) states that systems that provide the
  // optional CLOCK_REALTIME for clock_gettime() will #define its resolution as
  // _POSIX_CLOCKRES_MIN in limits.h.  We use this fact to automatically
  // determine the best timer available.  However, sometimes clock_gettime() is
  // defined, but neglects the above definition (e.g., Redhat Linux, which also
  // requires -lrt), although it does #define CLOCK_REALTIME.
#    if defined( _POSIX_CLOCKRES_MIN ) || defined( CLOCK_REALTIME )
#      define STAPL_TIMER_CLOCK_GETTIME
#    else
    // Unfortunately, there doesn't seem to be a similar test to insure that gettimeofday()
    // is available... just default to it for now (current behavior)
#      define STAPL_TIMER_GETTIMEOFDAY
#    endif
// this could be used later
//   #ifdef RUSAGE_SELF
//      #define STAPL_TIMER_GETRUSAGE
//    #else (etc.)
#  endif

// timer now selected... include the appropriate header

#  ifdef STAPL_TIMER_GETTIMEOFDAY
#    include "timer_gettimeofday.h"
#  endif

#  ifdef STAPL_TIMER_CLOCK_GETTIME
#    include "timer_clock_gettime.h"
#  endif

#  ifdef STAPL_TIMER_GETRUSAGE
#    include "timer_getrusage.h"
#  endif

#  ifdef STAPL_TIMER_PAPI
#    include "timer_papi.h"
#  endif

namespace stapl
{
// for later
#  ifdef LATER
  template < class T > class Timer
  {
  public:
    virtual ~ Timer(void);
    virtual T start(void);      // start timer... return start marker
    virtual double stop(T);     // stop timer and return counts
    virtual void name(void);    // give this timer a name
  };
#  endif
};                              // end namespace

#endif

// EOF
