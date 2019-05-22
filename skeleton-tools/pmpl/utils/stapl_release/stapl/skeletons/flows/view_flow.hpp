/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_VIEW_FLOW_HPP
#define STAPL_SKELETONS_VIEW_FLOW_HPP

#include <stapl/utility/integer_sequence.hpp>
#include <stapl/domains/indexed.hpp>
#include <stapl/skeletons/flows/producer_info.hpp>

namespace stapl {

template <typename T, typename A>
class proxy;

namespace skeletons {
namespace flows {

namespace view_flow_helpers {

//////////////////////////////////////////////////////////////////////
/// @brief Mapping function used for redefining the view domains from
/// zero.
//////////////////////////////////////////////////////////////////////
template <typename Dom>
struct offset_mapping_function
{
  using index_type = typename Dom::index_type;

  offset_mapping_function(Dom const&)
  { }

  index_type operator()(index_type const& index) const
  {
    return index;
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Mapping function used for redefining the view domains from
/// zero, specification for 1 Dimension domains.
//////////////////////////////////////////////////////////////////////
template <>
struct offset_mapping_function<indexed_domain<std::size_t, 1>>
{
  using dom_t = indexed_domain<std::size_t, 1>;
  using index_type = typename dom_t::index_type;
  using gid_type = typename dom_t::index_type;

  index_type const m_offset;

  offset_mapping_function(dom_t const& domain)
    : m_offset(domain.first())
  { }

  gid_type operator()(index_type const& index) const
  {
    return index_type(index + m_offset);
  }

  void define_type(typer& t)
  {
    t.member(m_offset);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Mapping function used for redefining the view domains from
/// zero, specification for N Dimensions domains.
//////////////////////////////////////////////////////////////////////
template <int n, typename T>
struct offset_mapping_function<indexed_domain<std::size_t, n, T>>
{
  using dom_t = indexed_domain<std::size_t, n, T>;
  using index_type = typename dom_t::index_type;
  using gid_type = typename dom_t::index_type;

  index_type const m_offset;

  offset_mapping_function(dom_t const& domain)
    : m_offset(domain.first())
  { }

  template <std::size_t... idx>
  gid_type
  apply(index_type const& index, index_sequence<idx...>) const
  {
    return index_type(stapl::get<idx>(index) + stapl::get<idx>(m_offset)...);
  }

  gid_type operator()(index_type const& index) const
  {
    return apply(index, make_index_sequence<n>());
  }

  void define_type(typer& t)
  {
    t.member(m_offset);
  }
};

} // view_flow_helpers


//////////////////////////////////////////////////////////////////////
/// @brief @c view_flow is a flow defined over a view in STAPL. It is
/// used later on (upon the invocation of @c spawn_element) to
/// retrieve an element from the enclosed view.
///
/// @tparam View        the type of the view that this flow is going
///                     to read/write to/from
/// @tparam QueryMapper the translator that should be used on the
///                     requests before passing it to the view
///
/// @ingroup skeletonsFlows
//////////////////////////////////////////////////////////////////////
template <typename View,
          typename MappingF = stapl::use_default>
class view_flow
{
  using map_func_type = default_type<MappingF,
    view_flow_helpers::offset_mapping_function<
      typename view_traits<View>::domain_type>>;

  View&               m_view;
  map_func_type const m_mapping_function;
public:
  using flow_value_type = typename View::reference;

  view_flow(View& v)
    : m_view(v),
      m_mapping_function(v.domain())
  { }

  template<typename F = skeletons::no_filter>
  using producer_type = view_producer_info<View>;

  template <typename Index, typename Filter = skeletons::no_filter>
  producer_type<Filter>
  consume_from(Index const& index, Filter const& f = Filter()) const
  {
    return producer_type<Filter>(m_view,
                                 m_mapping_function(stapl::get<0>(index)));
  }

  template <typename Coord>
  std::size_t consumer_count(Coord const& producer_coord) const
  {
    return 0;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief @copybrief view_flow
///
/// This specialization is used only if the given view is a proxy.
/// This type of @c view_flow can be seen in the nested sections.
///
/// @tparam T           the type of the view that this flow is going
///                     to read/write to/from
/// @tparam A           the type of the accessor for the given proxy
/// @tparam QueryMapper the translator that should be used on the
///                     requests before passing it to the view
//////////////////////////////////////////////////////////////////////
template <typename T, typename A, typename MappingF>
class view_flow<stapl::proxy<T, A>, MappingF>
{
  using view_t = proxy<T, A>;
  view_t const& m_view;
public:
  view_flow(view_t const& v)
    : m_view(v)
  { }

  template <typename Filter = skeletons::no_filter>
  struct producer_type
  {
    using type = view_producer_info<view_t>;
  };

  template <typename Index, typename Filter = skeletons::no_filter>
  producer_type<Filter>
  consume_from(Index const& index, Filter const& f = Filter()) const
  {
    return producer_type<Filter>(m_view, stapl::get<0>(index));
  }

  template <typename Coord>
  std::size_t consumer_count(Coord const& producer_coord) const
  {
   return 0;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Creates a @c view_flow over the given view.
///
/// @param view the input view
///
/// @return a @c view_flow over the given view
///
/// @todo needs to be specialized for any view that has @c indexed_domain
//////////////////////////////////////////////////////////////////////
template <typename View>
view_flow<View>
make_view_flow(View& view)
{
  return view_flow<View>(view);
}

} // namespace flows
} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_VIEW_FLOW_HPP

