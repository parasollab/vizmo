/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


//////////////////////////////////////////////////////////////////////
/// @file
/// Test creating mutliple executors with multiple tasks.
//////////////////////////////////////////////////////////////////////

#include <stapl/runtime.hpp>
#include <stapl/runtime/executor/scheduler/sched.hpp>
#include <iostream>
#include "simple_executor.hpp"
#include "../test_utils.h"

using namespace stapl;

void dummy_task(void)
{ }

class p_test
: public p_test_object
{
public:
  void execute(void)
  {
    typedef simple_executor<stapl::default_scheduler> executor_type;
    typedef executor_type::task_type                  task_type;

    const unsigned int N = 100;

    for (unsigned int i=0; i<N; ++i) {
      executor_type* ex = new executor_type;
      for (unsigned int j=0; j<(i+1); ++j) {
        ex->add_task(new task_type(&dummy_task));
      }
      get_executor().add_executor(ex);
    }
  }
};


stapl::exit_code stapl_main(int, char*[])
{
  p_test pt;
  pt.execute();
#ifndef _TEST_QUIET
  std::cout << stapl::get_location_id() << " successfully passed!" << std::endl;
#endif
  return EXIT_SUCCESS;
}
