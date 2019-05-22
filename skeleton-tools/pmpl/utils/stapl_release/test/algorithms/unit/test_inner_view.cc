/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <iostream>
#include <vector>

#include <stapl/algorithms/algorithm.hpp>
#include <p_array.h>

#include <pContainers/pgraph/p_graph.h>
#include <views/array_1D_view.hpp>

#include <views/graph_view.hpp>
#include <pContainers/pgraph/algorithms/p_graph_generators.h>

#include <stapl/views/balance_view.hpp>
#include <stapl/views/counting_view.hpp>
#include <stapl/views/overlap_view.hpp>
#include <stapl/views/native_view.hpp>

using namespace stapl;


struct elem_print
{
  template <typename Ref>
  void operator()(Ref v) const
  {
    std::size_t loc = get_location_id();
    std::stringstream vout;
    vout << loc << ": " << v.descriptor() << " -->";
    for (typename Ref::adj_edge_iterator e = v.begin(); e != v.end(); ++e)
      vout << " " << (*e).target();
    printf("%s\n",vout.str().c_str());
  }
};

struct test_full
{
  template <typename View>
  void operator()(View v)
  {
    std::for_each(v.vertices().begin(),v.vertices().end(),elem_print());
  }
};


struct test_inner
{
  template <typename View>
  void operator()(View v)
  {
    std::for_each(v.inner_graph().begin(),v.inner_graph().end(),elem_print());
  }
};


stapl::exit_code stapl_main(int argc, char* argv[])
{
  std::size_t nx = 4;
  std::size_t ny = 4;
  std::size_t bs = nx*ny;
  nx *= get_num_locations();
  std::size_t nv = nx*ny;

  std::size_t loc = get_location_id();

  typedef p_graph<DIRECTED,MULTIEDGES,int> PGR;
  PGR p(0,bs);
  add_edges_torous<PGR> AET(p,nx,ny);
  AET.add_vertices();
  rmi_fence();
  AET.add_edges_right();
  rmi_fence();

  typedef domset1D<std::size_t>    gdom_t;
  typedef graph_view<PGR,gdom_t>      graph_view_t;
  graph_view_t gv(p,gdom_t(0,nv-1));

  printf("%zu: ========== BEGIN TEST FULL ==========\n", loc);
  for_each(stapl::native_view(gv),test_full());
  printf("%zu: ========== END TEST FULL ==========\n", loc);

  printf("%zu: ========== BEGIN TEST INNER ==========\n", loc);
  for_each(stapl::native_view(gv),test_inner());
  printf("%zu: ========== END TEST INNER ==========\n", loc);

  return EXIT_SUCCESS;
}
