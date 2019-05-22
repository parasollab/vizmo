/*
 // Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
 // component of the Texas A&M University System.

 // All rights reserved.

 // The information and source code contained herein is the exclusive
 // property of TEES and may not be disclosed, examined or reproduced
 // in whole or in part without explicit written authorization from TEES.
 */
#ifndef STAPL_ALGORITHMS_MATRIX_NAIVE_MATRIX_MULTIPLY_HPP
#define STAPL_ALGORITHMS_MATRIX_NAIVE_MATRIX_MULTIPLY_HPP

#include <stapl/paragraph/paragraph.hpp>
#include <stapl/runtime/serialization.hpp>
#include "views.h"
#include <stapl/skeletons/explicit/coarse_map_wf.h>
#include <stapl/views/repeated_view.hpp>
//wraps user defined workfunction

namespace stapl{
namespace algo_details{

//////////////////////////////////////////////////////////////////////
/// @brief This work function naively multiplies a vector with a
/// matrix, as a part of naive matrix multiplication
///
/// @tparam ViewB the B matrix in a AxB = C multiplication
///
/// @todo can be simplified if 2D indices can be used over matrix views
//////////////////////////////////////////////////////////////////////
template <typename ViewB>
struct naive_multiply_wf
{
  ViewB view_b;

public:
  typedef void result_type;
  naive_multiply_wf(ViewB viewb)
    : view_b(viewb)
  { }


  void define_type(typer& t)
  {
    t.member(view_b);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief This function receives a row of A, the whole Matrix B, and
  /// a row of C, and creates tasks to perform rowAxB=rowC
  ///
  /// @param view_a a row of matrix A
  /// @param view_c the corresponding row of matrix C
  //////////////////////////////////////////////////////////////////////
  template <typename ViewA, typename ViewC>
  void operator()(ViewA view_a, ViewC view_c)
  {
    /// first of all we have to check if matrix sizes match
    stapl_assert((view_a.size() == view_b[0].size()) &&
                 (view_a.size() == view_c.size()),
                 "Matrix view size not match for matrix multiplication");

    typename ViewA::iterator it_a_j;
    typename ViewB::iterator it_b_i = view_b.begin();
    typename ViewC::iterator it_c_i = view_c.begin();

    for (unsigned int i = 0; i < view_a.size(); i++) {

      it_a_j = view_a.begin();
      //definitely we can check the iterator with the end_iterator
      //but why to bother we have the size anyway

      typename ViewB::value_type::iterator it_b_i_j((*it_b_i).begin());
      for (unsigned int j = 0; j < view_c.size() ; j++) {
        *(it_c_i) = *(it_c_i) + *(it_a_j) * (*it_b_i_j);
        ++it_a_j;
        ++it_b_i_j;
      }

      ++it_b_i;

      ++it_c_i;
    }

    /*
     if 2D indices are working this version should be used instead,
     which is much simpler multiplying a row of A to all columns of B and
     put the results in a row of C
     for all rows of A that it receives, it should create
     for (unsigned int i = 0; i < view_a.size(); i++) {
     for (unsigned int j = 0; j < view_a.size(); j++) {
     view_c[i] = view_c[i] + view_a[j]  view_b[i][j];
     }
     }
    */
  }

};
}//namespace algo_details

//////////////////////////////////////////////////////////////////////
/// @brief This is the algorithm that performs a naive matrix
/// multiplication on two matrices given by @c view_a and @c view_b
/// and stores the result in @c view_c (C = A x B)
///
/// @param view_a matrix A
/// @param view_b matrix B
/// @param view_c matrix C (C = A x B)
///
/// @note works only if A and B have the same dimensions
//////////////////////////////////////////////////////////////////////
template <typename ViewA, typename ViewB, typename ViewC>
void naive_matrix_multiply(ViewA& view_a, ViewB& view_b, ViewC& view_c)
{
  /// the three matrices have the same domain TYPE
  typedef typename ViewA::domain_type                 matrix_dom_t;
  /// A and B will have the same segment type
  typedef rows_segment<
            matrix_dom_t,
            indexed_domain<size_t> >                  rows_part_t;
  typedef columns_segment<
            matrix_dom_t,
            indexed_domain<size_t> >                  columns_part_t;
  /// now this is time to use rows_paritition_type over the matrix view
  /// and create a view of using the segments
  /// A and C are both row major, so they will have the same
  /// segmented_view_type
  typedef segmented_view<
            ViewA,
            rows_part_t,
            map_fun_gen1<
              fcol_2d<
                size_t,
                typename matrix_dom_t::index_type> >,
            typename ViewA::row_type>                 matrix_row_part_vt;


  /// but B should be column_segmented
  typedef segmented_view<
            ViewB,
            columns_part_t,
            map_fun_gen1<
              frow_2d<
                size_t,
                typename matrix_dom_t::index_type> >,
            typename ViewB::column_type>              matrix_column_part_vt;

  /// now we have to segment the domain of matrix to rows
  rows_part_t    rows_segment_a(view_a.domain());
  columns_part_t columns_segment_b(view_b.domain());
  rows_part_t    rows_segment_c(view_c.domain());

  /// and then use the segmented view on the original view to create
  /// row view for A and C and column view for B
  matrix_row_part_vt    view_a_row_major(view_a, rows_segment_a);
  matrix_column_part_vt view_b_column_major(view_b, columns_segment_b);
  matrix_row_part_vt    view_c_row_major(view_c, rows_segment_c);

  /// now in order to compute the results we have to create workfunctions from
  /// a row of A, complete B matrix and a row of C. That's why we use
  /// the naive multiply workfunction

  /// @note you can use repeated view instead of sending the B matrix to
  /// the workfunction
  typedef algo_details::naive_multiply_wf<matrix_column_part_vt> wf_t;
  map_func(wf_t(view_b_column_major), view_a_row_major, view_c_row_major);
}
}//namespace stapl
#endif // STAPL_ALGORITHMS_MATRIX_NAIVE_MATRIX_MULTIPLY_HPP
