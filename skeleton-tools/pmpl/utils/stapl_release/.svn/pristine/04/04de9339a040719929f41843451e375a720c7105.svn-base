/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_MATRIX_BASE_CONTAINER_TRAITS_H
#define STAPL_CONTAINERS_MATRIX_BASE_CONTAINER_TRAITS_H

#include <mtl/mtl.h>

#include <stapl/containers/multiarray/traversals.hpp>

namespace stapl {

struct internal_storage_tag;
struct external_storage_tag;


//////////////////////////////////////////////////////////////////////
/// @brief Common traits between dense and sparse container trait classes.
/// @ingroup pmatrixTraits
/// @tparam T Type of the stored elements in the base container.
/// @tparam Traversal Major of the matrix
////////////////////////////////////////////////////////////////////////
template<typename T, typename Traversal>
struct matrix_base_container_traits_base
{
  typedef typename std::conditional<
    std::is_same<Traversal, row_major>::value,
    mtl::row_major, mtl::column_major
  >::type                                                matrix_orientation;

  typedef T                                              value_type;
  typedef Traversal                                      traversal_type;
  typedef std::integral_constant<int, 2>                 dimension_type;
  typedef std::true_type                                 contiguous_storage;
};


//////////////////////////////////////////////////////////////////////
/// @brief Default traits for the matrix base container. Specifies customizable
///   type parameters that could be changed on a per-container basis.
/// @ingroup pmatrixTraits
/// @tparam T Type of the stored elements in the base container.
/// @tparam Traversal Major of the matrix
/// @see matrix_base_container
////////////////////////////////////////////////////////////////////////
template <typename T, typename Traversal>
struct matrix_base_container_traits
  : matrix_base_container_traits_base<T, Traversal>
{
  typedef typename mtl::matrix<
    T, mtl::rectangle<>, mtl::dense<>,
    typename matrix_base_container_traits_base<
      T, Traversal>::matrix_orientation
  >::type                                                container_type;
  typedef container_type                                 container_constructor;
  typedef std::false_type                                external_storage;
};


//////////////////////////////////////////////////////////////////////
/// @brief Traits used when @ref matrix_base_container should use
///  sparse storage.
/// @ingroup pmatrixTraits
/// @tparam T Type of the stored elements in the base container.
/// @tparam Traversal Major of the matrix
/// @tparam StorageTag Specifies whether external storage is passed
///   to the underlying MTL container or whether it should allocate
///   space internally.
/// @see matrix_base_container
////////////////////////////////////////////////////////////////////////
template <typename T, typename Traversal, typename StorageTag>
struct sparse_matrix_base_container_traits
  : matrix_base_container_traits_base<T, Traversal>
{
  typedef typename std::is_same<
   StorageTag, external_storage_tag>::type               external_storage;

  typedef typename mtl::matrix<
    T, mtl::rectangle<>,
    mtl::compressed<
      int,
      std::conditional<
        external_storage::value,
        std::integral_constant<int, mtl::external>,
        std::integral_constant<int, mtl::internal>
      >::type::value,
      mtl::index_from_one
    >,
    typename matrix_base_container_traits_base<
      T, Traversal>::matrix_orientation
  >::type                                                container_type;
  typedef container_type                                 container_constructor;
};

} // namespace stapl

#endif
