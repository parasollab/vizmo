/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <iostream>


#include <stapl/containers/graph/views/graph_view.hpp>
#include <stapl/containers/graph/algorithms/graph_io.hpp>
#include <stapl/containers/graph/algorithms/breadth_first_search.hpp>

#include "test_util.h"
#include "algorithm_wrappers.hpp"
#include "test_perf_util.h"
#include "test_algo_perf.h"
using namespace stapl;


template<stapl::graph_attributes Directedness>
void test_part1(std::string alg_name, std::string filename, size_t niter,
          size_t k_start, size_t k_end, int argc, char** argv)
{
  switch(algorithm_type()(alg_name)) {
  case 1:
    test_graph<bfs_algo_wrapper<Directedness>, no_generator>(
      filename, niter,
      k_start, k_end,
      argc, argv);
    break;
  case 2:
    test_graph<cc_algo_wrapper<Directedness>, no_generator>(
      filename, niter,
      k_start, k_end,
      argc, argv);
    break;
  case 3:
    test_graph<page_rank_algo_wrapper<Directedness>, no_generator>(
      filename, niter,
      k_start, k_end,
      argc, argv);
    break;
  case 4:
    test_graph<sssp_algo_wrapper<Directedness>, no_generator>(
      filename, niter,
      k_start, k_end,
      argc, argv);
    break;
  case 5:
    test_graph<k_core_algo_wrapper<Directedness>, no_generator>(
      filename, niter,
      k_start, k_end,
      argc, argv);
    break;
  default:
    if (get_location_id() == 0)
    std::cout << "ERROR: Unknown Algorithm Type." << std::endl;
    break;
  }
}

template void test_part1<stapl::DIRECTED>(
  std::string alg_name,
  std::string filename,
  size_t niter,
  size_t k_start, size_t k_end,
  int argc, char** argv);

template void test_part1<stapl::UNDIRECTED>(
  std::string alg_name,
  std::string filename,
  size_t niter,
  size_t k_start, size_t k_end,
  int argc, char** argv);
