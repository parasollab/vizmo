/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_BASE_CONTAINER_HPP
#define STAPL_CONTAINERS_BASE_CONTAINER_HPP

#include <stapl/runtime.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/functional/factory.hpp>
#include <boost/call_traits.hpp>

#include <stapl/containers/type_traits/container_wrapper_ref.hpp>
#include <stapl/views/view_packing.hpp>
#include <stapl/utility/loc_qual.hpp>
#include <stapl/utility/import_types.hpp>
#include <stapl/containers/base/distribution_policy.hpp>
#include <stapl/containers/base/container_base.hpp>
#include <stapl/containers/distribution/specifications_fwd.hpp>
#include <stapl/containers/distribution/is_distribution_view.hpp>
#include <stapl/containers/distribution/composed_specification_ptr.hpp>

namespace stapl {

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Function object called by a container whose value type is another
///  container to properly populate the former's base container.
/// @param dist The distribution of the outer container.
/// @param factory Functor that returns pointer to heap allocated instances
///   of the inner container.
//////////////////////////////////////////////////////////////////////
class default_nested_initializer
{
private:
  /// @brief True if container is to be created in an spmd fashion by all
  /// locations (even though only one will store it in the base container).
  bool m_b_spmd;

public:
  void define_type(typer& t)
  { t.member(m_b_spmd); }

  explicit default_nested_initializer(bool b_spmd)
    : m_b_spmd(b_spmd)
  { }

  template<typename Distribution, typename Factory>
  void operator()(Distribution& distribution, Factory const& factory) const
  {
    STAPL_IMPORT_DTYPE(Distribution, base_container_type)
    STAPL_IMPORT_DTYPE(Distribution, index_type)

    index_type idx           = distribution.domain().first();
    const index_type end_idx = distribution.domain().last();

    bool flag = false;

    for ( ; !flag ; )
    {
      if (idx == end_idx)
        flag = true;

      const bool b_contains = distribution.container_manager().contains(idx);

      if (m_b_spmd || b_contains)
      {
        auto elem_ptr = factory(idx, b_contains);

        typedef typename std::remove_pointer<
          decltype(elem_ptr)>::type                value_type;

        if (b_contains)
          distribution.container_manager().invoke(
            idx, &base_container_type::set_element,
            idx, container_wrapper_ref<value_type>(*elem_ptr));
      }

      idx = distribution.domain().advance(idx, 1);
    }
  }
}; // class default_nested_initializer


BOOST_MPL_HAS_XXX_TRAIT_DEF(nested_initializer_type)
BOOST_MPL_HAS_XXX_TRAIT_DEF(nested_stored_value_type)
BOOST_MPL_HAS_XXX_TRAIT_DEF(is_composed_dist_spec)


//////////////////////////////////////////////////////////////////////
/// @brief Metafunction that returns reflected @p nested_initializer_type if
/// the specified container @p Traits reflects a typedef. Otherwise, return
/// default value (@p default_nested_initializer).
//////////////////////////////////////////////////////////////////////
template<typename Traits, bool = has_nested_initializer_type<Traits>::value>
struct get_nested_initializer_type
{
  typedef default_nested_initializer type;
};


template<typename Traits>
struct get_nested_initializer_type<Traits, true>
{
  typedef typename Traits::nested_initializer_type type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Metafunction that returns reflected @p nested_stored_value_type if
/// the specified container @p Traits a typedef exists.  Otherwise, return
/// default value (@p Traits::value_type).
//////////////////////////////////////////////////////////////////////
template<typename Traits, bool = has_nested_stored_value_type<Traits>::value>
struct get_nested_stored_value_type
{
  typedef typename Traits::value_type type;
};


template<typename Traits>
struct get_nested_stored_value_type<Traits, true>
{
  typedef typename Traits::nested_stored_value_type type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Return the parameter that should be passed to a nested container
/// constructor.  For non leaf containers, pass the remaining dimensions.
//////////////////////////////////////////////////////////////////////
template<typename X, typename Y>
boost::tuples::cons<X,Y> const&
factory_parameter(boost::tuples::cons<X,Y> const& dims)
{
  return dims;
}


//////////////////////////////////////////////////////////////////////
/// @brief Return the parameter that should be passed to a nested container
/// constructor.  For the leaf container, just return it's size.
//////////////////////////////////////////////////////////////////////
template<typename X>
X const&
factory_parameter(boost::tuples::cons<X, boost::tuples::null_type> const& dims)
{
  return dims.get_head();
}


//////////////////////////////////////////////////////////////////////
/// @brief Factory encapsulating the creation of nested containers with the
///   the sizes view idiom for instantiation with variable sizes. Follows
///   interface of Boost.Factory, returns pointer to heap allocated instance
///   of the created object.
/// @tparam Container The type of the container this factory creates.
/// @tparam SizesView Descriptor of sizes for containers created at this
///   level (and below).
/// @tparam GID Index type of the outer container.  Used to index into
///   the sizes view.
//////////////////////////////////////////////////////////////////////
template<typename Container, typename SizesView, typename GID>
class variable_sizes_factory
{
private:
  /// @brief The sizes descriptor passed to the container constructor.
  SizesView const& m_sizes_view;

public:
  typedef Container* result_type;

  variable_sizes_factory(SizesView const& sizes_view)
    : m_sizes_view(sizes_view)
  { }

  result_type operator()(typename boost::call_traits<GID>::param_type gid,
                         bool) const
  {
    return new Container(m_sizes_view[gid]);
  }
}; // struct variable_sizes_factory


//////////////////////////////////////////////////////////////////////
/// @brief Factory encapsulating the creation of nested containers with the
///   the composed distribution specification for instantiation with
///   variable distributions. Follows interface of Boost.Factory, returns
///   pointer to heap allocated instance of the created object.
/// @tparam Container The type of the container this factory creates.
/// @tparam CompSpec Composed specification of the distributions for
///   containers created at this level (and below).
/// @tparam GID Index type of the outer container.  Used to index into
///   the composed specification.
//////////////////////////////////////////////////////////////////////
template<typename Container, typename CompSpec, typename GID>
class variable_distribution_factory
{
private:
  /// @brief The distribution specification passed to the container constructor.
  CompSpec const&   m_comp_spec;

public:
  typedef Container* result_type;

  variable_distribution_factory(CompSpec const& comp_spec)
    : m_comp_spec(comp_spec)
  { }

  result_type operator()(typename boost::call_traits<GID>::param_type gid,
                         bool contains) const
  {
    // Get distribution specification for subcontainer at index gid.
    // Extract a reference to its system_container.
    auto dist_spec = m_comp_spec[gid].spec();
    auto& sys_ct   = dist_spec.container().container().container();

    const bool b_level_specified = sys_ct.level_specified();

    stapl_assert(b_level_specified || sys_ct.explicit_locs(),
      "invalid configuration");

    bool b_participate = true;

    if (b_level_specified) {
      if (!contains && sys_ct.level_spec() == lowest_level)
        b_participate = false;
    }
    else
    {
      if (!sys_ct.domain().contains(m_comp_spec.get_location_id()))
        b_participate = false;
    }

    stapl_assert(b_participate || !contains,
      "not participating in creating a contained element");

    if (b_participate)
    {
      gang g(sys_ct.group());
      return new Container(m_comp_spec[gid]);
    }

    return nullptr;
  }
}; // struct variable_distribution_factory

///////////////////////////////////////////////////////////////////////
/// @brief Base class of containers that implements functionality that is
///   independent of whether nested container composition exists (mainly
///   distribution related functionality).
/// @tparam C derived container class for use with CRTP.
/// @todo Consider direct inheritiance of distribution by @p container,
///   instead of wrapping common access in this intermediate class.
///////////////////////////////////////////////////////////////////////
template<typename C>
class container_impl
  : public container_base,
    private container_traits<C>::template construct_distribution<C>::type
{
private:
  size_t m_version;

public:
  STAPL_IMPORT_TYPE(typename container_traits<C>, value_type)
  STAPL_IMPORT_TYPE(typename container_traits<C>, partition_type)
  STAPL_IMPORT_TYPE(typename container_traits<C>, mapper_type)

  typedef typename container_traits<C>::template
    construct_distribution<C>::type                      distribution_type;

  STAPL_IMPORT_TYPE(typename distribution_type, directory_type)
  STAPL_IMPORT_TYPE(typename distribution_type, container_manager_type)
  STAPL_IMPORT_TYPE(typename distribution_type, domain_type)
  STAPL_IMPORT_TYPE(typename distribution_type, reference)
  STAPL_IMPORT_TYPE(typename distribution_type, const_reference)

  STAPL_IMPORT_TYPE(typename domain_type, index_type)

  typedef index_type                                     gid_type;
  typedef size_t                                         size_type;

  container_impl(void)
    : m_version(0)
  { }

  container_impl(distribution_type const& distribution)
    : distribution_type(distribution), m_version(0)
  { }

  container_impl(directory_type const& dir, container_manager_type const& cm)
    : distribution_type(dir, cm), m_version(0)
  { }

  template <typename Reg>
  container_impl(directory_type const& dir, container_manager_type const& cm,
                 Reg const& reg)
    : distribution_type(dir, cm, reg), m_version(0)
  { }

  container_impl(partition_type const& ps, mapper_type const& map)
    : distribution_type(ps, map), m_version(0)
  { }

  template <typename Reg>
  container_impl(partition_type const& ps, mapper_type const& map,
                 Reg const& reg)
    : distribution_type(ps, map, reg), m_version(0)
  { }

  container_impl(partition_type const& ps,
                 mapper_type const& map,
                 value_type const& default_value)
    : distribution_type(ps, map, default_value), m_version(0)
  { }

  template <typename DistSpecView, typename PartitionContainer>
  container_impl(DistSpecView const& dist_view,
                 PartitionContainer const* const part_cont,
                 typename std::enable_if<
                   is_distribution_view<DistSpecView>::value &&
                   !detail::has_is_composed_dist_spec<DistSpecView>::value &&
                   std::is_same<typename PartitionContainer::value_type,
                     arbitrary_partition_info>::value>::type* = 0)
    : distribution_type(std::make_shared<DistSpecView>(dist_view), part_cont),
      m_version(0)
  { }

  template <typename DistSpecView, typename PartitionContainer>
  container_impl(DistSpecView const& dist_view,
                 PartitionContainer const* const part_cont,
                 value_type const& default_value,
                 typename std::enable_if<
                   is_distribution_view<DistSpecView>::value &&
                   !detail::has_is_composed_dist_spec<DistSpecView>::value &&
                   std::is_same<typename PartitionContainer::value_type,
                     arbitrary_partition_info>::value>::type* = 0)
    : distribution_type(std::make_shared<DistSpecView>(dist_view), part_cont,
        default_value),
      m_version(0)
  { }

  template <typename DistSpecView, typename PartitionContainer, typename Reg>
  container_impl(DistSpecView const& dist_view,
                 PartitionContainer const* const part_cont,
                 Reg const& reg,
                 typename std::enable_if<
                   is_distribution_view<DistSpecView>::value &&
                   !detail::has_is_composed_dist_spec<DistSpecView>::value &&
                   std::is_same<typename PartitionContainer::value_type,
                     arbitrary_partition_info>::value>::type* = 0)
    : distribution_type(std::make_shared<DistSpecView>(dist_view),
        part_cont, reg), m_version(0)
  { }

  /// @name Version Support
  /// @{

private:
  //////////////////////////////////////////////////////////////////////
  /// @brief  set the version number to the initial value
  //////////////////////////////////////////////////////////////////////
  void init_version(void)
  {
    m_version = 0;
  }

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief  increment the version number
  //////////////////////////////////////////////////////////////////////
  void incr_version(void)
  {
    ++m_version;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the current version number
  //////////////////////////////////////////////////////////////////////
  size_t version(void) const
  {
    return m_version;
  }

  /// @}
  /// @name Element Manipulation
  /// @{

  //////////////////////////////////////////////////////////////////////
  /// @brief Sets the element specified by the index to the provided value.
  /// @param idx GID of the element to be set.
  /// @param val The new value of the element.
  //////////////////////////////////////////////////////////////////////
  void set_element(index_type const& idx, value_type const& val)
  {
    distribution().set_element(idx, val);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the value of the element specified by the index.
  /// @param idx GID of the element to be set.
  /// @return The value of the element.
  //////////////////////////////////////////////////////////////////////
  value_type get_element(index_type const& idx) const
  {
    return distribution().get_element(idx);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns a @ref stapl::future holding the value of the element
  /// specified by the index.
  /// @param idx GID of the element to be set.
  /// @return The future holding the value of the element.
  //////////////////////////////////////////////////////////////////////
  future<value_type> get_element_split(index_type const& idx)
  {
    return distribution().get_element_split(idx);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Applies a function f to the element specified by the GID.
  /// @param gid The GID of the element.
  /// @param f The Functor to apply on the element.
  //////////////////////////////////////////////////////////////////////
  template<typename F>
  void apply_set(gid_type const& gid, F const& f)
  {
    this->distribution().apply_set(gid, f);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Applies a function f to the element specified by the GID,
  /// and returns the result.
  /// @param gid The GID of the element.
  /// @param f The Functor to apply on the element.
  /// @return The result of applying the functor to the element.
  //////////////////////////////////////////////////////////////////////
  template<typename F>
  typename F::result_type apply_get(gid_type const& gid, F const& f)
  {
    return this->distribution().apply_get(gid, f);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Applies a function f to the element specified by the GID,
  /// and returns the result.
  /// @param gid The GID of the element.
  /// @param f The Functor to apply on the element.
  /// @return The result of applying the functor to the element.
  //////////////////////////////////////////////////////////////////////
  template<typename F>
  typename F::result_type apply_get(gid_type const& gid, F const& f) const
  {
    return this->distribution().apply_get(gid, f);
  }

  /// @}

  /// @name Memory and Domain Management
  /// @{

  /// @todo vector_distribution returns domain const& which varies with this
  /// (unnecessarily?) and is confusing
  domain_type domain(void) const
  {
    return distribution().domain();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Migrates the element specified by the gid to the destination
  /// location.
  /// @param gid GID of the element to be migrated.
  /// @param destination Id of the location where the element is to be migrated.
  //////////////////////////////////////////////////////////////////////
  void migrate(gid_type const& gid, location_type destination)
  {
    incr_version();
    distribution().migrate(gid, destination);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the number of elements in the container.
  ///
  /// This method is one-sided, If other locations may be concurrently
  /// performing operations that change their local size and the effects
  /// are desired to be observed in a deterministic way, then appropriate
  /// synchronization, e.g. a fence, may be required before or after the
  /// call to size, to enforce appropriate ordering.
  //////////////////////////////////////////////////////////////////////
  size_type size(void) const
  {
    return distribution().size();
  }

  bool empty(void) const
  {
    return size() == 0;
  }

  distribution_type& distribution(void)
  {
    return *this;
  }

  distribution_type const& distribution(void) const
  {
    return *this;
  }

  distribution_type* get_distribution(void)
  {
    return &this->distribution();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return locality information about the element specified by
  ///   the gid.
  /// @return A locality_qualifier, affinity specifier, as well as this
  ///   object's handle and associated location for this affinity.
  //////////////////////////////////////////////////////////////////////
  locality_info locality(gid_type gid)
  {
    if (this->distribution().is_local(gid))
    {
      return locality_info(
        LQ_CERTAIN, get_affinity(),
        this->distribution().get_rmi_handle(),
        this->distribution().get_location_id()
      );
    }

    // else
    return this->distribution().directory().locality(gid);
   }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns true if the element specified by the GID formed from the
  /// index components provided is stored on this location, or false otherwise.
  //////////////////////////////////////////////////////////////////////
  template <typename... Indices>
  bool is_local(Indices const&... i) const
  {
    return distribution().is_local(i...);
  }

  rmi_handle::reference get_rmi_handle_reference(void)
  {
    return this->get_rmi_handle();
  }

  /// @}

  size_t get_num_locations(void) const
  {
    return distribution_type::get_num_locations();
  }

  location_type get_location_id(void) const
  {
    return distribution_type::get_location_id();
  }

  // Providing selective access to public interface of p_object (it's
  // inherited privately through the distribution.
  using distribution_type::get_rmi_handle;
  using distribution_type::advance_epoch;

}; // class container_impl

} // namspace detail


#define STAPL_CONTAINERS_REFLECT_TYPES \
  typedef detail::container_impl<C> base_t;                               \
  typedef container_traits<C>       traits_type;                          \
  STAPL_IMPORT_TYPE(typename traits_type, value_type)                     \
  STAPL_IMPORT_TYPE(typename traits_type, partition_type)                 \
  STAPL_IMPORT_TYPE(typename traits_type, mapper_type)                    \
  STAPL_IMPORT_TYPE(typename base_t, distribution_type)                   \
  STAPL_IMPORT_TYPE(typename distribution_type, directory_type)           \
  STAPL_IMPORT_TYPE(typename distribution_type, container_manager_type)


//////////////////////////////////////////////////////////////////////
/// @brief Base-class for pContainers that implements general functionality
///   needed by containers.
/// @ingroup pcf
/// @tparam C derived container class for use with CRTP.
//////////////////////////////////////////////////////////////////////
template<typename C,
         bool = is_p_object<
           typename detail::get_nested_stored_value_type<
             container_traits<C>
             >::type>::value
         >
struct container
  : public detail::container_impl<C>
{
public:
  STAPL_CONTAINERS_REFLECT_TYPES

  /// @name Constructors
  /// @{
  container(void)
  { this->advance_epoch(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with the given distribution.
  //////////////////////////////////////////////////////////////////////
  container(distribution_type const& distribution)
    : base_t(distribution)
  { this->advance_epoch(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with the given directory and
  /// base container manager.
  /// @param dir The directory for this container.
  /// @param cm The base container manager for this container.
  //////////////////////////////////////////////////////////////////////
  container(directory_type const& dir, container_manager_type const& cm)
    : base_t(dir, cm)
  { this->advance_epoch(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with the given directory,
  /// base container manager, and registry.
  /// @param dir The directory for this container.
  /// @param cm The base container manager for this container.
  /// @param reg The registry for this container.
  //////////////////////////////////////////////////////////////////////
  template <typename Reg>
  container(directory_type const& dir, container_manager_type const& cm,
            Reg const& reg)
    : base_t(dir, cm, reg)
  { this->advance_epoch(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with a given size, partition and mapper
  //////////////////////////////////////////////////////////////////////
  container(partition_type const& ps, mapper_type const& map)
    : base_t(ps, map)
  { this->advance_epoch(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with a given size, partition and mapper
  ///
  /// The @p reg parameter is used by @ref map constructors to prevent
  /// keys from being pre-registered as the container is constructed.
  //////////////////////////////////////////////////////////////////////
  template <typename Reg>
  container(partition_type const& ps, mapper_type const& map, Reg const& reg,
            typename std::enable_if<
              !detail::has_is_composed_dist_spec<Reg>::value>::type* = 0)
    : base_t(ps, map, reg)
  { this->advance_epoch(); }


  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with a given size, partition and mapper
  ///
  /// The @p reg parameter is used by @ref map constructors to prevent
  /// keys from being pre-registered as the container is constructed.
  //////////////////////////////////////////////////////////////////////
  template <typename DistSpecView, typename Reg>
  container(std::shared_ptr<DistSpecView> dist_view, Reg const& reg,
            typename std::enable_if<
              !detail::has_is_composed_dist_spec<Reg>::value>::type* = 0)
    : base_t(partition_type(dist_view), mapper_type(dist_view), reg)
  { this->advance_epoch(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with a given size, partition and mapper,
  /// constructing all elements with the default value provided.
  //////////////////////////////////////////////////////////////////////
  container(partition_type const& ps,
            mapper_type const& map,
            value_type const& default_value)
    : base_t(ps, map, default_value)
  { this->advance_epoch(); }


  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with the given distribution.
  //////////////////////////////////////////////////////////////////////
  template <typename DistSpecView>
  container(std::shared_ptr<DistSpecView> dist_view,
            typename std::enable_if<
              is_distribution_view<DistSpecView>::value &&
              !detail::has_is_composed_dist_spec<DistSpecView>::value
            >::type* = 0)
    : base_t(partition_type(dist_view), mapper_type(dist_view))
  { this->advance_epoch(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with the given distribution, constructing
  /// all elements with the default value provided.
  //////////////////////////////////////////////////////////////////////
  template <typename DistSpecView>
  container(std::shared_ptr<DistSpecView> dist_view,
            value_type const& default_value,
            typename std::enable_if<
              is_distribution_view<DistSpecView>::value &&
              !detail::has_is_composed_dist_spec<DistSpecView>::value
            >::type* = 0)
    : base_t(partition_type(dist_view), mapper_type(dist_view), default_value)
  { this->advance_epoch(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with the distribution specified by the
  /// partitions contained in @p part_cont.
  ///
  /// @todo Eliminate this constructor when a tagging infrastructure is
  /// available in the distribution_spec_view that enables specialization.
  /// The tagging infrastructure would allow further simplification of the
  /// constructors by eliminating the need for enable_if and allowing
  /// variadic forwarding of parameters.
  //////////////////////////////////////////////////////////////////////
  template <typename PartitionContainer>
  container(PartitionContainer const* const part_cont,
            typename std::enable_if<
            std::is_same<typename PartitionContainer::value_type,
              arbitrary_partition_info>::value>::type* = 0)
    : base_t(arbitrary(*part_cont), part_cont)
  { this->advance_epoch(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with the distribution specified by the
  /// partitions contained in @p part_cont, constructing all elements
  /// with the default value provided.
  ///
  /// @todo Eliminate this constructor when a tagging infrastructure is
  /// available in the distribution_spec_view that enables specialization.
  /// The tagging infrastructure would allow further simplification of the
  /// constructors by eliminating the need for enable_if and allowing
  /// variadic forwarding of parameters.
  //////////////////////////////////////////////////////////////////////
  template <typename PartitionContainer>
  container(PartitionContainer const* const part_cont,
            value_type const& default_value,
            typename std::enable_if<
            std::is_same<typename PartitionContainer::value_type,
              arbitrary_partition_info>::value>::type* = 0)
    : base_t(arbitrary(*part_cont), part_cont, default_value)
  { this->advance_epoch(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with the distribution specified by the
  /// partitions contained in @p part_cont.
  ///
  /// The @p reg parameter is used by @ref map constructors to prevent
  /// keys from being pre-registered as the container is constructed.
  ///
  /// @todo Eliminate this constructor when a tagging infrastructure is
  /// available in the distribution_spec_view that enables specialization.
  /// The tagging infrastructure would allow further simplification of the
  /// constructors by eliminating the need for enable_if and allowing
  /// variadic forwarding of parameters.
  //////////////////////////////////////////////////////////////////////
  template <typename PartitionContainer, typename Reg>
  container(PartitionContainer const* const part_cont, Reg const& reg,
            typename std::enable_if<
              std::is_same<typename PartitionContainer::value_type,
                arbitrary_partition_info>::value>::type* = 0)
    : base_t(arbitrary(*part_cont), part_cont, reg)
  { this->advance_epoch(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Base constructor for composed containers.  This constructor
  ///   is invoked for the containers at the bottom of a m level composed
  ///   container that has been constructed using distribution specifications.
  /// @param ps The partition for this container.
  /// @param map The mapper for this container.
  /// @param comp_spec Ignored because there are no nested containers to
  ///   initialize.
  //////////////////////////////////////////////////////////////////////
  template <typename DistSpecView, typename ComposedSpec>
  container(std::shared_ptr<DistSpecView> dist_view,
            ComposedSpec const& comp_spec,
            typename std::enable_if<
              is_distribution_view<DistSpecView>::value &&
              detail::has_is_composed_dist_spec<ComposedSpec>::value
            >::type* = 0)
    : base_t(partition_type(dist_view), mapper_type(dist_view))
  { this->advance_epoch(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Base constructor for composed containers.  This constructor
  ///   is invoked for the containers at the bottom of a m level composed
  ///   container that has been constructed using distribution specifications.
  /// @param ps The partition for this container.
  /// @param map The mapper for this container.
  /// @param comp_spec Ignored because there are no nested containers to
  ///   initialize.
  /// @param reg Flag to indicate that the GIDs of the container are not to be
  ///   registered.  This is used for associative containers such as @ref map.
  //////////////////////////////////////////////////////////////////////
  template <typename DistSpecView, typename ComposedSpec>
  container(std::shared_ptr<DistSpecView> dist_view,
            ComposedSpec const& comp_spec,
            boost::mpl::false_ const& reg,
            typename std::enable_if<
              is_distribution_view<DistSpecView>::value &&
              detail::has_is_composed_dist_spec<ComposedSpec>::value
            >::type* = 0)
    : base_t(partition_type(dist_view), mapper_type(dist_view), reg)
  { this->advance_epoch(); }

  /// @}
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization when this is a nested container.
/// @ingroup pcf
//////////////////////////////////////////////////////////////////////
template<typename C>
struct container<C, true>
  : public detail::container_impl<C>
{
public:
  STAPL_CONTAINERS_REFLECT_TYPES

private:
  typedef typename detail::get_nested_stored_value_type<traits_type>::type
    nested_stored_value_type;

  typedef typename detail::get_nested_initializer_type<traits_type>::type
    nested_initializer_type;

public:
  /// @name Constructors
  /// @{
  container(void)
  {
    this->advance_epoch();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with the given distribution.
  //////////////////////////////////////////////////////////////////////
  container(distribution_type const& distribution)
    : base_t(distribution)
  {
    {
      gang g;
      nested_initializer_type(false)(
        this->distribution(),
        boost::bind(boost::factory<nested_stored_value_type*>()));
      rmi_fence();
    }
    this->advance_epoch();
  }


  //////////////////////////////////////////////////////////////////////
  /// Creates a container with a given size, partition and mapper,
  /// default constructing all values.
  //////////////////////////////////////////////////////////////////////
  container(partition_type const& ps, mapper_type const& map)
    : base_t(ps, map)
  {
    {
      gang g;
      nested_initializer_type(false)(
        this->distribution(),
        boost::bind(
          boost::factory<nested_stored_value_type*>()
        )
      );
      rmi_fence();
    }
    this->advance_epoch();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with a given size, partition and mapper
  ///
  /// The @p reg parameter is used by @ref map constructors to prevent
  /// keys from being pre-registered as the container is constructed.
  //////////////////////////////////////////////////////////////////////
  container(partition_type const& ps, mapper_type const& map,
            boost::mpl::false_ const& reg)
    : base_t(ps, map, reg)
  {
    {
      gang g;
      nested_initializer_type(false)(
        this->distribution(),
        boost::bind(
          boost::factory<nested_stored_value_type*>()
        )
      );
      rmi_fence();
    }
    this->advance_epoch();
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with a given size, partition and mapper,
  ///   constructing all elements with the default value provided.
  /// @bug elements should be copy constructed with default_value instead
  ///   instead just grabbing its size.
  //////////////////////////////////////////////////////////////////////
  container(partition_type const& ps, mapper_type const& map,
            value_type const& default_value)
    : base_t(ps, map, default_value)
  {
    nested_initializer_type(true)(
      this->distribution(),
      boost::bind(
        boost::factory<nested_stored_value_type*>(), default_value.size()
      )
    );
    this->advance_epoch();
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a container with a given size, partition and mapper,
  /// constructing all elements with the default value provided.
  ///
  /// Distributes the nested containers on this location locally.
  ///
  /// @bug elements should be copy constructed with default_value instead
  ///   instead just grabbing its size.
  //////////////////////////////////////////////////////////////////////
  container(partition_type const& ps, mapper_type const& map,
            value_type const& default_value, policy::here)
    : base_t(ps, map, default_value)
  {
    {
      gang g;
      nested_initializer_type(false)(
        this->distribution(),
        boost::bind(
          boost::factory<nested_stored_value_type*>(), default_value.size()
        )
      );
      rmi_fence();
    }
    this->advance_epoch();
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Creates composed pContainers with given size-specifications,
  /// partition and mapper,
  /// Required for pC composition.
  /// @param ps The partition for this container.
  /// @param map The mapper for this container.
  /// @param dims dimensions of the internal containers.
  //////////////////////////////////////////////////////////////////////
  template <typename X, typename Y>
  container(partition_type const& ps, mapper_type const& map,
            boost::tuples::cons<X,Y> const& dims)
    : base_t(ps, map)
  {
    nested_initializer_type(true)(
      this->distribution(),
      boost::bind(
        boost::factory<nested_stored_value_type*>(),
        boost::cref(detail::factory_parameter(dims.get_tail()))
      )
    );
    this->advance_epoch();
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Creates composed pContainers with given size-specifications,
  /// partition and mapper,
  /// Required for pC composition.
  /// @param ps The partition for this container.
  /// @param map The mapper for this container.
  /// @param dims dimensions of the internal containers.
  ///
  /// Distributes the nested containers on this location locally.
  /// @todo Verify nested_initializer_type ctor shouldn't receive false.
  //////////////////////////////////////////////////////////////////////
  template <typename X, typename Y>
  container(partition_type const& ps, mapper_type const& map,
            boost::tuples::cons<X,Y> const& dims, policy::here)
    : base_t(ps, map)
  {
    {
      gang g;
      nested_initializer_type(false)(
        this->distribution(),
        boost::bind(
          boost::factory<nested_stored_value_type*>(),
          boost::cref(detail::factory_parameter(dims.get_tail()))
        )
      );
      rmi_fence();
    }
    this->advance_epoch();
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor for composed containers.  For a m level composed
  ///   container, @p sizes_view is a m-1 level composed view representing the
  ///   sizes of the nested containers.
  /// @param ps The partition for this container.
  /// @param map The mapper for this container.
  /// @param sizes_view Sizes for nested containers stored in the container.
  /// @todo replace factory functor with lambda.
  //////////////////////////////////////////////////////////////////////
  template<typename SizesView>
  container(partition_type const& ps, mapper_type const& map,
            SizesView const& sizes_view,
            typename std::enable_if<
              !detail::has_is_composed_dist_spec<SizesView>::value>::type* = 0)
    : base_t(ps, map)
  {
    typedef detail::variable_sizes_factory<
      nested_stored_value_type, SizesView, typename base_t::gid_type
    > factory_t;

    nested_initializer_type(true)(this->distribution(), factory_t(sizes_view));
    this->advance_epoch();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor for composed containers.  For a m level composed
  ///   container, @p comp_spec is a m-1 level composed specification of the
  ///   distributions of the nested containers.
  /// @param ps The partition for this container.
  /// @param map The mapper for this container.
  /// @param comp_spec Distributions for nested containers stored in the
  ///   container.
  /// @todo replace factory functor with lambda.
  //////////////////////////////////////////////////////////////////////
  template <typename DistSpecView, typename ComposedSpec>
  container(std::shared_ptr<DistSpecView> dist_view,
            ComposedSpec const& comp_spec,
            typename std::enable_if<
              is_distribution_view<DistSpecView>::value
              && detail::has_is_composed_dist_spec<ComposedSpec>::value
            >::type* = 0)
    : base_t(partition_type(dist_view), mapper_type(dist_view))
  {
    typedef detail::variable_distribution_factory<
      nested_stored_value_type,
      typename detail::strip_comp_spec_ptr<ComposedSpec>::type,
      typename base_t::gid_type>                                 factory_t;

    nested_initializer_type(true)(this->distribution(), factory_t(comp_spec));
    this->advance_epoch();
  }

  ~container(void)
  { }
  /// @}
};

#undef STAPL_CONTAINERS_REFLECT_TYPES

} // namespace stapl

#endif // STAPL_CONTAINERS_BASE_CONTAINER_HPP
