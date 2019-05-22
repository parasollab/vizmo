/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <stapl/algorithms/algorithm.hpp>
#include <stapl/algorithms/functional.hpp>
#include <stapl/runtime.hpp>
#include <stapl/skeletons/serial.hpp>
#include <stapl/containers/matrix/matrix.hpp>
#include <stapl/views/multiarray_view.hpp>
#include <stapl/utility/do_once.hpp>

using namespace std;

typedef stapl::matrix<size_t>            rix_type;
typedef rix_type::size_type              size_type;
typedef stapl::multiarray_view<rix_type> rix_view_type;

typedef stapl::sparse_matrix<size_t>      srix_type;
typedef srix_type::size_type              size_type;
typedef stapl::multiarray_view<srix_type> srix_view_type;


template<typename View>
int test_func(View const& view)
{
  return stapl::accumulate(view, 0);
}


stapl::exit_code stapl_main(int argc, char** argv)
{
  rix_type rix1(size_type(1,5));
  rix_view_type rix_view1(rix1);

  stapl::generate(stapl::linear_view(rix_view1), stapl::sequence<int>(0,1));

  const int valid_result = 10;

  const int result1 = test_func(stapl::linear_view(rix_view1));

  srix_type rix2(size_type(1,5));
  srix_view_type rix_view2(rix2);

  stapl::generate(stapl::linear_view(rix_view1), stapl::sequence<int>(0,1));

  const int result2 = test_func(stapl::linear_view(rix_view1));


  if (result1 == valid_result && result2 == valid_result)
    stapl::do_once([] {
      std::cout << "Test for matrix const correctness PASSED\n";
    });
  else
    stapl::do_once([] {
      std::cout << "Test for matrix const correctness FAILED\n";
    });

  return EXIT_SUCCESS;
}



