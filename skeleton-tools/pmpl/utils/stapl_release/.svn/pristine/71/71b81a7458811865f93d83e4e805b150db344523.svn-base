/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <stapl/containers/vector/vector.hpp>

#include "../p_container_profiler.hpp"
#include "../profiler_util.h"
#include "../value_type_util.h"
#include "index_generator.hpp"

using namespace stapl;

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for push_back() of vector
///
/// @tparam pC Profiled container type
/// @tparam vec_value_tye Value type of index vector
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >,
         typename vec_value_type = size_t>
class vec_push_back_profiler
  : public p_container_profiler<pC,Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;

  std::vector<vec_value_type> const& indices;
  const size_t sz;

public:
////////////////////////////////////////////////////////////////////////////////
/// @param pcname String containing container type for reporting
/// @param pc Profiled container
/// @param idx Vector of indices to test, or values to push
////////////////////////////////////////////////////////////////////////////////
  vec_push_back_profiler(std::string pcname, pC* pc,
               std::vector<vec_value_type> const& idx,
               int argc=0, char **argv=NULL)
    : base_type(pc, pcname+"_push_back", argc, argv),
      indices(idx), sz(idx.size())
  { }

  void initialize_iteration()
  {
    rmi_fence();
    this->m_pc = new pC(sz);
    rmi_fence();
  }

 void run()
  {
    for (size_t i=0; i<sz; ++i) {
      this->m_pc->push_back(indices[i]);
    }
    rmi_fence();
  }

  void finalize_iteration()
  {
    rmi_fence();
    delete this->m_pc;
    rmi_fence();
  }
};


exit_code stapl_main(int argc, char** argv)
{
  if (argc < 2) {
    if (get_location_id() == 0)
      std::cout << "Usage: exe n\n";
    exit(1);
  }

  size_t n = pow(2, atoi(argv[1])) * get_num_locations();
  size_t block = n / get_num_locations();
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

  typedef size_t                                  value_type;
  typedef vector<value_type>                      vector_type;
  typedef size_t                                  gid_type;
  typedef counter<default_timer>                  counter_type;

  vector_type c(n);

  std::vector<gid_type> indices;

  index_generator::generate(indices, block*get_location_id(),
                            block, premote, next_only);

  rmi_fence();

  std::string name = "vector<size_t>";

  constructor_size_profiler<vector_type, counter_type>
    csp(name, &c, block, argc, argv);
  csp.collect_profile();
  csp.report();

  set_element_profiler<vector_type, counter_type>
    sep(name, &c, indices, argc, argv);
  sep.collect_profile();
  sep.report();

  get_element_profiler<vector_type, counter_type>
    gep(name, &c, indices, argc, argv);
  gep.collect_profile();
  gep.report();

  get_element_split_profiler<vector_type, counter_type>
    gesp(name, &c, indices, argc, argv);
  gesp.collect_profile();
  gesp.report();

  operator_square_bracket_lhs_profiler<vector_type, counter_type>
    osblp(name, &c, indices, argc, argv);
  osblp.collect_profile();
  osblp.report();

  operator_square_bracket_rhs_profiler<vector_type, counter_type>
    osbrp(name, &c, indices, argc, argv);
  osbrp.collect_profile();
  osbrp.report();

  sum_op<value_type> so;
  apply_set_profiler<vector_type, sum_op<value_type>, counter_type>
    asp(name, &c, so, indices, argc, argv);
  asp.collect_profile();
  asp.report();

  get_sum<value_type> gs;
  apply_get_profiler<vector_type, get_sum<value_type>, counter_type>
    agp(name, &c, gs, indices, argc, argv);
  agp.collect_profile();
  agp.report();

  vec_push_back_profiler<vector_type, counter_type>
    pbp(name, &c, indices, argc, argv);
  pbp.collect_profile();
  pbp.report();

  return EXIT_SUCCESS;
}
