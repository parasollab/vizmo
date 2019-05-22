/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_METADATA_UTILITY_HAVE_EQUAL_SIZES_HPP
#define STAPL_VIEWS_METADATA_UTILITY_HAVE_EQUAL_SIZES_HPP

#include <stapl/containers/type_traits/index_bounds.hpp>
#include <stapl/utility/vs_map.hpp>

namespace stapl {

namespace metadata {

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Unary functor comparing member to input parameter's size method.
///   True if values (representing sizes) are the same or the parameter's size
///   is tagged as infinite.
//////////////////////////////////////////////////////////////////////
template<typename T>
class compare_size
{
private:
  const T m_sz;

public:
  typedef bool result_type;

  compare_size(T const& sz)
    : m_sz(sz)
  { }

  template<typename Partition>
  bool operator()(Partition const& p) const
  {
    const T partition_size = p.size();

    return partition_size == m_sz
           || partition_size == index_bounds<T>::highest();
  }
}; // class compare_size

} // namespace detail


//////////////////////////////////////////////////////////////////////
/// @brief Compute whether or not a set of views have the same size.
///
/// @param md Tuple of views
/// @tparam Guide Index in the tuple with which to compare sizes
//////////////////////////////////////////////////////////////////////
template<int Guide = 0, typename Containers>
bool have_equal_sizes(Containers const& md)
{
  return vs_map_reduce(
    detail::compare_size<
      typename stapl::tuple_element<
        Guide, Containers
      >::type::size_type
    >(get<Guide>(md).size()),
    std::logical_and<bool>(), true, md
  );
}

} // namespace metadata

} // namespace stapl

#endif // STAPL_VIEWS_METADATA_UTILITY_HAVE_EQUAL_SIZES_HPP
