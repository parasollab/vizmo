/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.
// All rights reserved.
// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef TEST_SKELETONS_SAMPLE_FLOWS_HPP
#define TEST_SKELETONS_SAMPLE_FLOWS_HPP

#include <utility>
#include <type_traits>
#include <stapl/skeletons/flows/flow_helpers.hpp>

namespace stapl {
namespace skeletons {
namespace flows {

//////////////////////////////////////////////////////////////////////
/// @brief AUTOMATICALLY GENERATED DOCUMENTATION
///
/// @dot
///  digraph sample_flows {
///    rankdir = LR;
///    node [shape=record];
///    in [label="Inflow", color="white"];
///    out [label="Outflow", color="white"];
///    subgraph cluster_sample_flows {
///    a1[label=<P<sub>1</sub>>]
///    a0[label=<P<sub>0</sub>>]
///    a2[label=<P<sub>2</sub>>]
///    label=<Compose<sub>[sample_flows]</sub>>;
///    }
///    a2 -> out;
///    a0 -> a1;
///    in -> a1[label="get<0>(in)"];
///    in -> a0[label="get<0>(in)"];
///    a1 -> a2;
///  }
/// @enddot
//////////////////////////////////////////////////////////////////////
struct sample_flows
{
  template <typename Compose>
  struct port_types
  {
  private:
    using skeletons_t = typename Compose::skeletons_type;

    Compose const& m_compose;

  public:
    using in_port_size = std::integral_constant<int, 2>;
    using in_port_type = flows::result_of::concat<
      std::tuple<typename std::tuple_element <
        0,
        typename std::tuple_element<0, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        1,
        typename std::tuple_element<1, skeletons_t>::type::in_port_type>::type
      >
    >;

    template <typename In>
    struct inner_ports_types
    {
    public:
      using in_flow_0_type = std::tuple<typename std::tuple_element <
        0,
        In>::type
      >;
      using out_port_0_type = typename
        std::tuple_element<0, skeletons_t>::type::
          template out_port_type<in_flow_0_type>::type;
      using in_flow_1_type = flows::result_of::concat<
        out_port_0_type,
        std::tuple<typename std::tuple_element <
          0,
          In>::type
        >
      >;
      using out_port_1_type = typename
        std::tuple_element<1, skeletons_t>::type::
          template out_port_type<in_flow_1_type>::type;
      using in_flow_2_type = out_port_1_type;
      using out_port_2_type = typename
        std::tuple_element<2, skeletons_t>::type::
          template out_port_type<in_flow_2_type>::type;

    };

    template <typename In>
    struct out_port_type
    {
    public:
      using type = typename inner_ports_types<In>::out_port_2_type;
    };

    port_types(Compose const& compose)
      : m_compose(compose)
    { }

    template <typename In>
    flows::result_of::concat<
      typename inner_ports_types<In>::out_port_0_type,
      std::tuple<typename std::tuple_element <
        0,
        In>::type
      >
    >
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 1>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_0_type , 0
        >(lid_offset),
        std::make_tuple(
          std::get<0>(in))
      );
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      0,
      typename std::tuple_element<2, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 1>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(m_compose.template get_in_port<2>(lid_offset)));
    }

    template <typename In>
    std::tuple<typename std::tuple_element <
      0,
      In>::type
    >
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 0>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(in));
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      0,
      typename std::tuple_element<1, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 0>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(m_compose.template get_in_port<1>(lid_offset)));
    }

    template <typename In>
    typename inner_ports_types<In>::out_port_1_type
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 2>,
            std::integral_constant<bool, true> /*!is_last*/)
    {
      return m_compose.template get_out_port<
        typename inner_ports_types<In>::in_flow_1_type , 1
      >(lid_offset);
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      0,
      Out>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 2>,
             std::integral_constant<bool, true> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(out));
    }

    typename port_types<Compose>::in_port_type
    in_port(std::size_t lid_offset)
    {
      return concat(
        std::make_tuple(
          std::get<0>(m_compose.template get_in_port<0>(lid_offset))),
        std::make_tuple(
          std::get<1>(m_compose.template get_in_port<1>(lid_offset)))
      );
    }

    template <typename In>
    typename out_port_type<In>::type
    out_port(std::size_t lid_offset)
    {
      return m_compose.template get_out_port<
        typename inner_ports_types<In>::in_flow_2_type , 2
      >(lid_offset);
    }

  };
};

} // namespace flows
} // namespace skeletons
} // namespace stapl

#endif // TEST_SKELETONS_SAMPLE_FLOWS_HPP
