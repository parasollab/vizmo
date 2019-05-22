/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_MATRIX_FWD_HPP
#define STAPL_CONTAINERS_MATRIX_FWD_HPP

#include <stapl/utility/use_default.hpp>
#include <stapl/containers/multiarray/traversals.hpp>
#include <stapl/containers/matrix/traits/matrix_traits.hpp>

namespace stapl {

#ifdef STAPL_DOCUMENTATION_ONLY

//////////////////////////////////////////////////////////////////////
/// @brief Parallel dense matrix container.
/// @ingroup pmatrix
///
/// @tparam T Type of the stored elements in the container. T must be
/// default assignable, copyable and assignable.
/// @tparam Traversal The major of the matrix. This type must be a
/// compile-time tuple of mpl::int_. The default traversal is row_major.
/// @tparam PS Partition strategy that defines how to partition
/// the original domain into subdomains. The default partition is
/// @ref matrix_impl::block_partition.
/// @tparam M Mapper that defines how to map the subdomains produced
/// by the partition to locations. The default mapper is
///   @ref multidimensional_mapper.
/// @tparam Traits A traits class that defines customizable components
///   of matrix, such as the domain type and base container type. The
///   default traits class is @ref matrix_traits.
/// @todo Update inheritance from multiarray to pass up
///   @ref matrix_container_traits.
////////////////////////////////////////////////////////////////////////
template<typename T,
         typename Traversal = typename stapl::row_major,
         typename PS        = multiarray_impl::block_partition<Traversal>,
         typename M         = multidimensional_mapper<
                                typename PS::index_type, Traversal
                              >,
         typename Traits = use_default >
class matrix;

#else

template<typename T, typename ...OptionalParams>
class matrix;

#endif

template<typename T,
         typename Traversal = typename stapl::row_major,
         typename PS        = multiarray_impl::block_partition<Traversal>,
         typename M         = multidimensional_mapper<
                                typename PS::index_type, Traversal>>
using sparse_matrix =
  matrix<T, Traversal, PS, M,
         sparse_matrix_traits<T, Traversal, PS, M, internal_storage_tag>>;


template<typename T,
         typename Traversal = typename stapl::row_major,
         typename PS        = multiarray_impl::block_partition<Traversal>,
         typename M         = multidimensional_mapper<
                                typename PS::index_type, Traversal>>
using sparse_matrix_external =
  matrix<T, Traversal, PS, M,
         sparse_matrix_traits<T, Traversal, PS, M, external_storage_tag>>;

} // namespace stapl

#endif
