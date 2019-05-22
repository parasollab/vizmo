/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_ALGORITHMS_LINEAR_ALGEBRA_OPERATIONS_HPP
#define STAPL_ALGORITHMS_LINEAR_ALGEBRA_OPERATIONS_HPP

#include <stapl/algorithms/algorithm.hpp>
#include <stapl/algorithms/numeric.hpp>

#include <stapl/views/repeated_view.hpp>

namespace stapl {

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Returns the power of two of a given element
//////////////////////////////////////////////////////////////////////
template <typename R>
struct power2
{
  typedef R result_type;
  template <typename E>
  result_type operator()(E e) const
  {
    return e*e;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Scales an element based on a given scaling value
//////////////////////////////////////////////////////////////////////
template <typename Scalar>
struct scale_wf
{
  typedef void result_type;
  Scalar value;
  scale_wf(Scalar s) : value(s) {}

  template <typename T>
  void operator()(T&& x) const
  {
    x = value*x;
  }

  template <typename T, typename U>
  void operator()(T&& x, U&& y) const
  {
    x = value*y;
  }

  void define_type(stapl::typer& t)
  {
    t.member(value);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Computes the inner product of the two given views and sets
/// the answer in the given result reference
//////////////////////////////////////////////////////////////////////
struct vec_vec_wf
{
  typedef void result_type;
  template <typename Row, typename Vec, typename T>
  result_type operator()(Row&& row, Vec&& vec, T&& res) const
  {
    res = std::inner_product(vec.begin(),vec.end(),row.begin(),0.0);
  }
};


////////////////////////////////////////////////////////////
/// @brief Workfunction to compute C = alpha*A + B
////////////////////////////////////////////////////////////
template <typename Scalar>
struct waxpy_wf
{
  typedef void result_type;

  Scalar alpha;

  waxpy_wf(Scalar a)
    : alpha(a)
  { }

  template <typename VectorA, typename VectorB, typename VectorC>
  result_type operator()(VectorA A, VectorB B, VectorC C) const
  {
    C = alpha*A + B;
  }

  void define_type(stapl::typer& t)
  {
    t.member(alpha);
  }
};

} // namespace detail


////////////////////////////////////////////////////////////
/// @brief Computes the norm2 of the values referenced for the given @p v view
/// @param v View to compute the norm2
////////////////////////////////////////////////////////////
template <typename V>
typename V::value_type norm2(const V& v)
{
  typedef typename V::value_type value_type;
  return sqrt(stapl::map_reduce(detail::power2<value_type>(),
                                stapl::plus<value_type>(),v));
}


////////////////////////////////////////////////////////////
/// @brief  Computes v[i] = scalar*v[i]
/// @param v View to scale
/// @param s Scalar value
////////////////////////////////////////////////////////////
template <typename V, typename Scalar>
V scale(V const& v, Scalar s)
{
  stapl::map_func(detail::scale_wf<Scalar>(s),v);
  return v;
}


////////////////////////////////////////////////////////////
/// @brief Computes v[i] = scalar*w[i]
/// @param v Resulting view
/// @param w View to scale
/// @param s Scalar value
////////////////////////////////////////////////////////////
template <typename V, typename Scalar, typename W>
V scale(V const& v, Scalar s, W const& w)
{
  stapl::map_func(detail::scale_wf<Scalar>(s),v,w);
  return v;
}


////////////////////////////////////////////////////////////
/// @brief Computes a matrix-vector multiplication
/// @param matrix Matrix view
/// @param vec View to multiply
/// @param res Resulting view
////////////////////////////////////////////////////////////
template <typename MatrixV, typename VectorV, typename VectorRes>
void matrix_vector_multiply(MatrixV const& matrix,
                            VectorV const& vec, VectorRes&& res)
{
  map_func(detail::vec_vec_wf(), matrix.rows(), make_repeat_view(vec), res);
}


////////////////////////////////////////////////////////////
/// @brief Computes  W = alpha*X + Y.
/// @param alpha Scalar value
/// @param X View to scale
/// @param Y View to add
/// @param W Resulting view
////////////////////////////////////////////////////////////
template <typename Scalar, typename VectorX, typename VectorY, typename VectorW>
void waxpy(Scalar alpha, VectorX X, VectorY Y, VectorW W)
{
  map_func(detail::waxpy_wf<Scalar>(alpha),X,Y,W);
}

} // namespace stapl


#endif /* STAPL_ALGORITHMS_LINEAR_ALGEBRA_OPERATIONS_HPP */
