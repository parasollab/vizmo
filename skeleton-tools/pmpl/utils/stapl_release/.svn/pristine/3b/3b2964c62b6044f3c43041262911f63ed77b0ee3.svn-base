/*
// Copyright (c) 2000-2010, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <iostream>

#include <stapl/array.hpp>
#include <stapl/algorithms/algorithm.hpp>
#include <stapl/containers/multiarray/multiarray.hpp>
#include <stapl/views/multiarray_view.hpp>
#include <stapl/views/stencil_view.hpp>
#include <stapl/utility/do_once.hpp>

#include "../../test_report.hpp"

using namespace stapl;

//////////////////////////////////////////////////////////////////////
/// @brief Helper struct that can return different coefficients based
///        on how far a certain point is from the initial point.
//////////////////////////////////////////////////////////////////////
struct coefficient_view
{
  const double c0, c1, c2;

  coefficient_view(double c0_, double c1_, double c2_)
    : c0(c0_), c1(c1_), c2(c2_)
  { }

  double const& operator()(int i, int j) const
  {
    const int distance = (i != 0 ? 1 : 0) + (j != 0 ? 1 : 0);

    switch (distance)
    {
      case 0: return c0;
      case 1: return c1;
      default: return c2;
    }
  }

  void define_type(stapl::typer& t)
  {
    t.member(c0);
    t.member(c1);
    t.member(c2);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Stencil work function that receives a single point and
///        the 9-point stencil view of its neighbors.
///
///        This computes a weighted average, where the weights for
///        the points with Manhattan distance of 0, 1 and 2 can be
///        specified in the constructor.
//////////////////////////////////////////////////////////////////////
struct stencil9
{
  typedef void result_type;

  coefficient_view m_coeff;

  stencil9(double const c0, double const c1, double const c2)
    : m_coeff(c0, c1, c2)
  { }

  template<typename T, typename View>
  void operator()(T x, View vw)
  {
    auto const first = vw.domain().first();
    auto const last = vw.domain().last();

    for (std::size_t i = get<0>(first); i <= get<0>(last); ++i)
      for (std::size_t j = get<1>(first); j <= get<1>(last); ++j)
      {
        // get coefficient based on distance
        const int x_off = static_cast<int>(i)-get<0>(first)-1;
        const int y_off = static_cast<int>(j)-get<1>(first)-1;
        const double c = m_coeff(x_off, y_off);

        // add contribution of this point
        x += c * vw[make_tuple(i,j)];
      }
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_coeff);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Compute the L2 norm of a multiarray view.
//////////////////////////////////////////////////////////////////////
template<typename View>
double norm(View const& view)
{
  const double sum =
    map_reduce(
      boost::bind(stapl::multiplies<double>(), _1, _1),
      stapl::plus<double>(),
      linear_view(view)
    );

  auto dims = view.dimensions();

  return std::sqrt(sum / get<1>(dims) / get<0>(dims));
}


stapl::exit_code stapl_main(int argc, char** argv)
{
  using stapl::fill;
  using stapl::do_once;

  if (argc < 2) {
    return EXIT_FAILURE;
  }

  const std::size_t n = atol(argv[1]);

  typedef stapl::multiarray<2, double>        grid_type;
  typedef stapl::multiarray_view<grid_type>   view_type;

  // create grids and views
  grid_type lhs_grid(make_tuple(n, n));
  view_type lhs(lhs_grid);

  grid_type rhs_grid(make_tuple(n, n));
  view_type rhs(rhs_grid);

  // fill grids with all zeros except for the first value, which is one
  fill(lhs, 0);
  fill(rhs, 0);

  do_once([&](){
    rhs[make_tuple(0, 0)] = 1;
  });

  // compute norm
  double n0 = norm(rhs);

  // create view for stencil
  auto stencil = make_stencil_view<2>(rhs);

  // compute 9-point stencil

  // FIXME (coarsening): this test needs to be executed coarse-grained when
  // coarsening for the stencil view is available.
#if 0
   map_func(stencil9(0.5, 0.25, 0.125), lhs, stencil);
#endif

  using span_t = skeletons::spans::balanced<2>;

  auto stencil_skeleton = skeletons::zip<2, span_t>(stencil9(0.5, 0.25, 0.125));

  skeletons::algorithm_executor<null_coarsener>().execute(
    stencil_skeleton, lhs, stencil
  );

  // compute post-stencil norm and make sure that it went down
  double n1 = norm(lhs);

  bool passed = n1 < n0;

  STAPL_TEST_REPORT(passed, "Testing 9-point stencil");

  return EXIT_SUCCESS;
}
