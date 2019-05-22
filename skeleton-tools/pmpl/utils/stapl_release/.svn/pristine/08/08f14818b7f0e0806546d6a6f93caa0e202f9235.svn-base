/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_DISTRIBUTION_IS_DISTRIBUTION_VIEW_HPP
#define STAPL_CONTAINERS_DISTRIBUTION_IS_DISTRIBUTION_VIEW_HPP

#include <stapl/views/system_view.hpp>

namespace stapl {

template <typename V, bool is_vw = is_view<V>::value>
struct is_distribution_view
  : public std::false_type
{ };

template <typename V>
struct is_distribution_view<V, true>
  : public is_distribution_view<typename V::view_container_type>
{ };

template <typename V>
struct is_distribution_view<V, false>
  : public std::is_same<V, dist_view_impl::system_container>
{ };


template <typename DistributionView, typename PartitionInfoContainer>
struct view_based_partition;

template <typename DistributionView>
struct view_based_mapper;

template <typename T>
struct is_view_based
  : public boost::mpl::false_
{ };

template <typename DistributionView, typename PartitionInfoContainer>
struct is_view_based<
         view_based_partition<DistributionView, PartitionInfoContainer>>
  : public boost::mpl::true_
{ };

template <typename DistributionView>
struct is_view_based<view_based_mapper<DistributionView>>
  : public boost::mpl::true_
{ };

} // namespace stapl

#endif // STAPL_CONTAINERS_DISTRIBUTION_IS_DISTRIBUTION_VIEW_HPP
