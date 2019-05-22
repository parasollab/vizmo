/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#include "../p_container_profiler.hpp"
#include <stapl/containers/array/array.hpp>

using namespace stapl;

#include "../profiler_util.h"

stapl::exit_code stapl_main(int argc, char* argv[])
{
  size_t N;
  if (get_location_id() == 0)
    std::cout<<"pContainer Performance Evaluation\n";
  if (argc > 2) {
    N = atoi(argv[1]);
    srand(atoi(argv[2])+get_location_id());
  } else {
    std::cout<<"Input size N required; Using 10 by default\n";
    N=10;
  }

  constructor_size_profiler<array<int>, counter_type> cep("p_array", NULL, N,
                                                          argc, argv);
  for (int i=1; i<argc; i++) {
    if ( !strcmp("--help", argv[i]) )
      cep.print_help_options();
  }
  cep.collect_profile();
  cep.report();

  array<int> p(N);

  std::vector<size_t> indices(p.size()/get_num_locations());
  for (size_t i=0; i<indices.size(); ++i) {
    indices[i] = lrand48() % N;
  }

  set_element_profiler<array<int>, counter_type> sep("p_array", &p, indices,
                                                     argc, argv);
  sep.collect_profile();
  sep.report();

  get_element_profiler<array<int>, counter_type > gep("p_array", &p, indices,
                                                      argc, argv);
  gep.collect_profile();
  gep.report();

  operator_square_bracket_lhs_profiler<array<int>,
    counter_type > oplep("p_array", &p, indices, argc, argv);
  oplep.collect_profile();
  oplep.report();

  operator_square_bracket_rhs_profiler<array<int>,
    counter_type > oprep("p_array", &p, indices, argc, argv);
  oprep.collect_profile();
  oprep.report();

  return EXIT_SUCCESS;
}
