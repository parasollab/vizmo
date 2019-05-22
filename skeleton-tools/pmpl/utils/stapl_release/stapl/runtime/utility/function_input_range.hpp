/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_RUNTIME_UTILITY_FUNCTION_INPUT_RANGE_HPP
#define STAPL_RUNTIME_UTILITY_FUNCTION_INPUT_RANGE_HPP

#include <cstddef>
#include <utility>
#include <boost/iterator/iterator_facade.hpp>

namespace stapl {

namespace runtime {

//////////////////////////////////////////////////////////////////////
/// @brief Creates a range from a nullary function object and a size.
///
/// @tparam Function Function type.
/// @tparam Size     Range size type.
///
/// @ingroup runtimeUtility
//////////////////////////////////////////////////////////////////////
template<typename Function, typename Size = std::size_t>
class function_input_range
{
public:
  using size_type            = Size;
private:
  using function_result_type = decltype(std::declval<Function>()());
public:
  class const_iterator
  : public boost::iterator_facade<
             const_iterator,
             function_result_type,
             boost::random_access_traversal_tag,
             function_result_type>
  {
  private:
    friend class boost::iterator_core_access;

    Function const& m_f;
    size_type       m_index;

  public:
    explicit const_iterator(Function const& f, const size_type i = 0) noexcept
    : m_f(f),
      m_index(i)
    { }

  private:
    value_type dereference(void) const
    { return m_f(); }

    bool equal(const_iterator const& other) const noexcept
    { return (m_index==other.m_index); }

    void increment(void) noexcept
    { ++m_index; }

    void decrement(void) noexcept
    { --m_index; }

    void advance(difference_type n) noexcept
    { m_index += n; }

    difference_type distance_to(const_iterator const& other) const noexcept
    {
      return ((m_index<=other.m_index)
                ? difference_type(other.m_index - m_index)
                : -difference_type(m_index - other.m_index));
    }
  };

private:
  Function  m_f;
  size_type m_size;

public:
  template<typename F, typename S>
  function_input_range(F&& f, S size)
  : m_f(std::forward<F>(f)),
    m_size(size)
  { }

  bool empty(void) const noexcept
  { return (m_size==0); }

  size_type size(void) const noexcept
  { return m_size; }

  const_iterator begin(void) const noexcept
  { return const_iterator{m_f}; }

  const_iterator end(void) const noexcept
  { return const_iterator{m_f, m_size}; }
};

} // namespace runtime

} // namespace stapl

#endif
