/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

// Coarsen partition based on locality information

#ifndef STAPL_VIEWS_METADATA_COARSEN_PARTITION_LOCALITY_HPP
#define STAPL_VIEWS_METADATA_COARSEN_PARTITION_LOCALITY_HPP

#include <stapl/views/metadata/extraction/extract_metadata.hpp>
#include <stapl/views/metadata/coarsen_view.hpp>

namespace stapl {

namespace coarsen_partition_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Helper functor used to coarsen metadata based on locality
///        for the given @c view.
///
/// @todo Function operator should return a shared_ptr.
//////////////////////////////////////////////////////////////////////
template<typename View>
class coarsen_partition
{
  using extractor_type = metadata::extract_metadata<View>;

public:
  using return_type = typename extractor_type::return_type;

  static return_type* apply(View* view)
  {
    return extractor_type()(view);
  }
};

} // namespace coarsen_partition_impl

} // namespace stapl

#endif // STAPL_VIEWS_METADATA_COARSEN_PARTITION_LOCALITY_HPP
