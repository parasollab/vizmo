/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_PARAM_DEPS_UTILITY_HPP
#define STAPL_SKELETONS_PARAM_DEPS_UTILITY_HPP

#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/utility/integer_sequence.hpp>
#include <boost/utility/result_of.hpp>

namespace stapl {
namespace skeletons {

//////////////////////////////////////////////////////////////////////
/// @brief Computes the output type of a given parametric dependency
/// by passing fine-grain value type of all input flows to the given
/// @c Op.
///
/// @tparam Op          the operation used in the parametric dependency
/// @tparam In          a tuple of input flows to be passed to the
///                     parametric dependency
/// @tparam n           the arity of the operation
/// @tparam homogeneous true if all the inputs are read from a single
///                     flow. Otherwise one element from each flow
///                     in @c In is used.
//////////////////////////////////////////////////////////////////////
template <typename Op, typename In, std::size_t n,
          bool homogeneous = false,
          typename IdxList = make_index_sequence<n>>
struct param_deps_output_type;

template <typename Op, typename In, std::size_t n,
          bool homogeneous,
          std::size_t... Indices>
struct param_deps_output_type<Op, In, n, homogeneous,
                              index_sequence<Indices...>>
{
  using type =
    typename boost::result_of<
      Op(flow_value_type<In, (homogeneous ? 0 : Indices)>...)>::type;
};

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_PARAM_DEPS_UTILITY_HPP
