/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_FLOWS_INDEXED_FLOW_HPP
#define STAPL_SKELETONS_FLOWS_INDEXED_FLOW_HPP

#include <vector>
#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/skeletons/flows/producer_info.hpp>

namespace stapl {
namespace skeletons {
namespace flows {

template <typename T, typename Span, typename QueryMapper = void>
class indexed_flow;

////////////////////////////////////////////////////////////////////////
/// @brief An elementary flow representing the output of a single node
/// in the dataflow graph corresponding to a single skeleton which allows
/// a skeleton to be connected to all its underlying spawned tasks.
///
/// @see elem.hpp
///
/// @ingroup skeletonsFlowsElem
////////////////////////////////////////////////////////////////////////
template <typename T, typename Span>
class indexed_flow<T, Span, void>
{
  Span const        m_span;
  std::size_t const m_initial_offset;

  using index_type = std::size_t;
public:
  using flow_value_type = T;

  indexed_flow(Span const& span, std::size_t initial_offset)
    : m_span(span),
      m_initial_offset(initial_offset)
  { }


  template <typename F = skeletons::no_filter>
  using producer_type = indexed_producer_info<flow_value_type, index_type, F>;

  template <typename Index, typename F = skeletons::no_filter>
  producer_type<F>
  consume_from(Index const& index, F const& f = F()) const
  {
    std::size_t offset = m_span.linearize(stapl::get<0>(index));
    return producer_type<F>(m_initial_offset + offset, f);
  }

  template <typename Index>
  index_type
  depend_on(Index const& index) const
  {
    return this->consume_from(index).get_index();
  }

  template <typename Indices, typename F = skeletons::no_filter>
  indexed_producers_info<flow_value_type, std::vector<index_type>, F>
  consume_from_many(Indices const& indices, F const& f = F()) const
  {
    using producers_type =
      indexed_producers_info<flow_value_type, std::vector<index_type>, F>;

    std::vector<index_type> deps;
    deps.reserve(indices.size());

    for (auto&& e : indices) {
      deps.push_back(this->consume_from(e).get_index());
    }

    return producers_type(deps, f);
  }
};

template <typename T,
          typename QueryMapper = void,
          typename Span>
indexed_flow<T, Span, QueryMapper>
make_indexed_flow(Span const& span, std::size_t const offset)
{
  return indexed_flow<T, Span, QueryMapper>(span, offset);
}

} // namespace flows
} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_FLOWS_INDEXED_FLOW_HPP
