/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_RUNTIME_COLLECTIVE_ALLREDUCE_OBJECT_HPP
#define STAPL_RUNTIME_COLLECTIVE_ALLREDUCE_OBJECT_HPP

#include "../context.hpp"
#include "../rmi_handle.hpp"
#include "../tags.hpp"
#include "../value_handle.hpp"
#include "../yield.hpp"
#include "../communicator/reduce.hpp"
#include "../concurrency/reduction.hpp"
#include "../non_rmi/response.hpp"
#include "../type_traits/is_non_commutative.hpp"
#include <functional>
#include <memory>
#include <utility>

namespace stapl {

namespace runtime {

//////////////////////////////////////////////////////////////////////
/// @brief Performs an allreduce over all locations of the current gang.
///
/// @tparam T               Object type that the reduction happens upon.
/// @tparam BinaryOperation Binary operation function object type to be applied.
///
/// @ingroup runtimeCollectives
///
/// @todo Support non-commutative operators.
//////////////////////////////////////////////////////////////////////
template<typename T, typename BinaryOperation>
class allreduce_object
: public value_handle<T>
{
public:
  using value_type          = T;
private:
  using distmem_reduce_type = reduce<value_type, BinaryOperation>;
  using shmem_reduce_type   = reduction<value_type, BinaryOperation>;
  using response_type       =
    handle_response<packed_handle_type, allreduce_object>;

  friend response_type;

  static_assert(!is_non_commutative<BinaryOperation>::value,
                "Non-commutative operations are not supported");

  //////////////////////////////////////////////////////////////////////
  /// @brief Base class for internal implementation.
  //////////////////////////////////////////////////////////////////////
  class impl_base
  {
  public:
    virtual ~impl_base(void) = default;
    virtual void operator()(const location_md::size_type, T const&) = 0;
    virtual void operator()(const location_md::size_type, T&&) = 0;
  };

  //////////////////////////////////////////////////////////////////////
  /// @brief Implementation for distributed memory only gangs.
  //////////////////////////////////////////////////////////////////////
  class dist_impl final
  : public impl_base
  {
  private:
    distmem_reduce_type m_red;

  public:
    dist_impl(allreduce_object& o, const collective_id cid, BinaryOperation op)
    : m_red(o.get_gang_md().get_id(),
            cid,
            o.get_gang_md().get_topology(),
            [&o](T t) { o.bcast_value(std::move(t)); },
            std::move(op))
    { }

    void operator()(const location_md::size_type, T const& t) final
    { m_red(t); }

    void operator()(const location_md::size_type, T&& t) final
    { m_red(std::move(t)); }
  };

  //////////////////////////////////////////////////////////////////////
  /// @brief Implementation for shared memory only gangs.
  //////////////////////////////////////////////////////////////////////
  class shmem_impl final
  : public impl_base
  {
  private:
    allreduce_object& m_parent;
    shmem_reduce_type m_red;

  public:
    shmem_impl(allreduce_object& o, BinaryOperation op)
    : m_parent(o),
      m_red(o.get_gang_md().local_size(), std::move(op))
    { }

    void operator()(const location_md::size_type idx, T const& t) final
    {
      if (m_red(idx, t))
        m_parent.bcast_value(m_red.get());
    }

    void operator()(const location_md::size_type idx, T&& t) final
    {
      if (m_red(idx, std::move(t)))
        m_parent.bcast_value(m_red.get());
    }
  };

  //////////////////////////////////////////////////////////////////////
  /// @brief Implementation for mixed-mode gangs.
  //////////////////////////////////////////////////////////////////////
  class mixed_impl final
  : public impl_base
  {
  private:
    shmem_reduce_type   m_shmem_red;
    distmem_reduce_type m_distmem_red;

  public:
    mixed_impl(allreduce_object& o, const collective_id cid, BinaryOperation op)
    : m_shmem_red(o.get_gang_md().local_size(), op),
      m_distmem_red(o.get_gang_md().get_id(),
                    cid,
                    o.get_gang_md().get_topology(),
                    [&o](T t) { o.bcast_value(std::move(t)); },
                    std::move(op))
    { }

    void operator()(const location_md::size_type idx, T const& t) final
    {
      if (m_shmem_red(idx, t))
        m_distmem_red(m_shmem_red.get());
    }

    void operator()(const location_md::size_type idx, T&& t) final
    {
      if (m_shmem_red(idx, std::move(t)))
        m_distmem_red(m_shmem_red.get());
    }
  };

  rmi_handle                 m_handle;
  std::shared_ptr<impl_base> m_impl;

public:
  explicit allreduce_object(context& ctx,
                            BinaryOperation op = BinaryOperation{})
  : m_handle(ctx, this)
  {
    if (m_handle.get_num_locations()==1) {
      // single location gang
      return;
    }

    auto& g = ctx.get_gang_md();

    if (g.local_size()==1) {
      // single location on the process
      const collective_id cid = m_handle.internal_handle().abstract();
      m_impl = std::make_shared<dist_impl>(std::ref(*this), cid, std::move(op));
    }
    else if (g.get_description().is_on_shmem()) {
      // shared memory gang
      m_impl = g.get_shared_object<shmem_impl>(m_handle,
                                               std::ref(*this),
                                               std::move(op));
    }
    else {
      // multiple locations on the process, multiple processes
      const collective_id cid = m_handle.internal_handle().abstract();
      m_impl = g.get_shared_object<mixed_impl>(m_handle,
                                               std::ref(*this),
                                               cid,
                                               std::move(op));
    }
  }

  rmi_handle::reference const& get_rmi_handle(void) noexcept
  { return m_handle; }

  rmi_handle::const_reference const& get_rmi_handle(void) const noexcept
  { return m_handle; }

private:
  gang_md const& get_gang_md(void) const noexcept
  { return m_handle.get_location_md().get_gang_md(); }

  using value_handle<T>::set_value;

  template<typename U>
  void bcast_value(U&& u)
  { response_type{}(m_handle, std::forward<U>(u)); }

public:
  void operator()(T const& t)
  {
    if (!m_impl)
      set_value(t);
    else
      (*m_impl)(m_handle.get_location_md().local_index(), t);
  }

  void operator()(T&& t)
  {
    if (!m_impl)
      set_value(std::move(t));
    else
      (*m_impl)(m_handle.get_location_md().local_index(), std::move(t));
  }
};

} // namespace runtime

} // namespace stapl

#endif
