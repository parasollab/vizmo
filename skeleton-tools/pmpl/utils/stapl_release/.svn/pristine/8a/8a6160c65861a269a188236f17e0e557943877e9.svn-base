/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_PARAM_DEPS_STENCIL_PD_HPP
#define STAPL_SKELETONS_PARAM_DEPS_STENCIL_PD_HPP

#include <type_traits>
#include <stapl/utility/tuple/tuple.hpp>
#include <stapl/utility/tuple/front.hpp>
#include <stapl/skeletons/utility/lightweight_multiarray.hpp>
#include <stapl/skeletons/utility/position.hpp>
#include <stapl/skeletons/operators/elem_helpers.hpp>
#include <stapl/skeletons/param_deps/utility.hpp>

namespace stapl {
namespace skeletons {

namespace stencil_utils {


//////////////////////////////////////////////////////////////////////
/// @brief Sets the direction of the stencil filter and returns a copy
/// of the filter.
//////////////////////////////////////////////////////////////////////
template <typename Tag, typename Filter>
Filter
get_stencil_filter(Filter f)
{
  f.set_direction(Tag());
  return f;
}

//////////////////////////////////////////////////////////////////////
/// @brief The special case that no filtering function is defined.
//////////////////////////////////////////////////////////////////////
template <typename Tag>
skeletons::no_filter
get_stencil_filter(skeletons::no_filter f)
{
  return f;
}

} // namespace stencil_utils

namespace skeletons_impl {

/////////////////////////////////////////////////////////////////////
/// @brief In a 1D stencil, parametric dependencies are defined in
/// such a way that each node depends on every node in its surroundings
/// including itself.
///
/// @tparam Op         the operation to be applied at each point on
///                    the points received
/// @tparam F          the filter type function
/// @tparam isPeriodic determines whether dependency between
///                    points is periodic or not for boundary points
/// @tparam dimensions specifies the number of
///                    input dimensions(1D, 2D, ... , nD)
/// @ingroup skeletonsParamDeps
/////////////////////////////////////////////////////////////////////
template <typename Op, typename F,
          bool isPeriodic, int dimensions, int numPoints>
class stencil_pd;

template <typename Op, typename F, bool isPeriodic>
class stencil_pd<Op, F, isPeriodic, 1, 3>
{
  Op                 m_op;
  F                  m_filter;

  using center_port_index         = std::integral_constant<int, 0>;
  using left_boundary_port_index  = std::integral_constant<
                                      int, (isPeriodic ? 0 : 1)>;
  using right_boundary_port_index = std::integral_constant<
                                      int, (isPeriodic ? 0 : 2)>;

public:
  using in_port_size = std::integral_constant<int, (isPeriodic ? 1 : 3)>;

  template <typename In>
  using output_type = param_deps_output_type<Op, In, 3, isPeriodic>;

  stencil_pd(Op const& op, F const& f)
    : m_op(op),
      m_filter(f)
  { }


  //////////////////////////////////////////////////////////////////////
  /// @brief Dispatches the case_of request based on the @c isPeriodic
  /// boolean variable.
  //////////////////////////////////////////////////////////////////////
  template <typename... Args>
  void case_of(Args&&... args) const
  {
    apply_case_of(std::integral_constant<bool, isPeriodic>(),
                  std::forward<Args>(args)...);
  }

private:
  //////////////////////////////////////////////////////////////////////
  /// @brief This specialization of case_of is used for the cases where
  /// the stencil computation is periodic.
  ///
  /// @param skeleton_size <n, m, p, ...> where each element is
  ///                     potentially multi-dimensional.
  /// @param coord        <i, j, k, ...> where i < n, j < m, k < p
  /// @param visitor      the information about WF and input is passed
  ///                     so that later this information can be converted
  ///                     to a node in the dependence graph
  /// @param in_flow      a tuple of input flows to consume from
  //////////////////////////////////////////////////////////////////////
  template <typename Coord, typename Visitor, typename In>
  void apply_case_of(std::true_type,
                     Coord const& skeleton_size, Coord const& coord,
                     Visitor& visitor, In&& in_flow) const
  {
    using stapl::get;
    auto i = get<0>(coord);
    auto m = get<0>(skeleton_size);

    using stencil_utils::get_stencil_filter;
    auto&& center_flow = get<        center_port_index::value>(in_flow);
    auto&&   left_flow = get< left_boundary_port_index::value>(in_flow);
    auto&&  right_flow = get<right_boundary_port_index::value>(in_flow);

    visitor(
      m_op,
      center_flow.consume_from(make_tuple(i)),
        left_flow.consume_from(make_tuple((i + m -1) % m),
                               get_stencil_filter<tags::direction<-1>>(
                                 m_filter)),
       right_flow.consume_from(make_tuple((i + 1) % m),
                               get_stencil_filter<tags::direction<+1>>(
                                 m_filter)));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief This specialization of case_of is used for the cases where
  /// the stencil computation is non-periodic.
  ///
  /// @param skeleton_size <n, m, p, ...> where each element is
  ///                     potentially multi-dimensional.
  /// @param coord        <i, j, k, ...> where i < n, j < m, k < p
  /// @param visitor      the information about WF and input is passed
  ///                     so that later this information can be converted
  ///                     to a node in the dependence graph
  /// @param in_flow      a tuple of input flows to consume from
  //////////////////////////////////////////////////////////////////////
  template <typename Coord, typename Visitor, typename In>
  void apply_case_of(std::false_type,
                     Coord const& skeleton_size, Coord const& coord,
                     Visitor& visitor, In&& in_flow) const
  {
    using stapl::get;
    auto i = get<0>(coord);
    auto m = get<0>(skeleton_size);

    using stencil_utils::get_stencil_filter;

    auto  left_filter = get_stencil_filter<tags::direction<-1>>(m_filter);
    auto right_filter = get_stencil_filter<tags::direction<+1>>(m_filter);

    auto&& center_flow = get<        center_port_index::value>(in_flow);
    auto&&   left_flow = get< left_boundary_port_index::value>(in_flow);
    auto&&  right_flow = get<right_boundary_port_index::value>(in_flow);

    if (m == 1) {
      visitor(
        m_op,
        center_flow.consume_from(make_tuple(i)),
          left_flow.consume_from(make_tuple(0)),
         right_flow.consume_from(make_tuple(0)));
    }
    else {

      switch(find_position(i, 0, m-1))
      {
        case position::first:
          visitor(
            m_op,
            center_flow.consume_from(make_tuple(i)),
              left_flow.consume_from(make_tuple(0)),
            center_flow.consume_from(make_tuple(i + 1), right_filter));
          break;
        case position::last:
          visitor(
            m_op,
            center_flow.consume_from(make_tuple(i)),
            center_flow.consume_from(make_tuple(i - 1), left_filter),
             right_flow.consume_from(make_tuple(0)));
          break;
        case position::middle:
        default:
          visitor(
            m_op,
            center_flow.consume_from(make_tuple(i)),
            center_flow.consume_from(make_tuple(i - 1), left_filter),
            center_flow.consume_from(make_tuple(i + 1), right_filter));
          break;
      }
    }
  }

public:
  ///////////////////////////////////////////////////////////////////
  /// @brief Determines how many of the instances of this parametric
  /// dependency will be consuming from a producer with a given
  /// coordinate. This is a reverse query as compared to case_of.
  ///
  /// @param  skeleton_size  the size of this skeleton
  /// @param  producer_coord the coordinate of the producer element
  ///                        which is providing data to this
  ///                        parametric dependency
  /// @tparam FlowIndex      the flow index on which this request
  ///                        is sent
  ///////////////////////////////////////////////////////////////////
  template <typename Size, typename Coord, typename FlowIndex>
  std::size_t consumer_count(Size const& skeleton_size,
                             Coord const& producer_coord,
                             FlowIndex const& /*flow_index*/) const
  {
    auto i = tuple_ops::front(producer_coord);
    auto m = tuple_ops::front(skeleton_size);
    return isPeriodic ? 3 : (m == 1 ? 1 : (i == 0 or i == (m-1) ? 2 : 3));
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



/////////////////////////////////////////////////////////////////////
/// @brief In a 2D 5 point stencil, parametric dependencies are
/// defined in  such a way that each node depends on every
/// node in its surroundings (top, left, down, right) including itself.
///
/// @tparam Op         the operation to be applied at each point
///                    on the points received.
/// @tparam F          the filter type function
/// @tparam isPeriodic determines whether dependency between points
///                    is periodic or not for boundary points.
/// @tparam dimensions specifies the number of input
///                    dimension(1D, 2D, ... , nD)
/// @ingroup skeletonsParamDeps
///
/////////////////////////////////////////////////////////////////////
template <typename Op, typename F, bool isPeriodic>
class stencil_pd<Op, F, isPeriodic, 2, 5>
{
  Op                 m_op;
  F                  m_filter;

  using center_port_index         = std::integral_constant<int, 0>;
  using up_boundary_port_index    = std::integral_constant<
                                      int, (isPeriodic ? 0 : 1)>;
  using left_boundary_port_index  = std::integral_constant<
                                      int, (isPeriodic ? 0 : 2)>;
  using down_boundary_port_index  = std::integral_constant<
                                      int, (isPeriodic ? 0 : 3)>;
  using right_boundary_port_index = std::integral_constant<
                                      int, (isPeriodic ? 0 : 4)>;

public:
  using in_port_size = std::integral_constant<int, (isPeriodic ? 1 : 5)>;

  template <typename In>
  using output_type = param_deps_output_type<Op, In, 5, isPeriodic>;

  stencil_pd(Op const& op, F const& f)
    : m_op(op),
      m_filter(f)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Dispatches the case_of request based on the @c isPeriodic
  /// boolean variable.
  //////////////////////////////////////////////////////////////////////
  template <typename... Args>
  void case_of(Args&&... args) const
  {

    apply_case_of(std::integral_constant<bool, isPeriodic>(),
                  std::forward<Args>(args)...);
  }

private:
  //////////////////////////////////////////////////////////////////////
  /// @brief This specialization of case_of is used for the cases where
  /// the stencil computation is periodic.
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
  void apply_case_of(std::true_type,
                     Coord&& skeleton_size, Coord&& coord,
                     Visitor&& visitor, In&& in_flow) const
  {
    using stapl::get;
    using stencil_utils::get_stencil_filter;

    auto index = get<0>(coord);
    auto dims  = get<0>(skeleton_size);

    std::size_t i = get<0>(index);
    std::size_t j = get<1>(index);

    std::size_t m = get<0>(dims);
    std::size_t n = get<1>(dims);

    auto&& up     = make_tuple((i + m -1) % m, j);
    auto&& left   = make_tuple(i, (j + n -1) % n);
    auto&& center = make_tuple(i,j);
    auto&& down   = make_tuple((i + 1) % m, j);
    auto&& right  = make_tuple(i, (j + 1) % n);

    using    up_dir = tags::direction<-1,  0>;
    using  left_dir = tags::direction< 0, -1>;
    using  down_dir = tags::direction<+1,  0>;
    using right_dir = tags::direction< 0, +1>;
    auto&& center_flow = get<center_port_index::value>(in_flow);
    visitor(
      m_op,
      center_flow.consume_from(make_tuple(center)),
      center_flow.consume_from(make_tuple(up),
                               get_stencil_filter<   up_dir>(m_filter)),
      center_flow.consume_from(make_tuple(left),
                               get_stencil_filter< left_dir>(m_filter)),
      center_flow.consume_from(make_tuple(down),
                               get_stencil_filter< down_dir>(m_filter)),
      center_flow.consume_from(make_tuple(right),
                               get_stencil_filter<right_dir>(m_filter)));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief This specialization of case_of is used for the cases where
  /// the stencil computation is non-periodic.
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
  void apply_case_of(std::false_type,
                     Coord&& skeleton_size, Coord&& coord,
                     Visitor&& visitor, In&& in_flow) const
  {
    auto index = std::get<0>(coord);
    auto dims  = std::get<0>(skeleton_size);

    auto i = std::get<0>(index);
    auto j = std::get<1>(index);

    auto m = std::get<0>(dims);
    auto n = std::get<1>(dims);

    auto&& up     = make_tuple(i - 1, j);
    auto&& left   = make_tuple(i, j - 1);
    auto&& center = make_tuple(i,j);
    auto&& down   = make_tuple(i + 1, j);
    auto&& right  = make_tuple(i, j + 1);

    using stapl::get;
    using stencil_utils::get_stencil_filter;
    auto&& center_flow = get<        center_port_index::value>(in_flow);
    auto&&     up_flow = get<   up_boundary_port_index::value>(in_flow);
    auto&&   down_flow = get< down_boundary_port_index::value>(in_flow);
    auto&&   left_flow = get< left_boundary_port_index::value>(in_flow);
    auto&&  right_flow = get<right_boundary_port_index::value>(in_flow);
    if (m == 1 and n == 1) {

      visitor(
        m_op,
        center_flow.consume_from(make_tuple(center)),
            up_flow.consume_from(make_tuple(center)),
          left_flow.consume_from(make_tuple(center)),
          down_flow.consume_from(make_tuple(center)),
         right_flow.consume_from(make_tuple(center)));
    }
    else
    {
      auto cur_pos0 = find_position(i, 0, m-1);
      auto cur_pos1 = find_position(j, 0, n-1);

      auto    up_filter =
        get_stencil_filter<tags::direction<-1,  0>>(m_filter);
      auto  left_filter =
        get_stencil_filter<tags::direction< 0, -1>>(m_filter);
      auto  down_filter =
        get_stencil_filter<tags::direction<+1,  0>>(m_filter);
      auto right_filter =
        get_stencil_filter<tags::direction< 0, +1>>(m_filter);

      switch(condition_value( cur_pos0, cur_pos1))
      {
        case condition_value(position::first, position::first) :
        {
          visitor(
            m_op,
            center_flow.consume_from(make_tuple(center)),
                up_flow.consume_from(make_tuple(make_tuple(0, j))),
              left_flow.consume_from(make_tuple(make_tuple(i, 0))),
            center_flow.consume_from(make_tuple(down), down_filter),
            center_flow.consume_from(make_tuple(right), right_filter));
            break;
        }
        case condition_value( position::middle, position::first) :
        {
          visitor(
            m_op,
            center_flow.consume_from(make_tuple(center)),
            center_flow.consume_from(make_tuple(up), up_filter),
              left_flow.consume_from(make_tuple(make_tuple(i, 0))),
            center_flow.consume_from(make_tuple(down), down_filter),
            center_flow.consume_from(make_tuple(right), right_filter));
            break;
        }
        case condition_value(position::last, position::first) :
        {
          visitor(
            m_op,
            center_flow.consume_from(make_tuple(center)),
            center_flow.consume_from(make_tuple(up), up_filter),
              left_flow.consume_from(make_tuple(make_tuple(i, 0))),
              down_flow.consume_from(make_tuple(make_tuple(0, j))),
            center_flow.consume_from(make_tuple(right), right_filter));
            break;
        }
        case condition_value(position::last, position::middle) :
        {
         visitor(
            m_op,
            center_flow.consume_from(make_tuple(center)),
            center_flow.consume_from(make_tuple(up), up_filter),
            center_flow.consume_from(make_tuple(left), left_filter),
              down_flow.consume_from(make_tuple(make_tuple(0, j))),
            center_flow.consume_from(make_tuple(right), right_filter));
          break;
        }
        case condition_value(position::last, position::last) :
        {
          visitor(
            m_op,
            center_flow.consume_from(make_tuple(center)),
            center_flow.consume_from(make_tuple(up), up_filter),
            center_flow.consume_from(make_tuple(left), left_filter),
              down_flow.consume_from(make_tuple(make_tuple(0, j))),
             right_flow.consume_from(make_tuple(make_tuple(i, 0))));
          break;
        }
        case condition_value(position::middle, position::last) :
        {
          visitor(
            m_op,
            center_flow.consume_from(make_tuple(center)),
            center_flow.consume_from(make_tuple(up), up_filter),
            center_flow.consume_from(make_tuple(left), left_filter),
            center_flow.consume_from(make_tuple(down), down_filter),
             right_flow.consume_from(make_tuple(make_tuple(i, 0))));
          break;
        }
        case condition_value( position::first, position::last):
        {
          visitor(
            m_op,
            center_flow.consume_from(make_tuple(center)),
                up_flow.consume_from(make_tuple(make_tuple(0, j))),
            center_flow.consume_from(make_tuple(left), left_filter),
            center_flow.consume_from(make_tuple(down), down_filter),
             right_flow.consume_from(make_tuple(make_tuple(i, 0))));
          break;
        }
        case condition_value( position::first, position::middle) :
        {
          visitor(
            m_op,
            center_flow.consume_from(make_tuple(center)),
                up_flow.consume_from(make_tuple(make_tuple(0, j))),
            center_flow.consume_from(make_tuple(left), left_filter),
            center_flow.consume_from(make_tuple(down), down_filter),
            center_flow.consume_from(make_tuple(right),right_filter));
          break;
        }
        case condition_value( position::middle, position::middle) :
        default:
        {
          visitor(
            m_op,
            center_flow.consume_from(make_tuple(center)),
            center_flow.consume_from(make_tuple(up), up_filter),
            center_flow.consume_from(make_tuple(left), left_filter),
            center_flow.consume_from(make_tuple(down), down_filter),
            center_flow.consume_from(make_tuple(right), right_filter));
          break;
        }
      }
    }
  }
public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Determines how many of the instances of this parametric
  ///        dependency will be consuming from a producer with a given
  ///        coordinate.This is a reverse query as compared to case_of.
  ///
  /// @param  skeleton_size  the size of this skeleton
  /// @param  producer_coord the coordinate of the producer element
  ///                        which is providing data to this parametric
  ///                        dependency
  /// @tparam FlowIndex      the flow index on which this request is
  ///                        sent
  //////////////////////////////////////////////////////////////////////
  template <typename Size, typename Coord, typename FlowIndex>
  std::size_t consumer_count(Size const& skeleton_size,
                             Coord const& producer_coord,
                             FlowIndex const& /*flow_index*/) const
  {
    auto index = tuple_ops::front(producer_coord);
    auto size  = tuple_ops::front(skeleton_size);

    auto i = std::get<0>(index);
    auto j = std::get<1>(index);

    auto m = std::get<0>(size);
    auto n = std::get<1>(size);

    std::size_t num_succs;

    if (isPeriodic) {
      num_succs = 5;
    }
    else if ( m == 1 and n == 1) {
      num_succs = 1;
    }
    else if ( (i == 0 or i == m-1) and (j == 0 or j == n-1)) {
      num_succs = 3;
    }
    else if ( (i == 0 or i == m-1 or j == 0 or j == n-1)) {
      num_succs = 4;
    }
    else { // middle point
      num_succs = 5;
    }

    return num_succs;
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
/// @brief Creates an nD stencil parametric dependency.
///
/// @tparam Op         the operation to be applied at each point
///                    on the points received.
/// @tparam F          the filter type function
/// @tparam isPeriodic determines whether dependency between points
///                    is periodic or not for boundary points.
/// @tparam dimensions specifies the number of input
///                    dimension(1D, 2D, ... , nD)
//////////////////////////////////////////////////////////////////////
template <bool isPeriodic = true,
          int dimensions = 1,
          int numPoints = 3,
          typename Op,
          typename F    = skeletons::no_filter>
skeletons_impl::stencil_pd<Op, F, isPeriodic, dimensions, numPoints>
stencil_pd(Op const& op, F const& f = F())
{
  return skeletons_impl::stencil_pd<
                          Op, F, isPeriodic, dimensions, numPoints>(op, f);
}

} // namespace skeletons
} // namespace stapl

#endif //STAPL_SKELETONS_PARAM_DEPS_STENCIL_PD_HPP
