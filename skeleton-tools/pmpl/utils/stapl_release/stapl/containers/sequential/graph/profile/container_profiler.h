/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_SEQUENTIAL_GRAPH_CONTAINER_PROFILER_HPP
#define STAPL_CONTAINERS_SEQUENTIAL_GRAPH_CONTAINER_PROFILER_HPP

#include <stapl/profiler/base_profiler.hpp>

namespace stapl {

/**
 * In addition to time other counters may print more information like
 * l2 or l3 cache misses; these will be reported using a
 * specialization of this class;
 */
template <class Metric, class Profiler>
struct print_extra_information{
  static void print(Profiler& p){}
};

/**
 * Container profiler class; It can be customized with the Counter
 * and predicate to extract a metric from the value type of the
 * counter;
 */
template <class C, class Counter>
class container_profiler
  : public base_profiler<Counter>
{
private:
  typedef base_profiler<Counter> base_type;
protected:
  C* m_c;
  size_t n_times;

public:
  container_profiler(C* c, std::string inname,
                     int argc = 0, char **argv = 0)
    : base_type(inname, argc, argv),
      m_c(c)
  {
    n_times = 1;
    for( int i = 1; i < argc; i++) {
      if( !strcmp("--ntimes", argv[i]))
        n_times=atoi(argv[++i]);
    }
  }

  void report(){
    std::cout<<"Test:"<<this->name<<"\n";
    std::cout<<this->name<<"_iterations="<<this->iterations<<"\n";
    std::cout<<this->name<<"_num_invocations_per_iteration="<<this->n_times<<"\n";
    std::cout<<this->name<<"="<<this->avg<<"\n";
    std::cout<<this->name<<"_conf="<<this->confidenceinterval<<"\n";
    std::cout<<this->name<<"_min="<<this->min<<"\n";
    std::cout<<this->name<<"_max="<<this->max<<"\n";
    print_extra_information<typename Counter::value_type,
                            container_profiler>::print(*this);
    std::cout<<"\n";
    base_type::report();
  }
};

}//end namespace stapl

#endif
