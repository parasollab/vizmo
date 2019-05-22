/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_PRODUCER_INFO_HPP
#define STAPL_SKELETONS_PRODUCER_INFO_HPP

#include <stapl/views/view_traits.hpp>
#include <stapl/skeletons/utility/utility.hpp>

namespace stapl {
namespace skeletons {
namespace flows {

//////////////////////////////////////////////////////////////////////
/// @brief A @c constant_producer_info is used as a wrapper for a
/// constant value which is going to be passed to a task in a
/// dependence graph. This class stores a copy of the value.
///
/// @see taskgraph_env.hpp
/// @see graphviz_env.hpp
/// @see local_env.hpp
///
/// @ingroup skeletonsFlowsProducers
//////////////////////////////////////////////////////////////////////
template <typename T>
class constant_producer_info
{
  T m_element;
public:
  using index_type = std::size_t;
  using value_type = T;

  constant_producer_info(T const& element)
    : m_element(element)
  { }

  T get_element() const
  {
    return m_element;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief A @c indexed_producer_info is used as a wrapper for
/// indexable producers in a dependence graph. This class offers the
/// possibility of a filtered request from a producer.
///
/// Producer information is then used in various environments in order
/// to create tasks, create visualization of dependence graphs, etc.
///
/// @tparam ValueType the type of the producer edge
/// @tparam IndexType the type of producer's index. For a 1D view this
///                   would as simple as @c size_t
/// @tparam F         the filter that can be applied on the request on
///                   the producer side
///
/// @see taskgraph_env.hpp
/// @see graphviz_env.hpp
/// @see local_env.hpp
///
/// @ingroup skeletonsFlowsProducers
//////////////////////////////////////////////////////////////////////
template <typename ValueType, typename IndexType,
          typename F = skeletons::no_filter>
class indexed_producer_info
{
  IndexType m_index;  // index of the producer
  F         m_filter; // which filtering function to apply on the producer edge
public:
  using index_type  = IndexType;
  using value_type  = ValueType;
  using filter_type = F;

  indexed_producer_info(IndexType const& index, F const& filter_func)
    : m_index(index),
      m_filter(filter_func)
  { }

  indexed_producer_info(IndexType const& index)
    : m_index(index),
      m_filter(F())
  { }

  IndexType get_index() const
  {
    return m_index;
  }

  F get_filter() const
  {
    return m_filter;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief A @c indexed_producers_info is used as a wrapper for
/// indexable producers in a dependence graph. This class offers the
/// possibility of a filtered request from a producer.
///
/// The difference between this producer class and the non-plural one
/// @c indexed_producer_info is in the fact that multiple dependencies
/// in this case are grouped to form one dependency. In dataflow
/// terminology this construct is called a joiner.
///
/// Producer information is then used in various environments in order
/// to create tasks, create visualization of dependence graphs, etc.
///
/// @tparam ValueType   the type of the producer edge
/// @tparam IndicesType the type of producer's indices. For a 1D view this
///                     would as simple as @c size_t
/// @tparam F           the filter that can be applied on the request on
///                     the producer side
///
/// @see taskgraph_env.hpp
/// @see graphviz_env.hpp
/// @see local_env.hpp
///
/// @ingroup skeletonsFlowsProducers
//////////////////////////////////////////////////////////////////////
template <typename ValueType, typename IndicesType,
          typename F>
class indexed_producers_info
{
  IndicesType m_indices; // indices of the producer
  F           m_filter;  // filtering function to apply on the producer edge
public:
  using indices_type = IndicesType;
  using value_type   = ValueType;
  using filter_type  = F;

  indexed_producers_info(IndicesType const& indices, F const& filter_func)
    : m_indices(indices),
      m_filter(filter_func)
  { }

  indexed_producers_info(IndicesType const& indices)
    : m_indices(indices),
      m_filter(F())
  { }

  IndicesType get_indices() const
  {
    return m_indices;
  }

  F get_filter() const
  {
    return m_filter;
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief A @c reflexive_producer_info is used as a wrapper for
/// using references as the producer to a task in a dependence graph.
/// This producer is only used directly in @c do_while skeleton, and
/// is not intended to be used often. This producer requires the task
/// to be executed on the same memory space as the producer.
///
/// For view accesses you should use @c view_producer_info.
///
/// Producer information is used in various environments in order
/// to create tasks, create visualization of dependence graphs, etc.
///
/// @tparam Element the type of the producer edge
///
/// @see taskgraph_env.hpp
/// @see graphviz_env.hpp
/// @see local_env.hpp
///
/// @ingroup skeletonsFlowsProducers
//////////////////////////////////////////////////////////////////////
template <typename Element, bool isRef = true>
class reflexive_producer_info
{
  Element& m_element;
public:
  using index_type = std::size_t;
  using value_type = Element;

  reflexive_producer_info(Element& element)
    : m_element(element)
  { }

  Element& get_element() const
  {
    return m_element;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief @copybrief reflexive_producer_info
///
/// This specialization is used for non-ref copies of a producers
///
/// @ingroup skeletonsFlowsProducers
//////////////////////////////////////////////////////////////////////
template <typename Element>
class reflexive_producer_info<Element, false>
{
  Element m_element;
public:
  using index_type = std::size_t;
  using value_type = Element;

  reflexive_producer_info(Element element)
    : m_element(element)
  { }

  Element get_element() const
  {
    return m_element;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief A @c view_producer_info is used as a wrapper for view
/// accesses by a task in a dependence graph. Therefore, it inherits
/// from both @c indexed_producer_info and @c reflexive_producer_info.
/// This producer is only used in @c do_while skeleton, and is not
/// intended to be used often. This producer requires the task to be
/// executed on the same memory space as the producer.
/// Producer information is then used in various environments in order
/// to create tasks, create visualization of dependence graphs, etc.
///
/// @tparam View the type of the view that is going to be accessed
///
/// @see taskgraph_env.hpp
/// @see graphviz_env.hpp
/// @see local_env.hpp
/// @see indexed_producer_info
/// @see reflexive_producer_info
///
/// @ingroup skeletonsFlowsProducers
//////////////////////////////////////////////////////////////////////
template <typename View>
class view_producer_info
  : public indexed_producer_info<typename View::value_type,
                                 typename View::index_type>,
    public reflexive_producer_info<View>
{
public:
  using index_type = typename view_traits<View>::index_type;
  /// What we mean in here as value type is the element referred to by the
  /// element in @c reflexive_producer_info. In other words accessing the
  /// indexed element is not the responsibility of this class
  using value_type = View;

  view_producer_info(View& view, index_type const& index)
    :  indexed_producer_info<typename View::value_type, index_type>(index),
       reflexive_producer_info<View>(view)
  { }
};


//////////////////////////////////////////////////////////////////////
/// @brief @copybrief view_producer_info
///
/// This specialization is used when the given view is a
/// @c proxy over an element. The index type for this class is
/// determined differently than the base case.
///
/// @tparam T the type of the element that the proxy is defined over
/// @tparam A the accessor to the underlying element
///
/// @see taskgraph_env.hpp
/// @see graphviz_env.hpp
/// @see local_env.hpp
/// @see indexed_producer_info
/// @see reflexive_producer_info
///
/// @ingroup skeletonsFlowsProducers
/////////////////////////////////////////////////////////////////////
template <typename T, typename A>
class view_producer_info<proxy<T, A> >
  : public indexed_producer_info<T, typename T::size_type>,
    public reflexive_producer_info<proxy<T, A> >
{
  using view_t = proxy<T, A>;
public:
  using index_type = typename T::size_type;
  /// What we mean in here as value type is the element referred to by the
  /// element in @c reflexive_producer_info. In other words accessing the
  /// indexed element is not the responsibility of this class
  using value_type = T;

  view_producer_info(view_t const& view, index_type const& index)
    : indexed_producer_info<T, typename T::size_type>(index),
      reflexive_producer_info<view_t>(const_cast<view_t&>(view))
  { }
};

//////////////////////////////////////////////////////////////////////
/// @brief creates a @c reflexive_producer_info over the given
/// @c element. This method should be used with care, since it is
/// intended for very rare cases such as @c do_while_pd.
///
/// @see view_producer_info
/// @see reflexive_producer_info
///
/// @ingroup skeletonsFlowsProducers
//////////////////////////////////////////////////////////////////////
template <typename Element>
reflexive_producer_info<Element>
make_reflexive_producer_info(Element& element)
{
  return reflexive_producer_info<Element>(element);
}

//////////////////////////////////////////////////////////////////////
/// @brief @copybrief make_reflexive_producer_info.
///
/// This specialization is used when a non-ref copy of a reflexive
/// producer needs to be stored.
///
/// @see reflexive_producer_info.
///
/// @ingroup skeletonsFlowsProducers
//////////////////////////////////////////////////////////////////////
template <bool isRef, typename Element>
reflexive_producer_info<Element>
make_reflexive_producer_info(Element& element)
{
  return reflexive_producer_info<Element, isRef>(element);
}

//////////////////////////////////////////////////////////////////////
/// @brief creates a @c constant_producer_info over the given value.
/// This value gets stored and can be accessed later on by each
/// environment that a task is spawned in.
///
/// @ingroup skeletonsFlowsProducers
//////////////////////////////////////////////////////////////////////
template <typename T>
constant_producer_info<T>
make_constant_producer_info(T const& value)
{
  return constant_producer_info<T>(value);
}

} // namespace flows
} // namespace skeletons
} // namespace stapl
#endif // STAPL_SKELETONS_PRODUCER_INFO_HPP
