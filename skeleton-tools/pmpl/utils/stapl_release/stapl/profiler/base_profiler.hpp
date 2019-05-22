/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_PROFILE_BASE_PROFILER_HPP
#define STAPL_PROFILE_BASE_PROFILER_HPP

#ifdef _STAPL
#include <stapl/runtime.hpp>
#endif
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <sys/time.h>

namespace stapl {


//////////////////////////////////////////////////////////////////////
/// @brief Print a message preceded by a time stamp.
///
/// Useful for coarse estimation of the time elapsed between two consecutive
/// invocations.
///
/// @ingroup performanceMonitor
//////////////////////////////////////////////////////////////////////
template <typename S>
void ts_print(S const& s)
{
#ifdef _STAPL
    if (get_location_id()!=0)
      return;
#endif
  struct timeval tv;
  gettimeofday(&tv,NULL);
  std::cout << "[" << tv.tv_sec << "," << tv.tv_usec << "]" << s;
}


//////////////////////////////////////////////////////////////////////
/// @brief Base class to built upon by programs that want to collect profile
///        information about certain functions.
///
/// @tparam Counter Counter type used.
///
/// @ingroup performanceMonitor
//////////////////////////////////////////////////////////////////////
template <typename Counter>
class base_profiler
#ifdef _STAPL
  : public p_object
#endif
{
private:
  typedef typename Counter::value_type metric_type;

protected:
  std::string              name;
  /// Collected measurements.
  std::vector<metric_type> results;
  /// Max value across threads.
  std::vector<metric_type> results_max;
  //add reduced
  std::size_t              iterations;
  time_t                   starttime;
  /// Total time.
  time_t                   wallclockseconds;
  double                   avg;
  double                   max;
  double                   min;
  double                   variance;
  double                   stddev;
  double                   confidenceinterval;
  std::size_t              miniterations;
  std::size_t              maxiterations;
  std::size_t              maxrunseconds;
  std::size_t              maxwallclockseconds;
  std::size_t              useconfrange;
  std::size_t              cachesize;
  double                   m_cache_tmp;
  /// Filename to dump metrics.
  std::string              m_file_name;
  /// Predicate to extract value from the counter.
  Counter                  m_timer;

  metric_type reflect_metric(metric_type const& m) const
  { return m; }

public:
  base_profiler(std::string inname = "unknown", int argc = 0, char** argv = 0)
  : name(inname),
    iterations(0),
    wallclockseconds(0),
    miniterations(1),
    maxiterations(1),
    maxwallclockseconds(0),
    useconfrange(0),
    cachesize(0)
  {
    if (argc>0)
      set_options(argc, argv);
  }

  virtual ~base_profiler(void)
  { }

  unsigned int get_iterations(void) const
  { return iterations; }

  double get_avg(void) const
  { return avg; }

  double get_min(void) const
  { return min; }

  double get_max(void) const
  { return max; }

  double get_std(void) const
  { return stddev; }

  double get_conf(void) const
  { return confidenceinterval; }

  std::string const& get_name(void) const
  { return name; }

  void update_stats(void)
  {
    avg = 0.0;
    min = max = results_max[0];
    for (std::size_t i = 0; i < iterations; i++)
    {
      const double d = results_max[i];
      avg += d;
      if ( d > max )
        max = d;
      if ( d < min )
        min = d;
    }
    avg /= iterations;
    variance=0.0;
    for (std::size_t i = 0; i < iterations; i++)
    {
      const double d = results_max[i] - avg;
      variance += d*d;
    }
    variance /= iterations;
    stddev = sqrt(variance);
    // 1.96 is z-table entry for 95%
    //  formula for min number of iteration is
    //  n=[(1.96*stddev)/(useconfrange*avg)]^2
    confidenceinterval = (1.96*stddev) / sqrt((double) iterations);
  }

  virtual void initialize(void)
  {
    //start the wall clock in case we don't want to run more than
    //a certain ammount of time
    starttime=time(NULL);
  }

  virtual void initialize_iteration(void)
  { }

  virtual void run(void) = 0;

  virtual void finalize_iteration(void)
  { }

  virtual bool continue_iterating(void)
  {
    time_t now = time(NULL);
    if (maxwallclockseconds && std::size_t(now-starttime) > maxwallclockseconds)
      return true; // stop
    if ( iterations >= maxiterations)
      return false; // stop
    if ( iterations < miniterations)
      return true; // keep going
    if ( useconfrange && (confidenceinterval < avg * useconfrange))
      return false;  // stop
    return true; // keep going
  }

  virtual void finalize(void)
  {
    time_t now = time(NULL);
    wallclockseconds=now-starttime;
  }

  void collect_profile(void)
  {
    initialize();

    // for flushing the cache if specified
    const std::size_t N = (cachesize * 1024 * 1024);
    std::vector<char> tdata;

    if (cachesize != 0) {
      tdata.resize(N);
      for (std::size_t i=0; i<N; ++i) {
        tdata[i] = char(rand() % 3);
      }
      m_cache_tmp = 0;
    }

    while (continue_iterating())
      {
        initialize_iteration();
#ifdef _STAPL
        rmi_fence();  // "come together,      //maybe mpi barrier
#endif

        // flush cache each iteration if specified
        if (cachesize != 0){
          for (std::size_t i=0; i<N; ++i) {
            m_cache_tmp += tdata[i];
            // make sure to save the total because the compiler may optimize
            // away this code
          }
        }

        m_timer.reset();
        m_timer.start();
          run(); // the real experiment happens here
        metric_type elapsed = m_timer.stop();
        results.push_back(elapsed);
#ifdef _STAPL
        metric_type max_elapsed = allreduce_rmi(
          [](metric_type const& x, metric_type const& y)
          { return metric_type{std::max(x,y)}; },
          this->get_rmi_handle(),
          &base_profiler::reflect_metric,
          elapsed).get();
#else
        metric_type max_elapsed = elapsed;
#endif
        results_max.push_back(max_elapsed); // over me" -- The Beatles

        finalize_iteration();
        ++iterations; // update iterations before updating stats
        update_stats();
      }

    finalize(); //finalize the experiment
  }

  void report(void)
  {
#ifdef _STAPL
    std::ostringstream os;
    os << get_num_locations() << "_" << get_location_id();
    const std::string fname = m_file_name + "_" + name + "_" + os.str();
#else
    const std::string fname = m_file_name + "_" + name;
#endif
    std::ofstream of(fname.c_str(), std::ios::out);
    of << "Test:"
#ifdef _STAPL
       << this->name << " P="          << get_num_locations() << "\n"
#endif
       << this->name << "_iterations=" << this->iterations    << "\n"
       << this->name << "="            << this->avg           << "\n"
       << this->name << "_min="        << this->min           << "\n"
       << this->name << "_max="        << this->max           << "\n"
       << ">Raw metrics:" << "\n";
    for (std::size_t i=0; i<results.size(); ++i) {
      of << this->name << "_iteration_" << i << "=" << this->results[i] << "\n";
    }
    of << "---->Random Value:" << m_cache_tmp << "\n";
  }

  void print_help_options(void)
  {
#ifdef _STAPL
    if (get_location_id()!=0)
      return;
#endif
    std::ostringstream os;
    os << "Displaying the base profiler options.\n"
       << "--flushcache #cachesize(MB)\n"
       << "--miniterations #min\n"
       << "--maxiterations #max\n"
       << "--file fname(the prefix of the file where all raw results will be"
          " written) \n"
       << "--maxwallclockseconds secs (maximum allowed time; evaluated between"
          " iterations)\n"
       << "--useconfrange [%p] The width of the 95% confidence interval is p%"
          " around the mean; if p not specified it will be 5% by default\n\n";
  }

  void set_options(int argc, char **argv)
  {
    m_file_name="profile";//the prefix of the filename
    cachesize = 0;
    for ( int i = 1; i < argc; i++) {
      if ( !strcmp("--miniterations", argv[i])) {
        miniterations = atoi(argv[++i]);
        if (maxiterations == 1)
          maxiterations = miniterations;
      }
      else if ( !strcmp("--flushcache", argv[i])) {
        cachesize = atoi(argv[++i]);
      }
      else if ( !strcmp("--maxiterations", argv[i])) {
        maxiterations = atoi(argv[++i]);
      }
      else if ( !strcmp("--file", argv[i])) {
        m_file_name = argv[++i];
      }
      else if ( !strcmp("--maxwallclockseconds", argv[i])) {
        maxwallclockseconds = atoi(argv[++i]);
      }
      else if ( !strcmp("--useconfrange", argv[i])) {
        // this is a bit more complicated since the range is optional
        if ( (++i < argc) && atoi(argv[i])) {
          // if there is another argument and it is an integer
          useconfrange = atoi(argv[i]); // use specified range
        }
        else {
          --i;
          useconfrange=5;  // default 5% around mean
        }
      }
      if (useconfrange) {
        if (miniterations == 1)
          miniterations=32;  // if still default (not specified above), change
        if (maxiterations == 1)
          maxiterations=100;  // if still default (not specified above), change
      }
    }
  }
};

} // namespace stapl

#endif
