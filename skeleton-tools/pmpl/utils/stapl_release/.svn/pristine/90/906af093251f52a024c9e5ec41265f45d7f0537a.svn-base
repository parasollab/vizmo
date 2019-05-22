/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef BOOST_PP_IS_ITERATING

#ifndef STAPL_VIEWS_STENCIL_VIEW_HPP
#define STAPL_VIEWS_STENCIL_VIEW_HPP

#include <stapl/utility/tuple.hpp>
#include <stapl/containers/type_traits/dimension_traits.hpp>
#include <stapl/views/periodic_boundary_view.hpp>
#include <stapl/views/segmented_view.hpp>
#include <stapl/containers/partitions/overlap.hpp>
#include <stapl/containers/partitions/ndim_partition.hpp>

#include <stapl/views/metadata/projection/stencil.hpp>
#include <stapl/views/metadata/projection/generic.hpp>
#include <stapl/views/metadata/coarsening_traits.hpp>

#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/repetition/enum.hpp>

namespace stapl {

namespace detail {

template<std::size_t D>
struct make_stencil_helper;

} // namespace detail

//////////////////////////////////////////////////////////////////////
/// @brief Helper function that creates a stencil view, where each element
///        of the stencil view is a subview of the original view of
///        the point and its adjacent neighbors.
///
///        For 2D, this creates a 9-point stencil and for 3D this creates
///        a 27-point stencil.
///
///        This essentially creates the view
///          make_overlap_view(periodic_boundary_view(vw), 1, 1, 1)
///
/// @param vw The original view
//////////////////////////////////////////////////////////////////////
template<std::size_t D, typename View>
auto make_stencil_view(View const& vw)
  -> decltype(detail::make_stencil_helper<D>::apply(vw))
{
  return detail::make_stencil_helper<D>::apply(vw);
}

#define BOOST_PP_ITERATION_LIMITS (1, MAX_NUM_DIM)
#define BOOST_PP_FILENAME_1       "stapl/views/stencil_view.hpp"
#include BOOST_PP_ITERATE()


} // namespace stapl

#endif // STAPL_VIEWS_STENCIL_VIEW_HPP

#else // BOOST_PP_IS_ITERATING

#define this_iteration BOOST_PP_ITERATION()
#define OVERLAP_part(z,i,var) overlap_partition<indexed_domain<size_t> >
#define CREATE_part(z,i,var) typename tuple_element<i, partitions_type>::type  \
                               (pbv.domain().template get_domain<i>(), 1, 1, 1)

template<typename View>
using BOOST_PP_CAT(BOOST_PP_CAT(stencil_, this_iteration), d_t) =
  segmented_view<
    periodic_boundary_view<View>,
    nd_partition<
      stapl::tuple<BOOST_PP_ENUM(this_iteration, OVERLAP_part, ~)>
    >
  >;

template<typename View, typename D, typename F>
using BOOST_PP_CAT(BOOST_PP_CAT(stencil_, this_iteration), d_aux_t) =
  multiarray_view<
    view_impl::view_container<
      periodic_boundary_view<View>,
      nd_partition<
        stapl::tuple<BOOST_PP_ENUM(this_iteration, OVERLAP_part, ~)>
      >,
      use_default,
      use_default
    >,
    D,
    F
  >;

namespace detail {

template<>
struct make_stencil_helper<this_iteration>
{
  template<typename View>
  static BOOST_PP_CAT(BOOST_PP_CAT(stencil_, this_iteration), d_t)<View>
  apply(View const& vw)
  {
    typedef BOOST_PP_CAT(BOOST_PP_CAT(stencil_, this_iteration), d_t)<View>
      stencil_t;
    typedef typename stencil_t::partition_type partition_type;
    typedef typename partition_type::partitions_type partitions_type;

    auto pbv = make_periodic_boundary_view(vw);

    partition_type part(
      std::make_tuple(BOOST_PP_ENUM(this_iteration, CREATE_part, ~))
    );

    return stencil_t(pbv, part);
  }
};

} // namespace detail


template<typename C>
struct coarsening_traits<
  BOOST_PP_CAT(BOOST_PP_CAT(stencil_, this_iteration), d_t)<C>
>
{
  typedef BOOST_PP_CAT(BOOST_PP_CAT(stencil_, this_iteration), d_t)<
    C
  > view_type;

  template<typename P>
  struct construct_projection
  {
    typedef stencil_metadata_projection<const view_type, P> type;
  };
};

template<typename C, typename D, typename F>
struct coarsening_traits<
  BOOST_PP_CAT(BOOST_PP_CAT(stencil_, this_iteration), d_aux_t)<C, D, F>
>
{
  typedef BOOST_PP_CAT(BOOST_PP_CAT(stencil_, this_iteration), d_aux_t)<
    C, D, F
  > view_type;

  template<typename P>
  struct construct_projection
  {
    typedef strided_stencil_metadata_projection<const view_type, P> type;
  };
};

#undef CREATE_PART
#undef OVERLAP_part
#undef this_iteration

#endif // BOOST_PP_IS_ITERATING
