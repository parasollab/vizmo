/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_TRANSFORMATIONS_WRAPPED_SKELETON_HPP
#define STAPL_SKELETONS_TRANSFORMATIONS_WRAPPED_SKELETON_HPP

#include <type_traits>
#include <stapl/skeletons/utility/dynamic_wf.hpp>
#include <stapl/skeletons/utility/identity_helpers.hpp>
#include <stapl/views/metadata/mix_view.hpp>
#include <boost/utility/result_of.hpp>
#include <stapl/skeletons/transformations/transform.hpp>
#include <stapl/skeletons/transformations/optimizer.hpp>

namespace stapl {

template <typename V, typename I, typename CID>
class mix_view;

template <typename V>
struct nfast_view;

namespace paragraph_impl {
template <typename Sched>
class paragraph_view;
}

namespace skeletons {

template <typename S, typename ExecutionTag, bool requiresTGV>
class wrapped_skeleton;

template <typename WF>
struct is_nested_skeleton
  : public std::false_type
{ };

template <typename S, typename Tag>
struct is_nested_skeleton<
         transformations::transformations_impl::transform<S, Tag>>
  : public is_nested_skeleton<
             typename transformations::transformations_impl::
               transform<S, Tag>::base_type>
{ };

template <typename S, bool B1, bool B2>
struct is_nested_skeleton<wrapped_skeleton<S, tags::nested_execution<B1>, B2>>
  : public std::true_type
{ };

template <typename S, bool B1, bool B2>
struct is_nested_skeleton<wrapped_skeleton<S, tags::doall_execution<B1>, B2>>
  : public std::true_type
{ };

namespace skeletons_impl {

//////////////////////////////////////////////////////////////////////
/// @brief The return type to be determined for an optimizer by passing
/// the fine-grain type.
///
/// @ingroup skeletonTransformationsInternal
//////////////////////////////////////////////////////////////////////
template <typename O, typename T>
struct compute_return_type
{
  using type = typename boost::result_of<O(T)>::type;
};


template <typename O>
struct compute_return_type<O, void>
{
  using type = void;
};


//////////////////////////////////////////////////////////////////////
/// @brief Check if the given view is a @ref paragraph_view
///
/// @ingroup skeletonTransformationsInternal
//////////////////////////////////////////////////////////////////////
template <typename V>
struct is_tgv
  : public std::false_type
{ };


template <typename Sched>
struct is_tgv<paragraph_impl::paragraph_view<Sched>>
  : public std::true_type
{ };


//////////////////////////////////////////////////////////////////////
/// @brief A fake_flow is used for making the result_type computation
/// possible for wrapped skeletons (@ref wrapped_skeleton).
///
/// A fake_flow is basically specialized to extract the underlying
/// value type of a given coarsened input and providing it to the
/// @ref wrapped_skeleton @c result struct.
///
/// @ingroup skeletonTransformationsInternal
//////////////////////////////////////////////////////////////////////
template <typename V>
struct fake_flow
{
  using flow_value_type = typename V::value_type;
};


//////////////////////////////////////////////////////////////////////
/// @brief A fake_flow is used for making the result_type computation
/// possible for wrapped skeletons (@ref wrapped_skeleton).
///
/// This specialization handles the case in which the given input is
/// a coarsened view over the original input.
///
/// @ingroup skeletonTransformationsInternal
//////////////////////////////////////////////////////////////////////
template <typename V, typename I, typename CID>
struct fake_flow<mix_view<V, I, CID> >
{
  using flow_value_type = typename mix_view<V, I, CID>::reference;
};


//////////////////////////////////////////////////////////////////////
/// @brief A fake_flow is used for making the result_type computation
/// possible for wrapped skeletons (@ref wrapped_skeleton).
///
/// This specialization handles the case in which the given input is
/// a fast view.
///
/// @ingroup skeletonTransformationsInternal
//////////////////////////////////////////////////////////////////////
template <typename V>
struct fake_flow<nfast_view<V> >
{
  using flow_value_type = typename nfast_view<V>::reference;
};


template <typename Optimizer, typename S,
          bool has_result_type = has_result_type<Optimizer>::value>
struct wrapped_skeleton_result_type
{
  using type = typename Optimizer::result_type;
};


template <typename Optimizer, typename S, typename V0, typename ...V>
struct wrapped_skeleton_result_type<Optimizer, S(V0, V...), false>
{
private:
  using in_flow_t =
    typename std::conditional<
      skeletons_impl::is_tgv<V0>::value,
      stapl::tuple<skeletons_impl::fake_flow<V>...>,
      stapl::tuple<skeletons_impl::fake_flow<V0>,
                   skeletons_impl::fake_flow<V>...>
    >::type;
  using out_flow_t = typename S::template out_port_type<in_flow_t>::type;
  using value_t = typename tuple_element<0, out_flow_t>::type::flow_value_type;

public:

  using type = typename skeletons_impl::compute_return_type<
                 Optimizer, value_t>::type;
};

//////////////////////////////////////////////////////////////////////
/// @brief A fake_flow is used for making the result_type computation
/// possible for @c wrapped_skeletons.
///
/// This specialization handles the case in which the given input is
/// a paragraph view. A paragraph view does not carry a value, hence,
/// typed as void.
///
/// @ingroup skeletonTransformationsInternal
//////////////////////////////////////////////////////////////////////
template <typename V>
struct fake_flow<paragraph_impl::paragraph_view<V> >
{
  using flow_value_type = void;
};


} // namespace skeletons_impl


//////////////////////////////////////////////////////////////////////
/// @brief A fat node is used in the nested execution of skeletons.
///
/// An @c ExecutionTag is used to customize the execution method. By
/// default, a @c PARAGRAPH will be created for a wrapped skeleton.
/// This behavior can be customized by providing a custom @c ExecutionTag
/// to this class. For example, passing a sequential execution tag
/// (tags::sequential_execution), avoids creating a PARAGRAPH for the
/// nested computation.
///
/// A @c wrapped_skeleton passes the skeleton to specializations of the
/// @ref optimizer. These specializations are defined over the skeleton
/// and @c ExecutionTag. For example, the specialization for the
/// @ref reduce skeleton (optimizer<tags::reduce, tags::sequential_execution>)
/// implements this skeleton using std::accumulate.
///
/// @see optimizers/default.hpp
/// @see optimizers/nested.hpp
/// @see optimizers/reduce.hpp
///
/// @tparam S            the skeleton to be wrapped
/// @tparam ExecutionTag a tag defining the execution strategy for this
///                      skeleton
/// @tparam requiresTGV  a boolean variable to determine whether this
///                      wrapped skeleton needs @ref paragraph_view or not
///
/// @ingroup skeletonTransformations
///
/// @todo requiresTGV should be removed from here.
//////////////////////////////////////////////////////////////////////
template <typename S, typename ExecutionTag, bool requiresTGV>
class wrapped_skeleton
  : public std::conditional<requiresTGV, dynamic_wf, stapl::use_default>::type
{
  S m_skeleton;

  using optimizer_t = optimizers::optimizer<
                        typename S::skeleton_tag_type,
                        ExecutionTag>;

public:
  wrapped_skeleton(S const& skeleton)
    : m_skeleton(skeleton)
  { }

  template <typename F>
  struct result;

  template <typename F, typename ...V>
  struct result<F(V...)>
  {
    using type = typename skeletons_impl::wrapped_skeleton_result_type<
                   optimizer_t, S(typename std::decay<V>::type...)>::type;
  };

  template <typename ...V>
  typename result<wrapped_skeleton(V...)>::type
  operator()(V&&... view) const
  {
    using return_t = typename result<wrapped_skeleton(V...)>::type;
    return optimizer_t::template execute<return_t>(
             m_skeleton, std::forward<V>(view)...);
  }

  template <typename ...V>
  typename result<wrapped_skeleton(V...)>::type
  operator()(V&&... view)
  {
    using return_t = typename result<wrapped_skeleton(V...)>::type;
    return optimizer_t::template execute<return_t>(
             m_skeleton, std::forward<V>(view)...);
  }

  S const& get_skeleton() const
  {
    return m_skeleton;
  }

  void define_type(typer& t)
  {
    t.member(m_skeleton);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Wraps a skeleton with a suggested execution strategy.
///
/// @tparam S            the skeleton to be wrapped
/// @tparam ExecutionTag a tag defining the execution strategy for this
///                      skeleton
/// @tparam requiresTGV  a boolean variable to determine whether this
///                      skeleton needs @ref paragraph_view or not
///
/// @return a @c wrapped_skeleton
///
/// @ingroup skeletonTransformationsInternal
//////////////////////////////////////////////////////////////////////
template <typename ExecutionTag = stapl::use_default,
          bool requiresTGV      = false,
          typename S,
          typename =
            typename std::enable_if<
              is_skeleton<typename std::decay<S>::type>::value>::type>
wrapped_skeleton<typename std::decay<S>::type, ExecutionTag, requiresTGV>
wrap(S&& skeleton)
{
  return wrapped_skeleton<
           typename std::decay<S>::type, ExecutionTag, requiresTGV
         >(std::forward<S>(skeleton));
}

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_TRANSFORMATIONS_WRAPPED_SKELETON_HPP
