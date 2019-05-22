/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_PARAM_DEPS_BITREVERSAL_PD_HPP
#define STAPL_SKELETONS_PARAM_DEPS_BITREVERSAL_PD_HPP

#include <type_traits>
#include <stapl/utility/tuple/tuple.hpp>
#include <stapl/skeletons/operators/elem_helpers.hpp>
#include <stapl/skeletons/utility/bitreverse.hpp>
#include <stapl/skeletons/param_deps/utility.hpp>

namespace stapl {
namespace skeletons {
namespace skeletons_impl {

//////////////////////////////////////////////////////////////////////
/// @brief A bitreversal parametric dependency used in the @c bitreversal
/// skeleton.
///
/// @see bitreversal
///
/// @ingroup skeletonsParamDepsInternal
//////////////////////////////////////////////////////////////////////
class bitreversal_pd
{
  using op_t = stapl::identity_op;
public:
  using in_port_size = std::integral_constant<int, 1>;

  template <typename In>
  using output_type = param_deps_output_type<op_t, In, 1>;

  //////////////////////////////////////////////////////////////////////
  /// @brief If coord is <i, h, ...> it wraps the identity op with the
  ///   following inputs and sends it to the visitor.
  /// @li in<0>[bitreversal_index(i)]
  ///
  /// @param skeleton_size <n, m, p, ...> where each element is
  ///                     potentially multi-dimensional.
  /// @param coord        <i, j, k, ...> where i < n, j < m, k < p
  /// @param visitor      the information about Op and input is passed
  ///                     so that later this information can be converted
  ///                     to a node in the dependence graph
  /// @param in_flow      a tuple of input flows to consume from
  ///
  /// @see bitreverse
  //////////////////////////////////////////////////////////////////////
  template <typename Coord, typename Visitor, typename In>
  void case_of(Coord const& skeleton_size, Coord const& coord,
               Visitor& visitor, In&& in_flow) const
  {
    using stapl::get;
    std::size_t cur_index = get<0>(coord);
    std::size_t size = get<0>(skeleton_size);
    std::size_t br_idx = bitreverse<std::size_t>(size)(cur_index);
    std::size_t level = get<1>(coord);

    visitor(op_t(),
            get<0>(in_flow).consume_from(make_tuple(br_idx, level - 1)));

  }

  //////////////////////////////////////////////////////////////////////
  /// @brief determines how many of the instances of this parametric
  /// dependency will be consuming from a producer with a given coordinate.
  /// This is a reverse query as compared to case_of.
  ///
  /// @tparam FlowIndex the flow index to which this request is sent
  //////////////////////////////////////////////////////////////////////
  template <typename Size, typename Coord, typename FlowIndex>
  std::size_t consumer_count(Size const&  skeleton_size,
                             Coord const& /*producer_coord*/,
                             FlowIndex) const
  {
    return 1;
  }

  op_t get_op(void) const
  {
    return op_t();
  }
};

} // namespace skeletons_impl

//////////////////////////////////////////////////////////////////////
/// @brief Creates a biterversal parametric dependency.
///
/// @ingroup skeletonsParamDeps
//////////////////////////////////////////////////////////////////////
skeletons_impl::bitreversal_pd
bitreversal_pd(void)
{
  return skeletons_impl::bitreversal_pd();
}


} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_PARAM_DEPS_BITREVERSAL_PD_HPP
