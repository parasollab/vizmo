/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_DISTRIBUTION_OPERATIONS_SETTABLE_HPP
#define STAPL_CONTAINERS_DISTRIBUTION_OPERATIONS_SETTABLE_HPP

#include <stapl/containers/type_traits/distribution_traits.hpp>

namespace stapl {

namespace operations {

//////////////////////////////////////////////////////////////////////
/// @brief Operations class for container distributions
/// that provides set_element.
///
/// Uses the CRTP pattern. Requires that the base container has a
/// set_element method that takes a single GID.
///
/// @tparam Derived The most derived distribution class
////////////////////////////////////////////////////////////////////////
template<typename Derived>
class settable
{
public:
  STAPL_IMPORT_TYPE(typename distribution_traits<Derived>, value_type)
  STAPL_IMPORT_TYPE(typename distribution_traits<Derived>, gid_type)

private:
  //////////////////////////////////////////////////////////////////////
  /// @brief Helper class to call @p Derived::set_element().
  ////////////////////////////////////////////////////////////////////////
  struct set_element_wf
  {
    typedef void result_type;

    value_type m_value;

    explicit set_element_wf(value_type const& val)
      : m_value(val)
    { }

    void operator()(p_object& d, gid_type const& gid) const
    {
      down_cast<Derived&>(d).set_element(gid, m_value);
    }

    void define_type(typer& t)
    {
      t.member(m_value);
    }
  };

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Update the element at a GID by replacing it with a given value.
  /// @param gid GID of the element to replace
  /// @param val The new value for the element
  //////////////////////////////////////////////////////////////////////
  void set_element(gid_type const& gid, value_type const& val)
  {
    Derived& derived_ref = static_cast<Derived&>(*this);

    STAPL_IMPORT_TYPE(typename distribution_traits<Derived>,
                      base_container_type)

    typedef void (base_container_type::* mem_fun_t)
      (gid_type const&, value_type const&);

    constexpr mem_fun_t mem_fun = &base_container_type::set_element;

    if (derived_ref.container_manager().contains_invoke(gid, mem_fun, gid, val))
      return;

    // else
    derived_ref.directory().invoke_where(set_element_wf(val), gid);
  }
}; // class settable

} // namespace operations

} // namespace stapl

#endif // STAPL_CONTAINERS_DISTRIBUTION_OPERATIONS_SETTABLE_HPP
