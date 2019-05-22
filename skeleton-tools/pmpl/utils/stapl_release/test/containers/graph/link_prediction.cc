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
#include <stapl/containers/graph/algorithms/link_prediction.hpp>
#include <stapl/containers/graph/algorithms/properties.hpp>
#include <stapl/containers/graph/generators/mesh.hpp>

#include "test_util.h"

using namespace std;

struct check_lp_wf
{
  size_t m_nx;

  check_lp_wf(size_t nx = 0)
    : m_nx(nx)
  { }

  typedef bool result_type;
  template<typename V>
  result_type operator()(V v) const
  {
    std::stringstream ss;
    const auto probabilities = v.property().link_probabilities();
    for (auto const& e : v)
      if (probabilities[e.target()] != 1)
        return false;
    auto max_it = std::max_element(probabilities.begin(), probabilities.end());
    unsigned long int max_tgt = std::distance(probabilities.begin(), max_it);
    auto max_p = *max_it;
    // diagonal neighbors of v in mesh.
    auto t0 = v.descriptor() - m_nx - 1;
    auto t1 = v.descriptor() - m_nx + 1;
    auto t2 = v.descriptor() + m_nx - 1;
    auto t3 = v.descriptor() + m_nx + 1;
    // predicted link should be one of the diagonals.
    if (max_p <= 1 ||
        (max_tgt != t0 && max_tgt != t1 && max_tgt != t2 && max_tgt != t3))
      return false;
    return true;
  }

  void define_type(stapl::typer& t)
  { t.member(m_nx); }
};


stapl::exit_code stapl_main(int argc,char** argv)
{
  size_t nx, ny;
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
  }

  typedef stapl::multidigraph<stapl::properties::lp_property> PGR;
  typedef stapl::graph_view<PGR> graph_view_t;
  graph_view_t vw = stapl::generators::make_mesh<graph_view_t>(nx, ny);

  // compute the link-prediction.
  one_print("Testing Link-Prediction...\t\t\t");
  stapl::link_prediction(vw, tuning);

  bool passed
    = stapl::map_reduce(check_lp_wf(nx), stapl::logical_and<bool>(), vw);
  one_print(passed);

  return EXIT_SUCCESS;
}
