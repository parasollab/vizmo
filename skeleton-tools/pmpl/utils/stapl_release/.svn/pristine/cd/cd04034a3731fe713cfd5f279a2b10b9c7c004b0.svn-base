/*
 // Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
 // component of the Texas A&M University System.

 // All rights reserved.

 // The information and source code contained herein is the exclusive
 // property of TEES and may not be disclosed, examined or reproduced
 // in whole or in part without explicit written authorization from TEES.
 */

#ifndef STAPL_ALGORITHMS_MATRIX_MATRIX_UTILS_HPP
#define STAPL_ALGORITHMS_MATRIX_MATRIX_UTILS_HPP

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <stapl/runtime.hpp>
#include <stapl/views/multiarray_view.hpp>
#include <stapl/domains/domain2D.hpp>
#include <stapl/containers/partitions/balanced.hpp>
#include <stapl/algorithms/algorithm.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Assigns random values to the elements in the given range.
///
/// @todo it should not be in a nested namespace
//////////////////////////////////////////////////////////////////////
struct assign_random
{
  int m_range;
  assign_random(int range)
    : m_range(range)
  { }

  void define_type(typer& t)
  {
    t.member(m_range);
  }

  template <typename Reference>
  void operator()(Reference element)
  {
    element = random() % m_range;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Prints the given element using the given output stream.
///
/// @todo it should not be in a nested namespace
//////////////////////////////////////////////////////////////////////
struct print_element
{
  std::ostream& m_o;
  print_element(std::ostream& o)
    : m_o(o)
  { }

  template <typename Reference>
  void operator()(Reference element)
  {
    m_o << std::setw(4) << element;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Prints all the elements in a given 1D view.
///
/// @see print_element
///
/// @todo It can be used for any input, should it be renamed and moved
/// to another location
//////////////////////////////////////////////////////////////////////
struct print_matrix_row
{
  std::ostream& m_o;
  print_matrix_row(std::ostream& o)
    : m_o(o)
  { }

  template <typename View>
  void operator()(View view) const
  {
    std::for_each(view.begin(), view.end(), print_element(m_o));
    m_o << "\n";
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief A helper function to print a matrix with the help of
/// @c print_matrix_row.
///
/// @see print_matrix_row
//////////////////////////////////////////////////////////////////////
template <typename View>
void print_matrix_rows(View& view, std::ostream& o)
{
  std::for_each(view.begin(), view.end(), print_matrix_row(o));
}

//////////////////////////////////////////////////////////////////////
/// @brief Outputs a matrix to a given output stream.
//////////////////////////////////////////////////////////////////////
template <typename View>
void print_matrix(View& view, std::ostream& o = std::cout)
{
#if 0
  typedef typename View::domain_type                  matrix_dom_t;
  /// create a row major partition of the matrix
  typedef rows_partition<
            matrix_dom_t,
            indexed_domain<size_t> >                  rows_p_t;
  /// now this is time to use rows_paritition_type over the matrix view
  /// and create a view of those partitions
  typedef segmented_view<
            View,
            rows_p_t,
            map_fun_gen1<
              fcol_2d<
                size_t,
                typename matrix_dom_t::index_type> >,
            typename View::row_type>                  mixed_pview_t;

  /// now we have to partition the domain of matrix to rows
  rows_p_t view_rows_partition(view.domain());

  /// and then use the segmented view on the original view to create
  /// rows view of the matrix
  mixed_pview_t matrix_segmented_view(view, view_rows_partition);
  print_matrix_rows(matrix_segmented_view, o);
#endif
}

//////////////////////////////////////////////////////////////////////
/// @brief Initializes a given matrix with random values in a given
/// range [0..random_range).
//////////////////////////////////////////////////////////////////////
template <typename View>
void initialize_matrix(View view, int random_range)
{
  /// make a linearized representation of 'matrix view(A)'
  typename View::linear_row_type lrow = view.linear_row();

  /// assign in parallel random values to A
  if (random_range == 0)
    stapl::for_each(lrow, stapl::bind1st(stapl::assign<int>(), 0));
  else
    stapl::for_each(lrow, assign_random(random_range));
}

//////////////////////////////////////////////////////////////////////
/// @brief initializes a matrix with a constant value of type T.
///
/// @param view the matrix_view over the matrix to be initialized
/// @param constant the constant value
//////////////////////////////////////////////////////////////////////
template <typename View, typename T>
void initialize_matrix_with_constant(View view, T constant)
{
  /// make a linearized representation of 'matrix view(A)'
  typename View::linear_row_type  lrow = view.linear_row();
  stapl::for_each(lrow, stapl::bind1st(stapl::assign<T>(), constant));
}
}//namespace stapl
#endif // STAPL_ALGORITHMS_MATRIX_MATRIX_UTILS_HPP
