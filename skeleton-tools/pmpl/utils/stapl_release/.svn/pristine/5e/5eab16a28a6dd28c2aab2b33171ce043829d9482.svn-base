/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_COUNTING_VIEW_HPP
#define STAPL_VIEWS_COUNTING_VIEW_HPP

#include <stapl/views/array_ro_view.hpp>
#include <stapl/views/proxy/trivial_accessor.hpp>
#include <stapl/views/metadata/extraction/generator.hpp>

#include <iostream>

namespace stapl {

namespace view_impl {

template <typename T>
struct counting_container;

template <typename Domain, typename T>
struct counting_container_distribution
  : public p_object
{
  typedef Domain                                      domain_type;
  typedef counting_container<T>*                      component_type;
  typedef metadata_entry<domain_type, component_type> dom_info_type;

  future<dom_info_type> metadata_at(size_t gid)
  {
    return make_ready_future(dom_info_type(
             typename dom_info_type::cid_type(), domain_type(gid, gid), 0,
             LQ_DONTCARE, invalid_affinity_tag,
             this->get_rmi_handle(), 0
           ));
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Container that represents an increasing sequence of elements
///
/// @tparam T element type
/// @ingroup counting_view
/// @internal
//////////////////////////////////////////////////////////////////////
template<typename T>
struct counting_container
{
  typedef T                                                 value_type;
  typedef proxy<T, trivial_accessor<T> >                    reference;
  // trivial_accessor provides read access only.
  // reference and const_reference are the same type.
  typedef reference                                         const_reference;
  typedef indexed_domain<size_t>                            domain_type;
  typedef size_t                                            gid_type;
  typedef size_t                                            cid_type;
  typedef counting_container_distribution<domain_type, T>   distribution_type;
  typedef metadata::generator_extractor<counting_container> loc_dist_metadata;

private:
  size_t                                 m_size;
  T                                      m_init;
  distribution_type                      m_dist;

public:
  //////////////////////////////////////////////////////////////////////
  /// @param size number of elements provided for the container.
  /// @param init initial value from start counting.
  //////////////////////////////////////////////////////////////////////
  counting_container(size_t size, T init)
    : m_size(size), m_init(init)
  { }

  distribution_type& distribution()
  {
    return m_dist;
  }

  value_type get_element(gid_type const& index) const
  {
    return index + m_init;
  }

  reference operator[](gid_type const& index)
  {
    return reference(trivial_accessor<T>(index + m_init));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the unchanging 0 version number
  //////////////////////////////////////////////////////////////////////
  size_t version(void) const
  {
    return 0;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Applies the provided function the the value referenced
  ///        for the given index returning the resulting value
  ///
  /// @param index of element to apply the function
  /// @param f function to apply
  /// @return result of evaluate the function f on the value
  ///         referenced for the index
  //////////////////////////////////////////////////////////////////////
  template <typename F>
  typename F::result_type
  apply_get(gid_type const& index, F const& f)
  {
    return f(this->operator[](index));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns a reference to the element indexed.
  ///
  /// @param index of element to return
  /// @return element reference
  //////////////////////////////////////////////////////////////////////
  reference make_reference(gid_type const& index)
  {
    return this->operator[](index);
  }

  size_t size(void) const
  {
    return m_size;
  }

  domain_type domain(void) const
  {
    return domain_type(m_size);
  }

  typedef std::true_type task_placement_dontcare;

  locality_info locality(gid_type const&) const
  {
    return LQ_DONTCARE;
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  //////////////////////////////////////////////////////////////////////
  void define_type(typer& t)
  {
    t.member(m_size);
    t.member(m_init);
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  /// @brief use to examine this class
  /// @param msg your message (to provide context)
  //////////////////////////////////////////////////////////////////////
  void debug(char *msg=0)
  {
    std::cerr << "COUNTING_VIEW " << this << " : ";
    if (msg) {
      std::cerr << msg;
    }
    std::cerr << std::endl;
    std::cerr << " m_size " << m_size;
    std::cerr << " m_init " << m_init << std::endl;
  }

}; // struct counting_container

} // namespace view_impl


namespace result_of {

//////////////////////////////////////////////////////////////////////
/// @brief Defines the type of a counting view parameterized with T
///
/// @tparam T element type of the elements represented for the
///         counting_view
/// @ingroup counting_view
//////////////////////////////////////////////////////////////////////
template<typename T>
struct counting_view
{
  typedef array_ro_view<view_impl::counting_container<T> > type;
};

} // result_of namespace


//////////////////////////////////////////////////////////////////////
/// @brief Helper function that creates a read-only view representing
///        a set of increasing elements.
///
/// @param n number of elements provided for the counting_view.
/// @param init initial value from start counting.
/// @return a counting_view that represents an increasing set of n
///         elements starting from init.
/// @ingroup counting_view
//////////////////////////////////////////////////////////////////////
template<typename T>
typename result_of::counting_view<T>::type
counting_view(size_t n, T init = 0)
{
  return typename result_of::counting_view<T>::type(
    new view_impl::counting_container<T>(n, init)
  );
}

} // namespace stapl

#endif // STAPL_VIEWS_COUNTING_VIEW_HPP
