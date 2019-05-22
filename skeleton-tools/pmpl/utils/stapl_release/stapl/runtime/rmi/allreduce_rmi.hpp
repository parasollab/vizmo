/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_RUNTIME_RMI_ALLREDUCE_RMI_HPP
#define STAPL_RUNTIME_RMI_ALLREDUCE_RMI_HPP

#include "../aggregator.hpp"
#include "../context.hpp"
#include "../exception.hpp"
#include "../future.hpp"
#include "../instrumentation.hpp"
#include "../primitive_traits.hpp"
#include "../rmi_handle.hpp"
#include "../value_handle.hpp"
#include "../yield.hpp"
#include "../non_rmi/response.hpp"
#include "../request/async_rmi_request.hpp"
#include "../request/sync_rmi_request.hpp"
#include "../type_traits/callable_traits.hpp"
#include "../type_traits/is_non_commutative.hpp"
#include "../type_traits/transport_qualifier.hpp"
#include "../utility/algorithm.hpp"
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <boost/optional.hpp>

namespace stapl {

namespace runtime {

//////////////////////////////////////////////////////////////////////
/// @brief Handle to wait for values from @ref allreduce_rmi().
///
/// @tparam T               Value type.
/// @tparam BinaryOperation Reduction operator type.
/// @tparam NonCommutative  @c true for non commutative operators, otherwise
///                         @c false.
///
/// @ingroup requestBuildingBlock
//////////////////////////////////////////////////////////////////////
template<typename T,
         typename BinaryOperation,
         bool NonCommutative = is_non_commutative<BinaryOperation>::value>
class allreduce_rmi_handle
: public value_handle<T>
{
public:
  typedef T                                      value_type;
private:
  typedef std::unordered_map<location_md::id, T> cache_type;

  rmi_handle         m_handle;
  gang_md&           m_gang;
  BinaryOperation    m_op;
  /// Number of reduction steps.
  const unsigned int m_steps;
  /// Current reduction step.
  unsigned int       m_step;
  boost::optional<T> m_t;
  /// From which location a value is expected.
  location_md::id    m_wait;
  /// Out-of-order values.
  cache_type         m_cache;

  //////////////////////////////////////////////////////////////////////
  /// @brief Sends @p t to location @p id.
  //////////////////////////////////////////////////////////////////////
  void send(const location_md::id id, T&& t)
  {
    const location_md::id myid = m_handle.get_location_id();

    aggregator a{this_context::get(), m_handle, id};
    const bool on_shmem = a.is_on_shmem();
    if (on_shmem) {
      constexpr auto pmf = &allreduce_rmi_handle::template receive_operand<T&&>;
      typedef nc_async_rmi_request<
                packed_handle_type,
                decltype(pmf),
                location_md::id,
                typename transport_qualifier<T&&>::type
              > request_type;
      const std::size_t size = request_type::expected_size(myid, std::move(t));
      new(a.allocate(size)) request_type{m_handle, pmf, myid, std::move(t)};
    }
    else {
      constexpr auto pmf =
        &allreduce_rmi_handle::template receive_operand<T const&>;
      typedef nc_async_rmi_request<
                packed_handle_type,
                decltype(pmf),
                location_md::id,
                T
              > request_type;
      const std::size_t size = request_type::expected_size(myid, std::move(t));
      new(a.allocate(size)) request_type{m_handle, pmf, myid, std::move(t)};
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Sends @p t to location @p id.
  //////////////////////////////////////////////////////////////////////
  void send(const location_md::id id, T const& t)
  {
    const location_md::id myid = m_handle.get_location_id();

    aggregator a{this_context::get(), m_handle, id};
    constexpr auto pmf =
      &allreduce_rmi_handle::template receive_operand<T const&>;
    typedef nc_async_rmi_request<
              packed_handle_type,
              decltype(pmf),
              location_md::id,
              T
            > request_type;
    const std::size_t size = request_type::expected_size(myid, t);
    new(a.allocate(size)) request_type{m_handle, pmf, myid, t};
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Makes progress on the reduction.
  //////////////////////////////////////////////////////////////////////
  void make_progress(void)
  {
    const location_md::id myid = m_handle.get_location_id();
    for (++m_step; m_step<=m_steps; ++m_step) {
      // find the receiver location
      const location_md::id recv_id = (myid - myid % (0x1<<m_step));
      if (myid!=recv_id) {
        // this location is not a receiver of a value
        send(recv_id, std::move(*m_t));
        break;
      }

      // find the location id that a value is expected from
      m_wait = (myid + (0x1<<(m_step-1)));
      if (m_wait>=m_handle.get_num_locations()) {
        // no more values expected
        continue;
      }

      // check if operand is already here
      auto it = m_cache.find(m_wait);
      if (it==m_cache.end()) {
        return;
      }
      m_t = m_op(std::move(*m_t), it->second);
      m_cache.erase(it);
    }
    STAPL_RUNTIME_ASSERT(m_cache.empty());

    if (myid==0) {
      // broadcasts the value to all locations
      typedef handle_response<
                packed_handle_type, allreduce_rmi_handle
              > response_type;
      response_type{}(m_handle, std::move(*m_t));
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Receives @p u from @p id.
  //////////////////////////////////////////////////////////////////////
  template<typename U>
  void receive_operand(const location_md::id id, U&& u)
  {
    if (id!=m_wait) {
      m_cache.emplace(id, std::forward<U>(u));
    }
    else {
      m_t = m_op(std::move(*m_t), std::forward<U>(u));
      make_progress();
    }
  }

public:
  allreduce_rmi_handle(context& ctx, BinaryOperation op)
  : m_handle(ctx, this),
    m_gang(ctx.get_gang_md()),
    m_op(std::move(op)),
    m_steps(integral_ceil_log2(m_handle.get_num_locations())),
    m_step(1),
    m_wait(invalid_location_id)
  { }

  rmi_handle::reference const& get_rmi_handle(void) noexcept
  { return m_handle; }

  rmi_handle::const_reference const& get_rmi_handle(void) const noexcept
  { return m_handle; }

  //////////////////////////////////////////////////////////////////////
  /// @brief Initiates the allreduce.
  ///
  /// @tparam u Value to contribute to the reduction.
  //////////////////////////////////////////////////////////////////////
  template<typename U>
  void operator()(U&& u)
  {
    if (m_handle.get_num_locations()==1) {
      // only one location; set the reduction value directly
      this->set_value(std::forward<U>(u));
      return;
    }

    m_wait = ((m_handle.get_location_id()%2==0) ? (m_handle.get_location_id()+1)
                                                : m_handle.get_num_locations());
    if (m_wait<m_handle.get_num_locations()) {
      // this location waits value from another one; check if it arrived prior
      // to the execution of this function
      auto it = m_cache.find(m_wait);
      if (it==m_cache.end()) {
        // value not in cache; reduction will be triggered by incoming request
        m_t = std::forward<U>(u);
        return;
      }
      // value in cache; process it and do some progress
      m_t = m_op(std::forward<U>(u), it->second);
      m_cache.erase(it);
      make_progress();
    }
    else {
      // does not wait from any other location; send to the receiver location
      const location_md::id myid = m_handle.get_location_id();
      for (; m_step<=m_steps; ++m_step) {
        const location_md::id recv_id = (myid - myid % (0x1<<m_step));
        if (myid!=recv_id) {
          send(recv_id, std::forward<U>(u));
          break;
        }
      }
    }
  }
};


//////////////////////////////////////////////////////////////////////
/// @internal
/// @brief Specialization of @ref allreduce_rmi_handle for commutative
///        reductions.
///
/// @ingroup requestBuildingBlock
//////////////////////////////////////////////////////////////////////
template<typename T, typename BinaryOperation>
class allreduce_rmi_handle<T, BinaryOperation, false>
: public value_handle<T>
{
public:
  typedef T value_type;

private:
  rmi_handle         m_handle;
  gang_md&           m_gang;
  BinaryOperation    m_op;
  /// Number of values that are pending from children.
  unsigned int       m_cnt;
  boost::optional<T> m_t;

  //////////////////////////////////////////////////////////////////////
  /// @brief Sends @p t to location @p id.
  //////////////////////////////////////////////////////////////////////
  void send(const location_md::id id, T&& t)
  {
    aggregator a{this_context::get(), m_handle, id};
    const bool on_shmem = a.is_on_shmem();
    if (on_shmem) {
      constexpr auto pmf = &allreduce_rmi_handle::template receive_operand<T&&>;
      typedef nc_async_rmi_request<
                packed_handle_type,
                decltype(pmf),
                typename transport_qualifier<T&&>::type
              > request_type;
      const std::size_t size = request_type::expected_size(std::move(t));
      new(a.allocate(size)) request_type{m_handle, pmf, std::move(t)};
    }
    else {
      constexpr auto pmf =
        &allreduce_rmi_handle::template receive_operand<T const&>;
      typedef nc_async_rmi_request<
                packed_handle_type,
                decltype(pmf),
                T
              > request_type;
      const std::size_t size = request_type::expected_size(std::move(t));
      new(a.allocate(size)) request_type{m_handle, pmf, std::move(t)};
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Sends @p t to location @p id.
  //////////////////////////////////////////////////////////////////////
  void send(const location_md::id id, T const& t)
  {
    aggregator a{this_context::get(), m_handle, id};
    constexpr auto pmf =
      &allreduce_rmi_handle::template receive_operand<T const&>;
    typedef nc_async_rmi_request<
              packed_handle_type,
              decltype(pmf),
              T
            > request_type;
    const std::size_t size = request_type::expected_size(t);
    new(a.allocate(size)) request_type{m_handle, pmf, t};
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Receives @p u, performs the reduction and forwards the result to
  ///        the parent if it is not the root, otherwise broadcasts the value to
  ///        all locations.
  //////////////////////////////////////////////////////////////////////
  template<typename U>
  void receive_operand(U&& u)
  {
    STAPL_RUNTIME_ASSERT(m_cnt > 0);

    --m_cnt;

    if (m_cnt>0) {
      // more values expected
      if (!m_t)
        m_t = std::forward<U>(u);
      else
        m_t = m_op(std::move(*m_t), std::forward<U>(u));
    }
    else {
      // last value to arrive
      const location_md::id myid = m_handle.get_location_id();
      if (myid==0) {
        // broadcast the value to all locations
        STAPL_RUNTIME_ASSERT(m_t);
        typedef handle_response<
                  packed_handle_type, allreduce_rmi_handle
                > response_type;
        response_type{}(m_handle, m_op(std::move(*m_t), std::forward<U>(u)));
      }
      else {
        const location_md::id parent_id = ((myid - 1) / 2);
        if (!m_t)
          send(parent_id, std::forward<U>(u));
        else
          send(parent_id, m_op(std::move(*m_t), std::forward<U>(u)));
      }
    }
  }

public:
  allreduce_rmi_handle(context& ctx, BinaryOperation op)
  : m_handle(ctx, this),
    m_gang(ctx.get_gang_md()),
    m_op(std::move(op)),
    m_cnt(1)
  {
    const location_md::id myid = m_handle.get_location_id();
    const location_md::id n    = m_handle.get_num_locations();
    if ((2u*(myid+1u)) < n) // check if left and right children exist
      m_cnt += 2;
    else if (((2u*(myid+1u)) - 1) < n) // check if left child exists
      m_cnt += 1;
  }

  rmi_handle::reference const& get_rmi_handle(void) noexcept
  { return m_handle; }

  rmi_handle::const_reference const& get_rmi_handle(void) const noexcept
  { return m_handle; }

  //////////////////////////////////////////////////////////////////////
  /// @brief Initiates the allreduce.
  ///
  /// @tparam u Value to contribute to the reduction.
  //////////////////////////////////////////////////////////////////////
  template<typename U>
  void operator()(U&& u)
  {
    if (m_handle.get_num_locations()==1) {
      // one location, set the reduction value directly
      --m_cnt;
      this->set_value(std::forward<U>(u));
    }
    else {
      receive_operand(std::forward<U>(u));
    }
  }
};

} // namespace runtime


//////////////////////////////////////////////////////////////////////
/// @brief Allreduce RMI primitive.
///
/// The given member function is called on all locations the object is defined
/// on and the result of the local invocations of the member function are
/// combined using @p binary_op and the result is distributed to all locations
/// that made the call. This result can be retrieved through the returned
/// @ref future object.
///
/// @param op  Reduction operator.
/// @param h   Handle to the target object.
/// @param pmf Member function to invoke.
/// @param t   Arguments to pass to the member function.
///
/// @return A @ref future object with the combined return values.
///
/// @ingroup ARMICollectives
///
/// @todo Needs to take advantage of platform optimized allreduce support.
//////////////////////////////////////////////////////////////////////
template<typename BinaryOperation,
         typename Handle,
         typename MemFun,
         typename... T>
future<
  decltype(
    std::declval<BinaryOperation>()(
      std::declval<typename callable_traits<MemFun>::result_type>(),
      std::declval<typename callable_traits<MemFun>::result_type>())
  )
>
allreduce_rmi(BinaryOperation op, Handle const& h, MemFun const& pmf, T&&... t)
{
  using namespace stapl::runtime;

  static_assert(std::is_convertible<
                  Handle, typename appropriate_handle<MemFun>::type
                >::value, "handle discards qualifiers");

  context& ctx = this_context::get();

  STAPL_RUNTIME_ASSERT_MSG(h.valid(), "Invalid handle");
  STAPL_RUNTIME_ASSERT_MSG((ctx.is_base() &&
                            ctx.get_gang_id()==h.get_gang_id()),
                           "Only allowed in SPMD");

  typedef decltype(
            op(std::declval<typename callable_traits<MemFun>::result_type>(),
               std::declval<typename callable_traits<MemFun>::result_type>())
          )                                                  result_type;
  typedef allreduce_rmi_handle<result_type, BinaryOperation> return_handle_type;
  typedef active_handle_response<
            packed_handle_type, return_handle_type
          >                                                  response_type;

  std::unique_ptr<return_handle_type>
    p{new return_handle_type{ctx, std::move(op)}};
  {
    STAPL_RUNTIME_PROFILE("allreduce_rmi()", (primitive_traits::non_blocking |
                                              primitive_traits::ordered      |
                                              primitive_traits::coll         |
                                              primitive_traits::comm));

    aggregator a{ctx, h, ctx.get_location_id(), no_implicit_flush};
    typedef sync_rmi_request<
              response_type,
              packed_handle_type,
              MemFun,
              typename transport_qualifier<decltype(t)>::type...
            > request_type;
    const std::size_t size = request_type::expected_size(std::forward<T>(t)...);
    new(a.allocate(size)) request_type{*p, h, pmf, std::forward<T>(t)...};
  }

  scheduling_point(ctx);
  return future<result_type>{std::move(p)};
}

} // namespace stapl

#endif
