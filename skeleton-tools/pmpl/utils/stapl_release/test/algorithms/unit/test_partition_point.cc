/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <stapl/runtime.hpp>
#include <iostream>
#include <stapl/algorithms/algorithm.hpp>
#include <cstdlib>
#include <stapl/containers/array/array.hpp>
#include <stapl/views/array_view.hpp>
#include <stapl/containers/generators/functor.hpp>

#include "test_util.hpp"


struct is_odd{
  typedef bool result_type;
  typedef int argument_type;
  template<typename Int>
  bool operator()(Int const& i) const{
    return ((i%2) == 1);
  }
};


stapl::exit_code stapl_main(int argc, char* argv[])
{
  typedef stapl::array<int>                    p_array;
  typedef stapl::array_view<p_array>           array_view_t;
  typedef stapl::functor_container<gen_func>   gen_container_t;
  typedef stapl::array_view<gen_container_t>   rand_gen_view_t;

  int result = 0;

  int nelem = 1000;
  if (argc>1){
    nelem = atoi(argv[1]);
  }

  srand(time(NULL));

  //Creating and array of nelem elements
  p_array int_array(nelem);

  //Creating a view over int_array
  array_view_t a_view(int_array);

  //Creating a container to generate random numbers
  gen_container_t rg_container(nelem,gen_func(nelem));

  //Creating a view over rg_container
  rand_gen_view_t rgc_view(rg_container);

  //Copying rgc_view into a_view
  stapl::copy(rgc_view, a_view);

  result = stapl::partition_point(a_view, is_odd());

  //Testing the partition_point algorithm. This value must be 6.
  std::cout << "Testing partition_point algorithm... ";
  if (result == 6){
    std::cout << "[PASSED]" << std::endl;
  }else{
    std::cout << "[FAILED]" << std::endl;
  }

  return EXIT_SUCCESS;
}
