/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_LINEAR_ALGEBRA_POWER_METHOD_HPP
#define STAPL_LINEAR_ALGEBRA_POWER_METHOD_HPP

#include "linear_algebra_operations.hpp"

#ifndef STAPL_NTIMER
#include <stapl/runtime/counter/default_counters.hpp>
#endif

namespace stapl {

namespace detail {

struct set_rand
{
  typedef void result_type;
  template <typename P>
  void operator()(P p)
  {
    p = std::rand() % 10;
  }
};


template <typename Matrix, typename VectorQ, typename VectorZ, typename VectorR>
typename Matrix::value_type
power_method(Matrix const& A,
             VectorQ const& qv, VectorZ const& zv, VectorR const& residv,
             size_t niters, const double tolerance)
{
#ifndef STAPL_NTIMER
  typedef stapl::counter<stapl::default_timer> counter_type;
#endif

  typedef typename Matrix::value_type                    scalar_type;
  typedef double                                         magnitude_type;

  // Fill the iteration vector z with random numbers to start. Don't
  // have grand expectations about the quality of our pseudorandom
  // number generator; this is usually good enough for eigensolvers.
  map_func(set_rand(),zv);

  // lambda: the current approximation of the eigenvalue of maximum magnitude.
  // normz: the 2-norm of the current iteration vector z.
  // residual: the 2-norm of the current residual vector "resid"
  scalar_type lambda = 0.0;
  magnitude_type normz = 0.0;
  magnitude_type residual = 0.0;
  const scalar_type one = 1.0;

  // How often to report progress in the power method. Reporting
  // progress requires computing a residual which can be expensive.
  // However, if you don't compute the residual often enough, you
  // might keep iterating even after you've converged.
  const size_t reportFrequency = 10;

  // Do the power method, until the method has converged or the
  // maximum iteration count has been reached.
#ifndef STAPL_NTIMER
  counter_type c;
  counter_type c2;
  double looptime = 0;
  double scaletime = 0;
#endif
  size_t iter = 0;
  for (; iter < niters; ++iter) {
#ifndef STAPL_NTIMER
    c.start();
#endif
    // Compute the 2-norm of z
    normz = norm2(zv);

#ifndef STAPL_NTIMER
    c2.start();
#endif
    // q := z / normz
    scale(qv,(one/normz),zv);
#ifndef STAPL_NTIMER
    c2.stop();
    if (c2.value()>scaletime)
      scaletime = c2.value();
    c2.reset();
#endif

    // z := A * q
    matrix_vector_multiply(A,qv,zv);

    // lambda = q.dot (z); // Approx. max eigenvalue
    lambda = inner_product(qv,zv,0.0);

    // Compute and report the residual norm every reportFrequency
    // iterations, or if we've reached the maximum iteration count.
    if (iter % reportFrequency == 0 || iter + 1 == niters) {
      // z := A*q - lambda*q
      waxpy(-lambda, qv, zv, residv);

      // 2-norm of the residual vector
      residual = norm2(residv);
    }
    if (residual < tolerance) {
#ifndef STAPL_NTIMER
      looptime = c.value()/iter;
      stapl::do_once([=] {
          std::cout << " Converged after " << iter << " iterations"
                    << std::endl;
        });
#endif
      break;
    }
    else
      if (iter+1 == niters) {
        std::cout << "Failed to converge after " << niters
                  << " iterations" << std::endl;
        break;
      }
#ifndef STAPL_NTIMER
    c.stop();
#endif
  }
#ifndef STAPL_NTIMER
  looptime = c.value()/iter;
  stapl::do_once([=] {std::cout << "    Loop time (avr): "
                                << looptime << std::endl;});
  stapl::do_once([=] {std::cout << "    scale time (min): "
                                << scaletime << std::endl;});
#endif

  return lambda;
}

} // namespace detail

} // namespace stapl

#endif /* STAPL_LINEAR_ALGEBRA_POWER_METHOD_HPP */
