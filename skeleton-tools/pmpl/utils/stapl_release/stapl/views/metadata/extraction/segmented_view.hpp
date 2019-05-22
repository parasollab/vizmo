/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_METADATA_EXTRACTION_SEGMENTED_VIEW_HPP
#define STAPL_VIEWS_METADATA_EXTRACTION_SEGMENTED_VIEW_HPP

#include <stapl/views/metadata/locality_dist_metadata.hpp>
#include <stapl/views/metadata/extraction/extract_metadata.hpp>

namespace stapl {

namespace metadata {
  //////////////////////////////////////////////////////////////////////
  /// @brief Helper struct whose function operator is invoked to extract
  /// the metadata of the underlying container when the view being segmented
  /// is not an extended_view.
  ///
  /// @todo Composed slices_view instances currently require this invocation,
  /// which "hops" over the underlying view to the container. This should be
  /// removed, or the reasons for it documented better.
  //////////////////////////////////////////////////////////////////////
  template<bool IsExtended, typename Extractor, typename UnderlyingContainer>
  struct extractor_invoker
  {
    template <typename Container>
    typename Extractor::return_type* operator()(Container* ct)
    { return Extractor()(ct->get_container()); }
  };

  //////////////////////////////////////////////////////////////////////
  /// @brief Helper struct whose function operator is invoked to extract
  /// the metadata of the underlying container when the view being segmented
  /// is an extended_view.
  ///
  /// In this case the metadata of the extended_view is extracted and
  /// projected.
  //////////////////////////////////////////////////////////////////////
  template <typename Extractor, typename UnderlyingContainer>
  struct extractor_invoker<true, Extractor, UnderlyingContainer>
  {
    template <typename Container>
    typename Extractor::return_type* operator()(Container* ct)
    {
      // const_cast required because view_wrapper returns the view
      // const qualified while the container is not.
      return Extractor()(
        const_cast<UnderlyingContainer*>(ct->get_view()));
    }
  };

//////////////////////////////////////////////////////////////////////
/// @brief Functor to extract the locality metadata for a view whose
///        elements are subviews (@ref segmented_view).
///
/// @tparam Container Container associated with the view that is segmented.
///         Requires that this container reflects a type called container_type,
///         which is the view that is being segmented. This type will most
///         likely be a @ref view_impl::view_container.
//////////////////////////////////////////////////////////////////////
template<typename Container>
struct segmented_view_extractor
{
private:
  /// Type of the container whose elements are subviews. Most likely
  /// a @ref view_impl::view_container
  using container_type = Container;

  /// Metadata entries are linearized, so the index is a scalar
  using index_type = std::size_t;

  /// Type of the view that is being segmented
  using underlying_container_type =
    typename std::conditional<
      is_extended_view<typename container_type::view_container_type>::value,
      typename container_type::view_container_type,
      typename container_type::container_type>::type;

  /// Type of the extractor for the view that is being segmented
  using extractor_t = extract_metadata<underlying_container_type>;

public:
  /// Metadata container that is generated from metadata extraction of
  /// the underlying view
  using return_type = typename extractor_t::return_type;

  return_type* operator()(Container* ct) const
  {
    return extractor_invoker<
      is_extended_view<typename container_type::view_container_type>::value,
      extractor_t, underlying_container_type>()(ct);
  }
};

} // namespace metadata

} // namespace stapl

#endif // STAPL_VIEWS_METADATA_EXTRACTION_SEGMENTED_VIEW_HPP
