/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <stapl/runtime.hpp>

// FIXME this should become just map_reduce
#include <stapl/skeletons/map_reduce.hpp>
#include <stapl/algorithms/functional.hpp>
#include <stapl/views/counting_view.hpp>

#include "../test_report.hpp"

using namespace stapl;

stapl::exit_code stapl_main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cout << "usage: exe n" << std::endl;
    exit(1);
  }

  long n = atol(argv[1]);

  long sum = map_reduce(
    stapl::identity<long>(), stapl::plus<long>(), counting_view<long>(n)
  );

  STAPL_TEST_REPORT(
    sum == (n*(n-1))/2,
    "Testing counting_view (default initial value)"
  );

  sum = map_reduce(
    stapl::identity<long>(), stapl::plus<long>(), counting_view<long>(n, -(n/2))
  );

  STAPL_TEST_REPORT(
    ((n%2==0) && (sum==-(n/2)))||(sum==0),
    "Testing counting_view (with initial value)"
  );

  return EXIT_SUCCESS;
}
