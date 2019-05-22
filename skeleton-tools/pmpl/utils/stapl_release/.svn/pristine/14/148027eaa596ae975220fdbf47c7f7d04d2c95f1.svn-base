/*
 // Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
 // component of the Texas A&M University System.

 // All rights reserved.

 // The information and source code contained herein is the exclusive
 // property of TEES and may not be disclosed, examined or reproduced
 // in whole or in part without explicit written authorization from TEES.
 */

#ifndef STAPL_ALGORITHMS_MATRIX_SUMMA_WORK_FUNCTIONS_HPP
#define STAPL_ALGORITHMS_MATRIX_SUMMA_WORK_FUNCTIONS_HPP

#include <boost/mpl/logical.hpp>
#include <boost/utility/enable_if.hpp>
#include "blas_utils.hpp"

#ifdef STAPL_USE_ATLAS
extern "C" {
  #include "cblas.h"
}
#endif
#ifdef STAPL_USE_MKL
extern "C" {
  #include "mkl_cblas.h"
}
#endif

namespace stapl {
namespace algo_details {

//////////////////////////////////////////////////////////////////////
/// @brief Matrix multiplication using BLAS is a little bit tricky.
/// The reason for that is that we use the Fortran implementation of
/// BLAS which means that matrices are assumed to be column major.
//////////////////////////////////////////////////////////////////////
struct summa_multiply_wf
{
    typedef void result_type;
    summa_multiply_wf()
    {
    }
    template <typename ViewA, typename ViewB, typename ViewC,
              typename Enable = void>
    struct matrix_multiplier
    {
        void operator()(ViewA view_a, ViewB view_b, ViewC view_c)
        {
          size_t c_row_size = get<0> (view_c.domain().size_twod());
          size_t c_column_size = get<1> (view_c.domain().size_twod());
          size_t a_column_size = get<1> (view_a.domain().size_twod());

          /// loop interchange optimization, and keeping a value
          for (unsigned int i = 0; i < c_row_size; i++) {
              for (unsigned int k = 0; k < a_column_size; k++) {
                typename ViewA::value_type a_i_k = view_a(i,k);
                for (unsigned int j = 0; j < c_column_size; j++) {
                  view_c(i, j) = view_c(i, j) + a_i_k * view_b(k, j);
              }
            }
          }
        }

    };

#if defined(STAPL_USE_MKL) || defined (STAPL_USE_ATLAS)
    template <typename ViewA, typename ViewB, typename ViewC>
    struct matrix_multiplier<
             ViewA, ViewB, ViewC,
             typename
               enable_if<
                 and_<
                   algorithm_impl::dblas_conformable<ViewA, ViewB, ViewC>,
                   is_row_major<ViewA>,
                   is_row_major<ViewB>,
                   is_row_major<ViewC> > >::type>
    {
        void operator()(ViewA view_a, ViewB view_b, ViewC view_c)
        {
#ifdef SHOW_TIMING
          timer t;
          t.reset();
          t.start();
#endif
          const int c_row_size = get<0> (view_c.domain().size_twod());
          const int c_column_size = get<1> (view_c.domain().size_twod());
          const int a_column_size = get<1> (view_a.domain().size_twod());

          /// remember that we have padded Bs for example
          /// for a matrix of size 5x5 we might broadcast a sub B with size
          /// 3x2 , in those cases, the last column of B should be ignored,
          /// for this reason we need sub B's column size too
          const int b_column_size = get<1> (view_b.domain().size_twod());

          typename ViewA::view_container_type::value_type* a =
              view_a.get_container()->get_raw_data();
          typename ViewB::view_container_type::value_type* b =
              view_b.get_container()->get_raw_data();
          typename ViewC::view_container_type::value_type* c =
              view_c.get_container()->get_raw_data();

          const double alpha = 1;
          const double beta = 1;
          //C = beta * C + alpha * A * B
          cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, c_row_size,
                      c_column_size, a_column_size, alpha, a, a_column_size,
                      b, b_column_size, beta, c, c_column_size);

#ifdef SHOW_TIMING
          std::cout << get_location_id() << " - time spent in multiplication : "
                    << t.stop() << std::endl;
#endif
        }
    };
#endif
    //////////////////////////////////////////////////////////////////////
    /// @brief This specialization uses normal multiplication - should be
    /// the slowest version. This work function receives a row of A, the
    /// whole matrix B, and a row of C.
    ///
    /// @param vA a row of matrix A
    /// @param vB a column of B
    /// @param vC the result row in C
    //////////////////////////////////////////////////////////////////////
    template <typename ViewA, typename ViewB, typename ViewC>
    result_type operator()(ViewA vA, ViewB vB, ViewC vC) const
    {
      matrix_multiplier<ViewA, ViewB, ViewC> ()(vA, vB, vC);
    }
};
}//namespace algo_details
}//namespace stapl

#endif // STAPL_ALGORITHMS_MATRIX_SUMMA_WORK_FUNCTIONS_HPP
