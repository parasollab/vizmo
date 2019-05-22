/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_PARAM_DEPS_ZIP_PD_HPP
#define STAPL_SKELETONS_PARAM_DEPS_ZIP_PD_HPP

#include <type_traits>
#include <stapl/utility/tuple/tuple.hpp>
#include <stapl/utility/tuple/front.hpp>
#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/skeletons/operators/elem_helpers.hpp>
#include <stapl/skeletons/param_deps/utility.hpp>
#include <stapl/utility/integer_sequence.hpp>

namespace stapl {
namespace skeletons {
namespace skeletons_impl {

//////////////////////////////////////////////////////////////////////
/// @brief A zip parametric dependency is usually used in @c zip skeleton.
/// It sends input with the same index of each flow to the given @c Op.
///
/// Example - the inputs to a spawned element created by this skeleton
/// would be:
/// @li in<0>[idx]
/// @li in<1>[idx]
/// @li in<2>[idx]
/// @li ...
///
/// @tparam Op the workfunction to be applied on each element
/// @tparam i  the number of input flows
/// @see zip
///
/// @ingroup skeletonsParamDepsInternal
//////////////////////////////////////////////////////////////////////
template <typename Op, int i, typename F, bool isResult>
class zip_pd
{
  Op m_op;
  F  m_filter;
public:
  using in_port_size = std::integral_constant<int, i>;

  template <typename In>
  using output_type = param_deps_output_type<Op, In, i>;

  zip_pd(Op const& op, F const& f)
    : m_op(op),
      m_filter(f)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief If coord is <idx, ...> it wraps the @c Op with the
  /// following inputs and sends it to the visitor along with the
  /// @c m_op
  /// @li in<0>[idx]
  /// @li in<1>[idx]
  /// @li ...
  ///
  /// @param coord        <i, j, k, ...> where i < n, j < m, k < p
  /// @param visitor      the information about Op and input is passed
  ///                     so that later this information can be converted
  ///                     to a node in the dependence graph
  /// @param in_flow      a tuple of input flows to consume from
  //////////////////////////////////////////////////////////////////////
  template <typename Coord, typename Visitor, typename In>
  void case_of(Coord const& /*skeleton_size*/, Coord const& coord,
               Visitor& visitor, In&& in_flow) const
  {
    apply_case_of(
      tuple_ops::front(coord),
      visitor, std::forward<In>(in_flow),
      stapl::make_index_sequence<i>());
  }

private:
  template <typename Index, typename Visitor, typename In,
            std::size_t... Indices>
  void apply_case_of(Index&& index,
                     Visitor& visitor, In&& in_flow,
                     index_sequence<Indices...>&&) const
  {
    visitor.template operator()<isResult>(
      m_op,
      stapl::get<Indices>(in_flow).consume_from(
        make_tuple(std::forward<Index>(index)),
        m_filter
      )...
    );
  }

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief determines how many of the instances of this parametric
  /// dependency will be consuming from a producer with a given coordinate.
  /// This is a reverse query as compared to case_of
  ///
  /// @tparam FlowIndex the flow index to which this request is sent
  //////////////////////////////////////////////////////////////////////
  template <typename Size, typename Coord, typename FlowIndex>
  std::size_t consumer_count(Size const&  /*skeleton_size*/,
                             Coord const& /*producer_coord*/,
                             FlowIndex) const
  {
    return 1;
  }

  Op get_op() const
  {
    return m_op;
  }

  F get_filter() const
  {
    return m_filter;
  }

  void define_type(typer& t)
  {
    t.member(m_op);
    t.member(m_filter);
  }
};

} // namespace skeletons_impl

//////////////////////////////////////////////////////////////////////
/// @brief Creates a zip parametric dependency given a @c op. This method
/// is used whenever filtering is needed on the input edge.
///
/// @copybrief skeletons_impl::zip_pd
///
/// @ingroup skeletonsParamDeps
//////////////////////////////////////////////////////////////////////
template <int i         = 2,
          bool isResult = false,
          typename Op,
          typename F    = skeletons::no_filter>
skeletons_impl::zip_pd<Op, i, F, isResult>
zip_pd(Op const& op, F const& f = F())
{
  return skeletons_impl::zip_pd<Op, i, F, isResult>(op, f);
}

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_PARAM_DEPS_ZIP_PD_HPP
