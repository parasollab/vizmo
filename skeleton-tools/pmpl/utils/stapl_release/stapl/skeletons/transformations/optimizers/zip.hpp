/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_OPTIMIZERS_ZIP_HPP
#define STAPL_SKELETONS_OPTIMIZERS_ZIP_HPP

#include <type_traits>
#include <vector>
#include <stapl/skeletons/transformations/optimizer.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/skeletons/utility/tags.hpp>
#include <stapl/skeletons/utility/skeleton.hpp>
#include <stapl/skeletons/utility/wf_iter_compare.hpp>
#include <stapl/skeletons/utility/dynamic_wf.hpp>
#include <stapl/skeletons/transformations/optimizers/utils.hpp>

namespace stapl {
namespace skeletons {
namespace optimizers {

template <typename SkeletonTag, typename ExecutionTag>
struct optimizer;

//////////////////////////////////////////////////////////////////////
/// @brief A @c zip optimizer is used in the cases that all views are
/// local and their traversal would be fast which improves the
/// performance. You have to notice that the inner vector creation
/// takes some time and you have to consider it before using this
/// optimizer.
///
/// @ingroup skeletonsTransformationsCoarse
//////////////////////////////////////////////////////////////////////
template <typename Tag, int arity>
struct optimizer<tags::zip<Tag, arity>, tags::sequential_execution>
{
  template <typename R>
  struct result;

  template <typename Optimizer, typename OutputValueType>
  struct result<Optimizer(OutputValueType)>
  {
    using type = std::vector<OutputValueType>;
  };

private:
  //////////////////////////////////////////////////////////////////////
  /// @brief Specialization of sequential execution when @ref paragraph_view
  /// is required by the zip operation
  //////////////////////////////////////////////////////////////////////
  struct dynamic
  {
    template <typename R, typename S, typename TGV,
              typename IterComp, typename ...Iter>
    static R apply(std::false_type, S&& skeleton, TGV tgv,
                   std::size_t size, IterComp& iter_compare, Iter... iter)
    {
      R result;
      result.reserve(size);
      auto zip_op = skeleton.get_op();
      for (; iter_compare(iter...); helpers::no_op(++iter...)) {
        result.push_back(tgv, zip_op(*(iter)...));
      }
      return result;
    }

    template <typename R, typename S, typename TGV,
              typename IterComp, typename ...Iter>
    static R apply(std::true_type, S&& skeleton, TGV tgv,
                   std::size_t, IterComp& iter_compare, Iter... iter)
    {
      auto zip_op = skeleton.get_op();
      for (; iter_compare(iter...); helpers::no_op(++iter...)) {
        zip_op(tgv, *(iter)...);
      }
    }

    template <typename R, typename S, typename TGV,
              typename V0, typename... V>
    static R execute(S&& skeleton, TGV tgv,
                     V0&& view0, V&&... view)
    {
      wf_iter_compare<
        typename std::decay<V0>::type,
        typename std::decay<V>::type...> iter_compare(
          std::forward<V0>(view0),
          std::forward<V>(view)...);
      return apply<R>(typename std::is_same<R, void>::type(),
                      std::forward<S>(skeleton), tgv,
                      view0.size(), iter_compare,
                      view0.begin(), view.begin()...);
    }
  }; // nested struct dynamic

  //////////////////////////////////////////////////////////////////////
  /// @brief Specialization of sequential execution when @ref paragraph_view
  /// is not required by the zip operation
  //////////////////////////////////////////////////////////////////////
  struct non_dynamic
  {
    template <typename R, typename S,
              typename IterComp, typename ...Iter>
    static R apply_iterator(std::false_type, S&& skeleton,
                            std::size_t size,
                            IterComp& iter_compare, Iter... iter)
    {
      R result;
      result.reserve(size);
      auto zip_op = skeleton.get_op();
      for (; iter_compare(iter...); helpers::no_op(++iter...)) {
        result.push_back(zip_op(*(iter)...));
      }
      return result;
    }

    template <typename R, typename S,
              typename IterComp, typename ...Iter>
    static R apply_iterator(std::true_type, S&& skeleton,
                            std::size_t size,
                            IterComp& iter_compare, Iter... iter)
    {
      auto zip_op = skeleton.get_op();
      for (; iter_compare(iter...); helpers::no_op(++iter...)) {
        zip_op(*(iter)...);
      }
    }

    template <typename R, typename S, typename... Tuple>
    static R apply_domain(std::true_type, S&& skeleton,
                          std::size_t size, Tuple... t)
    {
      auto zip_op = skeleton.get_op();

      for (std::size_t i = 0; i < size; ++i, helpers::no_op(
        helpers::advance_domain(std::get<1>(t), std::get<2>(t))...))
        zip_op(helpers::referencer(std::get<0>(t), std::get<2>(t))...);
    }

    template <typename R, typename S, typename... Tuple>
    static R apply_domain(std::false_type, S&& skeleton,
                          std::size_t size, Tuple... t)
    {
      R result;
      result.reserve(size);

      auto zip_op = skeleton.get_op();

      for (std::size_t i = 0; i < size; ++i, helpers::no_op(
        helpers::advance_domain(std::get<1>(t), std::get<2>(t))...))
        result.push_back(
          zip_op(helpers::referencer(std::get<0>(t), std::get<2>(t))...)
        );

      return result;
    }

    template <typename R, typename S, typename... V>
    static R apply(std::false_type, S&& skeleton, V&&... v)
    {
      const std::size_t size = helpers::view_size(std::forward<V>(v)...);

      return apply_domain<R>(typename std::is_same<R, void>::type(),
        std::forward<S>(skeleton), size,
        stapl::make_tuple(v, v.domain(), v.domain().first())...
      );
    }

    template <typename R, typename S, typename V0, typename... V>
    static R apply(std::true_type, S&& skeleton,
                   V0&& view0, V&&... view)
    {
      wf_iter_compare<
        typename std::decay<V0>::type,
        typename std::decay<V>::type...> iter_compare(
          std::forward<V0>(view0),
          std::forward<V>(view)...);

      return apply_iterator<R>(typename std::is_same<R, void>::type(),
                               std::forward<S>(skeleton),
                               view0.size(), iter_compare,
                               view0.begin(), view.begin()...);
    }

    template <typename R, typename S, typename... V>
    static R execute(S&& skeleton, V&&... v)
    {
      return apply<R>(
        typename helpers::pack_has_iterator<
          typename std::decay<V>::type...>::type(),
        std::forward<S>(skeleton), std::forward<V>(v)...
      );
    }
  }; // nested struct non_dynamic

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Sequential zip optimizer dispatches the execution to
  /// two different signatures based on the requirement of the given
  /// zip operation. If the zip operations needs the @ref paragraph_view,
  /// the dynamic specialization is called, otherwise the non_dynamic
  /// one is called.
  ///
  /// @return the result of the zip operation (can be void)
  //////////////////////////////////////////////////////////////////////
  template <typename R, typename S, typename... Args>
  static R execute(S&& skeleton, Args&&... args)
  {
    using dispatcher_t = typename std::conditional<
                           std::is_base_of<
                             dynamic_wf,
                             typename std::decay<S>::type::op_type>::value,
                           dynamic,
                           non_dynamic>::type;
    return dispatcher_t::template execute<R>(
             std::forward<S>(skeleton),
             std::forward<Args>(args)...);
  }
};


} // namespace optimizers
} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_OPTIMIZERS_ZIP_HPP
