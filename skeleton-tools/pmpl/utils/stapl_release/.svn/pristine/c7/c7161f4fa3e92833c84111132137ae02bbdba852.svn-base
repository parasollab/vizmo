/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_OPERATIONS_SEQUENCE_HPP
#define STAPL_VIEWS_OPERATIONS_SEQUENCE_HPP

#include <stapl/views/operations/iterator_selector.hpp>
#include <stapl/utility/use_default.hpp>

namespace stapl {

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Helper functor to construct an iterator base on the @c View
///        type and the @c Iterator type expected.
//////////////////////////////////////////////////////////////////////
template <typename View, typename Iterator, typename Enable = void>
struct make_iterator
{
  typedef typename View::value_type         value_t;
  typedef Iterator                          iterator;

  static iterator
  apply(View const& view, typename View::index_type const& index)
  {
    return view.container().make_iterator(view.mapfunc()(index));
  }
};


template <typename View, typename Category>
struct make_iterator<View, index_iterator<View,Category> >
{
  typedef index_iterator<View,Category>       iterator;

  static iterator
  apply(View const& view, typename View::index_type const& index)
  {
    return iterator(view,index);
  }
};


template <typename View>
struct make_iterator<View, detail::view_iterator<View> >
{
  typedef detail::view_iterator<View>       iterator;

  static iterator
  apply(View const& view, typename View::index_type const& index)
  {
    return iterator(view,index);
  }
};


template <template<typename, typename...> class V,
          typename T, typename Itr, typename ...OptionalParams>
struct make_iterator<V<std::vector<T>, OptionalParams...>, Itr>
{
  typedef V<std::vector<T>, OptionalParams...>     view_t;
  typedef typename std::vector<T>::iterator        iterator;

  static iterator
  apply(view_t const& view, typename view_t::index_type const& index)
  {
    return view.container().begin() + index;
  }
};


template <template<typename, typename...> class V,
         typename T, typename Itr, typename ...OptionalParams>
struct make_iterator<V<std::list<T>, OptionalParams...>, Itr>
{
  typedef V<std::list<T>, OptionalParams...> view_t;
  typedef typename std::list<T>::iterator    iterator;

  static iterator
  apply(view_t const& view, typename view_t::index_type const& index)
  {
    return index;
  }
};

} //detail namespace


namespace view_operations {

//////////////////////////////////////////////////////////////////////
/// @brief Defines the operation found in sequence containers
///        (e.g. list) used to traverse the elements.
//////////////////////////////////////////////////////////////////////
template<typename Derived, typename Iterator = use_default>
class sequence
{
private:
  typedef typename view_traits<Derived>::index_type         index_t;
  typedef typename view_traits<Derived>::value_type         value_t;
  typedef typename view_traits<Derived>::container          container_type;

public:
  typedef typename select_parameter<
    Iterator,
    typename detail::iterator_selector<
      Derived, value_t
    >::type
  >::type                                                   iterator;

  typedef typename select_parameter<
    Iterator,
    typename detail::const_iterator_selector<
      Derived, value_t
    >::type
  >::type                                                   const_iterator;

  typedef detail::make_iterator<Derived, iterator>          make_iterator_t;

private:
  Derived const& derived(void) const
  {
    return static_cast<const Derived&>(*this);
  }

public:

  /// @name Sequence Iterator
  /// @warning Methods in the Sequence Iterator group should only be used
  /// inside a work function which is processing a segmented view.
  /// @{

  iterator begin(void)
  {
    return make_iterator_t::apply(derived(), derived().domain().first());
  }

  iterator end(void)
  {
    return make_iterator_t::apply(derived(), derived().domain().open_last());
  }

  iterator make_iterator(index_t i)
  {
    return make_iterator_t::apply(derived(), i);
  }

  const_iterator begin(void) const
  {
    return make_iterator_t::apply(derived(), derived().domain().first());
  }

  const_iterator end(void) const
  {
    return make_iterator_t::apply(derived(), derived().domain().open_last());
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Computes the next index based on the given @c index.
  ///
  /// Overwriting this method allows produce a different way to
  /// traverse the elements referenced by the view.
  //////////////////////////////////////////////////////////////////////
  index_t next(index_t const& index) const
  {
    return derived().domain().advance(index,1);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Computes the previous index based on the given @c index.
  ///
  /// Overwriting this method allows produce a different way to
  /// traverse the elements referenced by the view.
  //////////////////////////////////////////////////////////////////////
  index_t prev(index_t const& index) const
  {
    return derived().domain().advance(index,-1);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Computes the new index after advance @c n positions from
  ///        the given @c index.
  ///
  /// Overwriting this method allows produce a different way to
  /// traverse the elements referenced by the view.
  //////////////////////////////////////////////////////////////////////
  template<typename Distance>
  index_t advance(index_t const& index, Distance n) const
  {
    return derived().domain().advance(index,n);
  }

  long distance(index_t const& index1, index_t const& index2) const
  {
    return derived().domain().distance(index1,index2);
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  //////////////////////////////////////////////////////////////////////

  bool less_than(index_t const& index1, index_t const& index2) const
  {
    return derived().domain().less_than(index1,index2);
  }

  /// @}

}; // class sequence

} // view_operations namespace

} // namespace stapl

#endif // STAPL_VIEWS_OPERATIONS_SEQUENCE_HPP
