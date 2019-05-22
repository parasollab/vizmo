/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_RUNTIME_UTILITY_OBJECT_TYPE_REGISTRY_HPP
#define STAPL_RUNTIME_UTILITY_OBJECT_TYPE_REGISTRY_HPP

#include "../exception.hpp"
#include <typeinfo>
#include <type_traits>
#include <boost/unordered/unordered_map.hpp>

namespace stapl {

namespace runtime {

//////////////////////////////////////////////////////////////////////
/// @brief Object registry that retains type information.
///
/// This class contains each registered object's address, type through @c typeid
/// and the epoch it was registered in.
///
/// @ingroup runtimeUtility
//////////////////////////////////////////////////////////////////////
class object_type_registry
{
private:
  //////////////////////////////////////////////////////////////////////
  /// @brief Information about a specific object registration.
  //////////////////////////////////////////////////////////////////////
  struct info_type
  {
    std::type_info const& class_type;

    template<typename T>
    explicit info_type(T*) noexcept
    : class_type(typeid(T))
    { }
  };

  /// Using Boost implementation over STL to avoid allocation at construction.
  typedef boost::unordered_map<void*, info_type> container_type;

  container_type m_registry;

public:
  ~object_type_registry(void)
  {
    if (!m_registry.empty())
      STAPL_RUNTIME_ERROR("There are p_objects registered.");
  }

  bool empty(void) const noexcept
  { return m_registry.empty(); }

  template<typename T>
  bool insert(T* const p)
  {
    return m_registry.emplace(std::piecewise_construct,
                              std::forward_as_tuple(p),
                              std::forward_as_tuple(p)).second;
  }

  template<typename T>
  bool check_type(void* const p) const noexcept
  {
    typedef typename std::conditional<
              std::is_base_of<p_object, T>::value, p_object, T
            >::type U;
    auto it = m_registry.find(p);
    if (it==m_registry.end())
      STAPL_RUNTIME_ERROR("p_object is not registered.");
    return (it->second.class_type==typeid(U));
  }

  bool erase(void* const p)
  { return (m_registry.erase(p)==1); }
};

} // namespace runtime

} // namespace stapl

#endif
