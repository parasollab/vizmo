/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <stapl/containers/matrix/matrix.hpp>
#include "../p_container_profiler.hpp"
#include "../profiler_util.h"
#include "../value_type_util.h"
#include "matrix_index_generator.hpp"

using namespace stapl;

exit_code stapl_main(int argc, char** argv)
{
  if (argc < 3) {
    if (get_location_id() == 0)
      std::cout << "Usage: exe n m\n";
    exit(1);
  }

  size_t n = atoi(argv[1])*get_num_locations();
  size_t m = atoi(argv[2])*get_num_locations();
  size_t blockn = n / get_num_locations();
  size_t blockm = m / get_num_locations();
  size_t premote(0);
  size_t next_only(0);

  for (int i = 1; i < argc; ++i) {
    if (!strcmp("--premote", argv[i])) {
      premote = atoi(argv[++i]);
    }
    if (!strcmp("--nextonly", argv[i])) {
      next_only = atoi(argv[++i]);
    }
  }

  typedef int                                      value_type;
  typedef indexed_domain<size_t>                   vector_domain_type;
  typedef balanced_partition<vector_domain_type>   balanced_type;
  typedef row_major                                traversal_type;
  typedef nd_partition<
            stapl::tuple<balanced_type, balanced_type>,
            traversal_type>                        partition_type;
  typedef tuple<size_t, size_t>                    gid_type;
  typedef matrix<value_type, traversal_type,
                 partition_type>                   matrix_type;

  balanced_type p0(vector_domain_type(0, n-1), get_num_locations());
  balanced_type p1(vector_domain_type(0, m-1), get_num_locations());
  partition_type part(p0, p1);

  matrix_type c(gid_type(n,m), part);

  std::vector<gid_type> indices;

  matrix_index_generator::generate(indices, blockn*get_location_id(),
                            blockm*get_location_id(), blockn, blockm,
                            premote, next_only);

  rmi_fence();

  /*
  constructor_size_profiler<matrix<value_type, traversal_type, partition_type>,
                            counter<default_timer> >
                            cep("matrix<int>", NULL, n*m, argc, argv);
  */

  set_element_profiler<matrix_type, counter<default_timer>, gid_type>
    sep("matrix<int>", &c, indices, argc, argv);
  sep.collect_profile();
  sep.report();

  rmi_fence();

  get_element_profiler<matrix_type, counter<default_timer>, gid_type>
    gep("matrix<int>", &c, indices, argc, argv);
  gep.collect_profile();
  gep.report();

  rmi_fence();

  get_element_split_profiler<matrix_type, counter<default_timer>, gid_type>
    gesp("matrix<int>", &c, indices, argc, argv);
  gesp.collect_profile();
  gesp.report();

  operator_square_bracket_lhs_profiler<matrix_type, counter<default_timer>,
                                       gid_type>
    osblp("matrix<int>", &c, indices, argc, argv);
  osblp.collect_profile();
  osblp.report();

  operator_square_bracket_rhs_profiler<matrix_type, counter<default_timer>,
                                       gid_type>
    osbrp("matrix<int>", &c, indices, argc, argv);
  osbrp.collect_profile();
  osbrp.report();

  return EXIT_SUCCESS;
}
