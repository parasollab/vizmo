/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_UTILITY_LIGHTWEIGHT_MULTIARRAY_HPP
#define STAPL_SKELETONS_UTILITY_LIGHTWEIGHT_MULTIARRAY_HPP

#include <stapl/skeletons/utility/lightweight_multiarray_base.hpp>
#include <stapl/views/proxy.h>
#include <stapl/views/view_traits.hpp>
#include <stapl/containers/type_traits/container_traits.hpp>
#include <stapl/domains/indexed.hpp>
#include <stapl/containers/iterators/local_accessor.hpp>
#include <stapl/containers/iterators/const_local_accessor.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief This container is a lightweight multiarray which is defined
/// over a @c lightweight_multiarray_base with the addition of its dimensions.
///
/// This container is used in skeletons in which copying large
/// matrices would result in performance degradation. As an example,
/// lightweight_multiarray is used in the matrix transpose algorithm
/// to avoid intermediate copies made by the PARAGRAPH at each step of
/// the @c alltoall skeleton.
///
/// @tparam T    the type of elements to store
/// @tparam dims the number of dimensions of this container.
///
/// @ingroup skeletonsUtilities
//////////////////////////////////////////////////////////////////////
template <typename T, int dims>
struct lightweight_multiarray
  : public lightweight_multiarray_base<T, dims>
{
public:
  using base_t          = lightweight_multiarray_base<T, dims>;
  using reference       = typename base_t::reference;
  using const_reference = typename base_t::const_reference;
  using iterator        = typename base_t::iterator;
  using const_iterator  = typename base_t::const_iterator;
  using domain_type     = indexed_domain<std::size_t, dims>;
  using index_type      = typename domain_type::index_type;
  using dimensions_type = typename domain_type::dimensions_type;
  using dimension_type  = std::integral_constant<int, dims>;
  using value_type      = typename base_t::value_type;

private:
  using traversal_t     = typename domain_type::traversal_type;
  using linearize_t     = nd_linearize<index_type, traversal_t>;

  linearize_t m_linear_mf;

  //////////////////////////////////////////////////////////////////////
  /// @brief Translate the n-dimensional tuple of indices into a 1D index.
  /// @param gid Tuple to linearize
  /// @return The linearization based on a specified traversal pattern.
  ///
  /// @todo Remove the static_cast and replace the explicit tuple argument by
  /// @p index_type once Kripke is fixed to use homogeneous index tuples.
  /// At this moment, tuples like <int, unsigned long, int> are used in Kripke
  /// - since they don't match the index_type (tuple<size_t, size_t, size_t>),
  /// wrong operator() (the variadic version) would be picked from nd_linearize.
  //////////////////////////////////////////////////////////////////////
  template <typename... Indices>
  size_t local_position(std::tuple<Indices...> const& idx) const
  {
    return m_linear_mf(static_cast<index_type const&>(idx));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Translate the n-dimensional index specified by a variadic pack
  /// into a 1D index.
  /// @param i Components of the n-dimensional index to linearize
  /// @return The linearization based on a specified traversal pattern.
  //////////////////////////////////////////////////////////////////////
  template <typename... Indices>
  size_t local_position(Indices... i) const
  {
    return m_linear_mf(i...);
  }

public:
  template <typename... Args>
  lightweight_multiarray(Args&&... args)
    : base_t(std::forward<Args>(args)...),
      m_linear_mf(this->dimensions())
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the iterator for the element at given @p GID (given
  /// by the index tuple or variadic pack).
  //////////////////////////////////////////////////////////////////////
  template <typename... GID>
  iterator get_iterator(GID const&... gid)
  {
    return this->m_content->begin() + local_position(gid...);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the const iterator for the element at given @p GID
  /// (given by the index tuple or variadic pack).
  //////////////////////////////////////////////////////////////////////
  template <typename... GID>
  const_iterator get_iterator(GID const&... gid) const
  {
    return this->m_content->cbegin() + local_position(gid...);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return reference to the element in buffer at index @p idx.
  //////////////////////////////////////////////////////////////////////
  reference operator[](index_type const& idx)
  {
    return this->m_content->operator[](m_linear_mf(idx));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return const reference to the element in buffer at index @p idx.
  //////////////////////////////////////////////////////////////////////
  const_reference operator[](index_type const& idx) const
  {
    return this->m_content->operator[](m_linear_mf(idx));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return reference to the element in buffer at index specified
  /// by variadic argument pack of elements from multidimensional index.
  //////////////////////////////////////////////////////////////////////
  template<typename... Indices>
  reference operator()(Indices... indices)
  {
    return this->m_content->operator[](m_linear_mf(indices...));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return const reference to the element in buffer at index specified
  /// by variadic argument pack of elements from multidimensional index.
  //////////////////////////////////////////////////////////////////////
  template<typename... Indices>
  const_reference operator()(Indices... indices) const
  {
    return this->m_content->operator[](m_linear_mf(indices...));
  }

  void define_type(typer& t)
  {
    t.base<base_t>(*this);
    t.member(m_linear_mf);
  }
};

template <typename T, int dims, typename Accessor>
class proxy<lightweight_multiarray<T, dims>, Accessor>
  : public Accessor
{
  STAPL_PROXY_TYPES(STAPL_PROXY_CONCAT(lightweight_multiarray<T, dims>),
                    Accessor)
  STAPL_PROXY_CONSTRUCTOR(STAPL_PROXY_CONCAT(lightweight_multiarray<T, dims>),
                    Accessor)
  STAPL_PROXY_METHODS(STAPL_PROXY_CONCAT(lightweight_multiarray<T, dims>),
                    Accessor)

  STAPL_PROXY_REFLECT_TYPE(value_type)
  STAPL_PROXY_REFLECT_TYPE(index_type)
  STAPL_PROXY_REFLECT_TYPE(domain_type)
  STAPL_PROXY_REFLECT_TYPE(dimension_type)
  STAPL_PROXY_METHOD_RETURN(domain, domain_type)
  STAPL_PROXY_METHOD_RETURN(dimensions, typename target_t::dimensions_type)
  STAPL_PROXY_METHOD_RETURN(size, typename target_t::dimensions_type)
  STAPL_PROXY_REFERENCE_METHOD_VARIADIC(inner, operator(), operator[], T)

  using iter_t          = typename target_t::iterator;
  using const_iter_t    = typename target_t::const_iterator;
  using iterator        = member_iterator<iter_t, Accessor>;
  using const_iterator  = member_iterator<const_iter_t, Accessor>;
  using reference       = inner_reference;
  using const_reference = const inner_reference;

}; // struct proxy<lightweight_multiarray<T, dims>, Accessor>


//////////////////////////////////////////////////////////////////////
/// @brief Specialization of @c container_traits for the cases that the
/// container is a lightweight_multiarray
//////////////////////////////////////////////////////////////////////
template<typename T, int dims>
struct container_traits<lightweight_multiarray<T, dims>>
{
  using domain_type     = indexed_domain<std::size_t, dims>;
  using container_type  = lightweight_multiarray<T, dims>;
  using value_type      = typename container_type::value_type;
  using gid_type        = typename container_type::index_type;
  using iterator        = typename container_type::iterator;
  using reference       = typename container_type::reference;
  using const_reference = typename container_type::const_reference;
};

//////////////////////////////////////////////////////////////////////
/// @brief Specialization of @c container_traits for the cases that the
/// container is a proxy of a lightweight_multiarray
//////////////////////////////////////////////////////////////////////
template<typename T, int dims, typename A>
struct container_traits<proxy<lightweight_multiarray<T, dims>, A>>
{
  using domain_type     = indexed_domain<std::size_t, dims>;
  using container_type  = proxy<lightweight_multiarray<T, dims>, A>;
  using value_type      = typename container_type::value_type;
  using gid_type        = typename container_type::index_type;
  using iterator        = typename container_type::iterator;
  using reference       = typename container_type::reference;
  using const_reference = typename container_type::const_reference;
};

template <typename C>
using lightweight_multiarray_accessor = local_accessor<C>;

template <typename C>
using lightweight_multiarray_const_accessor = const_local_accessor<C>;


template<typename T, bool a, bool b, int dims>
struct extract_reference_type<lightweight_multiarray<T, dims>, a, b>
{
  using type = proxy<
                 T,
                 lightweight_multiarray_accessor<
                   lightweight_multiarray<T, dims>>>;
};

template<typename T, bool a, bool b, int dims>
struct extract_const_reference_type<lightweight_multiarray<T, dims>, a, b>
{
  using type = proxy<
                 T,
                 lightweight_multiarray_const_accessor<
                   lightweight_multiarray<T, dims>>>;
};

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Specialization of make_reference for the cases that the
/// underlying container is a proxy of a lightweight_multiarray.
//////////////////////////////////////////////////////////////////////
template<typename T, int dims, typename A>
struct container_make_reference<proxy<lightweight_multiarray<T, dims>, A>>
{
  template<typename View>
  static
  typename view_traits<View>::reference
  apply(View const& view, typename view_traits<View>::index_type const& index)
  {
    return view.container()[view.mapfunc()(index)];
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Specialization of make_reference for the cases that the
/// underlying container is a lightweight_multiarray.
//////////////////////////////////////////////////////////////////////
template<typename T, int dims>
struct container_make_reference<lightweight_multiarray<T, dims>>
{
private:
  using accessor =
    lightweight_multiarray_accessor<lightweight_multiarray<T, dims>>;

  //////////////////////////////////////////////////////////////////////
  /// @brief Functor that can be passed to the @p apply_get of a
  /// multidimensional mapping function to facilitate mapping without
  /// intermediate tuple creation.
  ///
  /// Part of the reason to use in place of a lambda is to allow arity of
  /// arguments passed to container to vary from that the view passed to the
  /// mapping function (e.g., @ref sliced_mf).
  //////////////////////////////////////////////////////////////////////
  template<typename View>
  struct ref_getter
  {
  private:
    View const& m_view;
    using reference = typename view_traits<View>::reference;

  public:
    ref_getter(View const& view)
      : m_view(view)
    { }

    template<typename ...Args>
    reference operator()(Args&&... args) const
    {
      auto container_ptr = m_view.get_container();
      return
        reference(
          accessor(container_ptr,
                   container_ptr->get_iterator(std::forward<Args>(args)...)) );
    }
  };

public:
  template<typename View>
  static
  typename view_traits<View>::reference
  apply(View const& view, typename view_traits<View>::index_type const& index)
  {
    auto container_ptr = view.get_container();
    return
      typename view_traits<View>::reference(
        accessor(container_ptr,
                 container_ptr->get_iterator(view.mapfunc()(index))) );
  }

  template<typename View, typename... Indices>
  static
  typename view_traits<View>::reference
  apply(View const& view, Indices... indices)
  {
    return view.mapfunc().apply_get(ref_getter<View>(view), indices...);
  }
};

} // namespace detail
} // namespace stapl

#endif // STAPL_SKELETONS_UTILITY_LIGHTWEIGHT_MULTIARRAY_HPP
