/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_OVERLAP_VIEW_HPP
#define STAPL_VIEWS_OVERLAP_VIEW_HPP

#include <stapl/views/segmented_view.hpp>
#include <stapl/containers/partitions/overlap.hpp>

#include <stapl/views/common_view.hpp>

#include <iostream>

namespace stapl {

template <typename View>
class overlap_view;

template <typename View>
struct view_traits<overlap_view<View>>
{
private:
  typedef segmented_view<View, overlap_partition<typename View::domain_type>>
    base_type;
  typedef view_traits<base_type> base_traits_type;

public:
  STAPL_IMPORT_TYPE(typename base_traits_type, container)
  STAPL_IMPORT_TYPE(typename base_traits_type, map_fun_gen_t)
  STAPL_IMPORT_TYPE(typename base_traits_type, composed_mf_t)
  STAPL_IMPORT_TYPE(typename base_traits_type, subview_type)
  STAPL_IMPORT_TYPE(typename base_traits_type, value_type)
  STAPL_IMPORT_TYPE(typename base_traits_type, reference)
  STAPL_IMPORT_TYPE(typename base_traits_type, map_function)
  STAPL_IMPORT_TYPE(typename base_traits_type, index_type)
  STAPL_IMPORT_TYPE(typename base_traits_type, domain_type)
};

//////////////////////////////////////////////////////////////////////
/// @brief An overlap view is a segmented view whose segments overlap
/// by some number of elements in each dimension as specified by the user.
///
/// The overlap domains are defined specifying the number of elements
/// that are overlapped to the left (@c l), the number of elements that
/// are not overlapped (@c c) and the number of elements overlapped to the
/// right (@c r). Each subdomain has size: l+c+r.
/// @par Example:
///     Domain to partition: [0..8]<br/>
///     left overlap (l): 2<br/>
///     non overlap (c): 3<br/>
///     right overlap (r): 1<br/>
///     Resulting partition: {[0..5],[3..8]}<br/>
//////////////////////////////////////////////////////////////////////
template<typename View>
class overlap_view
  : public segmented_view<View, overlap_partition<typename View::domain_type>>
{
private:
  typedef segmented_view<View, overlap_partition<typename View::domain_type>>
    base_type;

public:
  STAPL_IMPORT_TYPE(typename base_type, view_type)
  STAPL_IMPORT_TYPE(typename base_type, value_type)
  STAPL_IMPORT_TYPE(typename base_type, view_container_type)
  STAPL_IMPORT_TYPE(typename base_type, domain_type)
  STAPL_IMPORT_TYPE(typename base_type, map_func_type)
  STAPL_IMPORT_TYPE(typename base_type, map_function)
  STAPL_IMPORT_TYPE(typename base_type, partition_type)
  STAPL_IMPORT_TYPE(typename base_type, index_type)
  STAPL_IMPORT_TYPE(typename base_type, dimension_type)
  typedef typename view_traits<overlap_view>::map_fun_gen_t       map_fun_gen_t;

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructs an overlap_view over the view @c v using
  ///        the partition @c part.
  ///
  /// @param v View to partition into overlapping partitions
  /// @param part partitioner describing the overlap partition to be used
  /// @param mfg mapping function generator
  //////////////////////////////////////////////////////////////////////
  overlap_view(View const& v, partition_type const& part,
               map_fun_gen_t const& mfg=map_fun_gen_t())
    : base_type(v, part, mfg)
   { }


  //////////////////////////////////////////////////////////////////////
  /// @brief Construct an overlap_view from the components of an existing
  /// overlap_view.
  ///
  /// This constructor is invoked during construction of a @ref mix_view
  /// over the overlap view during view coarsening.
  ///
  /// @param c Container of partitioned view components
  /// @param dom Domain of the partitions
  /// @param mf Mapping function generated for the view
  //////////////////////////////////////////////////////////////////////
  overlap_view(view_container_type const& c, domain_type const& dom,
               map_func_type const& mf)
    : base_type(c, dom, mf)
  { }


  //////////////////////////////////////////////////////////////////////
  /// @brief Construct an overlap_view from the components of an existing
  /// overlap_view.
  ///
  /// This constructor is invoked during construction of a @ref mix_view
  /// over the overlap view during view coarsening.
  ///
  /// @param c Container of partitioned view components
  /// @param dom Domain of the partitions
  /// @param mf Mapping function generated for the view
  /// @param other overlap_view instance from which state can be copied
  //////////////////////////////////////////////////////////////////////
  overlap_view(view_container_type const& c, domain_type const& dom,
               map_func_type const& mf, overlap_view const& other)
    : base_type(c, dom, mf, other)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructs an overlap_view over a base container using the
  /// components of an existing overlap_view.
  ///
  /// This constructor is invoked during localization of a view. It is called
  /// from the specialization of nfast_view for mix_view of overlap_view.
  ///
  /// @param c pointer to the base container underlying the view
  /// @param dom domain of the partitions defined over the base container
  /// @param mf Mapping function generated for the view
  //////////////////////////////////////////////////////////////////////
  overlap_view(view_container_type* c, domain_type const& dom,
               const map_func_type mf=map_func_type())
    : base_type(c, dom, mf)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  //////////////////////////////////////////////////////////////////////
  void define_type(typer& t)
  {
    t.base<base_type>(*this);
  }
};


namespace view_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Functor to construct a segmented view using an overlap
///        partitioner.
/// @ingroup overlap_view
//////////////////////////////////////////////////////////////////////
template<typename View>
class overlap_view_builder
{
  typedef typename View::domain_type      domain_type;
  typedef overlap_partition<domain_type>  pover_type;

public:
  typedef overlap_view<View>              view_type;

  //////////////////////////////////////////////////////////////////////
  /// @see stapl::overlap_view
  //////////////////////////////////////////////////////////////////////
  view_type operator()(View& v, size_t c = 1, size_t l = 0, size_t r = 0)
  {
    return view_type(
      v,
      pover_type(
        domain_type(v.domain().first(), v.domain().last(), v.domain()),
        c, l, r
      )
    );
  }

  //////////////////////////////////////////////////////////////////////
  /// @see stapl::overlap_view
  //////////////////////////////////////////////////////////////////////
  view_type operator()(View const& v, size_t c=1, size_t l=0, size_t r=0)
  {
    return view_type(v, pover_type(domain_type(v.domain().first(),
                                               v.domain().last(),
                                               v.domain()),
                                   c, l, r) );
  }
};

} // namespace view_impl


//////////////////////////////////////////////////////////////////////
/// @brief Helper function to construct an overlap segmented view.
///
/// The overlap domains are defined specifying the number of elements
/// that are overlap to the left (@c l), the number of elements that
/// are not overlapped (@c c) and the number of elements overlap to the
/// right (@c r). Each subdomain has size: l+c+r.
/// @par Example:
///     Domain to partition: [0..8]<br/>
///     left overlap (l): 2<br/>
///     non overlap (c): 3<br/>
///     right overlap (r): 1<br/>
///     Resulting partition: {[0..5],[3..8]}<br/>
/// @param view View to partition.
/// @param c Number of elements not overlapped.
/// @param l Number of elements overlapped to the left.
/// @param r Number of elements overlapped to the right.
/// @return An overlap segmented view.
//////////////////////////////////////////////////////////////////////
template<typename View>
overlap_view<View>
make_overlap_view(View const& view, size_t c, size_t l=0, size_t r=0)
{
  return view_impl::overlap_view_builder<View>()(view, c, l, r);
}

//////////////////////////////////////////////////////////////////////
/// @brief Specialization for changing the container used for a
///        segmented view
//////////////////////////////////////////////////////////////////////
template<typename View,
         typename NewC >
struct cast_container_view<overlap_view<View>, NewC>
{
  typedef overlap_view<View>              part_view_t;
  typedef typename part_view_t::view_container_type  part_container_t;

  typedef overlap_view<typename cast_container_view<View,NewC>::type> type;
};

} // namespace stapl

#endif // STAPL_VIEWS_OVERLAP_VIEW_HPP
