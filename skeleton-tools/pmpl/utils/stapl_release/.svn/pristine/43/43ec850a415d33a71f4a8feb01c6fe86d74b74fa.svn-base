/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_MIX_VIEW_HPP
#define STAPL_VIEWS_MIX_VIEW_HPP

#include <type_traits>

#include <stapl/views/type_traits/upcast.hpp>
#include <stapl/runtime.hpp>
#include <stapl/utility/loc_qual.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Defines a view that has the same behavior of the given
///        @c View type, but uses a base container as underlying
///        container to provide fast access over the data.
///
/// This view is constructed when the task is going to be executed in
/// this location and all the views required for the task access their
/// data locally.
/// @todo Rename to fast_view.
//////////////////////////////////////////////////////////////////////
template<typename View>
struct nfast_view
  : public cast_container_view<
      typename View::base_type, typename View::component_type
    >::type
{
  typedef cast_container_view<
    typename View::base_type,
    typename View::component_type
  >                                                 new_view_info;

  typedef typename new_view_info::type              base_type;

  nfast_view(View const& v)
    : base_type(*(v.get_component()), v.domain(), new_view_info().mapfunc(v))
  { }
}; // struct nfast_view


template<typename View,typename Info, typename CID>
class mix_view;

template <typename C,typename PS,typename MFG,typename SV>
class segmented_view;

template <typename View>
class overlap_view;

namespace view_impl {

template <typename C,typename PS,typename MFG,typename SV>
class view_container;

}


template<typename V, typename... Params>
class multiarray_view;

//////////////////////////////////////////////////////////////////////
/// @brief Specialization to construct a fast_view over a segmented view.
//////////////////////////////////////////////////////////////////////
template<typename C, typename PS, typename MFG,
         typename SVC, typename Info, typename CID>
struct nfast_view<mix_view<segmented_view<C,PS,MFG,SVC>,Info,CID> >
  : public cast_container_view<
      segmented_view<C,PS,MFG,SVC>,
      typename mix_view<segmented_view<C,PS,MFG,SVC>, Info, CID>::component_type
    >::type
{
  typedef segmented_view<C,PS,MFG,SVC>                   part_view_t;
  typedef mix_view<part_view_t,Info,CID>                view_base_type;
  typedef typename view_base_type::component_type       component_type;

  typedef typename cast_container_view<
    part_view_t, component_type
  >::type                                               base_type;

  typedef typename base_type::view_container_type       pc_type;

  nfast_view(view_base_type const& v)
    : base_type(new pc_type(v.get_container(),v.get_component()),
              v.domain(),
              v.mapfunc())
  { }
};

//////////////////////////////////////////////////////////////////////
/// @brief Specialization to construct a fast_view over an overlap view.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Info, typename CID>
struct nfast_view<mix_view<overlap_view<View>,Info,CID> >
  : public cast_container_view<
      overlap_view<View>,
      typename mix_view<overlap_view<View>, Info, CID>::component_type
    >::type
{
  typedef overlap_view<View>                            part_view_t;
  typedef mix_view<part_view_t,Info,CID>                view_base_type;
  typedef typename view_base_type::component_type       component_type;

  typedef typename cast_container_view<
    part_view_t, component_type
  >::type                                               base_type;

  typedef typename base_type::view_container_type       pc_type;

  nfast_view(view_base_type const& v)
    : base_type(new pc_type(v.get_container(),v.get_component()),
              v.domain(),
              v.mapfunc())
  { }
};

// disable fast views for now
template<typename C, typename PS, typename MFG, typename SV, typename Info,
         typename CID, typename... MParams>
struct nfast_view<mix_view<
  multiarray_view<view_impl::view_container<C,PS,MFG,SV>,MParams...> ,Info,
  CID> >
  : public multiarray_view<view_impl::view_container<C,PS,MFG,SV>,MParams...>
{
  typedef multiarray_view<view_impl::view_container<
    C,PS,MFG,SV
  >,MParams...> base_type;

  template<typename V>
  nfast_view(V const& v)
    : base_type(v)
  { }
};



namespace view_impl {

template <typename T>
struct helper_get_container
{
  static
  typename T::view_container_type&
  apply(T const& v)
  {
    return v.container();
  }
};


template <template <typename,typename,typename,typename> class V,
          typename C, typename A, typename D, typename F, typename Derived>
struct helper_get_container<V<proxy<C, A>, D, F, Derived> >
{
  typedef proxy<C, A> container_type;

  static
  container_type* apply(V<proxy<C, A>, D, F, Derived> const& v)
  {
    return new container_type(v.container());
  }
};

} // namespace view_impl

//////////////////////////////////////////////////////////////////////
/// @brief Defines a view that behaves as the given @c View type but
///        has information about data locality specified by the given
///        locality information (@c Info).
///
/// @tparam View Type of view that this view is based on.
/// @tparam Info Locality information type.
/// @tparam CID Type of the subview index
//////////////////////////////////////////////////////////////////////
template<typename View, typename Info, typename CID>
class mix_view
  : public View
{
public:
  typedef typename View::domain_type                       domain_type;
  typedef typename View::map_func_type                     map_func_type;

  typedef typename std::remove_pointer<
    typename Info::component_type
  >::type                                                  component_type;

  typedef View                                             base_type;
  typedef CID                                              index_type;

  /// @brief Defines the type of the resulting view when the base
  ///        container is used instead of the @c View's underlying
  ///        container.
  typedef nfast_view<mix_view>                             fast_view_type;

  typedef typename is_container<
    typename View::value_type
  >::type                                                  disable_localization;

private:
  index_type      m_id;
  Info            m_md_info;

public:
  mix_view(void)
    : m_id(),
      m_md_info()
  { }

  mix_view(mix_view const& other)
    : View(other),
      m_id(),
      m_md_info(other.m_md_info)
  { }

  mix_view(mix_view const& other, typename View::view_container_type* vcont)
    : View(vcont, other.domain(), other.mapfunc()),
      m_id(other.m_id),
      m_md_info(other.m_md_info)
  { }

  mix_view(typename View::view_container_type* vcont,
           domain_type const& dom,
           map_func_type mfunc,
           index_type const& id,
           Info const& md_info)
    : View(*vcont, dom, mfunc),
      m_id(id),
      m_md_info(md_info)
  { }

  mix_view(View const& vcont,
           domain_type const& dom,
           map_func_type mfunc,
           index_type const& vid,
           Info const& md_info)
    : View(view_impl::helper_get_container<View>::apply(vcont), dom, mfunc),
      m_id(vid),
      m_md_info(md_info)
  { }

  mix_view(base_type const& base_view,
           index_type const& vid,
           Info const& md_info)
    : View(base_view),
      m_id(vid),
      m_md_info(md_info)
  { }

  component_type* get_component(void) const
  {
    return m_md_info.component();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Determine if the data that this view references is local
  ///        (if the methods invoked have the the same location as
  ///        the referenced data).
  ///
  /// @todo Currently there is not specialization for comparing
  ///       locations on different gangs.
  //////////////////////////////////////////////////////////////////////
  bool is_local(void) const
  {
    typedef typename is_p_object<
              typename View::view_container_type
            >::type has_gethandle_t;

    return is_local_helper(has_gethandle_t());
  }

  index_type get_id(void) const
  {
    return m_id;
  }

  Info const& get_metadata(void) const
  {
    return m_md_info;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the location where the associated data is stored,
  ///        qualified based on the validity of the component.
  ///
  /// @todo Remapping of locations on different gangs is needed here.
  //////////////////////////////////////////////////////////////////////
  std::pair<location_type, loc_qual>
  get_preferred_location(void) const
  {
    if (m_md_info.get_component() != nullptr)
      return std::make_pair(m_md_info.location(), LQ_CERTAIN);
    else
      return std::make_pair(m_md_info.location(), LQ_DONTCARE);
  }

  void define_type(typer& t)
  {
    t.base<base_type>(*this);
    t.member(m_id);
    t.member(m_md_info);
  }

private:
  /////////////////////////////////////////////////////////////////////
  /// @brief Specialization when the container is a p_object.
  /// @todo Check of container as the view element can be statically resolved.
  //////////////////////////////////////////////////////////////////////
  bool is_local_helper(std::true_type) const
  {
    if (is_nested_container<typename View::value_type>::value)
      return false;

    const affinity_tag aff_tag = get_affinity();

    return ((this->container().get_location_id() == m_md_info.location())
            && (m_md_info.component() != nullptr)
            && (aff_tag == m_md_info.affinity()));
  }

  /////////////////////////////////////////////////////////////////////
  /// @brief General case, based on the validity of the component
  ///        reference.
  //////////////////////////////////////////////////////////////////////
  bool is_local_helper(std::false_type) const
  {
    if (is_container<typename View::value_type>::value)
      return false;

    const affinity_tag aff_tag = get_affinity();

    return (m_md_info.component() != nullptr)
            && (aff_tag == m_md_info.affinity());
  }
}; // class mix_view

} // namespace stapl

#endif // STAPL_VIEWS_MIX_VIEW_HPP
