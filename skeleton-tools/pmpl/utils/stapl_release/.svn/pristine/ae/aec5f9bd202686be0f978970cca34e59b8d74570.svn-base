/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_PARAM_DEPS_BUTTERFLY_PD_HPP
#define STAPL_SKELETONS_PARAM_DEPS_BUTTERFLY_PD_HPP

#include <type_traits>
#include <stapl/utility/tuple/tuple.hpp>
#include <stapl/utility/tuple/front.hpp>
#include <stapl/skeletons/operators/elem_helpers.hpp>
#include <stapl/skeletons/param_deps/utility.hpp>

namespace stapl {
namespace skeletons {
namespace skeletons_impl {

//////////////////////////////////////////////////////////////////////
/// @brief A butterfly parametric dependency defines the dependencies for
/// each element in a big @c butterfly skeleton. Some algorithms that
/// use @c butterfly_pd need to know the location of the element in the
/// big butterfly skeleton. An example is DIT FFT algorithm that
/// changes the computation based on the location of the parametric
/// dependency.
/// For the same reason @c is_position_aware should be specified by
/// user in such cases. In addition, in some cases one would like to
/// read partially from the inputs on a @c butterfly. That's when the
/// the @c F (filter) can be used.
///
/// @tparam Op                the workfunction to be applied on each
///                           element in the big @c butterfly skeleton
/// @tparam F                 the filter to be applied on the input
///                           edges
/// @tparam is_position_aware if the computation in the workfunction
///                           is dependent on the position of the
///                           element
///
/// @see butterfly
///
/// @ingroup skeletonsParamDepsInternal
//////////////////////////////////////////////////////////////////////
template <typename Op, typename F, bool is_position_aware,
          bool is_reversed = false>
class butterfly_pd
{
  Op m_op;
  F  m_filter;
public:
  using in_port_size = std::integral_constant<int, 1>;

  template <typename In>
  using output_type = param_deps_output_type<Op, In, 2, true>;

  butterfly_pd(Op const& op, F const& f)
   : m_op(op), m_filter(f)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief If coord is <i, h, ...> it wraps the @c Op with the
  /// following inputs and sends it to the visitor along with the @c m_op
  /// @li in<0>[i]
  /// @li in<0>[butterflied_index(i)]
  ///
  /// @param skeleton_size <n, m, p, ...> where each element is
  ///                     potentially multi-dimensional.
  /// @param coord        <i, j, k, ...> where i < n, j < m, k < p
  /// @param visitor      the information about Op and input is passed
  ///                     so that later this information can be converted
  ///                     to a node in the dependence graph
  /// @param in_flow      a tuple of input flows to consume from
  ///
  /// @see butterflied_index
  //////////////////////////////////////////////////////////////////////
  template <typename Coord, typename Visitor, typename In>
  void case_of(Coord const& skeleton_size, Coord const& coord,
               Visitor& visitor, In&& in_flow) const
  {
    using stapl::get;
    std::size_t cur_index = tuple_ops::front(coord);
    std::size_t bflied_index = butterflied_index(skeleton_size, coord);
    std::size_t level = get<1>(coord);

    if (get<0>(skeleton_size) == 1) {
      visitor(typename stapl::identity_selector<Op>::type(),
              get<0>(in_flow).consume_from(make_tuple(0, level - 1)));
    }
    else {
      std::size_t bfly_size = butterfly_size(
                                skeleton_size, coord,
                                std::integral_constant<bool, is_reversed>());

      //for now let's choose the normal one, but later on we have to choose
      //the correct one based on the coordinates of the task
      apply_set_position(
        std::integral_constant<bool, is_position_aware>(),
        m_op, bfly_size, cur_index, bflied_index, level);

      apply_set_position(
        std::integral_constant<
          bool,
          is_position_aware and
          (not std::is_same<F, skeletons::no_filter>::value)>(),
        m_filter, bfly_size, cur_index, bflied_index, level);

      visitor(m_op,
              get<0>(in_flow).consume_from(
                make_tuple(cur_index, level - 1), m_filter),
              get<0>(in_flow).consume_from(
                make_tuple(bflied_index, level - 1), m_filter));
    }
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
    return tuple_ops::front(skeleton_size) == 1 ? 1 : 2;
  }

private:
  template <typename Oper>
  void apply_set_position(
    std::true_type,
    Oper const& op, std::size_t bfly_size,
                    std::size_t cur_index,
                    std::size_t bflied_index,
                    std::size_t level) const
  {
    const_cast<Oper&>(op).set_position(
      bfly_size, cur_index, bflied_index, level
    );
  }

  template <typename Oper>
  void apply_set_position(
    std::false_type,
    Oper const& op, std::size_t bfly_size,
                    std::size_t cur_index,
                    std::size_t bflied_index,
                    std::size_t level) const
  { }

  template <typename Coord>
  std::size_t
  butterfly_size(Coord const& skeleton_size, Coord const& coord,
                 std::false_type) const
  {
    return pow(2, stapl::get<1>(skeleton_size) - stapl::get<1>(coord) - 1);
  }

  template <typename Coord>
  std::size_t
  butterfly_size(Coord const& skeleton_size, Coord const& coord,
                 std::true_type) const
  {
    return pow(2, stapl::get<1>(coord));
  }

  template <typename Coord>
  std::size_t
  butterflied_index(Coord const& skeleton_size, Coord const& coord) const
  {
    std::size_t cur_index = stapl::get<0>(coord);
    std::size_t bfly_size = butterfly_size(
                              skeleton_size, coord,
                              std::integral_constant<bool, is_reversed>());
    const int direction = cur_index % (bfly_size * 2) >= bfly_size ? -1 : 1;
    return (cur_index + direction * bfly_size);
  }

public:
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
/// @brief Creates a butterfly parametric dependency given a @c op and
/// the direction of the butterfly
///
/// @ingroup skeletonsParamDepsExchange
//////////////////////////////////////////////////////////////////////
template <bool pos_aware, bool is_reversed = false,
          typename Op,
          typename Filter = skeletons::no_filter>
skeletons_impl::butterfly_pd<Op, Filter, pos_aware, is_reversed>
butterfly_pd(Op const& op, Filter const& filter = Filter())
{
  return skeletons_impl::butterfly_pd<Op, Filter, pos_aware, is_reversed>(
           op, filter);
}

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_PARAM_DEPS_BUTTERFLY_PD_HPP
