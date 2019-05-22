/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_MATRIX_HPP
#define STAPL_CONTAINERS_MATRIX_HPP

#include <stapl/containers/multiarray/multiarray.hpp>
#include <stapl/containers/matrix/traits/matrix_traits.hpp>
#include <stapl/containers/matrix/matrix_fwd.hpp>
#include <boost/mpl/int.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Specialization of @ref container_traits for matrix.
/// @ingroup pmatrixTraits
/// @see matrix container_traits
//////////////////////////////////////////////////////////////////////
template<typename T, typename ...OptionalParams>
struct container_traits<matrix<T, OptionalParams...> >
  : public matrix_impl::compute_matrix_traits<T, OptionalParams...>::type
{ };


template<typename T, typename ...OptionalParams>
class matrix
  : public multiarray<2, T, OptionalParams...>
{
  typedef multiarray<2, T, OptionalParams...>               base_type;

public:
  typedef typename base_type::value_type                    value_type;
  typedef typename base_type::partition_type                partition_type;
  typedef typename base_type::mapper_type                   mapper_type;

  typedef typename partition_type::domain_type              domain_type;
  typedef typename domain_type::index_type                  index_type;
  typedef typename domain_type::index_type                  gid_type;
  typedef typename domain_type::size_type                   size_type;

  typedef typename base_type::distribution_type             distribution_type;
  typedef typename distribution_type::reference             reference;

  /// @copydoc multiarray::dimension_type
  typedef std::integral_constant<int, 2>                    dimension_type;

protected:
  /// Domain used by the partition
  typedef typename base_type::partition_type::value_type    part_dom_t;

public:

  /// @name Constructors
  /// @{

  //////////////////////////////////////////////////////////////////////
  /// @brief Create an matrix with a given size and partition,
  /// default constructing all elements.
  /// @param n The size of the matrix in each dimension
  /// @param ps The partition for the container
  ////////////////////////////////////////////////////////////////////////
  matrix(size_type const& n, partition_type const& ps)
    : base_type(ps, mapper_type(ps))
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a matrix with a given size that default constructs
  /// all elements.
  /// @param sizes The size of the matrix in each dimension
  ////////////////////////////////////////////////////////////////////////
  matrix(size_type const& sizes)
    : base_type(sizes)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a matrix with a given size and mapper
  /// @param sizes The size of the matrix in each dimension
  /// @param mapper An instance of the mapper for distribution
  ////////////////////////////////////////////////////////////////////////
  matrix(size_type const& sizes, mapper_type const& mapper)
    : base_type(sizes, mapper)
  { }

  ////////////////////////////////////////////////////////////////////////
  /// @brief Create a matrix with a given mapper and partition
  /// @param partitioner The partition for the container
  /// @param mapper An instance of the mapper for distribution
  ////////////////////////////////////////////////////////////////////////
  matrix(partition_type const& partitioner, mapper_type const& mapper)
    : base_type(partitioner, mapper)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a matrix with a given size and default value
  /// default constructing all elements.
  /// @param sizes The size of the matrix in each dimension
  /// @param default_value Initial value of elements in the matrix
  ////////////////////////////////////////////////////////////////////////
  matrix(size_type const& sizes, value_type const& default_value)
    : base_type(sizes, default_value)
  { }

  ////////////////////////////////////////////////////////////////////////
  /// @brief Create a matrix with a given partition
  /// @param ps The partition for the container
  ////////////////////////////////////////////////////////////////////////
  matrix(partition_type const& ps)
    : base_type(ps, mapper_type(ps))
  { }

  /// @}
}; // class matrix

} // namespace stapl

#endif
