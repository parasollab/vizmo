/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_BC_BASE_HPP
#define STAPL_CONTAINERS_BC_BASE_HPP

#include <stapl/containers/type_traits/is_container.hpp>
#include <stapl/containers/type_traits/define_value_type.hpp>
#include <stapl/containers/distribution/is_distribution_view.hpp>
#include <stapl/containers/distribution/composed_specification.hpp>
#include <stapl/containers/distribution/composed_specification_ptr.hpp>

namespace stapl {

namespace bc_base_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Called to conditionally wrap nested container default
///   construction with @ref container_wrapper_ref for the
///   @ref map_base_container.
//////////////////////////////////////////////////////////////////////
template<typename Value, bool = is_container<Value>::value>
struct default_construct_element
{
  static Value apply(void)
  {
    return Value();
  }
};


template<typename Value>
struct default_construct_element<Value, true>
{
  typedef typename define_value_type<Value>::type wrapper_type;

  static wrapper_type apply(void)
  {
    gang g;
    return wrapper_type(*(new Value()));
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Helper functor to identify container instances that have
/// viewbased distributions.
//////////////////////////////////////////////////////////////////////
template <typename Value, bool = has_distribution_type<Value>::value>
struct is_vbdist_container
{
  static const bool value = false;
};


template <typename Value>
struct is_vbdist_container<Value, true>
{
  static const bool value =
    is_view_based<typename Value::distribution_type::partition_type>::value;
};


//////////////////////////////////////////////////////////////////////
/// @brief Called to conditionally wrap a nested container construction
/// that accepts a distribution specification with @ref container_wrapper_ref
/// for the @ref map_base_container.
//////////////////////////////////////////////////////////////////////
template<typename Value,
         bool = is_container<Value>::value,
         bool = is_vbdist_container<Value>::value>
struct construct_vbdist_element
{
  template <typename Key>
  static Value apply(Key const&, composed_dist_spec*)
  {
    return Value();
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization used when the map value is a container, but
/// the container does not use a viewbased distribution.
//////////////////////////////////////////////////////////////////////
template<typename Value>
struct construct_vbdist_element<Value, true, false>
{
  typedef typename define_value_type<Value>::type wrapper_type;

  template <typename Key>
  static wrapper_type apply(Key const&, composed_dist_spec*)
  {
    gang g;
    return wrapper_type(*(new Value()));
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization used when the map value is a container with a
/// view-based distribution.
//////////////////////////////////////////////////////////////////////
template<typename Value>
struct construct_vbdist_element<Value, true, true>
{
  typedef typename define_value_type<Value>::type wrapper_type;

  template <typename Key>
  static wrapper_type apply(Key const& key, composed_dist_spec* comp_spec)
  {
    gang g;
    distribution_spec<> dist_spec((*comp_spec)[key]);
    g.leave();
    // The explicit access of view containers here is an argument to use
    // a class other than array_ro_view to implement view-based distributions.
    if (!dist_spec.container().container().container().explicit_locs() &&
        !dist_spec.container().container().container().level_specified())
    {
      future<rmi_handle::reference> cont_handle =
        construct<Value>(all_locations,
          detail::comp_spec_ptr<composed_dist_spec, Key>(*comp_spec, key)
        );
      return wrapper_type(*resolve_handle<Value>(cont_handle.get()));
    }
    else if (dist_spec.container().container().container().explicit_locs())
    {
      // system_view was specified with a subset of locations.
      // Create a gang over that set.
      typedef
        typename distribution_spec<>::view_container_type::view_container_type
        sys_view_t;
      typedef typename sys_view_t::view_container_type::domain_type loc_domain;
      loc_domain
        loc_dom(dist_spec.container().container().container().domain());
      if (loc_dom.contains(comp_spec->get_location_id()))
      {
        // TODO: make domset1D iterable to avoid vector.
        std::vector<location_type> locs(loc_dom.size());
        for (unsigned int i = 0, l = loc_dom.first();
            l != loc_dom.open_last(); l = loc_dom.advance(l, 1), ++i)
          locs[i] = l;
        future<rmi_handle::reference> cont_handle =
          construct<Value>(location_range(std::move(locs)),
            detail::comp_spec_ptr<composed_dist_spec, Key>(*comp_spec, key)
          );
        return wrapper_type(*resolve_handle<Value>(cont_handle.get()));
      }
      else
        return wrapper_type();
    }
    else
    {
      // distribution specification contains a tag indicating the level of
      // the system across which the new container will be distributed.
      level lvl = dist_spec.container().container().container().level_spec();

      if (lvl == current_level)
      {
        future<rmi_handle::reference> cont_handle =
          construct<Value>(comp_spec->get_rmi_handle(), all_locations,
            detail::comp_spec_ptr<composed_dist_spec, Key>(*comp_spec, key)
          );
        return wrapper_type(*resolve_handle<Value>(cont_handle.get()));
      }
      else if (lvl == lowest_level)
      {
        gang g;
        return wrapper_type(*(new Value((*comp_spec)[key])));
      }
      else
      {
        future<rmi_handle::reference> cont_handle =
          construct<Value>(lvl,
            detail::comp_spec_ptr<composed_dist_spec, Key>(*comp_spec, key)
          );
        return wrapper_type(*resolve_handle<Value>(cont_handle.get()));
      }
    }
  }
};



//////////////////////////////////////////////////////////////////////
/// @brief Called in destructor of a base container to destroy
///   heap-allocated value held by @ref container_wrapper_ref, if the
///   value type of the base container is a nested container.
//////////////////////////////////////////////////////////////////////
template<typename T, bool = is_container<T>::value>
struct cleanup_elements
{
  template<typename Storage>
  static void apply(Storage&)
  { }
};


template<typename T>
struct cleanup_elements<T, true>
{
  template<typename Storage>
  static void apply(Storage& s)
  {
    p_object_delete<T> d;
    for (auto&& wrapper : s) {
      d(wrapper.get().get_rmi_handle());
    }
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Called in destructor of @ref map_base_container to destroy
///   heap value held by @ref container_wrapper_ref, if the value
///   type of the base container is a nested container.
//////////////////////////////////////////////////////////////////////
template<typename Value, bool = is_container<Value>::value>
struct cleanup_map_elements
{
  template<typename Map>
  static void apply(Map&)
  { }
};


template<typename Value>
struct cleanup_map_elements<Value, true>
{
  template<typename Map>
  static void apply(Map& m)
  {
    p_object_delete<Value> d;
    for (auto&& val : m) {
      d(val.second.get().get_rmi_handle());
    }
  }
};

} // namespace bc_base_impl


//////////////////////////////////////////////////////////////////////
/// @brief Common base class for all base containers in the container
///   framework.  Use for single pointer type in
///   @ref base_container_ordering.
//////////////////////////////////////////////////////////////////////
struct bc_base
{
public:
  virtual ~bc_base(void) = default;

  void define_type(typer&)
  { abort("Attempting to Pack Base Container"); }

  size_t version(void) const
  { return 0; }
};

} // namespace stapl

#endif // STAPL_CONTAINERS_BASE_CONTAINER_BASE_HPP
