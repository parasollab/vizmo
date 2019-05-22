/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <stapl/runtime.hpp>
#include <stapl/views/strided_view.hpp>
#include <stapl/views/counting_view.hpp>
#include <stapl/algorithms/algorithm.hpp>
#include <stapl/algorithms/numeric.hpp>

#include "../test_report.hpp"

stapl::exit_code stapl_main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cout << "usage: exe n" << std::endl;
    exit(1);
  }

  size_t n = atoi(argv[1]);

  typedef size_t                                              value_type;
  typedef stapl::result_of::counting_view<value_type>::type   counting_type;
  typedef stapl::strided_view<counting_type>::type            strided_type;

  counting_type counting = stapl::counting_view<value_type>(n*2);

  strided_type strided0 = stapl::make_strided_view(counting, 2, 0);
  strided_type strided1 = stapl::make_strided_view(counting, 2, 1);

  value_type sum0 = stapl::accumulate(strided0, 0);
  value_type sum1 = stapl::accumulate(strided1, 0);

  STAPL_TEST_REPORT(sum0 == ((n-1)*n), "Testing strided_view_zero_offset: ")
  STAPL_TEST_REPORT(sum1 == n*n, "Testing strided_view_one_offset: ")

  return EXIT_SUCCESS;
}
