/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_CORE_VIEW_HPP
#define STAPL_VIEWS_CORE_VIEW_HPP

#ifdef _STAPL
 #include <stapl/utility/has_member_function.hpp>
 #include <stapl/runtime.hpp>
 #include <stapl/containers/base/container_base.hpp>
 #include <stapl/utility/loc_qual.hpp>
 #include <stapl/utility/tuple.hpp>
 #include <stapl/views/view_traits.hpp>
#endif

#include <stapl/views/base_view.hpp>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/utility/addressof.hpp>
#include <functional>

#include <iostream>
#include <type_traits>

namespace stapl {

namespace view_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Class to manage the reference counting used to determine when the
///        container needs to be destroyed if the view owns the container.
//////////////////////////////////////////////////////////////////////
template <typename Container>
class ref_counting
{
  typedef boost::shared_ptr<int>   ref_count_type;

  ref_count_type       m_nref;
  Container*           m_ptr;

public:
  ref_counting(void)
    : m_ptr(0)
  { }

  explicit ref_counting(Container* c)
    : m_ptr(c)
  {
    if (c)
      m_nref = boost::make_shared<int>(1);
  }

  ~ref_counting(void)
  {
    if (m_nref.unique())
      delete m_ptr;
  }

  ref_counting& operator=(ref_counting const& other)
  {
    if (m_nref.unique())
      delete m_ptr;
    m_nref = other.m_nref;
    m_ptr = other.m_ptr;
    return *this;
  }

  void set_ptr(Container* ptr)
  {
    m_ptr = ptr;
  }

  bool owned(void) const noexcept
  { return m_nref.unique(); }

  void define_type(typer& t)
  {
    t.member(m_nref);
    if (t.pass() == typer::UNPACK) {
      if (is_p_object<Container>::value) {
        // Counter disabled to behave as a weak_ptr
        m_nref=boost::make_shared<int>();
        m_ptr = NULL;
      }
      else
        // For non pObject the counting is needed to cleanup the copy
        m_nref=boost::make_shared<int>(1);
    }
  }
};


#ifdef _STAPL
//////////////////////////////////////////////////////////////////////
/// @brief Primary template handles containers which are p_objects,
///        holding a handle_ref as well as container*, so that it can
///        fall back to this when view moves to a hardware thread
///        where m_container doesn't have a location.
//////////////////////////////////////////////////////////////////////
template<typename Container, bool = is_p_object<Container>::value >
class object_holder
  : ref_counting<Container>
{
private:
  typedef ref_counting<Container> ref_counting_type;

  p_object_pointer_wrapper<Container> m_container;

protected:
  object_holder(void) = default;

  explicit object_holder(Container* ct)
    : ref_counting_type(ct),
      m_container(ct)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @todo breaking constness with boost::addressof
  //////////////////////////////////////////////////////////////////////
  object_holder(Container const& ct)
    : m_container(std::addressof(const_cast<Container&>(ct)))
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the container's pointer
  //////////////////////////////////////////////////////////////////////
  Container* container_ptr(void) const
  {
    stapl_assert(m_container,
                 "object_holder::container_ptr found NULL pointer");

    return m_container;
  }

public:
  using ref_counting_type::owned;

  void define_type(typer& t)
  {
    t.base<ref_counting_type>(*this);
    t.member(m_container);
  }
};
#endif

//////////////////////////////////////////////////////////////////////
/// @brief Specialization for handle containers which are not
///        p_objects.
//////////////////////////////////////////////////////////////////////
template<typename Container>
#ifdef _STAPL
// Specialization for non-p_objects
class object_holder<Container, false>
#else
// Primary Template
class object_holder
#endif
  : ref_counting<Container>
{
private:
  typedef ref_counting<Container> ref_counting_type;

  Container*             m_container;

protected:
  object_holder(void)
    : m_container(0)
  { }

  explicit object_holder(Container* ct)
    : ref_counting_type(ct),
      m_container(ct)
  { }

  explicit object_holder(Container const& ct)
    : m_container(boost::addressof(const_cast<Container&>(ct)))
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the container's pointer
  //////////////////////////////////////////////////////////////////////
  Container* container_ptr(void) const
  {
    stapl_assert(m_container,
                 "object_holder::container_ptr found NULL pointer");
    return m_container;
  }

#ifdef _STAPL
public:
  using ref_counting_type::owned;

  void define_type(typer& t)
  {
    t.base<ref_counting_type>(*this);
    t.member(m_container);
    if (t.pass() == typer::UNPACK) {
      // Coping the container to avoid use runtime's buffer memory
      m_container = new Container(*m_container);
      this->set_ptr(m_container);
    }
  }
#endif
}; // class object_holder


template <typename MapFunc>
struct mapfunc_types_helper
{
  typedef typename MapFunc::index_type                index_type;
  typedef typename MapFunc::gid_type                  gid_type;
};

template <typename Signature>
struct mapfunc_types_helper<std::function<Signature> >
{
  typedef typename std::function<Signature>::argument_type index_type;
  typedef typename std::function<Signature>::result_type   gid_type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Validators for versioning. Templates allow p_objects to be
///   versioned and non_p_objects, like stl objects, to ignore versioning
/// @tparam T Container type.
//////////////////////////////////////////////////////////////////////
template<typename T, bool = is_p_object<T>::value >
struct validator
{
  static bool apply(T const& t, size_t version)
  {
    return t.version() == version;
  }
};


template<typename T>
struct validator <T, false>
{
  static constexpr bool apply(T const&, size_t) noexcept
  {
    return true;
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Versioner for versioning. Templates allow p_objects to be
///   versioned and non_p_objects, like stl objects, to ignore versioning
/// @tparam T Container type.
//////////////////////////////////////////////////////////////////////
template<typename T, bool = is_p_object<T>::value >
struct versioner
{
  static size_t apply(T const& t)
  {
    return t.version();
  }

  static size_t apply(T* t)
  {
    return t->version();
  }
};

template<typename T>
struct versioner <T, false>
{
  static constexpr size_t apply(T const&) noexcept
  {
    return 0;
  }

  static constexpr size_t apply(T*) noexcept
  {
    return 0;
  }
};

} // namespace view_impl


#ifdef _STAPL

namespace view_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Boolean type metafunction checking whether template is both
///  a view and whether its container type is a proxy.
///
///  Reflects index type to use as return value for @p index_of().
///  gcc evaluates the return type even if @p enable_if disables the
///  function signature.
//////////////////////////////////////////////////////////////////////
template<typename View, bool = is_view<View>::value>
struct is_view_of_proxy
  : std::false_type
{ };


template<typename View>
struct is_view_of_proxy<View, true>
  : std::integral_constant<
      bool, is_proxy<typename view_traits<View>::container>::value>
{
  typedef decltype(proxy_core_access::accessor(
    std::declval<View>().container()).index())       index_type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Type metafunction returning true_type if the provided type
///  is either @ref p_object or a proxy to a @ref p_object.
//////////////////////////////////////////////////////////////////////
template<typename Container>
struct is_p_object_backed
  : is_p_object<Container>
{ };


template<typename Container, typename Accessor>
struct is_p_object_backed<proxy<Container, Accessor>>
  : is_p_object<Container>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Static functor redirecting locality queries to container
///  if it is a @ref p_object.  Otherwise return LQ_DONTCARE.
//////////////////////////////////////////////////////////////////////
template<typename Container, bool = is_p_object_backed<Container>::value>
struct query_locality
{
  template<typename Index>
  static
  locality_info apply(Container& c, Index const& index)
  { return c.locality(index); }
};


template<typename Container>
struct query_locality<Container, false>
{
  template<typename Index>
  static
  locality_info apply(Container const&, Index const&)
  { return LQ_DONTCARE; }
};

} // namespace view_impl


//////////////////////////////////////////////////////////////////////
/// @brief Alias template defining @p enable_if condition for proxy
///  related operations on views of nested containers.
//////////////////////////////////////////////////////////////////////
template<typename View>
using ViewProxyEnabler =
  typename std::enable_if<view_impl::is_view_of_proxy<View>::value>::type*;


//////////////////////////////////////////////////////////////////////
/// @brief Returns the associated index of the element referenced by
///        the given proxy.  Specialization for views of nested
///        containers.
//////////////////////////////////////////////////////////////////////
template<typename View>
typename view_impl::is_view_of_proxy<View>::index_type
index_of(View const& view, ViewProxyEnabler<View> = nullptr)
{
  return proxy_core_access::accessor(view.container()).index();
}


template<typename View>
bool is_null_reference(View const& view, ViewProxyEnabler<View> = nullptr)
{
  return proxy_core_access::accessor(view.container()).is_null();
}



#endif

//////////////////////////////////////////////////////////////////////
/// @brief Main class to define a pView, composed of a reference to a
///        collection of elements, a domain and a mapping function.
///
/// @tparam C Container type.
/// @tparam Dom Domain type.
/// @tparam MapFunc Mapping function type.
//////////////////////////////////////////////////////////////////////
template <typename C, typename Dom, typename MapFunc >
class core_view
  : public base_view,
    public view_impl::object_holder<C>
{
  typedef view_impl::mapfunc_types_helper<MapFunc>    mf_type_helper;
public:
  typedef C                                           view_container_type;
  typedef Dom                                         domain_type;
  typedef MapFunc                                     map_func_type;
  typedef typename mf_type_helper::index_type         index_type;
  typedef typename mf_type_helper::gid_type           gid_type;
  typedef std::size_t                                 size_type;

private:
  typedef view_impl::object_holder<C>                 ct_holder_t;

  /// Domain of the view.
  domain_type            m_domain;

  /// Mapping function used to transform indices to container's gids.
  map_func_type          m_mapfunc;
  size_t                 m_version;

public:
  core_view(void)
    : m_version(0)
  { }

  core_view& operator=(core_view const&) = default;
  core_view(core_view&&)                 = default;

  //////////////////////////////////////////////////////////////////////
  /// @brief core_view constructor based on another view.
  ///
  /// This constructor creates a view from another view.
  /// When we coarsen to base containers there is no versioning.
  ///
  /// @param other The other view to copy from.
  //////////////////////////////////////////////////////////////////////
  core_view(core_view const& other)
    : ct_holder_t(other),
      m_domain(other.m_domain),
      m_mapfunc(other.m_mapfunc),
      m_version(other.version())
  { }


  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor based on a container pointer. The views takes
  ///        ownership over the passed underlying container.
  ///
  /// @param vcont  pointer to the container used to forward the operations.
  /// @param dom  domain to be used by the view.
  /// @param mfunc mapping function to transform view indices to
  ///              container gids.
  //////////////////////////////////////////////////////////////////////
  core_view(view_container_type* vcont,
            domain_type const& dom,
            map_func_type mfunc = MapFunc())
    : ct_holder_t(vcont),
      m_domain(dom),
      m_mapfunc(mfunc)
  {
    m_version = view_impl::versioner<view_container_type>::apply(vcont);
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief core_view constructor based on a container reference.
  ///
  /// This constructor creates a view that doesn't take ownership over
  /// the container.
  ///
  /// @param vcont reference to the container used to forward the operations.
  /// @param dom domain to be used by the view.
  /// @param mfunc mapping function to transform view indices to container
  ///              gids.
  //////////////////////////////////////////////////////////////////////
  core_view(view_container_type const& vcont,
            domain_type const& dom,
            map_func_type mfunc = MapFunc())
    : ct_holder_t(vcont),
      m_domain(dom),
      m_mapfunc(mfunc)
  {
    m_version = view_impl::versioner<view_container_type>::apply(vcont);
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  /// @brief  Validates view and aborts if view is invalid. Only
  ///         pobject containers are validated (When we coarsen to base
  ///         containers there is no validation.)
  //////////////////////////////////////////////////////////////////////
  bool validate(void) const
  {
    if (view_impl::validator<C>::apply(this->container(), m_version))
    {
      return true;
    }
    else
    {
      abort("View validation failed. View/container out of sync.");
      return false;
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  /// @brief  Increment the version number
  //////////////////////////////////////////////////////////////////////
  void incr_version(void)
  {
    ++m_version;
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  /// @brief Returns the current version number
  //////////////////////////////////////////////////////////////////////
  size_t version(void) const
  {
    return m_version;
  }

  //////////////////////////////////////////////////////////////////////
  /// @todo Const qualifying the returned reference produces compiler errors
  //////////////////////////////////////////////////////////////////////
  view_container_type /*const*/* get_container(void) const
  {
    return this->container_ptr();
  }

  view_container_type& container(void)
  {
    return *this->container_ptr();
  }

  //////////////////////////////////////////////////////////////////////
  /// @todo Const qualifying the returned reference produces compiler errors
  //////////////////////////////////////////////////////////////////////
  view_container_type /*const*/& container(void) const
  {
    return *this->container_ptr();
  }

  domain_type const& domain(void) const
  {
    return m_domain;
  }

  domain_type& domain(void)
  {
    return m_domain;
  }

  void set_domain(domain_type const& dom)
  {
    m_domain = dom;
  }

  map_func_type const& mapfunc(void) const
  {
    return m_mapfunc;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the number of elements referenced for the view.
  //////////////////////////////////////////////////////////////////////
  size_type size(void) const
  {
    return m_domain.size();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns true if the view does not reference any element.
  //////////////////////////////////////////////////////////////////////
  bool empty(void) const
  {
    return m_domain.empty();
  }

#ifdef _STAPL
public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Determine the locality of the given index.  Common use is
  ///  in task placement within a @ref paragraph.
  /// @return A @ref locality_info object describing the locality of the
  ///   given element.
  //////////////////////////////////////////////////////////////////////
  locality_info locality(index_type const& index) const
  {
    return view_impl::query_locality<view_container_type>::apply(
      this->container(), this->mapfunc()(index));
  }

  rmi_handle::reference nested_locality(index_type const& index)
  {
    typedef typename container_traits<C>::value_type v_t;
    return this->container().apply_get(this->mapfunc()(index),
      boost::bind<rmi_handle::reference>(
        [](v_t& v) { return v.get_rmi_handle(); } , _1));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns true if all the elements referenced for the view are
  ///        local.
  //////////////////////////////////////////////////////////////////////
  bool is_local(void) const
  {
    return false;
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  //////////////////////////////////////////////////////////////////////
  void define_type(typer& t)
  {
    t.base<ct_holder_t>(*this);
    t.member(m_domain);
    t.member(m_mapfunc);
    t.member(m_version);
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  /// @brief use to examine this class
  /// @param msg your message (to provide context)
  //////////////////////////////////////////////////////////////////////
  void debug(char *msg=0)
  {
    std::cerr << "CORE_VIEW " << this << " : ";
    if (msg) {
      std::cerr << msg;
    }
    std::cerr << std::endl;
    m_domain.debug();
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  /// @brief used by paragraph
  //////////////////////////////////////////////////////////////////////
  void pre_execute(void)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  /// @brief used by paragraph
  //////////////////////////////////////////////////////////////////////
  void post_execute(void)
  { }

#endif // ifdef _STAPL

}; // class core_view

} // namespace stapl

#endif // STAPL_VIEWS_CORE_VIEW_HPP
