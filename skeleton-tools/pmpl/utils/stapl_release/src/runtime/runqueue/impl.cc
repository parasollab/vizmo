/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#include <stapl/runtime/tags.hpp>
#include <stapl/runtime/concurrency/thread_local_storage.hpp>
#include <stapl/runtime/request/rmi_executor.hpp>
#include <stapl/runtime/request/location_rpc_request.hpp>
#include <stapl/runtime/utility/bool_mutex.hpp>
#include <stapl/runtime/utility/timer.hpp>
#include <deque>
#include <iterator>
#include <memory>
#include <mutex>
#include <tuple>
#include <utility>
#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/range/size.hpp>
#include <boost/utility/typed_in_place_factory.hpp>
#ifdef STAPL_USE_PAPI
# include <stapl/runtime/utility/papi_clock.hpp>
#endif
#if defined(STAPL_RUNTIME_USE_BOOST_LOCKFREE)
# define STAPL_RUNTIME_USE_BOOST_LOCKFREE_QUEUE
# include <boost/lockfree/queue.hpp>
#elif defined(STAPL_RUNTIME_TBB_AVAILABLE)
# define STAPL_RUNTIME_USE_TBB_QUEUE
# include <tbb/concurrent_queue.h>
#endif


#ifndef STAPL_RUNTIME_POLL_INTERVAL
/// Poll timeout in milliseconds.
# define STAPL_RUNTIME_POLL_INTERVAL 200
#endif

#ifndef STAPL_RUNTIME_MAX_QUEUED_MESSAGES
/// Number of messages allowed to be queued before forcing execution.
# define STAPL_RUNTIME_MAX_QUEUED_MESSAGES 256
#endif

namespace stapl {

namespace runtime {

//////////////////////////////////////////////////////////////////////
/// @brief Returns the epoch @p m was sent from.
///
/// @ingroup runtimeMetadata
//////////////////////////////////////////////////////////////////////
static context::epoch_type get_message_epoch(message const& m) noexcept
{
  switch (m.type()) {
    case header::LOCATION_RPC:
      return m.get_extended_header<header::location_rpc>().get_epoch();

    case header::BCAST_LOCATION_RPC:
      return m.get_extended_header<header::bcast_location_rpc>().get_epoch();

    case header::RMI:
      return m.get_extended_header<header::request>().get_epoch();

    case header::BCAST_RMI:
    case header::UNORDERED_BCAST_RMI:
      return m.get_extended_header<header::bcast_request>().get_epoch();

    default:
      STAPL_RUNTIME_ERROR("Unexpected request type.");
      return context::epoch_type{};
  }
}


//////////////////////////////////////////////////////////////////////
/// @brief Returns the context id, the epoch and the process id @p m was sent
///        from as a @c std::tuple.
///
/// @param lid Location id of the receiving location.
///
/// @ingroup runtimeMetadata
//////////////////////////////////////////////////////////////////////
static std::tuple<context::id, context::epoch_type, process_id>
get_message_info(message const& m, const location_md::id lid) noexcept
{
  switch (m.type()) {
    case header::RMI: {
      auto const& h = m.get_extended_header<header::request>();
      STAPL_RUNTIME_ASSERT(lid==h.get_context_id().current.get_location_id());
      return std::make_tuple(h.get_context_id(),
                             h.get_epoch(),
                             h.get_process_id());
    }

    case header::BCAST_RMI:
    case header::UNORDERED_BCAST_RMI: {
      auto const& h = m.get_extended_header<header::bcast_request>();
      return std::make_tuple(h.make_context_id(lid),
                             h.get_epoch(),
                             h.get_process_id());
    }

    default:
      STAPL_RUNTIME_ERROR("Unexpected request type.");
      return std::tuple<context::id, context::epoch_type, process_id>{};
  }
}


// --------------------------------------------------------------------
// Mailbox / Shared runqueue
// --------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////
/// @brief Request queue.
///
/// @ingroup runtimeMetadata
//////////////////////////////////////////////////////////////////////
class STAPL_RUNTIME_CACHELINE_ALIGNED request_queue
{
private:
  //////////////////////////////////////////////////////////////////////
  /// @brief Queued request.
  //////////////////////////////////////////////////////////////////////
  class request_type
  {
  private:
    message* m_msg;
    bool     m_shared;

  public:
    constexpr explicit
    request_type(message* const m = nullptr, const bool shared = false) noexcept
    : m_msg(m),
      m_shared(shared)
    { }

    constexpr message* get(void) const noexcept
    { return m_msg; }

    constexpr bool is_shared(void) const noexcept
    { return m_shared; }
  };

#if defined(STAPL_RUNTIME_USE_BOOST_LOCKFREE_QUEUE)
  boost::lockfree::queue<request_type> m_queue;
#elif defined(STAPL_RUNTIME_USE_TBB_QUEUE)
  tbb::concurrent_queue<request_type>  m_queue;
#else
  mutable std::deque<request_type>     m_queue;
  mutable std::mutex                   m_mtx;
  mutable std::deque<request_type>     m_local_queue;
#endif

public:
  request_queue(void)
#if defined(STAPL_RUNTIME_USE_BOOST_LOCKFREE_QUEUE)
  : m_queue(128)
#endif
  { }

  ~request_queue(void)
  { STAPL_RUNTIME_ASSERT(m_queue.empty()); }

  bool empty(void) const
  {
#if defined(STAPL_RUNTIME_USE_BOOST_LOCKFREE_QUEUE) || \
    defined(STAPL_RUNTIME_USE_TBB_QUEUE)
    return m_queue.empty();
#else
    if (m_local_queue.empty()) {
      // local queue is empty, try to get requests from shared queue
      std::lock_guard<std::mutex> lock{m_mtx};
      if (m_queue.empty())
        return true;
      m_queue.swap(m_local_queue);
    }
    return false;
#endif
  }

  void push(message_ptr m)
  {
    message* const p = m.release();
#if defined(STAPL_RUNTIME_USE_BOOST_LOCKFREE_QUEUE)
    m_queue.push(request_type{p});
#elif defined(STAPL_RUNTIME_USE_TBB_QUEUE)
    m_queue.emplace(p);
#else
    std::lock_guard<std::mutex> lock{m_mtx};
    m_queue.emplace_back(p);
#endif
  }

  void push(message_shared_ptr& m)
  {
    message* const p = m.get();
#if defined(STAPL_RUNTIME_USE_BOOST_LOCKFREE_QUEUE)
    m_queue.push(request_type{p, true});
#elif defined(STAPL_RUNTIME_USE_TBB_QUEUE)
    m_queue.emplace(p, true);
#else
    std::lock_guard<std::mutex> lock{m_mtx};
    m_queue.emplace_back(p, true);
#endif
  }

  message_ptr pop(void)
  {
    request_type req;

#if defined(STAPL_RUNTIME_USE_BOOST_LOCKFREE_QUEUE)
    if (!m_queue.pop(req))
      return nullptr; // queue is empty
#elif defined(STAPL_RUNTIME_USE_TBB_QUEUE)
    if (!m_queue.try_pop(req))
      return nullptr; // queue is empty
#else
    if (empty())
      return nullptr;
    req = m_local_queue.front();
    m_local_queue.pop_front();
#endif

    message* const p = req.get();
    return (!req.is_shared() ? message_ptr{p} : make_message_ptr(p));
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Mailbox for multiple threads that are contiguously numbered.
///
/// It is a data structure that keeps requests that go to each thread and
/// offers the ability to place a request to multiple threads.
///
/// This implementation is based on an array and therefore can accept indexes
/// in the range <tt>[offset, ... , offset + nthreads - 1]</tt>.
///
/// @ingroup runtimeMetadata
//////////////////////////////////////////////////////////////////////
class array_mailbox
{
public:
  using id        = location_id;
  using slot_type = request_queue;
  using size_type = std::size_t;

private:
  std::unique_ptr<slot_type[]> m_slots;
  const size_type              m_size;
  const size_type              m_offset;

public:
  array_mailbox(const size_type offset, const size_type n)
  : m_slots(new slot_type[n]),
    m_size(n),
    m_offset(offset)
  { STAPL_RUNTIME_ASSERT(n>1); }

  array_mailbox(array_mailbox const& other)
  : m_slots(new slot_type[other.m_size]),
    m_size(other.m_size),
    m_offset(other.m_offset)
  { }

  size_type size(void) const noexcept
  { return m_size; }

  //////////////////////////////////////////////////////////////////////
  /// @brief Pushes the request to slot with id @p i.
  //////////////////////////////////////////////////////////////////////
  void push(const id i, message_ptr m)
  {
    const size_type idx = (i - m_offset);
    STAPL_RUNTIME_ASSERT((i >= m_offset) && (idx < m_size));
    m_slots[idx].push(std::move(m));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Pushes the request to any slot.
  ///
  /// @todo Needs load-balancing to avoid always sending to the first slot.
  //////////////////////////////////////////////////////////////////////
  void push_any(message_ptr m)
  { m_slots[0].push(std::move(m)); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Pushes the request to all slots.
  ///
  /// @todo Optimize for unordered requests.
  //////////////////////////////////////////////////////////////////////
  void push_all(const bool /*ordered*/, message_ptr m)
  {
    message_shared_ptr sm(std::move(m), m_size);
    for (auto idx = 0u; idx < m_size; ++idx)
      m_slots[idx].push(sm);
    sm.detach();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Pushes the request to the subset of slots with ids in @p r.
  ///
  /// @todo Optimize for unordered requests.
  //////////////////////////////////////////////////////////////////////
  template<typename Range>
  void push_range(Range const& r, const bool /*ordered*/, message_ptr m)
  {
    const auto n = r.size();
    STAPL_RUNTIME_ASSERT(n>0);
    if (n==1) {
      // only one location in the range
      const size_type idx = (*std::begin(r) - m_offset);
      STAPL_RUNTIME_ASSERT((*std::begin(r) >= m_offset) && (idx < m_size));
      m_slots[idx].push(std::move(m));
    }
    else {
      message_shared_ptr sm(std::move(m), n);
      for (auto const& i : r) {
        const size_type idx = (i - m_offset);
        STAPL_RUNTIME_ASSERT((i >= m_offset) && (idx < m_size));
        m_slots[idx].push(sm);
      }
      sm.detach();
    }
  }

  slot_type& get_slot(const id i) noexcept
  {
    const size_type idx = (i - m_offset);
    STAPL_RUNTIME_ASSERT((i >= m_offset) && (idx < m_size));
    return m_slots[idx];
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Mailbox for multiple threads with arbitrary numbering.
///
/// It is a data structure that keeps requests that go to each thread and
/// offers the ability to place a request to multiple threads.
///
/// This implementation is based on an associative container and therefore can
/// accept arbitrary indexing for the individual slots.
///
/// @ingroup runtimeMetadata
//////////////////////////////////////////////////////////////////////
class mailbox
{
public:
  using id        = location_id;
  using slot_type = request_queue;
  using size_type = std::size_t;

private:
  boost::unordered_map<id, slot_type> m_slots;

public:
  template<typename Range>
  explicit mailbox(Range const& r)
  {
    for (auto const& i : r) {
      STAPL_RUNTIME_ASSERT(m_slots.count(i)==0);
      m_slots.emplace(std::piecewise_construct,
                      std::forward_as_tuple(i),
                      std::forward_as_tuple());
    }
    STAPL_RUNTIME_ASSERT(!m_slots.empty());
  }

  mailbox(mailbox const& other)
  {
    m_slots.reserve(other.m_slots.size());
    for (auto const& i : other.m_slots) {
      m_slots.emplace(std::piecewise_construct,
                      std::forward_as_tuple(i.first),
                      std::forward_as_tuple());
    }
  }

  size_type size(void) const noexcept
  { return m_slots.size(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Pushes the request to slot @p i.
  //////////////////////////////////////////////////////////////////////
  void push(const id i, message_ptr m)
  {
    STAPL_RUNTIME_ASSERT(m_slots.count(i)==1);
    m_slots[i].push(std::move(m));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Pushes the request to any slot.
  ///
  /// @todo Needs load-balancing to avoid always sending to the first slot.
  //////////////////////////////////////////////////////////////////////
  void push_any(message_ptr m)
  { m_slots.begin()->second.push(std::move(m)); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Pushes the request to all slots.
  ///
  /// @todo Optimize for unordered requests.
  //////////////////////////////////////////////////////////////////////
  void push_all(const bool /*ordered*/, message_ptr m)
  {
    message_shared_ptr sm(std::move(m), m_slots.size());
    for (auto& t : m_slots) {
      auto& s = t.second;
      s.push(sm);
    }
    sm.detach();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Pushes the request to the slots with ids in range @p r.
  ///
  /// @todo Optimize for unordered requests.
  //////////////////////////////////////////////////////////////////////
  template<typename Range>
  void push_range(Range const& r, const bool /*ordered*/, message_ptr m)
  {
    const auto n = r.size();
    STAPL_RUNTIME_ASSERT(n>0);
    if (n==1) {
      // only one location in the range
      const auto i = *std::begin(r);
      STAPL_RUNTIME_ASSERT(m_slots.count(i)==1);
      m_slots[i].push(std::move(m));
    }
    else {
      message_shared_ptr sm(std::move(m), n);
      // add to all slots in the range
      for (auto const& i : r) {
        STAPL_RUNTIME_ASSERT(m_slots.count(i)==1);
        m_slots[i].push(sm);
      }
      sm.detach();
    }
  }

  slot_type& get_slot(const id i) noexcept
  {
    STAPL_RUNTIME_ASSERT(m_slots.count(i)==1);
    return m_slots[i];
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Shared runqueue implementation.
///
/// It is a mailbox structure that keeps the messages that are for each locally
/// managed location.
///
/// @ingroup runtimeMetadata
//////////////////////////////////////////////////////////////////////
class shared_runqueue::shared_runqueue_impl
{
public:
  using size_type  = std::size_t;
  using queue_type = request_queue;
private:
  //////////////////////////////////////////////////////////////////////
  /// @brief Internal mailbox type.
  //////////////////////////////////////////////////////////////////////
  enum mailbox_type
  {
    NONE = 0x0,
    SINGLE,
    ARRAY,
    ARBITRARY
  };

  mailbox_type    m_type;
  union
  {
    queue_type    m_single_mbox;
    array_mailbox m_array_mbox;
    mailbox       m_mbox;
  };

public:
  shared_runqueue_impl(void)
  : m_type(SINGLE)
  { new(&m_single_mbox) queue_type; }

  shared_runqueue_impl(const location_id first, const size_type n)
  : m_type((n==1 ? SINGLE : ARRAY))
  {
    switch (m_type) {
      case SINGLE:
        new(&m_single_mbox) queue_type;
        break;
      case ARRAY:
        new(&m_array_mbox) array_mailbox{first, n};
        break;
      default:
        STAPL_RUNTIME_ERROR("Incorrect mailbox type.");
        break;
    }
  }

  template<typename T>
  explicit shared_runqueue_impl(location_range_wrapper<T> const& r)
  : m_type((r.size()==1 ? SINGLE : ARBITRARY))
  {
    switch (m_type) {
      case SINGLE:
        new(&m_single_mbox) queue_type;
        break;
      case ARBITRARY:
        new(&m_mbox) mailbox{r};
        break;
      default:
        STAPL_RUNTIME_ERROR("Incorrect mailbox type.");
        break;
    }
  }

  shared_runqueue_impl(shared_runqueue_impl const& other)
  : m_type(other.m_type)
  {
    switch (m_type) {
      case SINGLE:
        new(&m_single_mbox) queue_type;
        break;
      case ARRAY:
        new(&m_array_mbox) array_mailbox{other.m_array_mbox};
        break;
      case ARBITRARY:
        new(&m_mbox) mailbox{other.m_mbox};
        break;
      default:
        STAPL_RUNTIME_ERROR("Incorrect mailbox type.");
        break;
    }
  }

  ~shared_runqueue_impl(void)
  {
    switch (m_type) {
      case SINGLE:
        m_single_mbox.~queue_type();
        break;
      case ARRAY:
        m_array_mbox.~array_mailbox();
        break;
      case ARBITRARY:
        m_mbox.~mailbox();
        break;
      default:
        STAPL_RUNTIME_ERROR("Incorrect mailbox type.");
        break;
    }
  }

  size_type size(void) const noexcept
  {
    switch (m_type) {
      case SINGLE:
        return 1;
      case ARRAY:
        return m_array_mbox.size();
      case ARBITRARY:
        return m_mbox.size();
      default:
        STAPL_RUNTIME_ERROR("Incorrect mailbox type.");
        return 0;
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Enqueues a message for location @p lid.
  ///
  /// @warning The location has to be locally managed.
  //////////////////////////////////////////////////////////////////////
  void enqueue(const location_id lid, message_ptr m)
  {
    switch (m_type) {
      case SINGLE:
        m_single_mbox.push(std::move(m));
        break;
      case ARRAY:
        m_array_mbox.push(lid, std::move(m));
        break;
      case ARBITRARY:
        m_mbox.push(lid, std::move(m));
        break;
      default:
        STAPL_RUNTIME_ERROR("Incorrect mailbox type.");
        break;
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Enqueues a message for any locally managed location.
  //////////////////////////////////////////////////////////////////////
  void enqueue_any(message_ptr m)
  {
    switch (m_type) {
      case SINGLE:
        m_single_mbox.push(std::move(m));
        break;
      case ARRAY:
        m_array_mbox.push_any(std::move(m));
        break;
      case ARBITRARY:
        m_mbox.push_any(std::move(m));
        break;
      default:
        STAPL_RUNTIME_ERROR("Incorrect mailbox type.");
        break;
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Enqueues a message for all the locally managed locations.
  //////////////////////////////////////////////////////////////////////
  void enqueue_all(const bool ordered, message_ptr m)
  {
    switch (m_type) {
      case SINGLE:
        m_single_mbox.push(std::move(m));
        break;
      case ARRAY:
        m_array_mbox.push_all(ordered, std::move(m));
        break;
      case ARBITRARY:
        m_mbox.push_all(ordered, std::move(m));
        break;
      default:
        STAPL_RUNTIME_ERROR("Incorrect mailbox type.");
        break;
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Enqueues a message for the range @p r of locally managed locations.
  //////////////////////////////////////////////////////////////////////
  template<typename Range>
  void enqueue_range(Range const& r, const bool ordered, message_ptr m)
  {
    switch (m_type) {
      case SINGLE:
        STAPL_RUNTIME_ASSERT(boost::size(r)==1);
        m_single_mbox.push(std::move(m));
        break;
      case ARRAY:
        m_array_mbox.push_range(r, ordered, std::move(m));
        break;
      case ARBITRARY:
        m_mbox.push_range(r, ordered, std::move(m));
        break;
      default:
        STAPL_RUNTIME_ERROR("Incorrect mailbox type.");
        break;
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the queue for location @p lid.
  //////////////////////////////////////////////////////////////////////
  queue_type& get_queue(const location_id lid) noexcept
  {
    switch (m_type) {
      case SINGLE:
        return m_single_mbox;
      case ARRAY:
        return m_array_mbox.get_slot(lid);
      case ARBITRARY:
        return m_mbox.get_slot(lid);
      default:
        STAPL_RUNTIME_ERROR("Incorrect mailbox type.");
        return m_single_mbox;
    }
  }
};


// --------------------------------------------------------------------
// Runqueue
// --------------------------------------------------------------------

/// Intrusive list hook.
using list_hook_type =
  boost::intrusive::list_base_hook<
    boost::intrusive::link_mode<boost::intrusive::auto_unlink>>;


//////////////////////////////////////////////////////////////////////
/// @brief Wrapper to store a @ref context and allow queueing it into a
///        @c boost::intrusive::list.
///
/// @ingroup runtimeMetadata
//////////////////////////////////////////////////////////////////////
class ctx_impl
: public context,
  public list_hook_type
{
public:
  using size_type = std::size_t;

private:
  message_slist m_incoming;
  rmi_executor  m_ex;
  bool_mutex    m_executing;

  bool is_idle(void) const noexcept
  { return (!m_executing.is_locked() && m_incoming.empty() && m_ex.empty()); }

public:
  ctx_impl(context::id const& cid, location_md& l, const process_id pid)
  : context(cid, l, pid)
  { STAPL_RUNTIME_ASSERT(!this->is_base()); }

  ~ctx_impl(void)
  {
    if (!is_idle())
      STAPL_RUNTIME_ERROR("There are outstanding RMI requests");
  }

  template<typename Runqueue>
  void message_push(message_ptr m, const epoch_type e, Runqueue& rq)
  {
    STAPL_RUNTIME_ASSERT(!is_base());
    if (is_idle()) {
      // context was idle, move it to the waiting queue
      m_incoming.push_back(std::move(m));
      rq.context_waiting(*this, e);
    }
    else {
      m_incoming.push_back(std::move(m));
    }
  }

  template<typename Runqueue>
  size_type operator()(const bool light_yield, Runqueue& rq)
  {
    STAPL_RUNTIME_ASSERT(!m_incoming.empty() || !m_ex.empty());

    std::unique_lock<decltype(m_executing)> lock{m_executing};

    if (!m_ex.empty()) {
      // process requests left over from previous invocation
      const bool done = m_ex(*this);
      if (!done)
        return 0;
      if (m_incoming.empty()) {
        // no more requests, become idle
        lock.unlock();
        rq.context_idle(*this);
        return 1;
      }
    }

    // process pending requests
    for (size_type n = 0; ;) {
      auto const& m = m_incoming.front();
      const auto e  = get_message_epoch(m);

      // if request in future epoch, become blocked
      if (e > get_location_md().get_epoch()) {
        rq.context_blocked(*this, e);
        return n;
      }

      // inform of the new epoch
      this->set_epoch(e);

      // process requests, if could not process one, return
      const bool done = m_ex(*this, m_incoming.pop_front());
      if (!done)
        return n;

      ++n;

      // if there are no more requests, become idle
      if (m_incoming.empty()) {
        lock.unlock();
        rq.context_idle(*this);
        return n;
      }

      // if only some requests had to be processed, return
      if (light_yield) {
        auto const& m = m_incoming.front();
        const auto e  = get_message_epoch(m);
        rq.context_waiting(*this, e);
        return n;
      }
    }
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Location RMI and RPC runqueue.
///
/// This class provides support for the scheduling of requests. It enforces the
/// Causal RMI Ordering model by keeping an @c unordered_map of @ref context
/// objects and queue the requests for each one of them.
///
/// @ingroup runtimeMetadata
///
/// @todo It should be evaluated if @ref context objects should be created on
///       the stack and execute requests or created inside the unordered map.
/// @todo This class gets a list of messages from the
///       @ref shared_runqueue::shared_runqueue_impl and then distributes it to
///       the map of context queues. It is not clear if this is the fastest way
///       to do it.
//////////////////////////////////////////////////////////////////////
class runqueue::runqueue_impl
: public list_hook_type
{
private:
  using size_type          = std::size_t;
  using epoch_type         = runqueue::epoch_type;
  using queue_type         = request_queue;
  /// Map type of epoch and RPC requests.
  using epoch_rpc_map_type = boost::unordered_map<epoch_type, message_slist>;

  //////////////////////////////////////////////////////////////////////
  /// @brief Hash function object for @ref context_id objects.
  //////////////////////////////////////////////////////////////////////
  struct ctx_hasher
  {
    std::size_t operator()(context::id const& id) const noexcept
    {
      std::size_t seed = 0;
      boost::hash_combine(seed, id.initiator);
      boost::hash_combine(seed, id.source);
      boost::hash_combine(seed, id.intragang);
      boost::hash_combine(seed, id.nesting);
      return seed;
    }
  };

  /// Map type of @ref context_id and @ref ctx_impl.
  using id_ctx_map_type =
    boost::unordered_map<context::id, ctx_impl, ctx_hasher>;

  /// List of @ref ctx_impl objects.
  using ctx_list =
    typename boost::intrusive::make_list<
               list_hook_type, boost::intrusive::constant_time_size<false>
             >::type;

  /// Map type of epoch and @ref ctx_list objects.
  using epoch_ctx_map_type = boost::unordered_map<epoch_type, ctx_list>;

#ifdef STAPL_USE_PAPI
  using timer_type = timer<papi_clock>;
#else
  using timer_type = timer<>;
#endif

  //////////////////////////////////////////////////////////////////////
  /// @brief Enumerates over the @ref ctx_impl objects that are runnable.
  ///
  /// This class creates a dummy object to act as a marker so that when
  /// processing the list there is a well known point to stop.
  //////////////////////////////////////////////////////////////////////
  struct enumerator
  : public list_hook_type
  {
    runqueue_impl& m_rq;
    bool           m_owns_marker;

    explicit enumerator(runqueue_impl& rq) noexcept
    : m_rq(rq),
      m_owns_marker(false)
    {
      STAPL_RUNTIME_ASSERT(!m_rq.m_waiting_ctx.empty());
      if (!(m_rq.m_marker)) {
        // there is no marker, add one that you own
        m_owns_marker = true;
        m_rq.m_waiting_ctx.push_back(*this);
        m_rq.m_marker = this;
      }
    }

    ctx_impl* next(void) noexcept
    {
      // in the worst case only the marker exists
      STAPL_RUNTIME_ASSERT(!m_rq.m_waiting_ctx.empty());
      list_hook_type& p = m_rq.m_waiting_ctx.front();
      m_rq.m_waiting_ctx.pop_front();
      if (&p==m_rq.m_marker) {
        // marker found, stop processing loop
        if (m_owns_marker)
          m_rq.m_marker = nullptr;
        else
          m_rq.m_waiting_ctx.push_back(p);
        return nullptr;
      }
      return static_cast<ctx_impl*>(&p);
    }
  };

  /// Poll interval in milliseconds.
  static std::chrono::milliseconds s_poll_interval;

public:
  static void initialize(option const& opts)
  {
    const auto i = opts.get<unsigned int>("STAPL_RUNTIME_POLL_INTERVAL",
                                          STAPL_RUNTIME_POLL_INTERVAL);
    if (i<1)
      STAPL_RUNTIME_ERROR("STAPL_RUNTIME_POLL_INTERVAL has to be a positive "
                          "number.");
    s_poll_interval = std::chrono::milliseconds{i};
  }

  static void finalize(void) noexcept
  { }

  static runqueue_impl* create(void* p, location_md& l)
  { return new(p) runqueue_impl{l}; }

  static void destroy(runqueue_impl* r) noexcept
  { r->~runqueue_impl(); }

private:
  /// Actual @ref runqueue object.
  location_md&                      m_location;
  /// Message queue associated with this @ref runqueue_impl.
  queue_type&                       m_queue;

  /// Blocked RPCs from a future epoch.
  epoch_rpc_map_type                m_blocked_rpc;
  /// Unfinished RPCs.
  std::deque<location_rpc_executor> m_pending_rpc;

  /// All @ref ctx_impl objects.
  id_ctx_map_type                   m_all_ctx;
  /// Waiting @ref ctx_impl objects.
  ctx_list                          m_waiting_ctx;
  /// Blocked @ref ctx_impl objects from a future epoch.
  epoch_ctx_map_type                m_blocked_ctx;

  /// Number of pending requests.
  size_type                         m_pending;
  /// Marker to stop executing when @ref enumerator is active.
  list_hook_type*                   m_marker;
  /// Poll timer.
  timer_type                        m_poll_timer;
  /// Flag to determine if the location is currently inside an @ref rmi_fence().
  bool                              m_in_fence;
  /// Number of times the @ref runqueue_impl::operator() is recursively called.
  size_type                         m_processing;
  /// Associated base @ref context. If @c nullptr, then runqueue inactive.
  context*                          m_base_ctx;

  explicit runqueue_impl(location_md& l)
  : m_location(l),
    m_queue(l.get_gang_md().get_runqueue().get_impl().get_queue(l.get_id())),
    m_pending(0),
    m_marker(nullptr),
    m_poll_timer(s_poll_interval),
    m_in_fence(false),
    m_processing(0),
    m_base_ctx(nullptr)
  { m_poll_timer.reset(); }

  ~runqueue_impl(void)
  {
    STAPL_RUNTIME_ASSERT(!m_marker && !m_in_fence && !is_active());
    if (!empty())
      STAPL_RUNTIME_ERROR("There are outstanding requests");
  }

  bool is_overloaded(void) const noexcept
  { return (m_pending>STAPL_RUNTIME_MAX_QUEUED_MESSAGES); }

public:
  location_md& get_location_md(void) noexcept
  { return m_location; }

  bool empty(void) const noexcept
  { return (m_pending==0 && m_queue.empty()); }

  bool is_active(void) const noexcept
  { return (m_base_ctx || (m_processing!=0)); }

  void set_base_context(context& base_ctx) noexcept
  {
    STAPL_RUNTIME_ASSERT(!m_base_ctx && (m_processing<2));
    m_base_ctx = &base_ctx;
  }

  void unset_base_context(void) noexcept
  {
    STAPL_RUNTIME_ASSERT(m_base_ctx);
    m_base_ctx = nullptr;
  }

  context* get_base_context(void) const noexcept
  { return m_base_ctx; }

  unsigned int yield_intensity(void) const noexcept
  {
    if (m_in_fence || m_processing!=0)
      return 0; // already servicing requests

    if (is_overloaded())
      return 2; // runqueue has too many requests

    if (m_poll_timer.expired())
      return 1; // some progress has to be made

    return 0;
  }

  template<typename Stack>
  runqueue::yield_status operator()(Stack& stack, bool light_yield)
  {
    ++m_processing;

    size_type num_exec_rpc = 0; // number of executed RPC requests

    // process pending RPCs; if still not done, put them back in pending queue
    for (size_type i = m_pending_rpc.size(); i > 0; --i) {
      location_rpc_executor lre{std::move(m_pending_rpc.front())};
      m_pending_rpc.pop_front();
      const bool done = lre(m_location);
      if (!done)
        m_pending_rpc.emplace_back(std::move(lre));
      else
        ++num_exec_rpc;
    }

    // process incoming requests
    for (auto m = m_queue.pop(); m; m = m_queue.pop()) {
      ++m_pending;
      switch (m->type()) {
        case header::LOCATION_RPC:
        case header::BCAST_LOCATION_RPC: {
          const auto e = get_message_epoch(*m);
          if (e<=m_location.get_epoch()) {
            // RPC immediately executable
            location_rpc_executor lre{std::move(m)};
            const bool done = lre(m_location);
            if (!done)
              m_pending_rpc.emplace_back(std::move(lre));
            else
              ++num_exec_rpc;
          }
          else {
            // RPC in future epoch, has to go to blocked queue
            m_blocked_rpc[e].push_back(std::move(m));
          }
        } break;

        default: {
          const auto t = get_message_info(*m, m_location.get_id());
          auto it      = m_all_ctx.find(std::get<0>(t));
          if (it==m_all_ctx.end()) {
            auto r =
              m_all_ctx.emplace(std::piecewise_construct,
                                std::forward_as_tuple(std::get<0>(t)),
                                std::forward_as_tuple(std::get<0>(t),
                                                      m_location,
                                                      std::get<2>(t)));
            STAPL_RUNTIME_ASSERT(r.second);
            it = r.first;
          }
          auto& c = it->second;
          c.message_push(std::move(m), std::get<1>(t), *this);
        } break;
      }
    }
    m_pending -= num_exec_rpc;

    if (m_waiting_ctx.empty()) {
      // no RMIs waiting to be executed; we can safely return
      --m_processing;
      if (num_exec_rpc!=0)
        return runqueue::YIELDED;
      return ((m_processing==0) ? runqueue::IDLE_CAN_BLOCK
                                : runqueue::IDLE_CANNOT_BLOCK);
    }

    light_yield = (is_overloaded() ? false : light_yield);

    // process waiting requests
    size_type num_exec_rmi = 0; // number of executed RMI requests
    enumerator e{*this};
    for (auto* ctx = e.next(); ctx; ctx = e.next()) {
      stack.push(*ctx);
      num_exec_rmi += (*ctx)(light_yield, *this);
      stack.pop();
    }
    m_pending -= num_exec_rmi;

    --m_processing;
    if (m_processing==0) {
      m_poll_timer.reset();
      if ((num_exec_rmi + num_exec_rpc)!=0)
        return runqueue::YIELDED;
      return runqueue::IDLE_CAN_BLOCK;
    }

    if ((num_exec_rmi + num_exec_rpc)!=0)
      return runqueue::YIELDED;
    return runqueue::IDLE_CANNOT_BLOCK;
  }

  void fence_enter(void) noexcept
  {
    STAPL_RUNTIME_ASSERT(!m_in_fence);
    m_in_fence = true;
  }

  void fence_exit(const epoch_type e)
  {
    STAPL_RUNTIME_ASSERT(m_in_fence);
    m_in_fence = false;
    advance_epoch(e);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Make runnable anything that was blocked because of the epoch.
  //////////////////////////////////////////////////////////////////////
  void advance_epoch(const epoch_type e)
  {
    auto rit = m_blocked_rpc.find(e);
    if (rit!=m_blocked_rpc.end()) {
      // make RPCs runnable by adding them to the pending queue
      message_slist& sl = rit->second;
      STAPL_RUNTIME_ASSERT(!sl.empty());
      do {
        m_pending_rpc.emplace_back(sl.pop_front());
      } while (!sl.empty());
      m_blocked_rpc.erase(rit);
    }

    auto cit = m_blocked_ctx.find(e);
    if (cit!=m_blocked_ctx.end()) {
      // make RMIs runnable by adding them to the waiting queue
      ctx_list& sl = cit->second;
      m_waiting_ctx.splice(m_waiting_ctx.begin(), sl);
      m_blocked_ctx.erase(cit);
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Add the @ref ctx_impl to the idle queue.
  ///
  /// If @p ctx is in a big nesting level (>2), then it is deleted.
  ///
  /// @todo Implement a deletion policy (LRU, LFU, something else?). Another
  ///       option would be if m_all_ctx.size()>sqrt(gang.size()) delete_some();
  //////////////////////////////////////////////////////////////////////
  void context_idle(ctx_impl& ctx)
  {
    STAPL_RUNTIME_ASSERT(!ctx.is_linked());
    ctx.flush();
    if (!ctx.is_intragang() || ctx.get_nesting()>2)
      m_all_ctx.erase(ctx.get_id());
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Add the @ref ctx_impl to the blocked queue, because it has requests
  ///        in a future epoch.
  //////////////////////////////////////////////////////////////////////
  void context_blocked(ctx_impl& ctx, const epoch_type e)
  {
    STAPL_RUNTIME_ASSERT(!ctx.is_linked() && (e>m_location.get_epoch()));
    ctx.flush();
    m_blocked_ctx[e].push_back(ctx);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Add the @ref ctx_impl to the waiting queue, unless the epoch is in
  ///        the future.
  //////////////////////////////////////////////////////////////////////
  void context_waiting(ctx_impl& ctx, const epoch_type e)
  {
    STAPL_RUNTIME_ASSERT(!ctx.is_linked());
    if (e<=m_location.get_epoch()) {
      if (!m_in_fence || !m_marker)
        m_waiting_ctx.push_back(ctx);
      else
        m_waiting_ctx.insert(m_waiting_ctx.iterator_to(*m_marker), ctx);
    }
    else {
      context_blocked(ctx, e);
    }
  }
};
std::chrono::milliseconds runqueue::runqueue_impl::s_poll_interval;


//////////////////////////////////////////////////////////////////////
/// @brief Thread stack information object.
///
/// @ingroup runtimeMetadata
///
/// @todo Release any contexts that are associated with it for correct
///       integration with external multithreaded code that is not STAPL-aware.
//////////////////////////////////////////////////////////////////////
class stack
{
private:
  //////////////////////////////////////////////////////////////////////
  /// @brief Wrapper for holding placeholders for @ref context objects or
  ///        @ref context objects in the stack.
  //////////////////////////////////////////////////////////////////////
  class element_type
  {
  private:
    union
    {
      context*                m_context;
      location_md*            m_location;
    };
    boost::optional<context>* m_placeholder;

  public:
    explicit element_type(context& ctx) noexcept
    : m_context(&ctx),
      m_placeholder(nullptr)
    { }

    explicit element_type(boost::optional<context>& pl) noexcept
    : m_location(nullptr),
      m_placeholder(&pl)
    { }

    element_type(boost::optional<context>& pl, location_md& l) noexcept
    : m_location(&l),
      m_placeholder(&pl)
    { m_location->add_ref(); }

    element_type(element_type const&) = delete;
    element_type& operator=(element_type const&) = delete;

    ~element_type(void)
    {
      if (!has_context() && m_location)
        m_location->release();
    }

    bool has_context(void) const noexcept
    { return !m_placeholder; }

    context& get_context(void) const noexcept
    {
      STAPL_RUNTIME_ASSERT(has_context());
      return *m_context;
    }

    boost::optional<context>& get_placeholder(void) const noexcept
    {
      STAPL_RUNTIME_ASSERT(!has_context());
      return *m_placeholder;
    }

    location_md* get_location_md(void) const noexcept
    {
      STAPL_RUNTIME_ASSERT(!has_context());
      return m_location;
    }

    context* try_get_context(void) const noexcept
    { return (has_context() ? m_context : nullptr); }
  };

  using runqueue_impl_list =
    typename boost::intrusive::make_list<
               runqueue::runqueue_impl,
               boost::intrusive::constant_time_size<false>
             >::type;

  /// Common gang metadata for gangs of 1 location.
  common_gang_md           m_single_loc_md;
  /// Context stack.
  std::deque<element_type> m_ctx_stack;
  /// Runqueues of this thread. Head is the active one.
  runqueue_impl_list       m_runqueues;

public:
  stack(void)
  { m_single_loc_md.add_ref(); }

  stack(stack const&) = delete;
  stack& operator=(stack const&) = delete;

  ~stack(void)
  {
    if (!m_single_loc_md.unique() ||
        !m_ctx_stack.empty()      ||
        !m_runqueues.empty())
      STAPL_RUNTIME_ERROR("Destroying stack while location(s) active on it.");
  }

private:
  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a new @ref location_md object for a single location gang.
  //////////////////////////////////////////////////////////////////////
  location_md* create_single_location_md(const gang_md::id parent_gid)
  {
    gang_md* const g = new gang_md{parent_gid, m_single_loc_md};
    return new location_md{0, *g};
  }

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the @ref context on the top of the stack.
  ///
  /// If the @ref context creation was deferred (e.g. one location gangs defer
  /// the creation of metadata) then this function will create all the required
  /// metadata and return the associated @ref context object.
  ///
  /// If the location metadata is already known (e.g. during gang switching)
  /// then it will be used to create the @ref context.
  ///
  /// @warning If this function encounters a gang of one location without
  ///          initialized metadata, it will create the required metadata. Since
  ///          the creation of the metadata requires information about the
  ///          parent gang id, this may lead to a recursive call to @ref top()
  ///          until fully initialized location metadata is encountered.
  //////////////////////////////////////////////////////////////////////
  context& top(void)
  {
    STAPL_RUNTIME_ASSERT(!m_ctx_stack.empty());

    // if context is available, return it
    if (m_ctx_stack.back().has_context())
      return m_ctx_stack.back().get_context();

    // create or switch to existing base context
    boost::intrusive_ptr<location_md> l = m_ctx_stack.back().get_location_md();
    boost::optional<context>& pl        = m_ctx_stack.back().get_placeholder();
    m_ctx_stack.pop_back();

    if (!l) {
      // create metadata and new base context
      const gang_md::id parent_id = top().get_gang_id();
      l = create_single_location_md(parent_id);
    }
    else {
      // attempt to switch to the base context associated with the metadata
      auto& rq         = l->get_runqueue().get_impl();
      auto* const bctx = rq.get_base_context();
      if (bctx) {
        // base context already exists
        m_ctx_stack.emplace_back(*bctx);
        return *bctx;
      }
    }

    // create context in placeholder; it will push context to stack
    pl = boost::in_place<context>(std::ref(*l));
    STAPL_RUNTIME_ASSERT((&(m_ctx_stack.back().get_context())==&(pl.get())));
    return pl.get();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns a pointer to the @ref context on the top of the stack.
  ///
  /// If the @ref context creation was deferred, then it returns @c nullptr.
  //////////////////////////////////////////////////////////////////////
  context* try_top(void)
  {
    if (m_ctx_stack.empty())
      return nullptr; // stack is empty

    auto* const ctx = m_ctx_stack.back().try_get_context();
    if (ctx)
      return ctx; // context found

    // attempt to switch to existing base context
    boost::intrusive_ptr<location_md> l = m_ctx_stack.back().get_location_md();
    if (!l)
      return nullptr;

    auto& rq         = l->get_runqueue().get_impl();
    auto* const bctx = rq.get_base_context();
    if (bctx) {
      // base context already exists
      m_ctx_stack.pop_back();
      m_ctx_stack.emplace_back(*bctx);
      return bctx;
    }

    // create context in placeholder; it will push context to stack
    boost::optional<context>& pl = m_ctx_stack.back().get_placeholder();
    m_ctx_stack.pop_back();
    pl = boost::in_place<context>(std::ref(*l));
    STAPL_RUNTIME_ASSERT((&(m_ctx_stack.back().get_context())==&(pl.get())));
    return &(pl.get());
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns a pointer to the @ref context on the top of the stack.
  ///
  /// If the @ref context creation was deferred, then it returns @c nullptr.
  //////////////////////////////////////////////////////////////////////
  context* peek(void) const noexcept
  {
    return (m_ctx_stack.empty() ? nullptr
                                : m_ctx_stack.back().try_get_context());
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Pushes a base @ref context on the stack and makes its runqueue
  ///        active.
  //////////////////////////////////////////////////////////////////////
  void push_base(context& ctx)
  {
    STAPL_RUNTIME_ASSERT(ctx.is_base());
    auto& rq = ctx.get_location_md().get_runqueue().get_impl();
    rq.unlink();
    m_runqueues.push_front(rq);
    m_ctx_stack.emplace_back(ctx);
    rq.set_base_context(ctx);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Pops a base @ref context from the stack.
  //////////////////////////////////////////////////////////////////////
  void pop_base(void)
  {
    STAPL_RUNTIME_ASSERT(!m_ctx_stack.empty());
    auto& ctx = m_ctx_stack.back().get_context();
    STAPL_RUNTIME_ASSERT(ctx.is_base());
    auto& rq = ctx.get_location_md().get_runqueue().get_impl();
    rq.unset_base_context();
    m_ctx_stack.pop_back();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Pushes a placeholder for a new @ref context on the stack.
  //////////////////////////////////////////////////////////////////////
  void push_placeholder(boost::optional<context>& pl)
  { m_ctx_stack.emplace_back(pl); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Switches to the base @ref context of @p l.
  ///
  /// The location metadata @p l will be used to either create a new base
  /// context in @p pl or to switch to an existing one.
  //////////////////////////////////////////////////////////////////////
  void switch_to(location_md& l, boost::optional<context>& pl)
  { m_ctx_stack.emplace_back(pl, l); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Pops the placeholder from the stack.
  //////////////////////////////////////////////////////////////////////
  void pop_placeholder(void)
  {
    STAPL_RUNTIME_ASSERT(!m_ctx_stack.empty());
    m_ctx_stack.pop_back();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Pushes @p ctx on the top of the stack.
  //////////////////////////////////////////////////////////////////////
  void push(context& ctx)
  {
    STAPL_RUNTIME_ASSERT(!ctx.is_base());
    m_ctx_stack.emplace_back(ctx);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Pops the context from the top of the stack.
  //////////////////////////////////////////////////////////////////////
  void pop(void)
  {
    STAPL_RUNTIME_ASSERT(!m_ctx_stack.empty() &&
                         m_ctx_stack.back().has_context());
    m_ctx_stack.pop_back();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns a pointer to the location metadata of the gang @p gid if
  ///        it is in the stack, otherwise @c nullptr.
  ///
  /// @warning This function has a \f$O(n)\f$ time complexity, where \f$n\f$ is
  ///          the number of parallel sections for this thread.
  //////////////////////////////////////////////////////////////////////
  location_md* try_get_location_md(const gang_id gid) noexcept
  {
    for (auto it = m_runqueues.begin(); it != m_runqueues.end(); ++it) {
      location_md& l = it->get_location_md();
      if (l.get_gang_md().get_id()==gid)
        return &l;
    }
    return nullptr;
  }

  runqueue::yield_status yield(const bool light_yield = false)
  {
    auto status = runqueue::IDLE_CAN_BLOCK;

    // execute requests of the active runqueue
    auto* const ctx = peek();
    if (ctx) {
      auto& active_rq = ctx->get_location_md().get_runqueue().get_impl();
      status          = active_rq(*this, light_yield);
      if (status==runqueue::YIELDED)
        return runqueue::YIELDED;
    }

    // execute requests of the rest of the runqueues
    // the iterator has to be advanced prior to destroying the intrusive_ptr
    // otherwise the iterator may be invalidated
    for (auto it = m_runqueues.begin(); it != m_runqueues.end();) {
      auto& rq = *it;
      if (rq.empty()) {
        ++it;
        continue;
      }

      boost::intrusive_ptr<location_md> l;
      if (!rq.is_active()) {
        // A non-active runqueue may be deleted if an event decreases the
        // location metadata reference count and makes it 0 (e.g. p_object
        // unregistration). This protects against this.
        l = &(rq.get_location_md());
      }
      switch (rq(*this, light_yield)) {
        case runqueue::YIELDED:
          status = runqueue::YIELDED;
          break;
        case runqueue::IDLE_CAN_BLOCK:
          break;
        case runqueue::IDLE_CANNOT_BLOCK:
          if (status==runqueue::IDLE_CAN_BLOCK)
            status = runqueue::IDLE_CANNOT_BLOCK;
          break;
        default:
          STAPL_RUNTIME_ERROR("Incorrect runqueue status.");
          break;
      }

      ++it;
    }

    return status;
  }
};


// --------------------------------------------------------------------
// this_context
// --------------------------------------------------------------------

/// Thread local stack.
static STAPL_RUNTIME_THREAD_LOCAL(stack, thread_stack)

/// Arbitrates the restore() primitive. @todo It is a contention point.
static gang_md::id  owner_gid   = invalid_gang_id;
static unsigned int owner_nlocs = 0;
static std::mutex   owner_mtx;


//////////////////////////////////////////////////////////////////////
/// @brief Returns the thread-local stack.
///
/// @ingroup runtimeMetadata
//////////////////////////////////////////////////////////////////////
stack& get_thread_stack(void)
{
  return thread_stack.get();
}


//////////////////////////////////////////////////////////////////////
/// @brief Returns the executor associated with the location of gang with id
///        @p gid from the stack.
///
/// @warning If the location is not found, this function aborts execution. Right
///          now we only support nested parallel sections that have a parent on
///          the stack.
//////////////////////////////////////////////////////////////////////
static executor_base& get_executor(const gang_md::id gid)
{
  location_md* const l = get_thread_stack().try_get_location_md(gid);
  if (!l) {
    // for now, this should always succeed - how to treat it if not?
    STAPL_RUNTIME_ERROR("Location metadata not found.");
  }
  return l->get_executor();
}


namespace this_context {

// Pushes a context on the stack
void push_base(context& ctx)
{
  auto& st = get_thread_stack();
  STAPL_RUNTIME_ASSERT(st.peek()!=&ctx);

  location_md& l = ctx.get_location_md();
  l.add_ref();
  st.push_base(ctx);
}


// Pops the context from the stack
void pop_base(context& ctx)
{
  auto& st = get_thread_stack();
  STAPL_RUNTIME_ASSERT(st.peek()==&ctx);

  location_md& l = ctx.get_location_md();
  st.pop_base();
  if (!l.release()) {
    // location still has things to do (p_objects, executors etc)
    auto& ex = l.get_executor();
    if (!ex.is_bound()) {
      // location's executor is not bound, so bind it to its parent
      const gang_md::id parent_gid = l.get_gang_md().get_parent_id();
      STAPL_RUNTIME_ASSERT(parent_gid!=invalid_gang_id);
      if (ex.empty()) {
        ex.set_deferred_notifier([parent_gid, &ex]
                                 {
                                   auto& parent_ex = get_executor(parent_gid);
                                   parent_ex.add_executor(&ex, true);
                                 });
      }
      else {
        auto& parent_ex = get_executor(parent_gid);
        parent_ex.add_executor(&ex, true);
      }
    }
  }
}


// Pushes a placeholder a new context on the stack
void push_placeholder(boost::optional<context>& placeholder)
{
  get_thread_stack().push_placeholder(placeholder);
}


// Pops the placeholder from the stack
void pop_placeholder(void)
{
  get_thread_stack().pop_placeholder();
}


// Switches to the base context of l. The placeholder will be used if there is
// an existing base context.
void switch_to(location_md& l, boost::optional<context>& placeholder)
{
  get_thread_stack().switch_to(l, placeholder);
}


// Pops a switched base context from the stack
void unswitch(void)
{
  auto& s = get_thread_stack();

  auto* const ctx = s.peek();
  if (ctx) {
    STAPL_RUNTIME_ASSERT(
      ctx==ctx->get_location_md().get_runqueue().get_impl().get_base_context());
    // inherited base context, flush requests
    ctx->flush_requests();
    s.pop();
  }
  else {
    s.pop_placeholder();
  }
}


// Returns the current context
context& get(void)
{
  return get_thread_stack().top();
}


// Tries to return the current context
context* try_get(void) noexcept
{
  return get_thread_stack().try_top();
}


// Returns the current context's id
context_id const& get_id(void)
{
  return get().get_id();
}


// Returns the location metadata if it is in the current stack
location_md* try_get_location_md(const gang_id gid) noexcept
{
  return get_thread_stack().try_get_location_md(gid);
}


// Tries to acquire the runqueue for the given location's execution
bool can_restore(location_md& l)
{
  // if not an orphan or too deeply nested, do not lock
  auto& rq = l.get_runqueue().get_impl();
  if (rq.is_active())
    return false;

  // attempt to acquire execution thread
  gang_md const& g    = l.get_gang_md();
  const auto nmanaged = g.local_size();
  if (nmanaged==1) {
    return true;
  }
  std::lock_guard<std::mutex> lock{owner_mtx};
  if (owner_gid==g.get_id()) {
    if (--owner_nlocs==0)
      owner_gid = invalid_gang_id;
    return true;
  }
  if (owner_gid==invalid_gang_id) {
    owner_gid   = g.get_id();
    owner_nlocs = (nmanaged - 1);
    return true;
  }
  return false;
}

} // namespace this_context

} // namespace runtime

} // namespace stapl
