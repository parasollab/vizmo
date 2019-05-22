/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_ELEM_FLOW_HPP
#define STAPL_SKELETONS_ELEM_FLOW_HPP

#include <boost/mpl/has_xxx.hpp>
#include <stapl/skeletons/utility/filters.hpp>
#include <stapl/skeletons/flows/flow_helpers.hpp>
#include <stapl/skeletons/flows/indexed_flow.hpp>
#include <stapl/skeletons/flows/cloned_flow.hpp>

namespace stapl {
namespace skeletons {

BOOST_MPL_HAS_XXX_TRAIT_DEF(consumer_count_filter_type)

namespace flows {
namespace flows_impl {

template <typename S,
          bool B = has_consumer_count_filter_type<S>::value>
struct find_filter_type
{
  using type = skeletons::filters::filter<1>;
};

template <typename S>
struct find_filter_type<S, true>
{
  using type = typename S::consumer_count_filter_type;
};

} // namespace flows_impl

namespace elem_f {

////////////////////////////////////////////////////////////////////////
/// @brief An elementary input port which is the default input port
/// of elementary skeletons is a single dimension input port which allows
/// simply redirects the consumer_count requests to its underlying
/// parametric dependency.
///
/// @see elem.hpp
///
/// @ingroup skeletonsFlowsElem
////////////////////////////////////////////////////////////////////////
template <typename S>
class elem_in_port
{
  using nested_p_t = typename S::nested_p_type;
  using filter_t   = typename flows_impl::find_filter_type<nested_p_t>::type;
  S const          m_skeleton;
public:
  elem_in_port(S const& skeleton, std::size_t)
    : m_skeleton(skeleton)
  { }

  ////////////////////////////////////////////////////////////////////////
  /// @brief The number of consumers in a default elementary flow is
  /// determined by its enclosed parametric dependency. Each parametric
  /// dependency has a @c consumer_count defined.
  ///
  ///@param producer_coord is the position of the producer in its
  ///                      coordinate space
  ///
  ///@return number of consumers returned by the parametric dependency
  ///        underneath
  ////////////////////////////////////////////////////////////////////////
  template <int N, typename Coord>
  std::size_t consumer_count(Coord const& producer_coord) const
  {
    //remember the coord is from the requester, and the dimension is from
    //the skeleton wrapped by this concurrent_p
    return m_skeleton.consumer_count(
             stapl::make_tuple(m_skeleton.dimensions()),
             filter_t()(producer_coord),
             std::integral_constant<int, N>()
           );
  }
};


////////////////////////////////////////////////////////////////////////
/// @brief a doall elementary flow considers all elements inside of
/// it as @a connectable entities in the dependence graph, whether each
/// node is connected or not in the dependence graph, depends on the
/// underlying skeleton.
///
/// This flow description is used in the cases that no dependencies
/// exist between the skeletons in the given @c elem.
/// @ingroup skeletonsFlowsElem
////////////////////////////////////////////////////////////////////////
struct doall
{
  template <typename S>
  struct port_types
  {
    using in_port_size = typename S::in_port_size;
    using in_port_type =
      typename flows::result_of::cloned_flow<
        elem_f::elem_in_port<S>, in_port_size::value>::type;

    template <typename In>
    struct out_port_type
    {
      using type = tuple<
                     flows::indexed_flow<
                       typename S::nested_p_type::
                         template output_type<In>::type,
                       typename S::span_type>>;
    };

  private:
    S const& m_elem;

  public:
    port_types(S const& s)
      : m_elem(s)
    { }

    template <typename In>
    In
    in_flow(In const& in, std::size_t lid_offset)
    {
      return in;
    }

    template <typename Out>
    Out
    out_flow(Out const& out, std::size_t lid_offset)
    {
      return out;
    }

    //////////////////////////////////////////////////////////////////////
    /// @brief doall<sub>in-flow</sub> ->
    ///        cloned-flow[elem_in_port<sub>S</sub>,
    ///                    S<sub>flow-size</sub>]
    //////////////////////////////////////////////////////////////////////
    typename port_types<S>::in_port_type
    in_port(std::size_t lid_offset)
    {
      return cloned_flow<port_types<S>::in_port_size::value>(
               elem_f::elem_in_port<S>(m_elem, lid_offset));
    }

    //////////////////////////////////////////////////////////////////////
    /// @brief doall<sub>out-flow</sub> ->
    ///        indexed_flow<sub>S</sub>
    //////////////////////////////////////////////////////////////////////
    template <typename In>
    typename port_types<S>::template out_port_type<In>::type
    out_port(std::size_t lid_offset)
    {
      return stapl::make_tuple(
               make_indexed_flow<
                 typename S::template output_type<In>::type
               >(m_elem.span(), lid_offset));
    }
  };
};


////////////////////////////////////////////////////////////////////////
/// @brief a doacross elementary flow considers all elements inside of
/// it as @a connectable entities in the dependence graph, whether each
/// node is connected or not in the dependence graph, depends on the
/// underlying skeleton.
///
/// This flow description is used in the cases that dependencies
/// exist between the skeletons in the given @c elem.
///
/// @ingroup skeletonsFlowsElem
////////////////////////////////////////////////////////////////////////
struct doacross
{
  template <typename S>
  struct port_types
    : public doall::port_types<S>
  {
    using base_t = doall::port_types<S>;

    port_types(S const& s)
      : base_t(s)
    { }

    template <typename In>
    flows::result_of::concat<
      In,
      typename base_t::template out_port_type<In>::type
    >
    in_flow(In const& in, std::size_t lid_offset)
    {
      return flows::concat(
               in,
               base_t::template out_port<In>(lid_offset));
    }

    template <typename Out>
    flows::result_of::concat<
      Out,
      stapl::tuple<
        typename tuple_ops::result_of::back<
          typename std::enable_if<
            !std::is_same<Out, stapl::use_default>::value, base_t
          >::type::in_port_type
        >::type>
    >
    out_flow(Out const& out, std::size_t lid_offset)
    {
      return flows::concat(
               out,
               stapl::make_tuple(tuple_ops::back(this->in_port(lid_offset))));
    }
  };

};

} // namespace elem_f

} // namespace flows
} // namespace stapl
} // namespace skeletons

#endif // STAPL_SKELETONS_ELEM_FLOW_HPP
