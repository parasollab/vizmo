/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <random>

#include <stapl/runtime.hpp>

#include <stapl/containers/multiarray/multiarray.hpp>
#include <stapl/views/multiarray_view.hpp>
#include <stapl/containers/matrix/matrix.hpp>

#include <stapl/containers/array/array.hpp>
#include <stapl/views/array_view.hpp>

#include <stapl/runtime/counter/default_timer.hpp>
#include <stapl/runtime/counter/default_counters.hpp>

#include <stapl/views/band_view.hpp>
#include <stapl/views/matrix_view.hpp>

#include "power_method.hpp"
#include "../test_report.hpp"

using namespace stapl;


struct fill_matrix
{
  typedef void result_type;

  int m_left_value;
  int m_diag_value;
  int m_right_value;

  fill_matrix(int lv, int diag, int rv)
    : m_left_value(lv), m_diag_value(diag), m_right_value(rv)
  { }

  template <typename T>
  void operator()(T m)
  {
    auto first_point = m.domain().first();

    size_t n = get<1>(m.domain().dimensions());
    size_t i = get<0>(first_point);
    size_t j = get<1>(first_point);

    if (i==0) {
      m(i,j++) = m_diag_value;
      m(i,j) = m_right_value;
    }
    if (n==2 && i>0) {
      m(i,j++) = m_left_value;
      m(i,j) = m_diag_value;
    }
    if (n==3) {
      m(i,j++) = m_left_value;
      m(i,j++) = m_diag_value;
      m(i,j)   = m_right_value;
    }
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_left_value);
    t.member(m_diag_value);
    t.member(m_right_value);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Power method for max eigenvalue using stapl array to
///     support containers
//////////////////////////////////////////////////////////////////////
template <class matrix>
typename matrix::value_type
stapl_power_method(matrix const& A, size_t m, size_t n,
                   size_t niters, double tolerance)
{
  typedef typename matrix::value_type                value_type;
  typedef value_type                                 scalar_type;

  typedef array<value_type>                          vec_type;

  // Create three vectors for iterating the power method. Since the
  // power method computes z = A*q, q should be in the domain of A and
  // z should be in the range. (Obviously the power method requires
  // that the domain and the range are equal, but it's a good idea to
  // get into the habit of thinking whether a particular vector
  // "belongs" in the domain or range of the matrix.) The residual
  // vector "resid" is of course in the range of A.
  vec_type q(n);
  vec_type z(m);
  vec_type resid(m);

  typedef array_view<vec_type>   view_t;
  auto qv = view_t(q);
  auto zv = view_t(z);
  auto residv = view_t(resid);

  return stapl::detail::power_method(A, qv, zv, residv, niters, tolerance);
}


//////////////////////////////////////////////////////////////////////
void test_only_stapl(size_t n)
{
  typedef stapl::multiarray<2,double>             matrix_type;

  typedef stapl::matrix_view<matrix_type>         matrix_view_type;
  typedef typename matrix_type::dimensions_type   dimensions_type;
  dimensions_type dims = dimensions_type(n,n);
  matrix_type A(dims);

  // Filling elements in the band diagonal
  auto matview = matrix_view_type(A);
  auto bandv = stapl::make_band_view(matview,1,1);
  stapl::map_func(fill_matrix(-1,2,-1), bandv);

  // Number of iterations
  size_t niters = 500;

  // Desired (absolute) residual tolerance
  typedef double magnitude_type;
  const magnitude_type tolerance = 1.0e-2;

  // Run the power method and report the result.
  typedef stapl::counter<stapl::default_timer> counter_type;
  counter_type ctr;
  ctr.start();
  double lambda = stapl_power_method(matview, n, n, niters, tolerance);
  ctr.stop();
  double lapse = ctr.value();

  stapl::do_once([=] {
      std::cout << "Only-STAPL Estimated max eigenvalue: "
                << lambda
                << " time:" << lapse
                << std::endl;
    });
  STAPL_TEST_REPORT(
                    (3.8<lambda && lambda<4.0),
                    "Testing STAPL test_power_method"
                    );
}


stapl::exit_code stapl_main(int argc, char* argv[])
{
  size_t n = 100;

  if (argc>1)
    n = atoi(argv[1]);

  test_only_stapl(n);

  return EXIT_SUCCESS;
}
