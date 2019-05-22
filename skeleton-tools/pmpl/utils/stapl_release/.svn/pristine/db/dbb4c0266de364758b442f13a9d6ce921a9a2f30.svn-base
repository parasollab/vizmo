/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_ALGORITHMS_MATRIX_SUMMA_HPP
#define STAPL_ALGORITHMS_MATRIX_SUMMA_HPP

#include <stapl/skeletons/functional/matrix_multiply.hpp>
#include <stapl/skeletons/utility/tags.hpp>

namespace stapl {

/// @addtogroup numericAlgorithms
/// @{

//////////////////////////////////////////////////////////////////////
/// @brief Compute matrix multiplication using the SUMMA Algorithm.
///
/// @tparam MatrixView View of matrices
/// @param a, b, c The A, B, and C matrices in computation.
/// @ingroup numericAlgorithms
//////////////////////////////////////////////////////////////////////
template <typename MatrixView>
void summa(MatrixView&& a, MatrixView&& b, MatrixView&& c)
{

  using namespace skeletons;
  algorithm_executor<null_coarsener> exec;

  using ValueType = typename view_traits<typename
                      std::decay<MatrixView>::type>::value_type;

  exec.execute(sink<ValueType,
    spans::summa<2>>(skeletons_impl::matrix_multiply<ValueType,
                     tags::summa>()), a, b, c, c);
}

/// @}

} // namespace stapl

#endif // STAPL_ALGORITHMS_MATRIX_SUMMA_HPP
