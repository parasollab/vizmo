/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_UTILITY_LIGHTWEIGHT_MULTIARRAY_BASE_HPP
#define STAPL_SKELETONS_UTILITY_LIGHTWEIGHT_MULTIARRAY_BASE_HPP

#include <vector>
#include <memory>
#include <boost/make_shared.hpp>
#include <stapl/views/proxy.h>
#include <stapl/domains/indexed.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief A base class for lightweight sequential containers used in
/// the skeleton framework applications such as the NAS IS and FT
/// benchmarks. This class wraps an std::vector and keeps a
/// @c shared_ptr to the underlying std::vector.
///
/// @note This class is extended by @c lightweight_vector and
/// @c lightweight_multiarray to reduce the copying overhead of large
/// intermediate results between the STAPL layers (Skeleton Framework,
/// PARAGRAPH, and runtime) to copies of @c shared_ptrs.
///
/// @tparam T    the type of elements to store
/// @tparam dims the number of dimensions of this container.
///
/// @ingroup skeletonsUtilities
//////////////////////////////////////////////////////////////////////
template <typename T, int dims>
struct lightweight_multiarray_base
{
protected:
  using value_type      = T;
  using content_t       = std::vector<value_type>;
private:
  using storage_t       = std::shared_ptr<content_t>;
public:
  using reference       = typename content_t::reference;
  using const_reference = typename content_t::const_reference;
  using iterator        = typename content_t::iterator;
  using const_iterator  = typename content_t::const_iterator;
  using domain_type     = indexed_domain<std::size_t, dims>;
  using index_type      = typename domain_type::index_type;
  using dimensions_type = typename domain_type::dimensions_type;
  using size_type       = std::size_t;

protected:
  storage_t   m_content;
  domain_type m_domain;

public:
  lightweight_multiarray_base(domain_type const& domain)
    : m_content(std::make_shared<content_t>(domain.size())),
      m_domain(domain)
  { }

  template <typename... D>
  lightweight_multiarray_base(std::size_t d0, D const&... dimensions)
    : lightweight_multiarray_base(
        domain_type(std::forward_as_tuple(d0, dimensions...)))
  { }

  lightweight_multiarray_base()
    : lightweight_multiarray_base(domain_type())
  { }

  domain_type domain() const
  {
    return m_domain;
  }

  dimensions_type dimensions() const
  {
    return m_domain.dimensions();
  }

  size_type size() const
  {
    return m_content->size();
  }

  bool empty() const
  {
    return this->m_content->empty();
  }

protected:
  void resize_domain(dimensions_type const& dimensions)
  {
    m_domain = domain_type(dimensions);
  }

public:
  void define_type(typer& t)
  {
    t.member(m_content);
    t.member(m_domain);
  }
};

} // namespace stapl

#endif // STAPL_SKELETONS_UTILITY_LIGHTWEIGHT_MULTIARRAY_BASE_HPP
