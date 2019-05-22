/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <stapl/containers/graph/multigraph.hpp>
#include <stapl/containers/graph/views/graph_view.hpp>
#include <stapl/containers/graph/algorithms/k_core.hpp>
#include <stapl/containers/graph/algorithms/graph_io.hpp>
#include <stapl/containers/graph/generators/random_neighborhood.hpp>
#include <stapl/containers/graph/generators/mesh.hpp>

#include "test_util.h"

using namespace stapl;

struct size_wf
{
  typedef int result_type;
  template<typename V>
  int operator() (V v)
  {
    if (v.property() >= 0)
      return 1;
    else return 0;
  }
};


template<typename Graph>
bool k_core_test(Graph& graph, int k_core_sz, size_t k, double nx, double ny)
{
  size_t num_iterations = k_core(graph, k_core_sz, k);

  bool passed
    = (num_iterations == ceil((ceil(nx/2) + ceil(ny/2) - 1) / double(k+1)));
  int total_size = map_reduce(size_wf(), stapl::plus<int>(), graph);
  passed &= (total_size == 0);

  return passed;
}

void test_graph(int k_core_sz, size_t nx, size_t ny)
{
  typedef multigraph<int> graph_type;
  typedef graph_view<graph_type>                           graph_view_t;

  graph_view_t v = generators::make_mesh<graph_view_t>(nx, ny);

  one_print("Testing k-core\t\t\t\t");

  bool all_passed = true;

  size_t k = 0;
  all_passed &= k_core_test(v, k_core_sz, k, nx, ny);

  k = 1;
  all_passed &= k_core_test(v, k_core_sz, k, nx, ny);

  for (size_t i=0; i<4; ++i) {
    k *= 2;
    all_passed &= k_core_test(v, k_core_sz, k, nx, ny);
  }

  one_print(all_passed);
}

stapl::exit_code stapl_main(int argc, char* argv[])
{
  if (argc < 3) {
    std::cerr << "usage: exe nx ny" << std::endl;
    exit(1);
  }
  int k_core_sz = 3;
  size_t nx = atoi(argv[1]);
  size_t ny = atoi(argv[2]);

  test_graph(k_core_sz, nx, ny);

  return EXIT_SUCCESS;
}
