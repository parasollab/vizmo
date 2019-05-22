/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_SEGMENTED_VIEW_HPP
#define STAPL_VIEWS_SEGMENTED_VIEW_HPP

#include <stapl/containers/partitions/balanced.hpp>
#include <stapl/views/array_ro_view.hpp>
#include <stapl/views/mapping_functions/mapping_functions.hpp>
#include <stapl/views/type_traits/upcast.hpp>
#include <stapl/views/operations/paragraph_requirements.hpp>
#include <stapl/views/segmented_view_base.hpp>
#include <stapl/views/metadata/extraction/segmented_view.hpp>
#include <stapl/utility/use_default.hpp>

#include <stapl/containers/type_traits/dimension_traits.hpp>
#include <stapl/views/multiarray_view.hpp>

#include <stapl/views/common_view.hpp>
#include <stapl/views/type_traits/upcast.hpp>

#include <iostream>
#include <type_traits>

namespace stapl {

namespace view_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Functor used to create a subview for a given index in segmented_view
///
/// @tparam SV The subview type that will be created
//////////////////////////////////////////////////////////////////////
template<typename SV>
struct default_subview_creator
{
  using type = SV;

  default_subview_creator() {}

  // Metafunction to recast this subview creator with a new subview type
  template<typename NewSV>
  struct with_subview { using type = default_subview_creator<NewSV>; };

  template<typename Container, typename Index, typename Partitioner,
           typename ViewMF, typename MFG>
  type operator()(Container* cnt, Index const& index, Partitioner const& part,
                  ViewMF const& view_mf, MFG const& mfg) const
  {
    using mf_creator = compose_func<ViewMF, typename MFG::mapfunc_type>;

    auto const& mf = mf_creator::apply(view_mf, mfg[index]);
    auto const domain = part[index];

    return type{*cnt, domain, mf};
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Define a container of views, specified by a partition function
///        and a mapping function generator
///
/// The partition function defines the domain of each view in the
/// container and the mapping function generator, the mapping function
/// to use by each view
/// @tparam View which every view in the container, reference as
///         underneath container
/// @tparam P partition functor type
/// @tparam MFG Mapping function generator type
/// @tparam SVC Functor that creates subviews
/// @todo Determine proper type for const_reference.  This requires
/// determining how to make a view's access methods return const references.
//////////////////////////////////////////////////////////////////////
template <typename View, typename P, typename MFG, typename SVC>
class view_container :
  public common_view,
  public p_object,
  public view_container_base
{
  typedef typename container_traits<
    view_container>::map_fun_gen_t                  map_fun_gen_t;

  typedef typename container_traits<
    view_container>::subview_t                      subview_t;

  typedef typename container_traits<
    view_container>::subview_creator_type           subview_creator_type;


  typedef typename View::domain_type                view_domain_type;
  typedef typename View::map_func_type              view_map_func_type;

  typedef typename subview_t::map_func_type         subview_map_func_type;

private:
  size_t m_version;

public:
  typedef typename P::index_type                    index_type;
  typedef typename View::view_container_type        view_view_container_type;
  typedef typename View::view_container_type        container_type;

  typedef size_t                                    gid_type;
  typedef View                                      view_container_type;
  typedef subview_t                                 value_type;

  typedef metadata::segmented_view_extractor<
    view_container
  >                                                 loc_dist_metadata;

  typedef value_type                                reference;
  typedef const value_type                          const_reference;
  typedef P                                         partition_type;
  typedef map_fun_gen_t                             mapfunc_generator_type;
  typedef typename P::domain_type                   domain_type;
  typedef void traversal_type;

  /// the type to repesent the size in each dimension
  typedef typename P::index_type                    dimensions_type;

  /// integral constant representing the number of dimensions
  typedef typename dimension_traits<dimensions_type>::type dimension_type;

  // Members
  view_view_container_type*       m_container;
  view_map_func_type              m_view_map_func;
  View const*                     m_view;
  partition_type                  m_partitioner;
  map_fun_gen_t                   m_map_func_gen;


  void define_type(typer& t)
  {
    t.member(m_container);
    t.member(m_view_map_func);
    t.member(m_view);
    t.member(m_partitioner);
    t.member(m_map_func_gen);
    t.member(m_version);
  }

  view_container(view_container const& other)
    : p_object(),
      m_version(0),
      m_container(other.m_container),
      m_view_map_func(other.m_view_map_func),
      m_view(other.m_view),
      m_partitioner(other.m_partitioner),
      m_map_func_gen(other.m_map_func_gen)
  {
// not exercised by test suite as of 8/20/2013
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Copy constructor that replaces the container for a new container.
  /// @note Constructor used during the fast view transformation.
  /// @bug Failing to initialize the m_view member in this constructor is
  /// suspicious.  Determine if this constructor can be removed or how m_view
  /// can be initialized so the view can be held by copy instead of by pointer.
  //////////////////////////////////////////////////////////////////////
  template<typename Other>
  view_container(Other* other, view_view_container_type* new_cont)
    : p_object(),
      m_version(0),
      m_container(new_cont),
      m_view_map_func(other->m_view_map_func),
      m_view(nullptr),
      m_partitioner(other->m_partitioner),
      m_map_func_gen(other->m_map_func_gen)
  { }


  //////////////////////////////////////////////////////////////////////
  /// @brief Default constructor, where the underneath container, the
  ///        partition functor and mapping function generator are
  ///        specified.
  ///
  /// @param container is the view from the subviews are created.
  /// @param part domain partition.
  /// @param mfg mapping function generator.
  //////////////////////////////////////////////////////////////////////
  view_container(view_container_type const& container,
            partition_type const& part,
            mapfunc_generator_type const& mfg)
    : p_object(),
      m_version(0),
      m_container(container.get_container()),
      m_view_map_func(container.mapfunc()),
      m_view(&container),
      m_partitioner(part),
      m_map_func_gen(mfg)
  { }

  view_container_type const* get_view(void)
  {
    return m_view;
  }

  view_container_type const& view(void)
  {
    stapl_assert(m_view != nullptr,"view_container has null view.");
    return *m_view;
  }

  view_view_container_type* get_container(void)
  {
    return m_container;
  }

  partition_type const& partition(void) const
  {
    return m_partitioner;
  }

  mapfunc_generator_type const& mapfunc_generator(void) const
  {
    return m_map_func_gen;
  }

  value_type operator[](index_type index) const
  {
    return this->get_element(index);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief  increment the version number
  //////////////////////////////////////////////////////////////////////
  void incr_version(void)
  {
    ++m_version;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the current version number
  //////////////////////////////////////////////////////////////////////
  size_t version(void) const
  {
    return m_version;
  }

 //////////////////////////////////////////////////////////////////////
  /// @brief Return the view associated with the position index (linear
  ///        position) The returned view mapping function is the
  ///        function composition of top view mapping function and the
  ///        mapping function generated for the mapping function
  ///        generator.
  ///
  /// @param index to an element
  /// @return the generated subview at position index
  //////////////////////////////////////////////////////////////////////
  value_type get_element(index_type index) const
  {
    const subview_creator_type svc;
    return svc(
      m_container, index, m_partitioner, m_view_map_func, m_map_func_gen
    );
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the locality information of the segment containing the
  /// index provided.
  /// @param idx GID of a view element whose locality is being requested
  /// @return The locality information of the segment containing the GID.
  ///
  /// @todo Extend the computation to return the locality of all elements
  /// in the segment when locality information allows for multiple locations
  /// @todo Eliminate the gang construction and replace the call to get_element
  /// with a call to retrieve the metadata from the location that holds it. This
  /// will eliminate the construction of a p_object and blocking communication
  /// in cases where the segement is not local.
  //////////////////////////////////////////////////////////////////////
  locality_info locality(index_type const& idx) const
  {
    gang g;
    auto subvw = get_element(idx);
    auto first = subvw.domain().first();

    return subvw.locality(first);

  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Applies the provided function the the value referenced
  ///        for the given index returning the resulting value
  ///
  /// @param index of element to apply the function
  /// @param f function to apply
  /// @return result of evaluating the function f on the value
  ///         referenced for the index
  //////////////////////////////////////////////////////////////////////
  template<class Functor>
  typename Functor::result_type
  apply_get(index_type index, Functor const& f)
  {
    return f(get_element(index));
  }

  size_t size(void) const
  {
    return this->m_partitioner.size();
  }

  domain_type domain(void) const
  {
    return this->m_partitioner.domain();
  }

  dimensions_type dimensions(void) const
  {
    return this->m_partitioner.dimensions();
  }
}; //class view_container


//////////////////////////////////////////////////////////////////////
/// @brief Helper static polymorphic function to compute view container
///   size based on either size() or dimensions() call, based on the
///   the dimensionality of the partitioned data it represents.
//////////////////////////////////////////////////////////////////////
template<typename ViewContainer, int = ViewContainer::dimension_type::value>
struct view_container_size
{
  static
  typename ViewContainer::dimensions_type
  apply(ViewContainer const& vc)
  { return vc.dimensions(); }
};


template<typename ViewContainer>
struct view_container_size<ViewContainer, 1>
{
  static
  size_t apply(ViewContainer const& vc)
  { return vc.size(); }
};

} // namespace view_impl


template <typename View, typename P, typename MFG, typename SVC>
struct container_traits<view_impl::view_container<View,P,MFG,SVC> >
{
  typedef size_t                    gid_type;

  typedef typename select_parameter<
    MFG,
    map_fun_gen<f_ident<typename View::domain_type::index_type> >
    >::type                                                       map_fun_gen_t;

  typedef typename compose_func<
    typename View::map_func_type,
    typename map_fun_gen_t::mapfunc_type>::type                   composed_mf_t;

  using default_subview_type = typename upcast_view<
    View,
    typename P::value_type,
    composed_mf_t
  >::type;

  using subview_creator_type = typename select_parameter<
    SVC, view_impl::default_subview_creator<default_subview_type>
  >::type;

  using subview_t = typename subview_creator_type::type;

  typedef subview_t                 value_type;
  typedef typename P::domain_type   domain_type;
  typedef value_type                reference;
  typedef const value_type          const_reference;
};



template <typename C, typename PS, typename MFG, typename SVC>
class segmented_view;


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for view_trait to expose the types provided
///        for segmented_view.
//////////////////////////////////////////////////////////////////////
template <typename C, typename PS, typename MFG, typename SVC>
struct view_traits<segmented_view<C, PS, MFG, SVC> >
{
  typedef view_impl::view_container<C,PS,MFG,SVC> container;

  typedef typename select_parameter<
    MFG,
    map_fun_gen<f_ident<typename C::domain_type::index_type> >
    >::type                                                map_fun_gen_t;

  typedef typename compose_func<
    typename C::map_func_type,
    typename map_fun_gen_t::mapfunc_type>::type            composed_mf_t;

  using subview_type = typename container_traits<container>::value_type;

  typedef subview_type                                     value_type;
  typedef value_type                                       reference;
  typedef f_ident<typename C::domain_type::index_type>     map_function;
  typedef typename PS::index_type                          index_type;

  using base_view_type = typename std::conditional<
      dimension_traits<index_type>::type::value == 1,
      array_ro_view<container>,
      multiarray_view<container>
    >::type;

  using domain_type = typename base_view_type::domain_type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Define a view over a virtual container of views
///        (view_container)
///
/// @tparam C type of view to be segmented
/// @tparam PS Partition functor type
/// @tparam MFG Mapping function generator type
/// @tparam SVC Type of the functor used to create subviews
/// @ingroup segmented_view
//////////////////////////////////////////////////////////////////////
template <typename C,
          typename PS,
          typename MFG = use_default,
          typename SVC = use_default
          >
class segmented_view
  : public segmented_view_base,
    public std::conditional<
      dimension_traits<typename PS::index_type>::type::value == 1,
      array_ro_view<view_impl::view_container<C,PS,MFG,SVC>>,
      multiarray_view<view_impl::view_container<C,PS,MFG,SVC>>
    >::type
{
  typedef view_impl::view_container<C,PS,MFG,SVC>            part_container_t;

  typedef typename std::conditional<
      dimension_traits<typename PS::index_type>::type::value == 1,
      array_ro_view<view_impl::view_container<C,PS,MFG,SVC> >,
      multiarray_view<view_impl::view_container<C,PS,MFG,SVC> >
    >::type base_type;

  using map_fun_gen_t = typename view_traits<segmented_view>::map_fun_gen_t;
  using subview_t     = typename view_traits<segmented_view>::subview_type;

public:
  typedef base_type                                         view_type;
  typedef subview_t                                         value_type;

  // Types required for every view
  typedef part_container_t                                  view_container_type;
  typedef typename base_type::domain_type                   domain_type;
  typedef f_ident<typename PS::index_type>                  map_func_type;

  typedef f_ident<typename PS::index_type>                  map_function;
  typedef PS                                                partition_type;
  typedef typename PS::index_type index_type;

  segmented_view(void) = delete;

  typedef typename dimension_traits<view_container_type>::type dimension_type;


  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor used to pass ownership of the container to
  ///        the view.
  ///
  /// @param c pointer to the container used to forward the operations.
  /// @param dom domain to be used by the view.
  /// @param mf mapping function to transform view indices to container
  ///        gids.
  //////////////////////////////////////////////////////////////////////
  segmented_view(view_container_type* c, domain_type const& dom,
                   const map_func_type mf=map_func_type())
    : base_type(c, dom, mf)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor that does not takes ownership over the passed
  ///        container.
  ///
  /// @param vcont reference to the container used to forward the operations.
  /// @param dom domain to be used by the view.
  /// @param mfunc mapping function to transform view indices to container
  ///        gids.
  //////////////////////////////////////////////////////////////////////
  segmented_view(view_container_type const& c, domain_type const& dom,
                   map_func_type const&mf=map_func_type())
    : base_type(c, dom, mf)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor that does not takes ownership over the passed
  ///        container.
  ///
  /// @param vcont reference to the container used to forward the operations.
  /// @param dom domain to be used by the view.
  /// @param mfunc mapping function to transform view indices to container
  ///        gids.
  //////////////////////////////////////////////////////////////////////
  segmented_view(view_container_type const& c, domain_type const& dom,
                   map_func_type const&mf,
                   segmented_view const&)
    : base_type(c, dom, mf)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructs a view that can reference all the elements of
  ///        the passed container.
  ///
  /// @param c reference to the container used to forward the operations.
  //////////////////////////////////////////////////////////////////////
  segmented_view(view_container_type const& c)
    : base_type(c,
                domain_type(view_impl::view_container_size<
                              view_container_type>::apply(c)),
                map_func_type())
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructs a segmented_view over the container c using
  ///        the partition part.
  ///
  /// @param c container to partition
  /// @param part partitioner
  /// @param mfg mapping function generator
  //////////////////////////////////////////////////////////////////////
  template<typename Container, typename Part>
  segmented_view(Container const& c, Part const& part,
                   map_fun_gen_t const& mfg=map_fun_gen_t(),
                   typename std::enable_if<
                     dimension_traits<
                       typename Part::index_type
                     >::type::value != 1
                   >::type* = 0)
    : base_type(new view_container_type(c,part,mfg),
                domain_type(part.dimensions()),map_func_type())
  { }

  template<typename Container, typename Part>
  segmented_view(Container const& c, Part const& part,
                   map_fun_gen_t const& mfg=map_fun_gen_t(),
                   typename std::enable_if<
                     dimension_traits<
                       typename Part::index_type
                     >::type::value == 1
                   >::type* = 0)
    : base_type(new view_container_type(c,part,mfg),
                domain_type(part.size()),map_func_type())
  { }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  //////////////////////////////////////////////////////////////////////
  void define_type(typer& t)
  {
    t.base<base_type>(*this);
  }
}; //class segmented_view

///////////////////////////////////////////////////////////////////////
/// @brief Helper function to construct a segmented view.
/// @tparam C reference to the container used to forward the operations.
/// @tparam Dom domain to be used by the view.
/// @tparam MFG mapping function to transform view indices to container
///        gids.
//////////////////////////////////////////////////////////////////////
template<typename C, typename Dom, typename MFG>
segmented_view<C, Dom, MFG>
make_segmented_view(C const& c, Dom const& d, MFG const& mfg)
{
  return segmented_view<C, Dom, MFG>(c, d, mfg);
}

//////////////////////////////////////////////////////////////////////
/// @brief Specialization for changing the container used for a
///        segmented view
//////////////////////////////////////////////////////////////////////
template<typename C,
         typename PS,
         typename MFG,
         typename SVC,
         typename NewC >
struct cast_container_view<segmented_view<C,PS,MFG,SVC>, NewC>
{
  using subview_creator_type = typename container_traits<
    typename view_traits<
      segmented_view<C,PS,MFG,SVC>
    >::container
  >::subview_creator_type;

  using original_subview_type = typename subview_creator_type::type;

  using type = segmented_view<
    typename cast_container_view<C,NewC>::type,
    PS,
    MFG,
    typename subview_creator_type::template with_subview<
      typename cast_container_view<original_subview_type,NewC>::type
    >::type
  >;
};

} // namespace stapl

#endif // STAPL_VIEWS_SEGMENTED_VIEW_HPP
