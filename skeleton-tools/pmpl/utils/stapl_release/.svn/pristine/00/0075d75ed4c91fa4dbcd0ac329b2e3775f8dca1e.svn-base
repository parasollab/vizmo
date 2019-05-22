/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <cstdlib>
#include <iostream>
#include <vector>

#include <stapl/containers/array/array.hpp>
#include <stapl/algorithm.hpp>
#include <stapl/views/array_view.hpp>
#include <stapl/views/counting_view.hpp>
#include <stapl/list.hpp>

#include "../../test_report.hpp"

using namespace stapl;

stapl::exit_code stapl_main(int argc, char* argv[])
{
  int numelem = 100;

  if (argc >= 2) {
    numelem = atoi(argv[1]);
  }

  typedef int                          value_t;
  typedef array<value_t>               p_array_type;
  typedef array_view<p_array_type>     view_type;
  typedef view_type::reference         ref_t;

  p_array_type array(numelem);
  view_type view(array);

  copy(counting_view<value_t>(numelem),view);

  typedef std::vector<value_t>         vector_t;
  typedef array_view<vector_t>         vec_view_t;

  vector_t vec(3);
  vec[0]=4;
  vec[1]=5;
  vec[2]=6;
  vec_view_t vec_view(vec);

  ref_t ref=search(view,vec_view);

  bool passed = !is_null_reference(ref);

  STAPL_TEST_REPORT(passed, "Testing search over p_array")

#if 0
  **** TO DO AFTER PROMOTION FIX FOR PLISTS - UNCOMMENT BELOW
  p_list test
  typedef list<int> list_t;
  typedef list_view<list_t> viewl_t;
  list_t pl(numelem);
  viewl_t viewl(pl);
  copy(counting_view<int>(numelem),viewl);
  do_once(std::cout<<constant("Testing search over list:"));
  typedef viewl_t::reference refl_t;
  refl_t refer=search(viewl,vec_view);
  std::cout << "ref: " << refer << " index: " << index_of(refer) << std::endl;
  do_once(if_then_else(constant(!is_null_reference(refer)),
  std::cout << constant(": PASSED\n"),
  std::cout << constant(": FAILED\n")
  )
    );
#endif

  return EXIT_SUCCESS;
}
