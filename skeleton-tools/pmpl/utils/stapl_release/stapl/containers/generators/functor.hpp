/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_GENERATOR_FUNCTOR_HPP
#define STAPL_CONTAINERS_GENERATOR_FUNCTOR_HPP

#include <stapl/domains/indexed.hpp>
#include <stapl/views/proxy/trivial_accessor.hpp>
#include <stapl/views/metadata/extraction/generator.hpp>
#include <stapl/containers/generators/generator_container_base.hpp>

namespace stapl {

template<typename Functor, int n = 1>
struct functor_container;

namespace view_impl {

template <typename Domain, typename Functor>
struct functor_container_distribution
  : public p_object
{
  typedef Domain                                      domain_type;
  typedef functor_container<Functor>*                 component_type;
  typedef metadata_entry<domain_type, component_type> dom_info_type;

  future<dom_info_type> metadata_at(typename domain_type::gid_type gid)// const
  {
    return make_ready_future(dom_info_type(
             typename dom_info_type::cid_type(), domain_type(gid, gid), 0,
             LQ_DONTCARE, invalid_affinity_tag,
             this->get_rmi_handle(), this->get_location_id()
           ));
  }

  void define_type(typer&)
  {
    abort("functor_container_distribution unexpectedly invoked");
  }
}; // struct functor_container_distribution

} // namespace view_impl


template <int n>
struct functor_container_domain_type
{
  typedef indexed_domain<
            std::size_t, n, typename default_traversal<n>::type>  type;
};


template <>
struct functor_container_domain_type<1>
{
  typedef indexed_domain<std::size_t> type;
};

//////////////////////////////////////////////////////////////////////
/// @brief Generator container whose value for the element at an index
/// is the application of a functor on the index itself.
///
/// If we have a functor_container c, then c[i] == f(i) must
/// be true. Note that the functor must publicly export a nested trait
/// for the index_type (which is the input to the functor) and a nested
/// trait for the result_type (which is the result of applying the functor
/// on the index_type). In addition, the functor's operator must be declared
/// const.
///
/// @tparam Func Function object
/// @todo Const qualification on const_reference.
////////////////////////////////////////////////////////////////////////
template<typename Func, int n>
struct functor_container
 : public generator_container_base
{
public:
  typedef typename Func::index_type                         gid_type;
  typedef typename Func::result_type                        value_type;
  typedef typename boost::result_of<Func(gid_type)>::type   reference;
  typedef reference                                         const_reference;
  typedef typename functor_container_domain_type<n>::type   domain_type;
  typedef std::integral_constant<int, n>                    dimension_type;

  typedef typename domain_type::size_type                   size_type;
  /// TODO: cid_type should be changed based on the coarsening method
  typedef size_t                                            cid_type;
  typedef view_impl::functor_container_distribution<
    domain_type, Func
  >                                                         distribution_type;
  typedef metadata::generator_extractor<functor_container>  loc_dist_metadata;

private:
  /// Size of this generator container
  size_type         m_size;
  /// The functor that is to be applied on every access
  Func              m_func;

  distribution_type m_dist;

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Create the container by initializing the functor and
  /// providing a size.
  ///
  /// @param size Size of this container
  /// @param func The functor to be applied
  ////////////////////////////////////////////////////////////////////////
  functor_container(size_type size, Func const& func)
    : m_size(size), m_func(func)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the current version number
  //////////////////////////////////////////////////////////////////////
  size_t version(void) const
  {
    return 0;
  }

  distribution_type& distribution()
  { return m_dist; }

  //////////////////////////////////////////////////////////////////////
  /// @brief Retrieve the value associated with a particular index.
  ///
  /// @param i Index of the value to compute
  /// @return The result of applying the functor on i
  ////////////////////////////////////////////////////////////////////////
  value_type get_element(gid_type const& i) const
  {
    return m_func(i);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Apply an arbitrary functor to the computed value at a particular
  /// index.
  ///
  /// @param idx Index of the value to compute
  /// @param f A functor to apply to the value at index idx. Note that f's
  /// function operator must be declared const.
  /// @return The result of applying the functor to the element at idx
  ////////////////////////////////////////////////////////////////////////
  template <typename F>
  value_type apply_get(gid_type const& idx, F const& f)
  {
    return f(this->get_element(idx));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Construct a reference to the value of a particular index.
  ///
  /// @param i Index of the value to retrieve
  /// @return Proxy of the index
  ////////////////////////////////////////////////////////////////////////
  reference operator[](gid_type const& i) const
  {
    return m_func(i);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Construct a reference to the value of a particular index.
  ///
  /// @param i Index of the value to retrieve
  /// @return Proxy of the index
  ////////////////////////////////////////////////////////////////////////
  reference make_reference(gid_type const& i) const
  {
    return reference(trivial_accessor<value_type>(m_func(i)));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the size of this container
  ////////////////////////////////////////////////////////////////////////
  size_type size(void) const
  {
    return m_size;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a linear domain of the GIDs in this container
  ////////////////////////////////////////////////////////////////////////
  domain_type domain(void) const
  {
    return domain_type(m_size);
  }

  locality_info locality(gid_type const&) const
  {
    return LQ_DONTCARE;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Serialization of this class
  ////////////////////////////////////////////////////////////////////////
  void define_type(typer& t)
  {
    t.member(m_size);
    t.member(m_func);
    t.member(m_dist);
  }
}; // struct functor_container

} // namespace stapl

#endif // STAPL_CONTAINERS_GENERATOR_FUNCTOR_HPP
