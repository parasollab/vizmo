/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <stapl/views/type_traits/is_view.hpp>
#include <stapl/containers/array/array.hpp>
#include <stapl/skeletons/map.hpp>
#include <type_traits>
#include "../../test_report.hpp"

struct empty_wf
{
  typedef void result_type;

  template <typename View>
  void operator()(View) const
  { }
};

template <typename T>
bool trait_holds(T const&, bool expected,
                 typename std::enable_if<stapl::is_view<T>::value>::type* = 0)
{
  return true == expected;
}

template <typename T>
bool trait_holds(T const&, bool expected,
                 typename std::enable_if<!stapl::is_view<T>::value>::type* = 0)
{
  return false == expected;
}


stapl::exit_code stapl_main(int argc, char* argv[])
{
  typedef stapl::array<int>                            parray;
  typedef stapl::array_view<parray>                    parray_view;

  parray      pa(100);
  parray_view pav(pa);

  stapl::map_func(empty_wf(), pav);

  bool result = true;
  result &= trait_holds(pa, false);
  result &= trait_holds(pav, true);

  STAPL_TEST_REPORT(result, "Testing is_view<T>");

  return EXIT_SUCCESS;
}
