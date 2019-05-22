/*
 // Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
 // component of the Texas A&M University System.

 // All rights reserved.

 // The information and source code contained herein is the exclusive
 // property of TEES and may not be disclosed, examined or reproduced
 // in whole or in part without explicit written authorization from TEES.
 */

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <stapl/runtime.hpp>
#include <stapl/array.hpp>
#include <stapl/algorithms/matrix/summa.hpp>
#include <stapl/multiarray.hpp>
#include "../../test_report.hpp"

using namespace stapl;

#define VALIDATE_RESULTS

#ifdef SHOW_RESULTS
template <typename V>
void print_2d_array(V&& v, std::string title)
{
  std::size_t nx, ny;
  std::tie(nx, ny) = v.domain().dimensions();
  std::size_t startx, starty;
  std::tie(startx, starty) = v.domain().first();

  std::cerr << title << "\n";
  for (std::size_t i = 0; i < nx; ++i)
  {
    for (std::size_t j = 0; j < ny; ++j) {
      std::cerr << std::setw(4)  << v[make_tuple(startx+i, starty+j)];
    }
    std::cerr << "\n";
  }
}
#endif

stapl::exit_code stapl_main(int argc, char* argv[])
{
  if (argc < 4) {
    std::cerr << "usage: test_summa n m k #defines matrices "
      "with sizes <n, m> and <m, k>.\n";
    return 1; // exit(1);
  }

  std::size_t n = atol(argv[1]);
  std::size_t m = atol(argv[2]);
  std::size_t p = atol(argv[3]);

  using value_t = int;
  multiarray<2, value_t> a(make_tuple(n, m));
  multiarray<2, value_t> b(make_tuple(m, p));
  multiarray<2, value_t> c_skeleton(make_tuple(n, p));
  multiarray<2, value_t> c_naive(make_tuple(n, p));

  auto a_vw = make_multiarray_view(a);
  auto b_vw = make_multiarray_view(b);

  auto c_skeleton_vw = make_multiarray_view(c_skeleton);
  auto c_naive_vw = make_multiarray_view(c_naive);

  typedef typename decltype(a_vw)::index_type i_idx;

  for (size_t i = 0; i < n; i++)
    for (size_t j = 0; j < m; j++)
      a_vw[i_idx(i,j)] = i;

  for (size_t i = 0; i < m; i++)
    for (size_t j = 0; j < p; j++)
      b_vw[i_idx(i,j)] = j;

  // Naive Matrix Multiplication
  stapl::do_once([&]() {
  for (size_t i = 0; i < n; i++)
    for (size_t j = 0; j < p; j++)
      for (size_t q = 0; q < m; q++)
      {
        c_naive_vw[i_idx(i,j)] +=
          a_vw[i_idx(i,q)] * b_vw[i_idx(q,j)];
      }
  });

  stapl::counter<stapl::default_timer> summa_timer;
  summa_timer.start();

  summa(a_vw, b_vw, c_skeleton_vw);

  double t = summa_timer.stop();


#ifdef VALIDATE_RESULTS
  stapl::do_once([&]() {
    bool is_valid = true;
    for (size_t i = 0; i < n; i++)
      for (size_t j = 0; j < p; j++)
        is_valid &= c_naive_vw[i_idx(i,j)] == c_skeleton_vw[i_idx(i,j)];

    STAPL_TEST_REPORT(is_valid, "SUMMA Multiplication Test");
    std::cerr << "Execution time: " << t << std::endl;
  });
#endif

#ifdef SHOW_RESULTS
  stapl::do_once([&]() {
    print_2d_array(a_vw, "input");
    print_2d_array(b_vw, "input2");
    print_2d_array(c_skeleton_vw, "skeleton_output");
    print_2d_array(c_naive_vw, "naive_output");
  });
#endif

  return EXIT_SUCCESS;
}
