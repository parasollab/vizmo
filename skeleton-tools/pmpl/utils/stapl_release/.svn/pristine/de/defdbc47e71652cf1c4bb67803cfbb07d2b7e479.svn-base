/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


//STAPL includes:
#include "test_util.h"
#include <stapl/containers/graph/multidigraph.hpp>
#include <stapl/containers/graph/views/graph_view.hpp>
#include <stapl/containers/graph/algorithms/graph_io.hpp>
#include <stapl/algorithms/algorithm.hpp>

using namespace stapl;

struct vertex_similarity_wf
{
  typedef bool result_type;
  template<typename V1, typename V2>
  result_type operator() (V1 v1, V2 v2)
  {
    if (v1.size() != v2.size())
      return false;

    typename V1::adj_edge_iterator aei1 = v1.begin();
    typename V1::adj_edge_iterator aei_end = v1.end();
    typename V2::adj_edge_iterator aei2 = v2.begin();
    typename V2::adj_edge_iterator aei2_end = v2.end();

    for (; aei1 != aei_end; ++aei1)
      if (graph_find(aei2, aei2_end,
                     eq_target<size_t>((*aei1).target())) == aei2_end)
        return false;

    return true;
  }
};


void test_sharder(std::string filename)
{
  typedef graph<DIRECTED, MULTIEDGES> graph_type;
  typedef graph_view<graph_type> graph_view_t;

  graph_view_t v1 = read_adj_list<graph_type>(filename);

  one_print("Testing Graph Shard Reader\t\t");

  graph_view_t v2
    = sharded_graph_reader<graph_type>(filename, read_adj_list_line());

  bool passed = true;
  if (v1.num_vertices() != v2.num_vertices() ||
      v1.num_edges() != v2.num_edges())
    passed = false;

  passed &= map_reduce(vertex_similarity_wf(), logical_and<bool>(), v1, v2);

  one_print(passed);
  rmi_fence();
}


exit_code stapl_main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "usage: exe filename" << std::endl;
    exit(1);
  }
  std::string filename  = argv[1];

  test_sharder(filename);

  return EXIT_SUCCESS;
}

