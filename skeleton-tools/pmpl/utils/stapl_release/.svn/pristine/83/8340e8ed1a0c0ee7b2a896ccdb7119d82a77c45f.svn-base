/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.
// All rights reserved.
// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_BENCHMARKS_FMM_FLOWS_FMM_HPP
#define STAPL_BENCHMARKS_FMM_FLOWS_FMM_HPP

#include <utility>
#include <stapl/skeletons/flows/flow_helpers.hpp>

namespace flows {

using namespace stapl::skeletons::flows;
using std::tuple_element;

//////////////////////////////////////////////////////////////////////
/// @brief AUTOMATICALLY GENERATED DOCUMENTATION
///
/// @dot
///  digraph fmm {
///    rankdir = LR;
///    node [shape=record];
///    in [label="Inflow", color="white"];
///    out [label="Outflow", color="white"];
///    subgraph cluster_fmm {
///    a20[label=<P<sub>20</sub>>]
///    a1[label=<Calculate Global Bounds>]
///    a0[label=<Generate Bodies>]
///    a3[label=<Domain Decomposition>]
///    a2[label=<Broadcast>]
///    a5[label=<P<sub>5</sub>>]
///    a4[label=<Exchange Partitions>]
///    a7[label=<Build Tree>]
///    a6[label=<Calculate Local Bounds>]
///    a9[label=<Upward Pass>]
///    a8[label=<Get Cell Bounds>]
///    a11[label=<P<sub>11</sub>>]
///    a10[label=<Gather Bounds>]
///    a13[label=<P<sub>13</sub>>]
///    a12[label=<Build LET>]
///    a15[label=<P<sub>15</sub>>]
///    a14[label=<Send LET Bodies>]
///    a17[label=<Dual Tree Traversal>]
///    a16[label=<Send LET Cells>]
///    a19[label=<Downward Pass>]
///    a18[label=<Get LET for each Rank>]
///    label=<Compose<sub>[fmm]</sub>>;
///    }
///    a19 -> a20;
///    a5 -> a20;
///    in -> a20[label="get<1>(in)"];
///    a0 -> a1;
///    in -> a0[label="get<0>(in)"];
///    a0 -> a3;
///    in -> a3[label="get<1>(in)"];
///    a2 -> a3;
///    a1 -> a2;
///    a4 -> a5;
///    a3 -> a4;
///    a5 -> a7;
///    a6 -> a7;
///    a5 -> a6;
///    a7 -> a9;
///    a5 -> a9;
///    a7 -> a8;
///    a6 -> a8;
///    a20 -> out;
///    in -> a11[label="get<1>(in)"];
///    a10 -> a11;
///    a8 -> a10;
///    a12 -> a13;
///    in -> a13[label="get<1>(in)"];
///    a11 -> a12;
///    a9 -> a12;
///    in -> a12[label="get<1>(in)"];
///    a14 -> a15;
///    in -> a15[label="get<1>(in)"];
///    a13 -> a14;
///    a9 -> a17;
///    a15 -> a16;
///    a18 -> a19;
///    a14 -> a18;
///    a16 -> a18;
///    in -> a18[label="get<1>(in)"];
///    a17 -> a18;
///    a5 -> a18;
///  }
/// @enddot
//////////////////////////////////////////////////////////////////////
struct fmm
{
  template <typename Compose>
  struct port_types
  {
  private:
    using skeletons_t = typename Compose::skeletons_type;

    Compose const& m_compose;

  public:
    using in_port_size = std::integral_constant<int, 8>;
    using in_port_type = flows::result_of::concat<
      std::tuple<typename std::tuple_element <
        0,
        typename std::tuple_element<0, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        1,
        typename std::tuple_element<3, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        0,
        typename std::tuple_element<11, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        2,
        typename std::tuple_element<12, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        1,
        typename std::tuple_element<13, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        1,
        typename std::tuple_element<15, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        2,
        typename std::tuple_element<18, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        2,
        typename std::tuple_element<20, skeletons_t>::type::in_port_type>::type
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
        out_port_0_type,
        std::tuple<typename std::tuple_element <
          1,
          In>::type
        >,
        out_port_2_type
      >;
      using out_port_3_type = typename
        std::tuple_element<3, skeletons_t>::type::
          template out_port_type<in_flow_3_type>::type;
      using in_flow_4_type = out_port_3_type;
      using out_port_4_type = typename
        std::tuple_element<4, skeletons_t>::type::
          template out_port_type<in_flow_4_type>::type;
      using in_flow_5_type = out_port_4_type;
      using out_port_5_type = typename
        std::tuple_element<5, skeletons_t>::type::
          template out_port_type<in_flow_5_type>::type;
      using in_flow_6_type = out_port_5_type;
      using out_port_6_type = typename
        std::tuple_element<6, skeletons_t>::type::
          template out_port_type<in_flow_6_type>::type;
      using in_flow_7_type = flows::result_of::concat<
        out_port_5_type,
        out_port_6_type
      >;
      using out_port_7_type = typename
        std::tuple_element<7, skeletons_t>::type::
          template out_port_type<in_flow_7_type>::type;
      using in_flow_8_type = flows::result_of::concat<
        out_port_7_type,
        out_port_6_type
      >;
      using out_port_8_type = typename
        std::tuple_element<8, skeletons_t>::type::
          template out_port_type<in_flow_8_type>::type;
      using in_flow_9_type = flows::result_of::concat<
        out_port_7_type,
        out_port_5_type
      >;
      using out_port_9_type = typename
        std::tuple_element<9, skeletons_t>::type::
          template out_port_type<in_flow_9_type>::type;
      using in_flow_10_type = out_port_8_type;
      using out_port_10_type = typename
        std::tuple_element<10, skeletons_t>::type::
          template out_port_type<in_flow_10_type>::type;
      using in_flow_11_type = flows::result_of::concat<
        std::tuple<typename std::tuple_element <
          1,
          In>::type
        >,
        out_port_10_type
      >;
      using out_port_11_type = typename
        std::tuple_element<11, skeletons_t>::type::
          template out_port_type<in_flow_11_type>::type;
      using in_flow_12_type = flows::result_of::concat<
        out_port_11_type,
        out_port_9_type,
        std::tuple<typename std::tuple_element <
          1,
          In>::type
        >
      >;
      using out_port_12_type = typename
        std::tuple_element<12, skeletons_t>::type::
          template out_port_type<in_flow_12_type>::type;
      using in_flow_13_type = flows::result_of::concat<
        out_port_12_type,
        std::tuple<typename std::tuple_element <
          1,
          In>::type
        >
      >;
      using out_port_13_type = typename
        std::tuple_element<13, skeletons_t>::type::
          template out_port_type<in_flow_13_type>::type;
      using in_flow_14_type = out_port_13_type;
      using out_port_14_type = typename
        std::tuple_element<14, skeletons_t>::type::
          template out_port_type<in_flow_14_type>::type;
      using in_flow_15_type = flows::result_of::concat<
        out_port_14_type,
        std::tuple<typename std::tuple_element <
          1,
          In>::type
        >
      >;
      using out_port_15_type = typename
        std::tuple_element<15, skeletons_t>::type::
          template out_port_type<in_flow_15_type>::type;
      using in_flow_16_type = out_port_15_type;
      using out_port_16_type = typename
        std::tuple_element<16, skeletons_t>::type::
          template out_port_type<in_flow_16_type>::type;
      using in_flow_17_type = out_port_9_type;
      using out_port_17_type = typename
        std::tuple_element<17, skeletons_t>::type::
          template out_port_type<in_flow_17_type>::type;
      using in_flow_18_type = flows::result_of::concat<
        out_port_14_type,
        out_port_16_type,
        std::tuple<typename std::tuple_element <
          1,
          In>::type
        >,
        out_port_17_type,
        out_port_5_type
      >;
      using out_port_18_type = typename
        std::tuple_element<18, skeletons_t>::type::
          template out_port_type<in_flow_18_type>::type;
      using in_flow_19_type = out_port_18_type;
      using out_port_19_type = typename
        std::tuple_element<19, skeletons_t>::type::
          template out_port_type<in_flow_19_type>::type;
      using in_flow_20_type = flows::result_of::concat<
        out_port_19_type,
        out_port_5_type,
        std::tuple<typename std::tuple_element <
          1,
          In>::type
        >
      >;
      using out_port_20_type = typename
        std::tuple_element<20, skeletons_t>::type::
          template out_port_type<in_flow_20_type>::type;

    };

    template <typename In>
    struct out_port_type
    {
    public:
      using type = typename inner_ports_types<In>::out_port_20_type;
    };

    port_types(Compose const& compose)
      : m_compose(compose)
    { }

    template <typename In>
    flows::result_of::concat<
      typename inner_ports_types<In>::out_port_19_type,
      typename inner_ports_types<In>::out_port_5_type,
      std::tuple<typename std::tuple_element <
        1,
        In>::type
      >
    >
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 20>,
            std::integral_constant<bool, true> /*!is_last*/)
    {
      return concat(
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_19_type , 19
        >(lid_offset),
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_5_type , 5
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
             std::integral_constant<int, 20>,
             std::integral_constant<bool, true> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(out));
    }

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
        0,
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
          std::get<0>(m_compose.template get_in_port<3>(lid_offset)))
      );
    }

    template <typename In>
    flows::result_of::concat<
      typename inner_ports_types<In>::out_port_0_type,
      std::tuple<typename std::tuple_element <
        1,
        In>::type
      >,
      typename inner_ports_types<In>::out_port_2_type
    >
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 3>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_0_type , 0
        >(lid_offset),
        std::make_tuple(
          std::get<1>(in)),
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_2_type , 2
        >(lid_offset)
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
    std::tuple<typename std::tuple_element <
      2,
      typename std::tuple_element<3, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 2>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<2>(m_compose.template get_in_port<3>(lid_offset)));
    }

    template <typename In>
    typename inner_ports_types<In>::out_port_4_type
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 5>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return m_compose.template get_out_port<
        typename inner_ports_types<In>::in_flow_4_type , 4
      >(lid_offset);
    }

    template <typename Out>
    flows::result_of::concat<
      std::tuple<typename std::tuple_element <
        0,
        typename std::tuple_element<6, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        0,
        typename std::tuple_element<7, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        1,
        typename std::tuple_element<9, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        4,
        typename std::tuple_element<18, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        1,
        typename std::tuple_element<20, skeletons_t>::type::in_port_type>::type
      >
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 5>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        std::make_tuple(
          std::get<0>(m_compose.template get_in_port<6>(lid_offset))),
        std::make_tuple(
          std::get<0>(m_compose.template get_in_port<7>(lid_offset))),
        std::make_tuple(
          std::get<1>(m_compose.template get_in_port<9>(lid_offset))),
        std::make_tuple(
          std::get<4>(m_compose.template get_in_port<18>(lid_offset))),
        std::make_tuple(
          std::get<1>(m_compose.template get_in_port<20>(lid_offset)))
      );
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

    template <typename In>
    flows::result_of::concat<
      typename inner_ports_types<In>::out_port_5_type,
      typename inner_ports_types<In>::out_port_6_type
    >
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 7>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_5_type , 5
        >(lid_offset),
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_6_type , 6
        >(lid_offset)
      );
    }

    template <typename Out>
    flows::result_of::concat<
      std::tuple<typename std::tuple_element <
        0,
        typename std::tuple_element<8, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        0,
        typename std::tuple_element<9, skeletons_t>::type::in_port_type>::type
      >
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 7>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        std::make_tuple(
          std::get<0>(m_compose.template get_in_port<8>(lid_offset))),
        std::make_tuple(
          std::get<0>(m_compose.template get_in_port<9>(lid_offset)))
      );
    }

    template <typename In>
    typename inner_ports_types<In>::out_port_5_type
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 6>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return m_compose.template get_out_port<
        typename inner_ports_types<In>::in_flow_5_type , 5
      >(lid_offset);
    }

    template <typename Out>
    flows::result_of::concat<
      std::tuple<typename std::tuple_element <
        1,
        typename std::tuple_element<7, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        1,
        typename std::tuple_element<8, skeletons_t>::type::in_port_type>::type
      >
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 6>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        std::make_tuple(
          std::get<1>(m_compose.template get_in_port<7>(lid_offset))),
        std::make_tuple(
          std::get<1>(m_compose.template get_in_port<8>(lid_offset)))
      );
    }

    template <typename In>
    flows::result_of::concat<
      typename inner_ports_types<In>::out_port_7_type,
      typename inner_ports_types<In>::out_port_5_type
    >
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 9>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_7_type , 7
        >(lid_offset),
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_5_type , 5
        >(lid_offset)
      );
    }

    template <typename Out>
    flows::result_of::concat<
      std::tuple<typename std::tuple_element <
        1,
        typename std::tuple_element<12, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        0,
        typename std::tuple_element<17, skeletons_t>::type::in_port_type>::type
      >
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 9>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        std::make_tuple(
          std::get<1>(m_compose.template get_in_port<12>(lid_offset))),
        std::make_tuple(
          std::get<0>(m_compose.template get_in_port<17>(lid_offset)))
      );
    }

    template <typename In>
    flows::result_of::concat<
      typename inner_ports_types<In>::out_port_7_type,
      typename inner_ports_types<In>::out_port_6_type
    >
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 8>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_7_type , 7
        >(lid_offset),
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_6_type , 6
        >(lid_offset)
      );
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      0,
      typename std::tuple_element<10, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 8>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(m_compose.template get_in_port<10>(lid_offset)));
    }

    template <typename In>
    flows::result_of::concat<
      std::tuple<typename std::tuple_element <
        1,
        In>::type
      >,
      typename inner_ports_types<In>::out_port_10_type
    >
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 11>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        std::make_tuple(
          std::get<1>(in)),
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_10_type , 10
        >(lid_offset)
      );
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      0,
      typename std::tuple_element<12, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 11>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(m_compose.template get_in_port<12>(lid_offset)));
    }

    template <typename In>
    typename inner_ports_types<In>::out_port_8_type
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 10>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return m_compose.template get_out_port<
        typename inner_ports_types<In>::in_flow_8_type , 8
      >(lid_offset);
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      1,
      typename std::tuple_element<11, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 10>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<1>(m_compose.template get_in_port<11>(lid_offset)));
    }

    template <typename In>
    flows::result_of::concat<
      typename inner_ports_types<In>::out_port_12_type,
      std::tuple<typename std::tuple_element <
        1,
        In>::type
      >
    >
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 13>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_12_type , 12
        >(lid_offset),
        std::make_tuple(
          std::get<1>(in))
      );
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      0,
      typename std::tuple_element<14, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 13>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(m_compose.template get_in_port<14>(lid_offset)));
    }

    template <typename In>
    flows::result_of::concat<
      typename inner_ports_types<In>::out_port_11_type,
      typename inner_ports_types<In>::out_port_9_type,
      std::tuple<typename std::tuple_element <
        1,
        In>::type
      >
    >
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 12>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_11_type , 11
        >(lid_offset),
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_9_type , 9
        >(lid_offset),
        std::make_tuple(
          std::get<1>(in))
      );
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      0,
      typename std::tuple_element<13, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 12>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(m_compose.template get_in_port<13>(lid_offset)));
    }

    template <typename In>
    flows::result_of::concat<
      typename inner_ports_types<In>::out_port_14_type,
      std::tuple<typename std::tuple_element <
        1,
        In>::type
      >
    >
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 15>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_14_type , 14
        >(lid_offset),
        std::make_tuple(
          std::get<1>(in))
      );
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      0,
      typename std::tuple_element<16, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 15>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(m_compose.template get_in_port<16>(lid_offset)));
    }

    template <typename In>
    typename inner_ports_types<In>::out_port_13_type
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 14>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return m_compose.template get_out_port<
        typename inner_ports_types<In>::in_flow_13_type , 13
      >(lid_offset);
    }

    template <typename Out>
    flows::result_of::concat<
      std::tuple<typename std::tuple_element <
        0,
        typename std::tuple_element<15, skeletons_t>::type::in_port_type>::type
      >,
      std::tuple<typename std::tuple_element <
        0,
        typename std::tuple_element<18, skeletons_t>::type::in_port_type>::type
      >
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 14>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        std::make_tuple(
          std::get<0>(m_compose.template get_in_port<15>(lid_offset))),
        std::make_tuple(
          std::get<0>(m_compose.template get_in_port<18>(lid_offset)))
      );
    }

    template <typename In>
    typename inner_ports_types<In>::out_port_9_type
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 17>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return m_compose.template get_out_port<
        typename inner_ports_types<In>::in_flow_9_type , 9
      >(lid_offset);
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      3,
      typename std::tuple_element<18, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 17>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<3>(m_compose.template get_in_port<18>(lid_offset)));
    }

    template <typename In>
    typename inner_ports_types<In>::out_port_15_type
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 16>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return m_compose.template get_out_port<
        typename inner_ports_types<In>::in_flow_15_type , 15
      >(lid_offset);
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      1,
      typename std::tuple_element<18, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 16>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<1>(m_compose.template get_in_port<18>(lid_offset)));
    }

    template <typename In>
    typename inner_ports_types<In>::out_port_18_type
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 19>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return m_compose.template get_out_port<
        typename inner_ports_types<In>::in_flow_18_type , 18
      >(lid_offset);
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      0,
      typename std::tuple_element<20, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 19>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(m_compose.template get_in_port<20>(lid_offset)));
    }

    template <typename In>
    flows::result_of::concat<
      typename inner_ports_types<In>::out_port_14_type,
      typename inner_ports_types<In>::out_port_16_type,
      std::tuple<typename std::tuple_element <
        1,
        In>::type
      >,
      typename inner_ports_types<In>::out_port_17_type,
      typename inner_ports_types<In>::out_port_5_type
    >
    in_flow(In const& in, std::size_t lid_offset,
            std::integral_constant<int, 18>,
            std::integral_constant<bool, false> /*!is_last*/)
    {
      return concat(
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_14_type , 14
        >(lid_offset),
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_16_type , 16
        >(lid_offset),
        std::make_tuple(
          std::get<1>(in)),
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_17_type , 17
        >(lid_offset),
        m_compose.template get_out_port<
          typename inner_ports_types<In>::in_flow_5_type , 5
        >(lid_offset)
      );
    }

    template <typename Out>
    std::tuple<typename std::tuple_element <
      0,
      typename std::tuple_element<19, skeletons_t>::type::in_port_type>::type
    >
    out_flow(Out const& out, std::size_t lid_offset,
             std::integral_constant<int, 18>,
             std::integral_constant<bool, false> /*!is_last*/)
    {
      return std::make_tuple(
        std::get<0>(m_compose.template get_in_port<19>(lid_offset)));
    }

    typename port_types<Compose>::in_port_type
    in_port(std::size_t lid_offset)
    {
      return concat(
        std::make_tuple(
          std::get<0>(m_compose.template get_in_port<0>(lid_offset))),
        std::make_tuple(
          std::get<1>(m_compose.template get_in_port<3>(lid_offset))),
        std::make_tuple(
          std::get<0>(m_compose.template get_in_port<11>(lid_offset))),
        std::make_tuple(
          std::get<2>(m_compose.template get_in_port<12>(lid_offset))),
        std::make_tuple(
          std::get<1>(m_compose.template get_in_port<13>(lid_offset))),
        std::make_tuple(
          std::get<1>(m_compose.template get_in_port<15>(lid_offset))),
        std::make_tuple(
          std::get<2>(m_compose.template get_in_port<18>(lid_offset))),
        std::make_tuple(
          std::get<2>(m_compose.template get_in_port<20>(lid_offset)))
      );
    }

    template <typename In>
    typename out_port_type<In>::type
    out_port(std::size_t lid_offset)
    {
      return m_compose.template get_out_port<
        typename inner_ports_types<In>::in_flow_20_type , 20
      >(lid_offset);
    }

  };
};

} // namespace flows


#endif // STAPL_FMM_FLOWS_HPP
