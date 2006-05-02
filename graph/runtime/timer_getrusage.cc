// 28Oct2004/jkp2866 - I'm not sure why, but getrusage() is returning
// mostly zeros
// rendering this file mostly useless at this point... 

// STAPL_TIMER_GETRUSAGE is used as a #define and as an environment
// variable...
// at run-time, the environment variable will be queried as to which
// metric to use...
// e.g.  export STAPL_TIMER_GETRUSAGE=ru_stime
// will use system time instead (the default is user time)

//#include <sys/time.h>
//#include <sys/resource.h>
#include <string.h>             // for strcmp()
#include <assert.h>             // for assert()

#include "timer_getrusage.h"

#warning I don't know why, but getrusage isn't working right... debugging enabled... jkp/02nov2004

namespace stapl
{
  typedef struct rusage timer;
  int which_timer = (-1);

  typedef struct
  {
    int num;
    char name[12];
  } rusage_timers;

  rusage_timers r_timers[] = {
    {0, "ru_utime"},            /* user time used */
    {1, "ru_stime"},            /* system time used */
    {2, "ru_maxrss"},           /* maximum resident set size */
    {3, "ru_ixrss"},            /* integral shared memory size */
    {4, "ru_idrss"},            /* integral unshared data size */
    {5, "ru_isrss"},            /* integral unshared stack size */
    {6, "ru_minflt"},           /* page reclaims */
    {7, "ru_majflt"},           /* page faults */
    {8, "ru_nswap"},            /* swaps */
    {9, "ru_inblock"},          /* block input operations */
    {10, "ru_oublock"},         /* block output operations */
    {11, "ru_msgsnd"},          /* messages sent */
    {12, "ru_msgrcv"},          /* messages received */
    {13, "ru_nsignals"},        /* signals received */
    {14, "ru_nvcsw"},           /* voluntary context switches */
    {15, "ru_nivcsw"}           /* involuntary context switches */
  };

  int select_timer(void)
  {
    int i, which = 0;
    char *env_timer;
    env_timer = getenv("STAPL_TIMER_GETRUSAGE");
    if (env_timer) {
      for (i = 0; i <= 15; i++) {
        if (!strcmp(env_timer, r_timers[i].name)) {
          which = i;
          break;
        }
      }
    }
    printf("which is %d - %s\n", which, r_timers[which].name);
    assert(which >= 0 && which <= 15);
    return which;
  }

  void dump_rusage(FILE * f, timer ru)
  {
    fprintf(f, "ru_utime.tv_sec = %ld\n", ru.ru_utime.tv_sec);
    fprintf(f, "ru_utime.tv_usec = %ld\n", ru.ru_utime.tv_usec);
    fprintf(f, "ru_stime.tv_sec = %ld\n", ru.ru_stime.tv_sec);
    fprintf(f, "ru_stime.tv_usec = %ld\n", ru.ru_stime.tv_usec);
    fprintf(f, "ru_maxrss = %ld\n", ru.ru_maxrss);
    fprintf(f, "ru_ixrss = %ld\n", ru.ru_ixrss);
    fprintf(f, "ru_idrss = %ld\n", ru.ru_idrss);
    fprintf(f, "ru_isrss = %ld\n", ru.ru_isrss);
    fprintf(f, "ru_minflt = %ld\n", ru.ru_minflt);
    fprintf(f, "ru_majflt = %ld\n", ru.ru_majflt);
    fprintf(f, "ru_nswap = %ld\n", ru.ru_nswap);
    fprintf(f, "ru_inblock = %ld\n", ru.ru_inblock);
    fprintf(f, "ru_oublock = %ld\n", ru.ru_oublock);
    fprintf(f, "ru_msgsnd = %ld\n", ru.ru_msgsnd);
    fprintf(f, "ru_msgrcv = %ld\n", ru.ru_msgrcv);
    fprintf(f, "ru_nsignals = %ld\n", ru.ru_nsignals);
    fprintf(f, "ru_nvcsw = %ld\n", ru.ru_nvcsw);
    fprintf(f, "ru_nivcsw = %ld\n", ru.ru_nivcsw);
  }

  void print_timer(void)
  {
    printf("start_timer: using getrusage( %s )\n", r_timers[which_timer]);
  }

  timer start_timer(void)
  {
    timer time;
    if (which_timer == -1)
      which_timer = select_timer();
#ifdef _PRINT_TIMER
    print_timer();
#endif
    timer_assert(getrusage(RUSAGE_SELF, &time));
    fprintf(stderr, "DEBUG - start_timer() - time\n");
    dump_rusage(stderr, time);
    return time;
  }

  double calc_diff(timer time1, timer time2)
  {
    double diff = 0;
    fprintf(stderr, "DEBUG - calc_diff() - time1\n");
    dump_rusage(stderr, time1);
    fprintf(stderr, "DEBUG - calc_diff() - time2\n");
    dump_rusage(stderr, time2);
    assert(which_timer >= 0 && which_timer <= 15);
    switch (which_timer) {
    case 0:
      diff =
          ((double) (time2.ru_utime.tv_sec - time1.ru_utime.tv_sec) *
           1000000 + (double) (time2.ru_utime.tv_usec -
                               time1.ru_utime.tv_usec)) / 1000000.0;
      break;
    case 1:
      diff =
          ((double) (time2.ru_stime.tv_sec - time1.ru_stime.tv_sec) *
           1000000 + (double) (time2.ru_stime.tv_usec -
                               time1.ru_stime.tv_usec)) / 1000000.0;
      break;
    case 2:
      diff = time2.ru_maxrss - time1.ru_maxrss;
      break;
    case 3:
      diff = time2.ru_ixrss - time1.ru_ixrss;
      break;
    case 4:
      diff = time2.ru_idrss - time1.ru_idrss;
      break;
    case 5:
      diff = time2.ru_isrss - time1.ru_isrss;
      break;
    case 6:
      diff = time2.ru_isrss - time1.ru_isrss;
      break;
    case 7:
      diff = time2.ru_isrss - time1.ru_isrss;
      break;
    case 8:
      diff = time2.ru_isrss - time1.ru_isrss;
      break;
    case 9:
      diff = time2.ru_isrss - time1.ru_isrss;
      break;
    case 10:
      diff = time2.ru_isrss - time1.ru_isrss;
      break;
    case 11:
      diff = time2.ru_isrss - time1.ru_isrss;
      break;
    case 12:
      diff = time2.ru_isrss - time1.ru_isrss;
      break;
    case 13:
      diff = time2.ru_isrss - time1.ru_isrss;
      break;
    case 14:
      diff = time2.ru_isrss - time1.ru_isrss;
      break;
    case 15:
      diff = time2.ru_isrss - time1.ru_isrss;
      break;
    };
    return diff;
  }

  double stop_timer(const timer time1)
  {
    timer time2;
    assert(which_timer >= 0 && which_timer <= 15);
    timer_assert(getrusage(RUSAGE_SELF, &time2));
    return calc_diff(time1, time2);
  }
}                               // end namespace

// EOF
