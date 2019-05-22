/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <stapl/containers/graph/multidigraph.hpp>
#include <stapl/containers/graph/views/graph_view.hpp>
#include <stapl/containers/graph/algorithms/triangle_count.hpp>
#include <stapl/containers/graph/algorithms/properties.hpp>
#include <stapl/containers/graph/algorithms/graph_io.hpp>
#include <stapl/containers/graph/generators/mesh.hpp>

#include "test_util.h"

using namespace std;

stapl::exit_code stapl_main(int argc,char** argv)
{
  size_t nx, ny, num_triangles = 2;
  size_t tuning = 0;

  if (argc > 2) {
    nx = atol(argv[1]);
    ny = atol(argv[2]);
  } else {
    cout<<"usage: exe x-dim y-dim\n";
    return EXIT_FAILURE;
  }

  for (int i = 1; i < argc; i++) {
    if (!strcmp("--tuning", argv[i]))
      tuning = atoi(argv[i+1]);
    if (!strcmp("--num_triangles", argv[i]))
      num_triangles = atoi(argv[i+1]);
  }

  typedef stapl::multidigraph<stapl::properties::triangle_count_property> PGR;
  typedef stapl::graph_view<PGR> graph_view_t;
  graph_view_t vw = stapl::generators::make_mesh<graph_view_t>(nx, ny);

  // base-case.
  std::pair<size_t, double> x0 = stapl::triangle_count(vw, tuning);
  size_t num_triangles_counted_0 = x0.first;

  // add some triangles to the mesh.
  stapl::do_once([&](void) {
    size_t size = vw.size();
    size_t j = 0;
    for (size_t i=0; i<num_triangles/2; ++i) {
      size_t vd = j++;
      if (vd % nx == nx-1)
        vd = j++;
      size_t tgt = vd + nx + 1;
      if (tgt < size) {
        vw.add_edge(vd, tgt);
        vw.add_edge(tgt, vd);
      }
    }
    });

  stapl::rmi_fence();  // needed for add_edge in do_once.

  // compute the actual number of triangles that were added.
  const size_t num_edges_mesh = (((nx-1)*(ny-1)*2) + (nx-1) + (ny-1))*2;
  const size_t extra_edges = (vw.num_edges_collective() - num_edges_mesh);

  one_print("Testing Triangle Count...\t\t\t");
  std::pair<size_t, double> x = stapl::triangle_count(vw, tuning);
  size_t num_triangles_counted = x.first;
  one_print(num_triangles_counted == extra_edges &&
            num_triangles_counted_0 == 0);

  one_print("Testing Clustering-Coefficient...\t\t");
  // This r should approach 4, the larger the mesh gets, and
  // should always be greater than 1, except in the base-case (no triangles).
  double r = double(3*num_triangles_counted/x.second - 9*num_triangles_counted)
           / num_edges_mesh;
  one_print(r > 1 && r <= 4 && x0.second == 0);

  return EXIT_SUCCESS;
}
