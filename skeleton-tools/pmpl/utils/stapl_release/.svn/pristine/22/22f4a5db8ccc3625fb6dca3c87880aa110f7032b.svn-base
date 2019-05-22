/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_RUNTIME_COMMUNICATOR_ALLREDUCE_HPP
#define STAPL_RUNTIME_COMMUNICATOR_ALLREDUCE_HPP

#include "collective.hpp"
#include "../exception.hpp"
#include "../message.hpp"
#include "../runqueue.hpp"
#include "../request/packed_value.hpp"
#include <mutex>
#include <utility>
#include <boost/function.hpp>
#include <boost/optional.hpp>

namespace stapl {

namespace runtime {

//////////////////////////////////////////////////////////////////////
/// @brief Distributed memory collective reduce.
///
/// @tparam T               Object type.
/// @tparam BinaryOperation Binary operation function object type to be applied.
///
/// @warning This class supports only commutative reductions.
///
/// @see collective
/// @ingroup runtimeCollectives
///
/// @todo Use platform optimized reduce implementation.
//////////////////////////////////////////////////////////////////////
template<typename T, typename BinaryOperation>
class reduce
{
public:
  using value_type   = T;
private:
  using packed_type  = packed_value<T>;
  using storage_type = typename packed_type::storage_type;

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the expected size to store @p u.
  //////////////////////////////////////////////////////////////////////
  template<typename U>
  static std::size_t expected_size(U&& u) noexcept
  {
    return (sizeof(storage_type) +
            storage_type::packed_size(std::forward<U>(u)));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Packs @p u in @p p.
  //////////////////////////////////////////////////////////////////////
  template<typename U>
  static void pack(void* p, U&& u) noexcept
  {
    std::size_t size = sizeof(storage_type);
    new(p) storage_type{std::forward<U>(u), p, size};
  }


  collective&                 m_handle;
  BinaryOperation             m_op;
  boost::optional<value_type> m_value;
  boost::function<void(T)>    m_signal;
  std::mutex                  m_mtx;

  ////////////////////////////////////////////////////////////////////
  /// @brief Sends @p u to the parent process.
  ////////////////////////////////////////////////////////////////////
  template<typename U>
  void send_parent(U&& u)
  {
    const std::size_t size = expected_size(std::forward<U>(u));
    auto m =
      message::create(header::COLL_GATHER, size, m_handle.get_id());
    pack(m->reserve(size), std::forward<U>(u));
    runqueue::add(m_handle.get_topology().parent_id(), std::move(m));
  }

  ////////////////////////////////////////////////////////////////////
  /// @brief Accumulates the values in @p l.
  ////////////////////////////////////////////////////////////////////
  T accumulate(message_slist l)
  {
    // get first value
    auto m  = l.pop_front();
    char* b = m->payload().begin();
    auto* p = reinterpret_cast<storage_type*>(b);
    T t     = packed_type{p, b, std::move(m)}.get();

    // accumulate the rest of the values
    while (!l.empty()) {
      m = l.pop_front();
      b = m->payload().begin();
      p = reinterpret_cast<storage_type*>(b);
      t = m_op(std::move(t), packed_type{p, b, std::move(m)}.get());
    }

    return t;
  }

  ////////////////////////////////////////////////////////////////////
  /// @brief Combines @p u and @p l and does progress on the reduction.
  ////////////////////////////////////////////////////////////////////
  template<typename U>
  void do_reduction(U&& u, message_slist l)
  {
    T t = m_op(std::forward<U>(u), accumulate(std::move(l)));
    if (m_handle.get_topology().is_root()) {
      // root; signal that reduction is done
      m_signal(std::move(t));
    }
    else {
      // non-root; send to result to parent and wait for release
      send_parent(std::move(t));
    }
  }

  ////////////////////////////////////////////////////////////////////
  /// @brief Initiates the collective with @p u for leafs.
  ////////////////////////////////////////////////////////////////////
  template<typename U>
  void init_leaf(U&& u)
  {
    if (m_handle.get_topology().is_root()) {
      // both root and leaf, thus only one process
      m_signal(std::forward<U>(u));
    }
    else {
      // forward to parent and wait for release
      send_parent(std::forward<U>(u));
    }
  }

  void notify_arrival(message_slist l)
  {
    std::lock_guard<std::mutex> lock{m_mtx};
    STAPL_RUNTIME_ASSERT(m_value);
    do_reduction(std::move(*m_value), std::move(l));
    m_value = boost::none;
  }

public:
  ////////////////////////////////////////////////////////////////////
  /// @brief Constructs an @ref reduce object.
  ///
  /// @param gid Id of the gang the collective executes in.
  /// @param cid Collective operation id.
  /// @param t   @ref topology object associated with the gang.
  /// @param f   Function to call when the collective has finished.
  /// @param op  Reduction operator.
  ////////////////////////////////////////////////////////////////////
  template<typename Function>
  reduce(const gang_id gid,
         const collective_id cid,
         topology const& t,
         Function&& f,
         BinaryOperation op = BinaryOperation{})
  : m_handle(collective::create(collective::id{gid, cid}, t)),
    m_op(std::move(op)),
    m_signal(std::forward<Function>(f))
  { }

  ~reduce(void)
  { m_handle.try_destroy(); }

  void operator()(T const& t)
  {
    std::lock_guard<std::mutex> lock{m_mtx};
    STAPL_RUNTIME_ASSERT(!m_value);
    if (m_handle.get_topology().is_leaf()) {
      init_leaf(t);
    }
    else {
      auto l = m_handle.collect([this](message_slist l)
                                { this->notify_arrival(std::move(l)); });
      if (!l.empty()) {
        do_reduction(t, std::move(l));
      }
      else {
        // wait for children
        m_value = t;
      }
    }
  }

  void operator()(T&& t)
  {
    std::lock_guard<std::mutex> lock{m_mtx};
    STAPL_RUNTIME_ASSERT(!m_value);
    if (m_handle.get_topology().is_leaf()) {
      init_leaf(std::move(t));
    }
    else {
      auto l = m_handle.collect([this](message_slist l)
                                { this->notify_arrival(std::move(l)); });
      if (!l.empty()) {
        do_reduction(std::move(t), std::move(l));
      }
      else {
        // wait for children
        m_value = std::move(t);
      }
    }
  }
};

} // namespace runtime

} // namespace stapl

#endif
