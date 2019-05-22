/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.
// All rights reserved.
// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_BENCHMARKS_NAS_FLOWS_NAS_IS_HPP
#define STAPL_BENCHMARKS_NAS_FLOWS_NAS_IS_HPP

#include <utility>
#include <stapl/skeletons/flows/flow_helpers.hpp>

namespace flows {

using namespace stapl::skeletons::flows;
using std::tuple_element;

//////////////////////////////////////////////////////////////////////
/// @brief  IS (Integer Sort) is one of the benchmarks in the NAS
/// benchmark and the below flows struct is specific to IS.
///
/// The goal in IS is to sort an input key sequence and to
/// store it in the output. For that to happen, the benchmark
/// first determines how many elements will be in each bucket per
/// partition. Then it computes the number of elements in each bucket
/// on all partitions (@c allreduce). After that it determines a
/// redistribution strategy in which each partition will get roughly
/// the same number of elements. Then an @c alltoallv happens in the
/// next stage and each partition will get the chunk of input sequence
/// for which it is responsible for. Finally, using the information
/// obtained in 2nd and 4th steps, the final sorting is done by simply
/// writing to the output in the right offset for each partition. The
/// following flow shows the flow of data in the algorithm as described
/// above.
///
///
/// @dot
///  digraph nas_is {
///    rankdir = LR;
///    node [shape=record];
///    in [label="Inflow", color="white"];
///    out [label="Outflow", color="white"];
///    subgraph cluster_nas_is {
///    a1[label=<P<sub>1</sub>>]
///    a0[label=<P<sub>0</sub>>]
///    a3[label=<P<sub>3</sub>>]
///    a2[label=<P<sub>2</sub>>]
///    a5[label=<P<sub>5</sub>>]
///    a4[label=<P<sub>4</sub>>]
///    label=<Compose<sub>[nas_is]</sub>>;
///    }
///    a5 -> out;
///    a0 -> a1;
///    in -> a0[label="get<0>(in)"];
///    a2 -> a3;
///    a0 -> a3;
///    in -> a3[label="get<0>(in)"];
///    a1 -> a2;
///    a4 -> a5;
///    a2 -> a5;
///    in -> a5[label="get<1>(in)"];
///    a3 -> a4;
///  }
/// @enddot
//////////////////////////////////////////////////////////////////////
struct nas_is
{
  template <typename Compose>
  struct port_types
  {
  private:
    using skeletons_t = typename Compose::skeletons_type;

    Compose const& m_compose;

  public:
    using in_port_size = std::integral_constant<int, 3>;
    using in_port_type = flows::result_of::concat<
      std::tuple<typename std::tuple_element <
        0,
        typename std::tuple_element<0, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        2,
        typename std::tuple_element<3, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        2,
        typename std::tuple_element<5, skeletons_t>::type::in_port_type>::type
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
      using in_flow_1_type = out_port_0_type;
      using out_port_1_type = typename
        std::tuple_element<1, skeletons_t>::type::
          template out_port_type<in_flow_1_type>::type;
      using in_flow_2_type = out_port_1_type;
      using out_port_2_type = typename
        std::tuple_element<2, skeletons_t>::type::
          template out_port_type<in_flow_2_type>::type;
      using in_flow_3_type = flows::result_of::concat<
        out_port_2_type,
        out_port_0_type,
        std::tuple<typename std::tuple_element <
          0,
          In>::type
        >
      >;
      using out_port_3_type = typename
        std::tuple_element<3, skeletons_t>::type::
          template out_port_type<in_flow_3_type>::type;
      using in_flow_4_type = out_port_3_type;
      using out_port_4_type = typename
        std::tuple_element<4, skeletons_t>::type::
          template out_port_type<in_flow_4_type>::type;
      using in_flow_5_type = flows::result_of::concat<
        out_port_4_type,
        out_port_2_type,
        std::tuple<typename std::tuple_element <
          1,
          In>::type
        >
      >;
      using out_port_5_type = typename
        std::tuple_element<5, skeletons_t>::type::
          template out_port_type<in_flow_5_type>::type;

    };

    template <typename In>
    struct out_port_type
    {
    public:
      using type = typename inner_ports_types<In>::out_port_5_type;
    };

    port_types(Compose const& compose)
      : m_compose(compose)
    { }

    template <typename In>
    typename inner_ports_types<In>::out_port_0_type
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 1>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return m_compose.template get_out_port<
        typename inner_ports_types<In>::in_flow_0_type , 0
      >(lid_offset);
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
    flows::result_of::concat<
      std::tuple<typename std::tuple_element <
        0,
        typename std::tuple_element<1, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        1,
        typename std::tuple_element<3, skeletons_t>::type::in_port_type>::type
      >
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 0>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        std::make_tuple(
          std::get<0>(m_compose.template get_in_port<1>(lid_offset))),
        std::make_tuple(
          std::get<1>(m_compose.template get_in_port<3>(lid_offset)))
      );
    }

    template <typename In>
    flows::result_of::concat<
      typename inner_ports_types<In>::out_port_2_type,
      typename inner_ports_types<In>::out_port_0_type,
      std::tuple<typename std::tuple_element <
        0,
        In>::type
      >
    >
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 3>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_2_type , 2
        >(lid_offset),
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
      typename std::tuple_element<4, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 3>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(m_compose.template get_in_port<4>(lid_offset)));
    }

    template <typename In>
    typename inner_ports_types<In>::out_port_1_type
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 2>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return m_compose.template get_out_port<
        typename inner_ports_types<In>::in_flow_1_type , 1
      >(lid_offset);
    }

    template <typename Out>
    flows::result_of::concat<
      std::tuple<typename std::tuple_element <
        0,
        typename std::tuple_element<3, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        1,
        typename std::tuple_element<5, skeletons_t>::type::in_port_type>::type
      >
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 2>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        std::make_tuple(
          std::get<0>(m_compose.template get_in_port<3>(lid_offset))),
        std::make_tuple(
          std::get<1>(m_compose.template get_in_port<5>(lid_offset)))
      );
    }

    template <typename In>
    flows::result_of::concat<
      typename inner_ports_types<In>::out_port_4_type,
      typename inner_ports_types<In>::out_port_2_type,
      std::tuple<typename std::tuple_element <
        1,
        In>::type
      >
    >
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 5>,
            std::integral_constant<bool, true> /*!is_last*/)
    {
      return concat(
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_4_type , 4
        >(lid_offset),
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_2_type , 2
        >(lid_offset),
        std::make_tuple(
          std::get<1>(in))
      );
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      0,
      Out>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 5>,
             std::integral_constant<bool, true> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(out));
    }

    template <typename In>
    typename inner_ports_types<In>::out_port_3_type
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 4>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return m_compose.template get_out_port<
        typename inner_ports_types<In>::in_flow_3_type , 3
      >(lid_offset);
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      0,
      typename std::tuple_element<5, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 4>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(m_compose.template get_in_port<5>(lid_offset)));
    }

    typename port_types<Compose>::in_port_type
    in_port(std::size_t lid_offset)
    {
      return concat(
        std::make_tuple(
          std::get<0>(m_compose.template get_in_port<0>(lid_offset))),
        std::make_tuple(
          std::get<2>(m_compose.template get_in_port<3>(lid_offset))),
        std::make_tuple(
          std::get<2>(m_compose.template get_in_port<5>(lid_offset)))
      );
    }

    template <typename In>
    typename out_port_type<In>::type
    out_port(std::size_t lid_offset)
    {
      return m_compose.template get_out_port<
        typename inner_ports_types<In>::in_flow_5_type , 5
      >(lid_offset);
    }

  };
};

} // namespace flows


#endif // STAPL_BENCHMARKS_NAS_FLOWS_NAS_IS_HPP
