/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_OPERATORS_ELEM_HELPERS_HPP
#define STAPL_SKELETONS_OPERATORS_ELEM_HELPERS_HPP

#include <type_traits>
#include <initializer_list>
#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/skeletons/flows/producer_info.hpp>
#include <stapl/skeletons/executors/memento.hpp>

namespace stapl {
namespace skeletons {
namespace skeletons_impl {

//////////////////////////////////////////////////////////////////////
/// @brief @c spawn_visitor helps in spawning the elements of
/// parametric dependencies by factoring out the task id and number
/// of consumers of a task in this class.
///
/// @tparam Spawner the spawner in which the skeleton should be spawned
///
/// @ingroup skeletonsOperatorsInternal
//////////////////////////////////////////////////////////////////////
template <typename Spawner>
class spawn_visitor
{
  Spawner&    m_spawner;
  std::size_t m_tid;
  std::size_t m_num_succs;

public:
  spawn_visitor(Spawner& spawner) :
    m_spawner(spawner),
    m_tid(0),
    m_num_succs(0)
  { }

  void set_info(std::size_t tid, std::size_t num_succs)
  {
    m_tid = tid;
    m_num_succs = num_succs;
  }

  memento& get_memento_stack()
  {
    return m_spawner.get_memento_stack();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Puts the given set of precedence dependencies in a vector.
  ///
  /// @param in    the set of inputs to be used for the spawn request
  ///
  /// @todo should be generalized if the task ID's are no longer assumed
  ///       to be of type @c size_t
  /// @todo should support a signature for statically known cases where
  ///       where the number of predecessors is known at compile time.
  ///       This requires paragraph::add_task to accept std::array
  //////////////////////////////////////////////////////////////////////
  std::vector<std::size_t>
  notification_list(std::initializer_list<std::size_t>&& a) const
  {
    return std::vector<std::size_t>(a);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Each parametric dependency gets an instance of this struct and
  /// will call the function operator of this visitor based on the number
  /// of arguments it has.
  ///
  /// @param wf    the workfunction which will be used for the spawn
  ///              request
  /// @param in    the set of inputs to be used for the spawn request
  //////////////////////////////////////////////////////////////////////
  template <bool isResult = false,
            typename WF, typename... Args,
            typename = typename std::enable_if<
                         !std::is_same<
                            WF, std::vector<std::size_t>>::value,
                         void>::type>
  void operator()(WF&& wf, Args&&... args)
  {
    m_spawner.template spawn_element<isResult>(
      m_tid, std::forward<WF>(wf),
      m_num_succs + (isResult ? 1 : 0),
      std::forward<Args>(args)...);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Each parametric dependency gets an instance of this struct and
  /// will call the function operator of this visitor based on the number
  /// of arguments it has.
  ///
  /// @param wf    the workfunction which will be used for the spawn
  ///              request
  /// @param notifications the set of nodes that depend on the completion
  ///              of this node
  /// @param args  the set of inputs to be used for the spawn request
  //////////////////////////////////////////////////////////////////////
  template <bool isResult = false,
            typename WF, typename... Args>
  void operator()(std::vector<std::size_t> const& notifications,
                  WF&& wf, Args&&... args)
  {
    m_spawner.template spawn_element<isResult>(
      m_tid, notifications, std::forward<WF>(wf),
      m_num_succs + (isResult ? 1 : 0),
      std::forward<Args>(args)...);
  }
};

} // namespace skeletons_impl


//////////////////////////////////////////////////////////////////////
/// @brief If you would like to send a constant value to your spawn
/// request to the @c spawn_visitor in your parametric dependency, you
/// should use this method. Later on the created wrapper around the
/// constant value is used by the @c spawn_visitor to complete the
/// spawn request.
///
/// @param t  the constant value to be stored
/// @return a constan_input_wrapper
///
/// @ingroup skeletonsOperatorsProducerConsumer
//////////////////////////////////////////////////////////////////////
template <typename T>
auto
constant_input(T&& t)
STAPL_AUTO_RETURN((
  flows::constant_producer_info<typename std::decay<T>::type>(
    std::forward<T>(t))
))

//////////////////////////////////////////////////////////////////////
/// @brief If you would like to send a reference of an element to your
/// spawn request, you should use this method. Later on the created
/// wrapper around the reference is used by the @c spawn_visitor to
/// complete the spawn request
///
/// @param element the reference to the element to be stored
/// @return a reflexive_input_wrapper
///
/// @ingroup skeletonsOperatorsProducerConsumer
//////////////////////////////////////////////////////////////////////
template <bool isRef = false,
          typename Element>
auto
reflexive_input(Element&& element)
STAPL_AUTO_RETURN((
  flows::reflexive_producer_info<
    typename std::decay<Element>::type, isRef>(
      std::forward<Element>(element))
))

} // namespace stapl
} // namespace skeletons

#endif // STAPL_SKELETONS_OPERATORS_ELEM_HELPERS_HPP
