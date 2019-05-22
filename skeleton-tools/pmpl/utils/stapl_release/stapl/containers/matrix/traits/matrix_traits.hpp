/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_MATRIX_TRAITS_HPP
#define STAPL_CONTAINERS_MATRIX_TRAITS_HPP

#include <stapl/domains/domains.hpp>

#include <stapl/containers/distribution/distribution.hpp>

#include <stapl/containers/iterators/container_accessor.hpp>
#include <stapl/containers/iterators/container_iterator.hpp>

#include <stapl/containers/distribution/operations/base.hpp>
#include <stapl/containers/distribution/operations/random_access.hpp>

#include <stapl/containers/distribution/directory/container_directory.hpp>
#include <stapl/containers/mapping/mapper.hpp>

#include <stapl/containers/matrix/base_container.hpp>
#include <stapl/containers/matrix/traits/base_container_traits.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Common traits between dense and sparse container trait classes.
/// @ingroup pmatrixTraits
/// @tparam T Type of the stored elements in the container.
/// @tparam Traversal The major of the multiarray.
/// @tparam P Partition strategy that defines how to partition
/// the original domain into subdomains.
/// @tparam M Mapper that defines how to map the subdomains produced
/// by the partition to locations.
//////////////////////////////////////////////////////////////////////
template<class T, class Traversal, class P, class M>
struct matrix_traits_base
{
  typedef T                                             value_type;
  typedef Traversal                                     traversal_type;
  typedef P                                             partition_type;
  typedef M                                             mapper_type;
  typedef typename P::value_type                        domain_type;
  typedef typename domain_type::gid_type                gid_type;
  typedef typename domain_type::index_type              index_type;
  typedef typename P::index_type                        cid_type;
  typedef typename domain_type::size_type               size_type;

  typedef container_directory<
    partition_type, mapper_type
  >                                                     directory_type;

  //////////////////////////////////////////////////////////////////////
  /// @brief Metafunction to compute the distribution type based on a
  /// container type.
  /// @tparam C Type of the container.
  //////////////////////////////////////////////////////////////////////
  template <typename C>
  struct construct_distribution
  {
    typedef distribution<C, operations::base, operations::random_access> type;
  };
};


//////////////////////////////////////////////////////////////////////
/// @brief Default traits for the matrix container. Specifies customizable
/// type parameters that could be changed on a per-container basis.
/// @ingroup pmatrixTraits
///
/// @tparam T Type of the stored elements in the container.
/// @tparam Traversal The major of the multiarray.
/// @tparam P Partition strategy that defines how to partition
/// the original domain into subdomains.
/// @tparam M Mapper that defines how to map the subdomains produced
/// by the partition to locations.
////////////////////////////////////////////////////////////////////////
template<class T, class Traversal, class P, class M>
struct matrix_traits
  : matrix_traits_base<T, Traversal, P, M>
{
  typedef matrix_base_container<
    T, typename P::value_type, typename P::index_type,
    matrix_base_container_traits<T, Traversal>
  >                                                     base_container_type;

  typedef container_manager_multidimensional<
    base_container_type
  >                                                     container_manager_type;
}; // struct matrix_traits


//////////////////////////////////////////////////////////////////////
/// @brief Traits used when the matrix should use sparse storage.
/// @ingroup pmatrixTraits
/// @tparam T Type of the stored elements in the container.
/// @tparam Traversal The major of the multiarray.
/// @tparam P Partition strategy that defines how to partition
/// the original domain into subdomains.
/// @tparam M Mapper that defines how to map the subdomains produced
/// by the partition to locations.
/// @tparam StorageTag Specifies whether external storage is passed
///   to the underlying MTL container or whether it should allocate
///   space internally.
//////////////////////////////////////////////////////////////////////
template<class T, class Traversal, class P, class M, typename StorageTag>
struct sparse_matrix_traits
  : matrix_traits_base<T, Traversal, P, M>
{
  typedef matrix_base_container<
    T, typename P::value_type, typename P::index_type,
    sparse_matrix_base_container_traits<
      T, Traversal, StorageTag>
  >                                                     base_container_type;

  typedef container_manager_multidimensional<
    base_container_type
  >                                                     container_manager_type;
};


namespace matrix_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Metafunction to compute traits class for @ref matrix
///   container. Primary template should not be instantiated.  Partial
///   specialization used to handle dependence of @p partition and
///   @p mapper types on @p traversal.
/// @ingroup pmatrixTraits
//////////////////////////////////////////////////////////////////////
template<typename T, typename ...OptionalParams>
struct compute_matrix_traits;


//////////////////////////////////////////////////////////////////////
/// @brief Specialization when none of the optional parameters are
///   provided at template instantiation.  Use @ref matrix_traits,
///   with default partition and mapper argument types.
/// @ingroup pmatrixTraits
//////////////////////////////////////////////////////////////////////
template<typename T>
struct compute_matrix_traits<T>
{
private:
  typedef typename stapl::row_major                            traversal_t;
  typedef multiarray_impl::block_partition<traversal_t>        partition_t;
  typedef multidimensional_mapper<
    typename partition_t::index_type, traversal_t
  >                                                            mapper_t;

public:
  typedef matrix_traits<T, traversal_t, partition_t, mapper_t> type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization when only the @p traversal optional
///   parameter is specified.
/// @ingroup pmatrixTraits
//////////////////////////////////////////////////////////////////////
template<typename T, typename Traversal>
struct compute_matrix_traits<T, Traversal>
{
private:
  typedef multiarray_impl::block_partition<Traversal>         partition_t;
  typedef multidimensional_mapper<
    typename partition_t::index_type, Traversal
  >                                                           mapper_t;

public:
  typedef matrix_traits<T, Traversal, partition_t, mapper_t>  type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization when only the @p traversal and @p partition
///   optional parameters are defined.
/// @ingroup pmatrixTraits
//////////////////////////////////////////////////////////////////////
template<typename T, typename Traversal, typename Partition>
struct compute_matrix_traits<T, Traversal, Partition>
{
private:
  typedef multidimensional_mapper<
    typename Partition::index_type, Traversal
  >                                                         mapper_t;

public:
  typedef matrix_traits<T, Traversal, Partition, mapper_t>  type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization when only the @p traversal, @p partition and
///   @p Mapper optional parameters are defined.
/// @ingroup pmatrixTraits
//////////////////////////////////////////////////////////////////////
template<typename T, typename Traversal, typename Partition, typename Mapper>
struct compute_matrix_traits<T, Traversal, Partition, Mapper>
{
  typedef matrix_traits<T, Traversal, Partition, Mapper> type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization when traits is actually passed at class
//    template instantiation. Use passed type.
/// @ingroup pmatrixTraits
//////////////////////////////////////////////////////////////////////
template<typename T,typename Traversal, typename Partition,
         typename Mapper, typename Traits>
struct compute_matrix_traits<T, Traversal, Partition, Mapper, Traits>
{
  typedef Traits type;
};

} // namespace matrix_impl

} // namespace stapl

#endif // STAPL_CONTAINERS_MATRIX_TRAITS_HPP
