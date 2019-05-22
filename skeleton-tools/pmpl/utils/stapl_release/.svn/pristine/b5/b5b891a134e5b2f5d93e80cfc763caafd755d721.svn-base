/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_LIST_DIRECTORY_HPP
#define STAPL_LIST_DIRECTORY_HPP

#include <stapl/containers/distribution/directory/registry.hpp>
#include <stapl/domains/indexed.hpp>


namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Implementation for the manager of the list directory.
///
/// As the GID for each element has a location, the list_manager simply
/// returns the location associated with the specified GID.
//////////////////////////////////////////////////////////////////////
struct list_manager
{
  template <typename GID>
  std::pair<location_type, loc_qual> operator()(GID const& x) const
  {
    return std::make_pair(x.m_location, LQ_CERTAIN);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief The container directory is responsible for distributed metadata for
/// GIDs. It knows in which location GIDs reside. It also provides methods to
/// invoke arbitrary functors on the location of GIDs without requiring external
/// entities to know exact locality information.
///
/// This directory differs from @ref container_directory in that it supports
/// the list container which stores location information in the GID of each of
/// its elements.
///
/// @tparam Partition The partition used by the container
/// @tparam Mapper The mapper used by the container
/// @tparam Base The base directory from which the list_directory is derived.
/// The manager type is taken from the base directory. For the list, this
/// should be the @ref list_manager.
///
/// @todo This class should be removed.  Inherit directly from directory.
//////////////////////////////////////////////////////////////////////
template<typename Partition, typename Mapper, typename Base>
class list_directory
  : public Base
{
public:
  typedef Partition                     partition_type;
  typedef Mapper                        mapper_type;
  typedef typename Base::manager_type   manager_type;
  typedef typename Base::registry_type  registry_type;

public:

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a directory without an explicit partition and mapper.
  /// @todo Remove commented block_size. How is this decided?
  //////////////////////////////////////////////////////////////////////
  list_directory(/*size_t block_size = 3*/)
    : Base(manager_type(/*block_size*/),
     registry_type(manager_type(/*block_size*/)))
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a directory with a given manager.
  ///
  /// @param manager Manager that this directory will use.
  //////////////////////////////////////////////////////////////////////
  list_directory(manager_type const& manager)
    : Base(manager, registry_type(manager))
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a directory with a given partition and mapper
  ///
  /// @note The partition and mapper are accepted for interface
  ///   consistency. They are not actually used in the construction
  ///   of the directory.
  //////////////////////////////////////////////////////////////////////
  list_directory(partition_type const&, mapper_type const&)
    : Base(manager_type(), registry_type(manager_type()))
  { }

  partition_type partition() const
  {
    return partition_type();
  }
};

} // namespace stapl

#endif // STAPL_LIST_DIRECTORY_HPP
