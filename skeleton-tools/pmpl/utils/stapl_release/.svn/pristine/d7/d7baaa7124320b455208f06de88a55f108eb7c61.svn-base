/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_PARAM_DEPS_REVERSE_BINARY_TREE_PD_HPP
#define STAPL_SKELETONS_PARAM_DEPS_REVERSE_BINARY_TREE_PD_HPP

#include <type_traits>
#include <stapl/utility/tuple/tuple.hpp>
#include <stapl/utility/tuple/front.hpp>
#include <stapl/skeletons/operators/elem_helpers.hpp>
#include <stapl/skeletons/param_deps/utility.hpp>

namespace stapl {
namespace skeletons {
namespace skeletons_impl {

//////////////////////////////////////////////////////////////////////
/// @brief A simple broadcast parametric dependency is usually used in
/// combination with @c reverse_tree to define @c broadcast and defines
/// the parametric dependency for each element in a broadcast tree.
///
/// @tparam Op     the workfunction to be used for each element
/// @tparam Tag    specifies the type of the parametric dependency
/// @tparam Filter the filter to be applied on the inputs of each @c Op
/// @see broadcast
///
/// @ingroup skeletonsParamDepsInternal
//////////////////////////////////////////////////////////////////////
template <typename Op, typename Tag, typename Filter, bool is_position_aware>
class reverse_binary_tree_pd
{
  Op     m_op;
  Filter m_filter;
public:
  using in_port_size = std::integral_constant<int, 1>;

  template <typename In>
  using output_type = param_deps_output_type<Op, In, 1>;

  reverse_binary_tree_pd(Op const& op, Filter const& filter)
    : m_op(op),
      m_filter(filter)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Dispatches the case_of request to the appropriate
  /// implementation based on the given tag.
  //////////////////////////////////////////////////////////////////////
  template <typename... Args>
  void case_of(Args&&... args) const
  {
    apply_case_of(std::forward<Args>(args)..., Tag());
  }

private:
  template <typename Oper>
  void apply_set_position(
    std::true_type,
    Oper const& op, std::size_t cur_index, bool is_downedge) const
  {
    const_cast<Oper&>(op).set_position(cur_index, is_downedge);
  }

  template <typename Oper>
  void apply_set_position(
    std::false_type,
    Oper const& op, std::size_t cur_index, bool is_downedge) const
  { }

  void update_position_info(std::size_t cur_index, bool is_downedge) const
  {
    apply_set_position(
      std::integral_constant<
        bool, is_position_aware>(),
        m_op, cur_index, is_downedge);

    apply_set_position(
      std::integral_constant<
        bool,
        is_position_aware and
        (not std::is_same<Filter, skeletons::no_filter>::value)>(),
        m_filter, cur_index, is_downedge);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Dependency specification of a reverse binary left-skewed tree.
  ///
  /// If coord is <i, h, ...> and skeleton_size is <n, m, p, ...>
  /// it wraps the @c Op with one of the following inputs and sends it
  /// to the visitor along with the @c m_op
  /// @li in<0>[i/2, m-1]
  ///
  /// @param skeleton_size <n, m, p, ...> where each element is
  ///                     potentially multi-dimensional.
  /// @param coord        <i, j, k, ...> where i < n, j < m, k < p
  /// @param visitor      the information about Op and input is passed
  ///                     so that later this information can be converted
  ///                     to a node in the dependence graph
  /// @param in_flow      a tuple of input flows to consume from
  //////////////////////////////////////////////////////////////////////
  template <typename Coord, typename Visitor, typename In>
  void apply_case_of(Coord const& skeleton_size, Coord const& coord,
                     Visitor& visitor, In&& in_flow, tags::left_skewed) const
  {
    using stapl::get;
    std::size_t i = get<0>(coord);
    std::size_t level = get<1>(coord);

    if (get<0>(skeleton_size) == 1) {
      visitor(typename stapl::identity_selector<Op>::type(),
              get<0>(in_flow).consume_from(make_tuple(0, level - 1)));
    }
    else {
      bool is_downedge = !((get<0>(skeleton_size) == 1) or
                           (i % 2 == 0));
      update_position_info(i, is_downedge);

      visitor(
        m_op,
        get<0>(in_flow).consume_from(make_tuple(i/2, level - 1), m_filter));
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief If coord is <i, h, ...> and skeleton_size is <n, m, p, ...>
  /// it wraps the @c Op with one of the following inputs and sends it
  /// to the visitor along with the @c m_op
  /// @li in<0>[i - 2<sup>m - h</sup>, m-1]
  /// @li in<0>[i, m-1]
  ///
  /// The choice of input is dependent on the coordinate.
  ///
  /// @param skeleton_size <n, m, p, ...> where each element is
  ///                     potentially multi-dimensional.
  /// @param coord        <i, j, k, ...> where i < n, j < m, k < p
  /// @param visitor      the information about Op and input is passed
  ///                     so that later this information can be converted
  ///                     to a node in the dependence graph
  /// @param in_flow      a tuple of input flows to consume from
  ///
  /// @ingroup skeletonsParamDepsInternal
  //////////////////////////////////////////////////////////////////////
  template <typename Coord, typename Visitor, typename In>
  void apply_case_of(Coord const& skeleton_size, Coord const& coord,
                     Visitor& visitor, In&& in_flow, tags::left_aligned) const
  {
    using stapl::get;
    std::size_t i = get<0>(coord);
    std::size_t level = get<1>(coord);

    if (get<0>(skeleton_size) == 1) {
      visitor(typename stapl::identity_selector<Op>::type(),
              get<0>(in_flow).consume_from(make_tuple(0, level - 1)));
    }
    else {
      std::size_t dist =
        std::size_t(1) <<
          ((get<1>(skeleton_size) - 1) - get<1>(coord));

      bool is_downedge = !((get<0>(skeleton_size) == 1) or
                           (i % (dist * 2) == 0));
      update_position_info(i, is_downedge);

      if (!is_downedge) {
        visitor(
          m_op,
          get<0>(in_flow).consume_from(
            make_tuple(tuple_ops::front(coord), level - 1), m_filter));
      }
      else {
        visitor(
          m_op,
          get<0>(in_flow).consume_from(
            make_tuple(tuple_ops::front(coord) - dist, level - 1), m_filter));
      }
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief If coord is <i, h, ...> and skeleton_size is <n, m, p, ...>
  /// it wraps the @c Op with the following inputs and sends it to the
  /// visitor along with the @c m_op
  /// @li in<0>[i + 2<sup>m - h</sup>]
  ///
  /// @param skeleton_size <n, m, p, ...> where each element is
  ///                     potentially multi-dimensional.
  /// @param coord        <i, j, k, ...> where i < n, j < m, k < p
  /// @param visitor      the information about Op and input is passed
  ///                     so that later this information can be converted
  ///                     to a node in the dependence graph
  /// @param in_flow      a tuple of input flows to consume from
  ///
  /// @ingroup skeletonsParamDepsInternal
  //////////////////////////////////////////////////////////////////////
  template <typename Coord, typename Visitor, typename In>
  void apply_case_of(Coord const& skeleton_size, Coord const& coord,
                     Visitor& visitor, In&& in_flow, tags::right_aligned) const
  {
    using stapl::get;
    std::size_t i = get<0>(coord);
    std::size_t level = get<1>(coord);

    if (get<0>(skeleton_size) == 1) {
      visitor(typename stapl::identity_selector<Op>::type(),
              get<0>(in_flow).consume_from(make_tuple(0, level - 1)));
    }
    else {
      std::size_t dist =
        std::size_t(1) <<
          ((get<1>(skeleton_size) - 1) - get<1>(coord));

      // in a right-aligned tree, we have an up-edge instead of a down-edge
      bool is_downedge = ((get<0>(skeleton_size) == 1) or
                           ((i+1) % (dist * 2) == 0));

      update_position_info(i, is_downedge);

      if (is_downedge) {
        visitor(m_op, get<0>(in_flow).consume_from(
                        make_tuple(tuple_ops::front(coord), level - 1),
                        m_filter));
      }
      else {
        visitor(m_op, get<0>(in_flow).consume_from(
                        make_tuple(tuple_ops::front(coord) + dist, level - 1),
                        m_filter));
      }
    }
  }
public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Determines how many of the instances of this parametric
  /// dependency will be consuming from a producer with a given coordinate.
  /// This is a reverse query as compared to case_of.
  ///
  /// @param  skeleton_size the size of this skeleton
  /// @tparam FlowIndex    the flow index on which this request is sent
  //////////////////////////////////////////////////////////////////////
  template <typename Size, typename Coord, typename FlowIndex>
  std::size_t consumer_count(Size const&  skeleton_size,
                             Coord const& /*producer_coord*/,
                             FlowIndex) const
  {
    return tuple_ops::front(skeleton_size) == 1 ? 1 : 2;
  }

  Op get_op() const
  {
    return m_op;
  }

  Op get_filter() const
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
/// @brief Creates a broadcast parametric dependency given a @c op
/// and a given filter
///
/// @copybrief skeletons_impl::reverse_binary_tree_pd
///
/// @ingroup skeletonsParamDepsTree
//////////////////////////////////////////////////////////////////////
template <typename Tag,
          bool     is_position_aware = false,
          typename Op,
          typename Filter = skeletons::no_filter>
skeletons_impl::reverse_binary_tree_pd<Op, Tag, Filter, is_position_aware>
reverse_binary_tree_pd(Op const& op, Filter const& filter = Filter())
{
  return skeletons_impl::reverse_binary_tree_pd<
           Op, Tag, Filter, is_position_aware>(op, filter);
}

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_PARAM_DEPS_REVERSE_BINARY_TREE_PD_HPP
