/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_ALGORITHMS_ALGORITHM_HPP
#define STAPL_ALGORITHMS_ALGORITHM_HPP

#include <numeric>
#include <stapl/paragraph/paragraph.hpp>
#include "generator.hpp"
#include "functional.hpp"
#include "numeric.hpp"
#include <boost/random/uniform_int_distribution.hpp>
#include <stapl/utility/hash.hpp>
#include <stapl/utility/random.hpp>
#include <stapl/views/proxy/proxy.hpp>
#include <stapl/views/overlap_view.hpp>
#include <stapl/views/balance_view.hpp>
#include <stapl/views/native_view.hpp>
#include <stapl/views/repeated_view.hpp>
#include <stapl/views/counting_view.hpp>
#include <stapl/views/reverse_view.hpp>
#include <stapl/views/list_view.hpp>
#include <stapl/views/transform_view.hpp>
#include <stapl/containers/partitions/splitter.hpp>
#include <stapl/views/functor_view.hpp>
#include <stapl/containers/generators/functor.hpp>
#include <stapl/containers/array/static_array.hpp>
#include <stapl/containers/list/list.hpp>
#include "algorithm_fwd.hpp"

namespace stapl {

namespace algo_details {

//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref find(), which returns elements for which the
///   predicate is true, and a null reference otherwise.
/// @tparam Predicate Unary functor which is called on the argument.
//////////////////////////////////////////////////////////////////////
template<typename Predicate>
class find_map
{
private:
  Predicate m_pred;

public:
  find_map(Predicate const& pred)
    : m_pred(pred)
  { }

  void define_type(typer& t)
  {
    t.member(m_pred);
  }

  template<typename Reference>
  Reference operator()(Reference elem) const
  {
    if (m_pred(elem))
      return elem;
    else
      return Reference(null_reference());
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function which returns its first argument if
///   non-null, and the second otherwise.
//////////////////////////////////////////////////////////////////////
struct find_reduce
{
  template<typename Reference1, typename Reference2>
  Reference1 operator()(Reference1 lhs, Reference2 rhs) const
  {
    if (!is_null_reference(lhs))
      return lhs;
    else
      return rhs;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref mismatch(), which returns null when the
///   predicate is true, and the first element otherwise.
/// @tparam Predicate Binary predicate invoked on the arguments.
//////////////////////////////////////////////////////////////////////
template<typename Predicate>
class mismatch_map
{
private:
  Predicate m_pred;

public:
  mismatch_map(Predicate const& pred)
    : m_pred(pred)
  {}

  void define_type(typer& t)
  {
    t.member(m_pred);
  }

  template<typename Reference1, typename Reference2>
  Reference1 operator()(Reference1 const& x, Reference2 const& y)
  {
    if (m_pred(x, y))
      return Reference1(null_reference());

    return x;
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref mismatch(), which given two arguments, returns
///   the first if it is non-NULL, and the second otherwise.
//////////////////////////////////////////////////////////////////////
struct mismatch_reduce
{
  template<typename Reference1, typename Reference2>
  Reference1 operator()(Reference1 lhs, Reference2 rhs)
  {
    if (!is_null_reference(lhs))
      return lhs;
    else
      return rhs;
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref replace_if(), which assigns the stored value
///   to the argument if the predicate returns true.
/// @tparam Predicate Unary functor which is called on the argument.
//////////////////////////////////////////////////////////////////////
template<typename Predicate, typename T>
struct assign_if
{
  Predicate m_pred;
  T m_new_value;

  typedef void result_type;

  assign_if(Predicate const& pred,  T const& new_value)
    : m_pred(pred), m_new_value(new_value)
  { }

  void define_type(typer& t)
  {
    t.member(m_pred);
    t.member(m_new_value);
  }

  template<typename Reference1>
  void operator()(Reference1 x)
  {
    if (m_pred(x))
      x = m_new_value;
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function used to place a given element into one of two output
///   views, depending on whether the given functor returns true or false.
/// @tparam Pred Unary functor which is used to place the element.
/// @tparam ValueType Type of the values which are being partitioned.
//////////////////////////////////////////////////////////////////////
template <typename Pred, typename ValueType>
struct partition_apply_pred
{
private:
  typedef list<ValueType> p_list_type;

  Pred          m_pred;
  p_list_type*  m_pl_satisfy_pred;
  p_list_type*  m_pl_not_satisfy_pred;

public:
  typedef void result_type;

  partition_apply_pred(Pred pred,
                       p_list_type* pl_satisfy_pred,
                       p_list_type* pl_not_satisfy_pred)
    : m_pred(pred),
      m_pl_satisfy_pred(pl_satisfy_pred),
      m_pl_not_satisfy_pred(pl_not_satisfy_pred)
  { }

  template <typename View>
  void operator()(View elem_view)
  {
    if (m_pred(elem_view))
      m_pl_satisfy_pred->add(elem_view);
    else
      m_pl_not_satisfy_pred->add(elem_view);
  }

  void define_type(typer& t)
  {
    t.member(m_pred);
    t.member(m_pl_satisfy_pred);
    t.member(m_pl_not_satisfy_pred);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Used in @ref unique_copy() to compute the offsets of the elements to
///   be copied into the new array (keeping track of duplicates).
//////////////////////////////////////////////////////////////////////
struct partition_pred_counter
{
  unsigned int m_is_match;
  unsigned int m_no_match;

  partition_pred_counter()
    : m_is_match(0), m_no_match(0)
  {}

  void set(bool matches)
  {
    if (matches)
      m_is_match = 1;
    else
      m_no_match = 1;
  }

  unsigned int value(bool matches, unsigned int offset)
  {
    if (matches)
      return m_is_match - 1;
    else
      return offset + m_no_match - 1;
  }

  void define_type(typer& t)
  {
    t.member(m_is_match);
    t.member(m_no_match);
  }

  partition_pred_counter& operator=(partition_pred_counter const& other)
  {
    if (this != &other)
    {
      m_is_match = other.m_is_match;
      m_no_match = other.m_no_match;
    }
    return *this;
  }

  const partition_pred_counter
  operator+(partition_pred_counter const& other) const
  {
    partition_pred_counter result(*this);
    result.m_is_match += other.m_is_match;
    result.m_no_match += other.m_no_match;
    return result;
  }
};

} // namespace algo_details


STAPL_DEFINE_IDENTITY_VALUE(
  plus<algo_details::partition_pred_counter>,
  algo_details::partition_pred_counter,
  algo_details::partition_pred_counter()
)


//////////////////////////////////////////////////////////////////////
/// @brief Specialization of @ref stapl::proxy when the type is
///   @ref algo_details::partition_pred_counter.
/// @tparam Accessor Accessor for the given proxy.
//////////////////////////////////////////////////////////////////////
template <typename Accessor>
class proxy<algo_details::partition_pred_counter, Accessor>
  : public Accessor
{
private:
  friend  class proxy_core_access;

  typedef algo_details::partition_pred_counter target_t;

public:
  explicit proxy(Accessor const& acc)
    : Accessor(acc)
  {}

  operator target_t() const
  {
    return Accessor::read();
  }

  proxy const& operator=(proxy const& rhs)
  {
    Accessor::write(rhs);
    return *this;
  }

  proxy const& operator=(target_t const& rhs)
  {
    Accessor::write(rhs);
    return *this;
  }

  void set(bool matches)
  {
    Accessor::invoke(&target_t::set, matches);
  }

  unsigned int value(bool matches, unsigned int offset)
  {
    return Accessor::invoke(&target_t::value, matches, offset);
  }
};


namespace algo_details {

//////////////////////////////////////////////////////////////////////
/// @brief Work function which takes an input map and sets the given key equal
///   to the given value.
/// @tparam Value Type of the value to set.
/// @see merge
//////////////////////////////////////////////////////////////////////
template<typename Value>
struct merge_send_leader
{
  size_t m_sender;
  Value  m_val;

  merge_send_leader() : m_sender(0), m_val() {}

  merge_send_leader(size_t sender, Value val)
    : m_sender(sender)
    , m_val(val) {}

  template<typename Leaders>
  void operator()(Leaders& leaders) const {
    leaders[m_sender] = m_val;
  }

  void define_type(typer& t)
  {
    t.member(m_sender);
    t.member(m_val);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function which merges two local splitters into the global
///   set of splitters.
/// @see merge
//////////////////////////////////////////////////////////////////////
class merge_splitters
{
private:
  location_type m_me;

public:
  typedef void result_type;

  merge_splitters(location_type me)
    : m_me(me)
  { }

  template<typename LocalView, typename LocalSplitsA, typename LocalSplitsB,
           typename GlobalIndices>
  void operator()(LocalView A, LocalSplitsA locA, LocalSplitsB locB,
                  GlobalIndices globAix)
  {
    //note that locA/locB are assumed to be std::vectors
    // and globAix is assumed to be a static_array
    typedef typename LocalSplitsA::iterator     locAIter;
    typedef typename LocalSplitsB::iterator     locBIter;

    //find the corresponding bounds in the splitters of B
    locAIter mystart = locA.begin() + m_me;
    locBIter c = std::lower_bound(locB.begin(), locB.end(), *(mystart));
    locBIter d = (mystart+1 != locA.end())
                ? std::lower_bound(locB.begin(), locB.end(), *(mystart+1))
                : locB.end();

    //there are me splitters before this (from other partitions)
    // there are dist(c) splitters before this (from locB)
    size_t ix = std::distance(locB.begin(), c) + m_me;
    globAix[ix] = A.domain().first();

    for (++ix; c != d; ++c, ++ix) {
      globAix[ix] =
        A.domain().advance(A.domain().first(),
                           std::distance(A.begin(),
                                         std::upper_bound(A.begin(), A.end(),
                                                          *c)));
    }
  }

  void define_type(typer& t)
  {
    t.member(m_me);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function which sets the third argument equal to the sums of the
///   sizes of the two input views.
/// @see merge
//////////////////////////////////////////////////////////////////////
struct merge_output_sizes
{
  typedef void result_type;

  template<typename View1, typename View2, typename SizeType>
  void operator()(View1 const& view1, View2 const& view2, SizeType size)
  {
    size = view1.size() + view2.size();
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Compute the index of the nth element of the provided view.
/// @see merge
//////////////////////////////////////////////////////////////////////
struct merge_output_indices
{
  typedef void result_type;

  template<typename Size, typename Index, typename View>
  void operator()(Size const& size, Index ix, View const& view)
  {
    ix = view.domain().advance(view.domain().first(), size);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function which merges the two input views into the output view.
/// @see merge
//////////////////////////////////////////////////////////////////////
struct merge_map
{
  typedef void result_type;

  template<typename View1, typename View2, typename MergeView>
  void operator()(View1 const& view1, View2 const& view2, MergeView merged)
  {
    std::merge(view1.begin(), view1.end(), view2.begin(), view2.end(),
               merged.begin());
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Random number generator that returns a random number between 0 and
///   the input size using the given generator.
/// @tparam UniformRandomNumberGenerator Generator to use for number generation.
//////////////////////////////////////////////////////////////////////
template<class UniformRandomNumberGenerator>
class shuffle_random_number_generator
{
private:
  UniformRandomNumberGenerator m_rng;

public:
  shuffle_random_number_generator(UniformRandomNumberGenerator const& rng)
    : m_rng(rng) {}
  int operator()(int n)
  {
    typedef boost::random::uniform_int_distribution<size_t> distr_type;
    typedef typename distr_type::param_type                 p_type;

    distr_type d;

    return d(m_rng, p_type(0, n-1));
  }


  void define_type(typer& t)
  {
    t.member(m_rng);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function which increments the value in the input map for the
///   given key.
/// @tparam T Type of the key.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct map_inc
{
  typedef T      first_argument_type;
  typedef void   result_type;

private:
  first_argument_type m_key;

public:
  map_inc(const T& key) : m_key(key) { }

  template<typename Reference>
  void operator()(Reference& y) const
  {
    y[m_key]++;
  }

  void define_type(typer& t)
  {
    t.member(m_key);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function which creates a histogram from
///   from the given input.
/// @see next_permutation
/// @see prev_permutation
/// @see is_permutation
//////////////////////////////////////////////////////////////////////
template<class T>
class histogram
{
public:
  typedef void result_type;

  histogram(static_array<std::map<T,int> >* samples, int min, int step)
    : m_samples(samples), m_min(min), m_step(step) {}

  template<class Elem>
  void operator()(Elem elem)
  {
    m_samples->apply_set((stapl::hash<T>()(elem)-m_min)/m_step,
                         map_inc<int>(elem));
  }

  void define_type(typer& t)
  {
    t.member(m_samples);
    t.member(m_min);
    t.member(m_step);
  }
private:
  static_array<std::map<T,int> >* m_samples;
  int m_min;
  int m_step;
};


//////////////////////////////////////////////////////////////////////
/// @brief Work function which sets the second argument if the given predicate
///   returns true for the given value.
/// @tparam UnaryPredicate Unary functor which is called on the input element.
//////////////////////////////////////////////////////////////////////
template <typename UnaryPredicate>
struct partition_count_matches
{
  UnaryPredicate m_predicate;

  typedef void result_type;

  partition_count_matches(UnaryPredicate const& predicate)
    : m_predicate(predicate)
  {}

  template <typename ValueRef, typename CountRef>
  void operator()(ValueRef value, CountRef count)
  {
    count.set(m_predicate(value));
  }

  void define_type(typer& t)
  {
    t.member(m_predicate);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function which counts the number of duplicated elements for
///   @ref unique_copy().
/// @tparam BinaryPredicate Binary functor which implements the equal operation.
//////////////////////////////////////////////////////////////////////
template <typename BinaryPredicate>
struct unique_count_matches
{
  BinaryPredicate m_predicate;

  typedef void result_type;

  unique_count_matches(BinaryPredicate const& predicate)
    : m_predicate(predicate)
  {}

  template <typename ValueRef, typename CountRef>
  void operator()(ValueRef value, CountRef count)
  {
    count.set(!m_predicate(*(value.begin()),*(++value.begin())));
  }

  void define_type(typer& t)
  {
    t.member(m_predicate);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref stable_partition() which copies the input
///   element to the appropriate position in the output view.
/// @tparam UnaryPredicate Unary functor which is used to partition the input.
//////////////////////////////////////////////////////////////////////
template <typename UnaryPredicate>
struct partition_fill_temp
{
  UnaryPredicate m_predicate;
  unsigned int   m_offset;

  typedef void result_type;

  partition_fill_temp(UnaryPredicate const& predicate, unsigned int offset)
    : m_predicate(predicate), m_offset(offset)
  {}

  template <typename ValueRef, typename CountRef, typename TempView>
  void operator()(ValueRef value, CountRef count, TempView temp_view)
  {
    temp_view.set_element(count.value(m_predicate(value), m_offset), value);
  }

  void define_type(typer& t)
  {
    t.member(m_predicate);
    t.member(m_offset);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref unique_copy() which places the input
///   values into the proper places inside the temporary view.
/// @tparam BinaryPredicate Binary functor which implements the equal operation.
//////////////////////////////////////////////////////////////////////
template <typename BinaryPredicate>
struct unique_fill_temp
{
  BinaryPredicate m_predicate;
  unsigned int   m_offset;

  typedef void result_type;

  unique_fill_temp(BinaryPredicate const& predicate, unsigned int offset)
    : m_predicate(predicate), m_offset(offset)
  {}

  template <typename ValueRef, typename CountRef, typename TempView>
  void operator()(ValueRef value, CountRef count, TempView temp_view)
  {
    size_t pos =
      count.value(!m_predicate(*(value.begin()),*(++value.begin())),
                  m_offset)-1;
    typename TempView::index_type idx =
      temp_view.domain().advance(temp_view.domain().first(),pos);
    temp_view.set_element(idx, *(++value.begin()));
  }

  void define_type(typer& t)
  {
    t.member(m_predicate);
    t.member(m_offset);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function which implements a lexicographical ordering based on
///   the provided functor.
/// @tparam Pred Binary functor implementing the less operation.
//////////////////////////////////////////////////////////////////////
template<typename Pred>
class lexico_compare
{
private:
  Pred m_pred;

public:
  lexico_compare(Pred const& pred)
    : m_pred(pred) {}

  void define_type(typer& t)
  {
    t.member(m_pred);
  }

  typedef int result_type;

  template <typename Elm1, typename Elm2>
  int operator()(Elm1 element1, Elm2 element2) const
  {
    if (m_pred(element1,element2))
      return 1;
    else if (m_pred(element2,element1))
      return -1;

    return 0;
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function which returns the second argument if the first
///   compares equal to 0, and the first otherwise.
/// @tparam T The type of the arguments.
//////////////////////////////////////////////////////////////////////
template <typename T>
struct lexico_reduce
  : public ro_binary_function<T, T, T>
{
  template <typename Elm>
  T operator()(Elm elem1, Elm elem2){
    if (elem1 == 0){
      return (T)elem2;
    } else {
      return (T)elem1;
    }
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Copies the first argument to the
///   second unless the first meets the predicate provided, in which case
///   the given value is copied to the second argument.
/// @tparam Predicate Unary functor to test for the need for replacement.
/// @tparam T Type of the value to replace.
//////////////////////////////////////////////////////////////////////
template<typename Predicate, typename T>
struct copy_with_replacement
{
  Predicate m_pred;
  T m_new_value;

  typedef void result_type;

  copy_with_replacement(Predicate const& pred, T const& new_value)
    : m_pred(pred), m_new_value(new_value)
  { }

  template<typename Reference1, typename Reference2>
  void operator()(Reference1 x, Reference2 y)
  {
    if (m_pred(x))
      y = m_new_value;
    else
      y = x;
  }

  void define_type(typer& t)
  {
    t.member(m_pred);
    t.member(m_new_value);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref find_first_of() which returns the
///   first element which matches any element in the given pattern
///   according to the given functor.
/// @tparam Pred Binary functor which implements the equal operation.
//////////////////////////////////////////////////////////////////////
template<typename Pred>
class wf_compare_find_first_of {

  Pred m_pred;

public :

  wf_compare_find_first_of(Pred const& pred)
    : m_pred(pred) {}

  void define_type(typer& t)
  {
    t.member(m_pred);
  }

  template <typename Ref, typename View>
  Ref operator ()(Ref element_search, View view_search)
  {
    typename View::iterator it_result =
      std::find_if(view_search.begin(), view_search.end(),
                   stapl::bind2nd(m_pred,element_search));
    if ((it_result != view_search.end())){
      return element_search;
    } else {
      return Ref(null_reference());
    }
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function which assigns the new value to the input element if
///   the input element is equal to the given value.
/// @tparam T Types of the old and new values.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct assign_if_equal
{
  const T m_old_value;
  const T m_new_value;

  typedef void   result_type;

  assign_if_equal(T const& old_value, T const& new_value)
    : m_old_value(old_value), m_new_value(new_value)
  { }

  void define_type(typer& t)
  {
    t.member(m_old_value);
    t.member(m_new_value);
  }

  template<typename Reference1>
  void operator()(Reference1 x) const
  {
    if (x == m_old_value)
      x = m_new_value;
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref is_sorted() which takes a reference to
///   two elements, and returns true if the given functor returns false when
///   called with (second, first).
/// @tparam Functor Binary functor which is used to compare the elements.
//////////////////////////////////////////////////////////////////////
template<typename Functor>
class gen_comp_func
  : public ro_unary_function<typename Functor::first_argument_type,
                             typename Functor::result_type>
{
private:
  Functor m_f;

public:
  gen_comp_func(Functor const& f)
    : m_f(f)
  { }

  template<typename Reference>
  typename Functor::result_type
  operator()(Reference x) const
  {
    return (!m_f(*(x.begin()+1),*(x.begin())));
  }

  void define_type(typer &t)
  {
    t.member(m_f);
  }

};

//////////////////////////////////////////////////////////////////////
/// @brief Random number generator used by default in @ref random_shuffle
//////////////////////////////////////////////////////////////////////
template <typename Difference>
class default_random_number_generator
{
  typedef boost::random::uniform_int_distribution<Difference> distrib_type;
  boost::random::mt19937 m_gen;
public:
  default_random_number_generator(unsigned int seed)
    : m_gen(seed)
  { }

  Difference operator()(void)
  {
    return distrib_type()(m_gen);
  }

  Difference operator()(Difference n)
  {
    return distrib_type(0, n)(m_gen);
  }

  void define_type(typer& t)
  {
    t.member(m_gen);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref random_shuffle(), which computes the
///        random shuffle over the input, computes the output's index
///        targets.
///
/// The given set of elements are locally shuffled and the values are
/// scattered to specific well-defined positions on the output (@p v2)
/// avoiding write collisions among different locations.
///
/// @tparam RandomNumberGenerator The random number generator used to
///         shuffle.
//////////////////////////////////////////////////////////////////////
template<typename RandomNumberGenerator>
class shuffle_wf
{
  size_t m_num_partitions;
  RandomNumberGenerator m_rng;

public:
  typedef void result_type;

  //////////////////////////////////////////////////////////////////////
  /// @brief Work function constructor with a given number of
  ///        partitions (@p n) and the random number generator used to
  ///        compute the local shuffle.
  //////////////////////////////////////////////////////////////////////
  shuffle_wf(size_t n, RandomNumberGenerator rng)
    : m_num_partitions(n)
    , m_rng(rng)
  { }

  template<class View1, class View2>
  void operator()(View1 v1, View2 v2)
  {
    size_t target_size  = v2.size();

    std::random_shuffle(v1.begin(), v1.end(), m_rng);

    size_t blksz = round(target_size/m_num_partitions);

    // Generating the scatter target indices
    size_t j,pid,w,k=0;
    for (size_t i = v1.domain().first();i<=v1.domain().last();++i) {
      j = i % blksz;          // relative local index
      pid = i / blksz;        // partition id
      w = pid + (j * blksz);  // scatter target position
      k = (w>=target_size) ? i : w;
      v2[k] = v1[i];
    }
  }

  void define_type(typer &t)
  {
    t.member(m_num_partitions);
    t.member(m_rng);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Returns a random access view to the given view. If the
///   input view does not have a scalar domain (e.g., domain used for
///   stapl::list), the elements are copied into a new container.
/// @tparam View Type of the input view.
/// @tparam Is_Index_Scalar Type of the view domain.
//////////////////////////////////////////////////////////////////////
template <typename View,typename Is_Index_Scalar>
struct get_input_view
{
  typedef static_array<typename View::value_type>  tmp_container_t;
  typedef array_view<tmp_container_t>              view_type;
  view_type operator()(const View& vw)
  {
    view_type tmp_view(new tmp_container_t(vw.size()));
    copy(vw,tmp_view);
    return tmp_view;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization of @ref get_input_view, which just returns the input
///   view, because it already is random access.
/// @tparam View The type of the input view.
/// @todo Specialize more based on the domain (sparse?).
//////////////////////////////////////////////////////////////////////
template <typename View>
struct get_input_view<View,size_t>
{
  typedef View view_type;
  view_type operator()(const View& vw)
  {
    return vw;
  }
};


//Work function for adjacent_find
//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref adjacent_find(), which accepts a view over two
///   elements, and returns a reference to the first if the predicate returns
///   true, and NULL otherwise.
/// @tparam BinPredicate Predicate which is used to compare the elements for
///   equality.
//////////////////////////////////////////////////////////////////////
template< typename BinPredicate >
class adj_find_map
{
  BinPredicate mypred;
public:
  adj_find_map(BinPredicate pred)
    :mypred(pred)
  {}

  void define_type(typer & t)
  {
    t.member(mypred);
  }

  template<typename View>
  typename View::reference operator()(View elem)
  {
    if (mypred( *(elem.begin()), *(elem.begin()+1)))
      return *(elem.begin());
    else
      return typename View::reference(null_reference());
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref search(), which returns a reference to the
///   beginning of the first argument if it equals the second argument using
///   the provided predicate, or NULL otherwise.
/// @tparam BPredicate Functor which is used to compare the arguments for
///   equality.
//////////////////////////////////////////////////////////////////////
template<class BPredicate>
class search_map_wf
{
private:
  BPredicate pred;
public:
  search_map_wf(const BPredicate & bpred)
    : pred(bpred)
  { }

  void define_type(typer& t)
  {
    t.member(pred);
  }

  template<typename View1,typename View>
  typename View1::reference
  operator()(View1 v,View m_val) const
  {
    if (std::equal(m_val.begin(),m_val.end(),v.begin(),pred))
    {
      return *(v.begin());
    }
    else
      return typename View1::reference(null_reference());
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref search_n(), which returns a reference to the
///   first input element if the input consists solely of the given value, and
///   NULL otherwise.
/// @tparam T Type of the value which is being searched for.
/// @tparam BPredicate Functor which is used to compare element equality.
///
/// This work function is given an overlapped view of the size being searched
/// for, so it simply checks if all the elements in the input are equal to the
/// given value.
//////////////////////////////////////////////////////////////////////
template<typename T,class BPredicate>
class search_n_map_wf
{
private:
  T m_val;
  BPredicate pred;
public:
  search_n_map_wf(T const& val,const BPredicate &predicate)
    : m_val(val), pred(predicate)
  {}

  void define_type(typer& t)
  {
    t.member(m_val);
    t.member(pred);
  }

  //v is a view from the overlapped view
  template<typename View1>
  typename View1::reference
  operator()(View1 v) const
  {
     if (std::search_n(v.begin(),v.end(),v.size(),m_val,pred)==v.begin())
       return *(v.begin());
     else return typename View1::reference(null_reference());
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref binary_search(), which performs an
///   stl::binary_search on a native_view over the input.
/// @tparam value_t Type of the searched value.
/// @tparam StrictWeakOrdering Binary functor which implements the less
///   operation.
///
/// This function must be passed a native view over the input.
//////////////////////////////////////////////////////////////////////
template <typename value_t, typename StrictWeakOrdering >
struct binary_search_map_wf
{
private:
  value_t            m_val;
  StrictWeakOrdering m_comp;
public:
  binary_search_map_wf(value_t val, StrictWeakOrdering comp)
    : m_val(val), m_comp(comp)
  {}

  void define_type(typer& t)
  {
    t.member(m_val);
    t.member(m_comp);
  }

  //view is from a native partitioned view
  template < class View >
  bool operator() ( const View& view ) const
  {
    if (m_comp(m_val, *(view.begin())))
        return false;
    else if (m_comp(*(view.end()-1),m_val))
        return false;
    else if (!m_comp(m_val,*(view.begin())) && !m_comp(*(view.begin()),m_val))
        return true;
    else if (!m_comp(m_val,*(view.end()-1)) && !m_comp(*(view.end()-1),m_val))
        return true;
    else
        return std::binary_search(view.begin(), view.end(), m_val, m_comp);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref lower_bound(), which takes a view over
///   a sorted input, and computes the std::lower_bound on it, or null
///   if the range is all less than the given value.
/// @tparam value_t Type of value being searched.
/// @tparam StrictWeakOrdering Binary functor which implements the less
///   operation.
//////////////////////////////////////////////////////////////////////
template < typename value_t, typename StrictWeakOrdering >
struct lower_bound_map_wf
{
  value_t            m_val;
  StrictWeakOrdering m_comp;

  lower_bound_map_wf(value_t val, StrictWeakOrdering comp)
    :m_val(val),m_comp(comp)
  {}

  void define_type(typer& t)
  {
    t.member(m_val);
    t.member(m_comp);
  }

  template < typename View >
  typename View::reference
  operator()(const View& view) const
  {
    typedef typename View::reference ref_t;
    typedef typename View::iterator iter_t;

    if (m_comp(m_val, view[view.domain().first()]))
      return view[view.domain().first()];
    else if (m_comp(view[view.domain().last()],m_val))
      return ref_t(null_reference());

    iter_t iter = std::lower_bound(view.begin(), view.end(), m_val, m_comp);
    if (iter == view.end())
      return ref_t(null_reference());

    return *iter;
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref upper_bound(), which takes a view over
///   a sorted input, and computes the std::upper_bound on it, or null
///   if the range is all less than or equal to the given value.
/// @tparam Value Type of value being searched.
/// @tparam StrictWeakOrdering Binary functor which implements the less
///   operation.
//////////////////////////////////////////////////////////////////////
template<typename Value, typename StrictWeakOrdering>
struct upper_bound_map_wf
{
private:
  Value              m_val;
  StrictWeakOrdering m_comp;

public:
  upper_bound_map_wf(Value const& val,
                  StrictWeakOrdering const& comp)
    : m_val(val), m_comp(comp)
  { }

  void define_type(typer& t)
  {
    t.member(m_val);
    t.member(m_comp);
  }

  template<typename View>
  typename View::reference
  operator()(View const& view) const
  {
    typedef typename View::reference   ref_t;
    typedef typename View::iterator    iter_t;

    if (m_comp(m_val, view[view.domain().first()]))
     return view[view.domain().first()];
    else if (m_comp(view[view.domain().last()], m_val))
      return ref_t(null_reference());

    if (!m_comp(m_val,view[view.domain().last()])
        && !m_comp(view[view.domain().last()],m_val))
      return ref_t(null_reference());

    iter_t iter = std::upper_bound(view.begin(), view.end(), m_val, m_comp);

    if (iter == view.end())
        return ref_t(null_reference());

    return *iter;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref lower_bound() and @ref upper_bound(), which
///   takes two arguments are returns the first if non-NULL, and the second
///   otherwise.
//////////////////////////////////////////////////////////////////////
struct upper_lower_bound_reduce_wf
{
  template < typename Ref1, typename Ref2 >
  Ref1
  operator()(Ref1 lhs, Ref2 rhs) const
  {
    if (is_null_reference(lhs))
    {
      return rhs;
    }
    return lhs;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref equal_range() which returns the position of
///   its argument if the argument compares equal to the given value according
///   to the given ordering, or a NULL position otherwise.
/// @tparam value_t Type of the elements being compared.
/// @tparam StrictWeakOrdering Binary functor which, given two values, will
///   return true if the first precedes the second. If the two values return
///   false for both orderings, then they are equal.
/// @tparam return_t The type of the domain of the input view, used to construct
///   the range of equality for the output.
//////////////////////////////////////////////////////////////////////
template < typename value_t, typename StrictWeakOrdering, typename return_t>
struct equal_range_map_wf
{
    value_t            m_val;
    StrictWeakOrdering m_comp;

    equal_range_map_wf(value_t const& val, StrictWeakOrdering const& comp)
      :m_val(val),m_comp(comp)
    {}

    void define_type(typer& t)
    {
        t.member(m_val);
        t.member(m_comp);
    }

    typedef return_t result_type;

    template < class Reference >
    return_t
    operator()(const Reference val) const
    {
        if (m_comp(m_val, val))
            return return_t();

        if (m_comp(val,m_val))
            return return_t();

        return return_t(index_of(val), index_of(val));
    }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref equal_range() which, given two domains,
///   returns the one which is non-empty, or merges them together.
/// @tparam return_t The type of the domains to merge.
//////////////////////////////////////////////////////////////////////
template < typename return_t >
struct equal_range_reduce_wf
{
  typedef return_t result_type;

  return_t operator()(return_t lhs, return_t rhs) const
  {
    if (lhs.empty())
      return rhs;

    if (rhs.empty())
      return lhs;

    return return_t(lhs.first(), rhs.last());
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref find_end() which returns the last
///   occurrence of the second sequence in the first sequence, or NULL
///   otherwise.
/// @tparam Pred Binary functor which implements the equal operation.
//////////////////////////////////////////////////////////////////////
template<typename Pred>
class find_end_map
{
private:
  Pred m_pred;

public:
  find_end_map(Pred const& pred)
    : m_pred(pred) {}

  void define_type(typer& t)
  {
    t.member(m_pred);
  }

  template <typename Elt1, typename Elt2>
  typename Elt1::reference operator()(Elt1 elt1, Elt2 elt2)
  {
    if (equal(elt1, elt2, m_pred))
      return *(elt1.begin());
    else
      return typename Elt1::reference(null_reference());
  }

};

//////////////////////////////////////////////////////////////////////
/// @brief Work function which returns the second argument if it is non-NULL,
///   and the first otherwise.
//////////////////////////////////////////////////////////////////////
struct find_end_reduce
{
  template <typename Result1, typename Result2>
  Result1 operator()(Result1 result1, Result2 result2)
  {
    if (is_null_reference(result2))
      return (result1);
    else
      return (result2);
  }
};

} //namespace algo_details


//////////////////////////////////////////////////////////////////////
/// @brief Copy the elements of the input view to the output view.
/// @param vw0 One-dimensional view of the input.
/// @param vw1 One-dimensional view of the output.
/// @ingroup generatingAlgorithms
///
/// This algorithm mutates the output.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename View1>
void copy(View0 const& vw0, View1 const& vw1)
{
  stapl::map_func(assign<typename View1::value_type>(), vw0, vw1);
}


//////////////////////////////////////////////////////////////////////
/// @brief Copy the first n elements from the input view to the output view.
/// @param vw0 One-dimensional view of the input.
/// @param vw1 One-dimensional view of the output.
/// @param n Number of elements to copy.
/// @ingroup generatingAlgorithms
///
/// This algorithm mutates the output.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename View1, typename Size>
void copy_n(View0 const& vw0, View1 const& vw1, Size n)
{
  stapl_assert(n >= 0, "requested copy of negative number of elements");

  if (n == 0)
    return;

  typedef typename View0::domain_type dom_t;
  typedef typename View1::domain_type dom_t2;
  copy(
    View0(vw0.container(),
          dom_t(vw0.domain().first(),
                vw0.domain().advance(vw0.domain().first(),n-1))),
    View1(vw1.container(),
          dom_t2(vw1.domain().first(),
                 vw1.domain().advance(vw1.domain().first(),n-1))));
}


//////////////////////////////////////////////////////////////////////
/// @brief Copy elements of the input view to the output view in reverse order.
/// @param vw0 One-dimensional view of the input.
/// @param vw1 One-dimensional view of the output.
/// @ingroup reorderingAlgorithms
///
/// This algorithm mutates the output.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename View1>
void reverse_copy(View0 const& vw0, View1 const& vw1)
{
  copy(reverse_view(vw0), vw1);
}


//////////////////////////////////////////////////////////////////////
/// @brief Reverse the order of the elements in the input view.
/// @param vw0 One-dimensional view of the input.
/// @ingroup reorderingAlgorithms
///
/// This algorithm is mutating.
//////////////////////////////////////////////////////////////////////
template<typename View0>
void reverse(View0 const& vw0)
{
  typedef typename View0::value_type new_value;
  static_array<new_value> tmp_array(vw0.size());
  array_view<static_array<new_value> > tmp_vw(tmp_array);

  copy(vw0,tmp_vw);
  reverse_copy(tmp_vw,vw0);
}


//////////////////////////////////////////////////////////////////////
/// @brief Copy the elements in the input view to the output view rotated
///   to the left by k positions.
/// @param vw0 One-dimensional view of the input.
/// @param vw1 One-dimensional view of the output.
/// @param k Number of positions to shift the elements.
/// @ingroup reorderingAlgorithms
///
/// This algorithm mutates the output.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename View1>
void rotate_copy(View0 const& vw0, View1 const& vw1, int k)
{
  typedef typename View0::domain_type dom0_t;
  typedef typename View1::domain_type dom1_t;

  dom0_t dom0 = vw0.domain();
  dom1_t dom1 = vw1.domain();

  int size_vw = vw0.size();
  if (size_vw == 0)
    return;
  k=k%size_vw;
  if ((k)!=0){
    if (k<0){
      k+=size_vw;
    }
    size_t size1 = k;
    size_t size2 = size_vw - size1;

    dom0_t dom0_1(dom0.first(), dom0.advance(dom0.first(),size1-1), dom0);
    dom1_t dom1_1(dom1.advance(dom1.first(),size2), dom1.last(), dom1);
    //TODO: Find a more elegant way to construct a view with a restricted domain
    copy(View0(vw0.container(), dom0_1), View1(vw1.container(), dom1_1));

    dom0_t dom0_2(dom0.advance(dom0.first(),size1), dom0.last(), dom0);
    dom1_t dom1_2(dom1.first(), dom1.advance(dom1.first(),size2-1), dom1);
    copy(View0(vw0.container(), dom0_2), View1(vw1.container(), dom1_2));
  }
  else {
    copy(vw0, vw1);
  }
}


//////////////////////////////////////////////////////////////////////
/// @brief Rotate the elements in the view to the left by k positions.
/// @param vw1 One-dimensional view of the input.
/// @param k Number of positions to shift the elements.
/// @ingroup reorderingAlgorithms
///
/// This algorithm mutates the input.
//////////////////////////////////////////////////////////////////////
template<typename View0>
void rotate(View0 const& vw1, int k)
{
  typedef typename View0::value_type new_value;
  static_array<new_value> tmp_array(vw1.size());
  array_view<static_array<new_value> > tmp_vw(tmp_array);
  copy(vw1, tmp_vw);
  rotate_copy(tmp_vw, vw1, k);
}

//////////////////////////////////////////////////////////////////////
/// @brief Returns true if the given predicate returns true for all of the
///   elements in the input view.
/// @param view One-dimensional view of the input.
/// @param predicate Unary functor which is called on the input elements.
/// @return True if the functor returns true for all elements, false otherwise.
/// @ingroup countingAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename Pred>
bool all_of(View0 const& view, Pred predicate)
{
  return stapl::map_reduce(predicate, stapl::logical_and<bool>(), view);
}


//////////////////////////////////////////////////////////////////////
/// @brief Returns true if the given predicate returns false for all of the
///   elements in the input view, or the view is empty.
/// @param view One-dimensional view of the input.
/// @param predicate Unary functor which is called on the input elements.
/// @return True if the functor returns false for all elements, false otherwise.
/// @ingroup countingAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename Pred>
bool none_of(View0 const& view, Pred predicate)
{
  unary_negate<Pred> thenegatepred(predicate);
  return stapl::map_reduce(thenegatepred, stapl::logical_and<bool>(), view);
}

//////////////////////////////////////////////////////////////////////
/// @brief Returns true if the given predicate returns true for any of the
///   elements in the input view.
/// @param view One-dimensional view of the input.
/// @param predicate Unary functor which is called on the input elements.
/// @return True if the functor returns true for any element, false otherwise.
/// @ingroup countingAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename Pred>
bool any_of(View0 const& view, Pred predicate)
{
  return stapl::map_reduce(predicate, stapl::logical_or<bool>(), view);
}


//////////////////////////////////////////////////////////////////////
/// @brief Finds the first element in the input which matches any of the
///   elements in the given view, according to the given predicate.
/// @param view0 One-dimensional view of the input.
/// @param view1 One-dimensional view of the elements to find.
/// @param predicate Binary functor which implements the less operation.
/// @return A reference to the first element that matches, or NULL otherwise.
/// @ingroup searchAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template <typename View0, typename View1, typename Pred>
typename View0::reference find_first_of(View0 const& view0, View1 const& view1,
                                        Pred const& predicate)
{
  return stapl::map_reduce(
            algo_details::wf_compare_find_first_of<Pred>(predicate),
            algo_details::find_reduce(),
            view0, make_repeat_view(view1));
}


//////////////////////////////////////////////////////////////////////
/// @brief Finds the first element in the input which matches any of the
///   elements in the given view.
/// @param view0 One-dimensional view of the input.
/// @param view1 One-dimensional view of the elements to find.
/// @return A reference to the first element that matches, or NULL otherwise.
/// @ingroup searchAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::equal_to.
//////////////////////////////////////////////////////////////////////
template <typename View0, typename View1>
typename View0::reference find_first_of(View0 const& view0, View1 const& view1)
{
  return find_first_of(view0, view1, equal_to<typename View0::reference>());
}


//////////////////////////////////////////////////////////////////////
/// @brief Determines if the first view is lexicographically less than the
///   second view, using the given functor.
/// @param pview1 One-dimensional view of the first input.
/// @param pview2 One-dimensional view of the second input.
/// @param pred Binary functor which implements the less operation.
/// @return True if the first input is lexicographically less than the second.
/// @ingroup sortrelatedAlgorithms
///
/// This algorithm is non-mutating.
////////////////////////////////////////////////////////////////////
template <typename View, typename View2, typename Pred>
bool lexicographical_compare(View const& pview1, View2 const& pview2,
                             Pred const& pred)
{
  int minelem = 0;
  int result;
  int size1 = pview1.size();
  int size2 = pview2.size();
  if (size1 > size2){
    minelem = size2;
    View viewtrunc(pview1.container(), typename View::domain_type(0,minelem-1));
    result = stapl::map_reduce(algo_details::lexico_compare<Pred>(pred),
                               algo_details::lexico_reduce<int>(),
                               viewtrunc, pview2);
  }
  else if (size1 < size2) {
    minelem = size1;
    View2 viewtrunc(pview2.container(),typename View::domain_type(0,minelem-1));
    result = stapl::map_reduce(algo_details::lexico_compare<Pred>(pred),
                               algo_details::lexico_reduce<int>(),
                               pview1, viewtrunc);
  }
  else {
    result = stapl::map_reduce(algo_details::lexico_compare<Pred>(pred),
                               algo_details::lexico_reduce<int>(),
                               pview1, pview2);
  }

  if (result == 1){
    return true;
  }
  else if (result == -1){
    return false;
  }
  return size1<size2;
}

//////////////////////////////////////////////////////////////////////
/// @brief Determines if the first view is lexicographically less than the
///   second view.
/// @param pview1 One-dimensional view of the first input.
/// @param pview2 One-dimensional view of the second input.
/// @return True if the first input is lexicographically less than the second.
/// @ingroup sortrelatedAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::less.
////////////////////////////////////////////////////////////////////
template <typename View, typename View2>
bool lexicographical_compare(View const& pview1, View2 const& pview2)
{
  return lexicographical_compare(pview1, pview2,
                                 less<typename View::value_type>());
}


//////////////////////////////////////////////////////////////////////
/// @brief Partition the input such that all elements for which the predicate
///   returns true are ordered before those for which it returned false, while
///   also maintaining the relative ordering of the elements.
/// @param pview One-dimensional view of the input.
/// @param predicate Unary functor used to partition the input.
/// @return The number of elements for which the predicate returned true.
/// @ingroup reorderingAlgorithms
///
/// This algorithm is mutating.
/// @note Replacing static_array with array will eliminate possible
///       synchronous communications generated when the data is not
///       aligned.
//////////////////////////////////////////////////////////////////////
template <typename View, typename Pred>
size_t stable_partition(View const& pview, Pred predicate)
{
  typedef static_array<algo_details::partition_pred_counter>  cnt_container_t;
  typedef array_view<cnt_container_t>                         cnt_view_t;

  typedef static_array<typename View::value_type>  tmp_container_t;
  typedef array_view<tmp_container_t>              tmp_view_t;

  unsigned int num_elements = pview.size();
  cnt_view_t count_view(new cnt_container_t(num_elements));
  tmp_view_t temp_view(new tmp_container_t(num_elements));

  map_func(algo_details::partition_count_matches<Pred>(predicate),
           pview, count_view);

  algo_details::partition_pred_counter num_satisfy_elements =
    stapl::accumulate(count_view, algo_details::partition_pred_counter());
  stapl::partial_sum(count_view, count_view, false);

  map_func(algo_details::partition_fill_temp<Pred>(
           predicate, num_satisfy_elements.m_is_match),
           pview, count_view, make_repeat_view(temp_view));

  stapl::copy(temp_view, pview);
  return num_satisfy_elements.m_is_match;
}

//////////////////////////////////////////////////////////////////////
/// @brief Copies all elements from the input for which the functor returns
///   true into the first output view, and all others into the second output.
/// @param pview0 One-dimensional view of the input.
/// @param pview1 One-dimensional view of the output of elements returning true.
/// @param pview2 One-dimensional view of the output of elements returning
///   false.
/// @param predicate Unary function used to partition the input.
/// @return Pair containing views over the copied output ranges.
/// @ingroup reorderingAlgorithms
///
/// This algorithm mutates the two output views.
//////////////////////////////////////////////////////////////////////
template <typename View0,typename View1,typename View2,typename Pred>
std::pair<View1, View2>
partition_copy(View0 const& pview0,View1 const& pview1,View2 const& pview2,
               Pred predicate)
{
  typedef list<typename View0::value_type> plist_type;
  typedef list_view <plist_type>           plist_view_type;
  typedef typename View1::domain_type      dom_t1;
  typedef typename View2::domain_type      dom_t2;

  plist_type                               list_pred;
  plist_type                               list_not_pred;

  typedef algo_details::partition_apply_pred<
    Pred, typename View0::value_type
  > wf_t;

  map_func(wf_t(predicate, &list_pred, &list_not_pred), pview0);

  size_t n1;
  size_t n2;

  if (list_pred.size() != 0)
    n1 = list_pred.size() - 1;
  else
    n1 = list_pred.size();

  if (list_not_pred.size() != 0)
    n2 = list_not_pred.size() - 1;
  else
    n2 = list_not_pred.size();

  View1 nv1(pview1.container(),
            dom_t1(pview1.domain().first(),
                   pview1.domain().advance(pview1.domain().first(),n1)),
            pview1.mapfunc());

  View2 nv2(pview2.container(),dom_t2(pview2.domain().first(),
            pview2.domain().advance(pview2.domain().first(),n2)),
            pview2.mapfunc() );

  if (list_pred.size() != 0)
  {
    copy(plist_view_type(list_pred), nv1);
  }

  if (list_not_pred.size()!=0)
  {
    copy(plist_view_type(list_not_pred), nv2);
  }

  return std::make_pair(nv1, nv2);
}

//////////////////////////////////////////////////////////////////////
/// @brief Returns a pair containing the minimum and maximum values returned
///   by the given predicate when called on all values in the input view.
/// @param view One-dimensional view of the input.
/// @param pred Unary predicate called on all elements.
/// @return A pair containing the minimum and maximum result values.
/// @ingroup extremaAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Pred>
std::pair<typename View::value_type,typename View::value_type>
minmax_value(View const& view, Pred const& pred)
{
  typedef typename view_traits<View>::value_type value_t;

  return map_reduce(
    element_to_pair_map<value_t>(), minmax<value_t, Pred>(pred), view
  );
}

template<typename View>
std::pair<typename View::value_type,typename View::value_type>
minmax_value(View const& view)
{
  typedef typename view_traits<View>::value_type value_t;

  return minmax_value(view, stapl::less<value_t>());
}

//////////////////////////////////////////////////////////////////////
/// @brief Partition the input such that all elements for which the predicate
///   returns true are ordered before those for which it returned false.
/// @param pview One-dimensional view of the input.
/// @param predicate Unary functor used to partition the input.
/// @ingroup reorderingAlgorithms
///
/// This algorithm is mutating, and currently calls @ref stable_partition().
//////////////////////////////////////////////////////////////////////
template <typename View, typename Pred>
size_t partition(View const& pview, Pred predicate)
{
  return stable_partition(pview,predicate);
};

//////////////////////////////////////////////////////////////////////
/// @brief Finds the smallest value in the input view.
/// @param view One-dimensional view of the input.
/// @param comp Binary functor which implements the less operation.
/// @return The smallest value in the input.
/// @ingroup extremaAlgorithms
///
/// This algorithm is non-mutating.
/// @todo Replace with reduce() when available.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Compare>
typename View::value_type
min_value(View const& view, Compare comp)
{
  typedef typename View::value_type value_t;
  return stapl::map_reduce(identity<value_t>(),
                         min_comp<value_t, Compare>(comp), view);
}

//////////////////////////////////////////////////////////////////////
/// @brief Finds the smallest value in the input view.
/// @param view One-dimensional view of the input.
/// @return The smallest value in the input.
/// @ingroup extremaAlgorithms
///
/// This version uses the < operator to compare the elements.
/// @todo Replace with reduce() when available.
//////////////////////////////////////////////////////////////////////
template<typename View>
typename View::value_type
min_value(View const& view)
{
  typedef typename View::value_type value_t;
  return stapl::map_reduce(identity<value_t>(), min<value_t>(), view);
}

//////////////////////////////////////////////////////////////////////
/// @brief Finds the largest value in the input view.
/// @param view One-dimensional view of the input.
/// @param comp Binary functor which implements the less operation.
/// @return The largest value in the input.
/// @ingroup extremaAlgorithms
///
/// This algorithm is non-mutating.
/// @todo Replace with reduce() when available.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Compare>
typename View::value_type
max_value(View const& view, Compare comp)
{
  typedef typename View::value_type value_t;
  return stapl::map_reduce(identity<value_t>(),
                            max_comp<value_t, Compare>(comp), view);
}

//////////////////////////////////////////////////////////////////////
/// @brief Finds the largest value in the input view.
/// @param view One-dimensional view of the input.
/// @return The largest value in the input.
/// @ingroup extremaAlgorithms
///
/// This version uses the < operator to compare the elements.
/// @todo Replace with reduce() when available.
//////////////////////////////////////////////////////////////////////
template<typename View>
typename View::value_type
max_value(View const& view)
{
  typedef typename View::value_type value_t;
  return stapl::map_reduce(identity<value_t>(), max<value_t>(), view);
}

//////////////////////////////////////////////////////////////////////
/// @brief Finds the smallest element in the input view (or the first smallest
///   if there are multiple), which compares less than any other element using
///   the given functor.
/// @param view One-dimensional view of the input.
/// @param comp Binary functor which implements the less operation.
/// @return Reference to the smallest element in the input view.
/// @ingroup extremaAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Compare>
typename View::reference
min_element(View const& view, Compare comp)
{
  typedef typename View::value_type value_t;
  return stapl::map_reduce(element_map(),
      min_element_reduce<value_t, Compare>(comp), view);
}

//////////////////////////////////////////////////////////////////////
/// @brief Finds the smallest element in the input view (or the first smallest
///   if there are multiple).
/// @param view One-dimensional view of the input.
/// @return Reference to the smallest element in the input view.
/// @ingroup extremaAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::less.
//////////////////////////////////////////////////////////////////////
template<typename View>
typename View::reference
min_element(View const& view)
{
  typedef typename View::value_type value_t;
  return min_element(view,less<value_t>());
}

//////////////////////////////////////////////////////////////////////
/// @brief Finds the largest element in the input view (or the first largest
///   if there are multiple), which does not compare less than any other element
///   using the given functor.
/// @param view One-dimensional view of the input.
/// @param comp Binary functor which implements the less operation.
/// @return Reference to the largest element in the input view.
/// @ingroup extremaAlgorithms
///
/// This is a non-mutating algorithm.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Compare>
typename View::reference
max_element(View const& view, Compare comp)
{
typedef typename View::value_type value_t;

return stapl::map_reduce(element_map(),
                         max_element_reduce<value_t, Compare>(comp), view);
}


//////////////////////////////////////////////////////////////////////
/// @brief Finds the largest element in the input view (or the first largest
///   if there are multiple).
/// @param view One-dimensional view of the input.
/// @return Reference to the largest element in the input view.
/// @ingroup extremaAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::less.
//////////////////////////////////////////////////////////////////////
template<typename View>
typename View::reference
max_element(View const& view)
{
  typedef typename View::value_type value_t;
  return max_element(view, less<value_t>());
}

//////////////////////////////////////////////////////////////////////
/// @brief Assign each value of the input view to the result of a successive
///   call to the provided functor.
/// @param view One-dimensional view of the input.
/// @param gen Functor which is called to generate elements (accepts no input).
/// @ingroup generatingAlgorithms
///
/// This algorithm mutates the input view.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Generator>
void generate(View const& view, Generator gen)
{
  typedef typename View::value_type             value_t;
  copy(functor_view(view.size(),
                    offset_gen<std::size_t,value_t,Generator>(gen)),
       view);
}

//////////////////////////////////////////////////////////////////////
/// @brief Assign the n values of the input view starting at the given element
///   to the result of a successive call to the provided functor.
/// @param view One-dimensional view of the input.
/// @param first_elem First element to fill with the generated value.
/// @param n Number of elements to fill with the generated value.
/// @param gen Functor which is called to generate elements (accepts no input).
/// @ingroup generatingAlgorithms
///
/// This algorithm mutates the input view.
//////////////////////////////////////////////////////////////////////
template <typename View, typename Generator>
void generate_n(View const& view, size_t first_elem, size_t n, Generator gen)
{
  typedef typename View::domain_type dom_t;
  dom_t dom = view.domain();
  dom_t new_dom(dom.advance(dom.first(),first_elem),
                dom.advance(dom.first(),first_elem + n - 1), dom);
  View view2(view.container(), new_dom);
  generate(view2, gen);
}

//////////////////////////////////////////////////////////////////////
/// @brief Replace the values from the input view for which the given predicate
///   returns true with the new value.
/// @param vw One-dimensional view of the input.
/// @param pred Unary functor which returns true for replaced elements.
/// @param new_value Value used to replace elements.
/// @ingroup generatingAlgorithms
///
/// This algorithm mutates the input view.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Predicate>
void replace_if(View& vw, Predicate pred,
                typename View::value_type const& new_value)
{
  stapl::map_func(algo_details::assign_if<Predicate, typename View::value_type>
                   (pred, new_value), vw);
}

//////////////////////////////////////////////////////////////////////
/// @brief Replace the given value in the input with the new value.
/// @param vw One-dimensional view of the input.
/// @param old_value Value replaced in the input.
/// @param new_value Value used to replace old values.
/// @ingroup generatingAlgorithms
///
/// This algorithm mutates the input view. The comparison is done with
/// stapl::equal_to.
///
/// @todo Track down why std::bind1st seems to be seeping into stapl namespace
/// with icc compiler and requiring explicit qualification of stapl::bind1st.
//////////////////////////////////////////////////////////////////////
template<typename View>
void replace(View& vw, typename View::value_type const& old_value,
             typename View::value_type const& new_value)
{
  replace_if(vw,
             stapl::bind2nd(equal_to<typename View::value_type>(),old_value),
             new_value);
}

//////////////////////////////////////////////////////////////////////
/// @brief Copy the values from the input view to the output, except for those
///   elements for which the given predicate returns true, which are replaced
///   with the given value.
/// @param vw0 One-dimensional view of the input.
/// @param vw1 One-dimensional view of the output.
/// @param pred Unary functor which returns true for replaced elements.
/// @param new_value Value used to replace elements for which the functor
///   returns true.
/// @return Iterator pointing to the end of the output view.
/// @ingroup generatingAlgorithms
///
/// This algorithm mutates the output view. The input and output views must be
/// the same size.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename View1, typename Predicate>
typename View1::iterator
replace_copy_if(View0 const& vw0, View1 const& vw1, Predicate pred,
                typename View0::value_type new_value)
{
  stapl::map_func(
      algo_details::copy_with_replacement<Predicate, typename View0::value_type>
        (pred,new_value),vw0, vw1);
  return vw1.end();
}

//////////////////////////////////////////////////////////////////////
/// @brief Assigns the given value to the first n elements of the input view.
/// @param vw One-dimensional view of the input.
/// @param value The value to fill into the input.
/// @param n Number of elements to fill.
/// @ingroup generatingAlgorithms
///
/// This algorithm mutates the input view. The input must be at least n in size.
///
/// @todo Track down why std::bind1st seems to be seeping into stapl namespace
/// with icc compiler and requiring explicit qualification of stapl::bind1st.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Size>
void fill_n(View& vw, typename View::value_type value, Size n)
{
  View nview(vw.container(),
             typename View::domain_type(vw.domain().first(),
                                        vw.domain().first() + n - 1));
  stapl::map_func(stapl::bind1st(assign<typename View::value_type>(), value),
                  nview);
}

//////////////////////////////////////////////////////////////////////
/// @brief Assigns the given value to the elements of the input view.
/// @param vw One-dimensional view of the input.
/// @param value The value to fill into the input.
/// @ingroup generatingAlgorithms
///
/// This algorithm mutates the input view.
///
/// @todo Track down why std::bind1st seems to be seeping into stapl namespace
/// with icc compiler and requiring explicit qualification of stapl::bind1st.
//////////////////////////////////////////////////////////////////////
template<typename View>
void fill(View const& vw, typename View::value_type value)
{
  stapl::map_func(stapl::bind1st(assign<typename View::value_type>(), value),
                  vw);
}

//////////////////////////////////////////////////////////////////////
/// @brief Remove the values from the input view for which the given predicate
///   returns false.
/// @param pview One-dimensional view of the input.
/// @param predicate Unary functor which returns true for removed elements.
/// @return View over the copied range, which is the same as or smaller than the
///   provided input view.
/// @ingroup removingAlgorithms
///
/// This algorithm mutates the input view.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Pred>
View keep_if(View const& pview, Pred predicate)
{
  typedef typename View::domain_type dom_t;
  size_t truesize=partition(pview,predicate);

  if (truesize!=0) {
    dom_t new_dom(pview.domain().first(),
                  pview.domain().advance(pview.domain().first(),truesize - 1),
                  pview.domain());
    return View(pview.container(), new_dom);
  }
  else
    return View(pview.container(), dom_t());
}

//////////////////////////////////////////////////////////////////////
/// @brief Remove the values from the input view for which the given predicate
///   returns true.
/// @param pview One-dimensional view of the input.
/// @param predicate Unary functor which returns true for removed elements.
/// @return View over the copied range, which is the same as or smaller than the
///   provided input view.
/// @ingroup removingAlgorithms
///
/// This algorithm mutates the input view.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Pred>
View remove_if(View const& pview, Pred predicate)
{
  typedef typename View::domain_type dom_t;
  stapl::unary_negate<Pred> thenegatepred(predicate);
  size_t truesize=partition(pview,thenegatepred);

  if (truesize!=0) {
    dom_t new_dom(pview.domain().first(),
                  pview.domain().advance(pview.domain().first(),truesize - 1),
                  pview.domain());
    return View(pview.container(), new_dom);
  }
  else
     return View(pview.container(), dom_t());
}

//////////////////////////////////////////////////////////////////////
/// @brief Copy the values from the input view to the output those
///   elements for which the given predicate returns true.
/// @param vw0 One-dimensional view of the input.
/// @param vw1 One-dimensional view of the output.
/// @param predicate Unary functor which returns true for copied elements.
/// @return View over the copied range, which is the same as or smaller than the
///   provided output view.
/// @ingroup removingAlgorithms
///
/// This algorithm mutates the output view. The input and output views must be
/// the same size.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename View1, typename Pred>
View1 copy_if(View0 const& vw0, View1 & vw1, Pred predicate)
{
  copy(vw0, vw1);
  return keep_if(vw1, predicate);
}

//////////////////////////////////////////////////////////////////////
/// @brief Copy the values from the input view to the output, except for those
///   elements for which the given predicate returns true.
/// @param vw0 One-dimensional view of the input.
/// @param vw1 One-dimensional view of the output.
/// @param predicate Unary functor which returns true for removed elements.
/// @return View over the copied range, which is the same as or smaller than the
///   provided output view.
/// @ingroup removingAlgorithms
///
/// This algorithm mutates the output view. The input and output views must be
/// the same size.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename View1, typename Pred>
View1 remove_copy_if(View0 const& vw0, View1 & vw1, Pred predicate)
{
  copy(vw0, vw1);
  return remove_if(vw1, predicate);
}

//////////////////////////////////////////////////////////////////////
/// @brief Remove the given value from the input.
/// @param vw0 One-dimensional view of the input.
/// @param valuetoremove Value removed from the input.
/// @return View over the new range, which is the same as or smaller than the
///   provided input view.
/// @ingroup removingAlgorithms
///
/// This algorithm mutates the input view. The comparison is done with
/// stapl::equal_to.
//////////////////////////////////////////////////////////////////////
template<typename View>
View remove(View & vw0, typename View::value_type valuetoremove)
{
  return remove_if(vw0, stapl::bind2nd(stapl::equal_to<
      typename View::value_type>(), valuetoremove));
}

//////////////////////////////////////////////////////////////////////
/// @brief Copy the values from the input view to the output, except for those
///   elements which are equal to the given value.
/// @param vw0 One-dimensional view of the input.
/// @param vw1 One-dimensional view of the output.
/// @param valuetoremove Value removed from the output.
/// @return View over the copied range, which is the same as or smaller than the
///   provided output view.
/// @ingroup removingAlgorithms
///
/// This algorithm mutates the output view. The comparison is done with
/// stapl::equal_to.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename View1>
View1 remove_copy(View0 const& vw0, View1 & vw1,
    typename View0::value_type valuetoremove)
{
  copy(vw0, vw1);
  return remove_if(vw1, stapl::bind2nd(stapl::equal_to<
      typename View0::value_type>(), valuetoremove));
}

//////////////////////////////////////////////////////////////////////
/// @brief Merges the two sorted input views into the output view in sorted
///   order.
/// @param view1 One-dimensional view of the first sorted input.
/// @param view2 One-dimensional view of the second sorted input.
/// @param merged One-dimensional view of the output.
/// @ingroup sortrelatedAlgorithms
///
/// This algorithm mutates the output.
//////////////////////////////////////////////////////////////////////
template<typename View1, typename View2, typename MergeView>
void merge(View1 const& view1, View2 const& view2, MergeView& merged)
{
  //TYPEDEFS:

  //Balanced Partitionings
  typedef typename View1::domain_type               dom1_t;
  typedef balanced_partition<dom1_t>                balanced_part1_t;
  typedef segmented_view<View1, balanced_part1_t> balanced1_t;

  typedef typename View2::domain_type               dom2_t;
  typedef balanced_partition<dom2_t>                balanced_part2_t;
  typedef segmented_view<View2, balanced_part2_t> balanced2_t;

  typedef typename MergeView::domain_type           domM_t;

  //Distribution of Local Splitters
  typedef typename View1::value_type          val1_t;
  typedef std::vector<val1_t>                 vals1_t;
  typedef static_array<vals1_t>               vals_array1_t;
  typedef array_view<vals_array1_t>           vals_array_view1_t;

  typedef typename View2::value_type          val2_t;
  typedef std::vector<val2_t>                 vals2_t;
  typedef static_array<vals2_t>               vals_array2_t;
  typedef array_view<vals_array2_t>           vals_array_view2_t;

  //Local Local Splitters
  typedef algo_details::merge_send_leader<val1_t>   send_leader1;

  typedef algo_details::merge_send_leader<val2_t>   send_leader2;

  //Splitters (Indices)
  typedef typename View1::index_type                  index1_t;
  typedef static_array<index1_t>                      indices1_t;
  typedef array_view<indices1_t>                      indices1_view_t;

  typedef typename View2::index_type                  index2_t;
  typedef static_array<index2_t>                      indices2_t;
  typedef array_view<indices2_t>                      indices2_view_t;

  typedef typename MergeView::index_type              indexM_t;
  typedef static_array<indexM_t>                      indicesM_t;
  typedef array_view<indicesM_t>                      indicesM_view_t;

  //Segmented Partitionings
  typedef splitter_partition<dom1_t, indices1_view_t> split_part1_t;
  typedef segmented_view<View1, split_part1_t>      split1_t;

  typedef splitter_partition<dom2_t, indices2_view_t> split_part2_t;
  typedef segmented_view<View2, split_part2_t>      split2_t;

  typedef splitter_partition<domM_t, indicesM_view_t> split_partM_t;
  typedef segmented_view<MergeView, split_partM_t>  splitM_t;

  //Sizes of Each Partition
  typedef static_array<size_t>                        sizes_t;
  typedef array_view<sizes_t>                         sizes_view_t;
  typedef typename sizes_view_t::domain_type          sizes_domain_t;

  //ALGORITHM:

  size_t num_locs = get_num_locations();
  size_t my_part = get_location_id();
  size_t num_split = num_locs*2;

  //partition views naturally
  balanced_part1_t    balanced_part1(view1.domain(), num_locs);
  balanced1_t         balanced1(view1, balanced_part1);
  balanced_part2_t    balanced_part2(view2.domain(), num_locs);
  balanced2_t         balanced2(view2, balanced_part2);

  //copy the list of leaders to each location
  //FIXME: use a repeated view of the leaders, not a view of vectors
  vals_array1_t       loc1array(num_locs, vals1_t(num_locs));
  vals_array_view1_t  loc1view(loc1array);
  send_leader1 send_func1(my_part,
                 balanced1[my_part][balanced1[my_part].domain().first()]);

  vals_array2_t       loc2array(num_locs, vals2_t(num_locs));
  vals_array_view2_t  loc2view(loc2array);
  send_leader2 send_func2(my_part,
                 balanced2[my_part][balanced2[my_part].domain().first()]);

  for (size_t i=0; i<num_locs; ++i) {
    loc1view.apply_set(i, send_func1);
    loc2view.apply_set(i, send_func2);
  }
  rmi_fence();

  //find the indices
  indices1_t        indices1(num_split, view1.domain().first());
  indices1_view_t   indices1_view(indices1);
  indices2_t        indices2(num_split, view2.domain().first());
  indices2_view_t   indices2_view(indices2);
  map_func(algo_details::merge_splitters(get_location_id()),
           balanced1, loc1view, loc2view, make_repeat_view(indices1_view));
  map_func(algo_details::merge_splitters(get_location_id()),
           balanced2, loc2view, loc1view, make_repeat_view(indices2_view));

  //partition view2 (so that ranges are same as view1)
  split_part1_t   split_part1(view1.domain(), indices1_view, true);
  split1_t        split1(view1, split_part1);

  split_part2_t   split_part2(view2.domain(), indices2_view, true);
  split2_t        split2(view2, split_part2);

  //TODO: combining merge_output_sizes, partial_sum, and merge_output_indices
  //      could reduce the time significantly on domains where advance() does
  //      not work in constant time

  //find partition on merged by finding sizes and advancing through the domain
  sizes_t       out_sizes(num_split+1);
  sizes_view_t  out_sizev(out_sizes);

  map_func(algo_details::merge_output_sizes(), split1, split2, out_sizev);

  //remove the last size to attain splitters
  sizes_domain_t cut_dom(0, num_split-1);
  out_sizev.set_domain(cut_dom);

  partial_sum(out_sizev, out_sizev);

  indicesM_t        ixMs(num_split);
  indicesM_view_t   ixMv(ixMs);
  map_func(algo_details::merge_output_indices(),
           out_sizev, ixMv, make_repeat_view(merged));

  split_partM_t split_partM(merged.domain(), ixMv, true);
  splitM_t      splitM(merged, split_partM);

  //merge individual pieces with std::merge
  map_func(algo_details::merge_map(), split1, split2, splitM);

  rmi_fence();
}


//////////////////////////////////////////////////////////////////////
/// @brief Swaps the elements of the two input views.
/// @param vw0 One-dimensional view of the first input.
/// @param vw1 One-dimensional view of the second input.
/// @ingroup generatingAlgorithms
///
/// This algorithm mutates both input views, and requires that both views are
/// the same size.
//////////////////////////////////////////////////////////////////////
template<typename View>
void swap_ranges(View& vw0, View& vw1)
{
  stapl::map_func(swap2(),vw0,vw1);
}


//////////////////////////////////////////////////////////////////////
/// @brief Copy the elements from the input to the output, replacing the given
///   old_value with the new_value.
/// @param vw0 One-dimensional view of the input.
/// @param vw1 One-dimensional view of the output.
/// @param old_value The old value to replace.
/// @param new_value The new value to substitute for occurrences of old_value.
/// @return Iterator to the end of the newly copied view.
/// @ingroup generatingAlgorithms
///
/// This algorithm mutates the output view, and requires a view with iterator
/// support. It uses stapl::equal_to for comparisons.
///
/// @todo Track down why std::bind1st seems to be seeping into stapl namespace
/// with icc compiler and requiring explicit qualification of stapl::bind1st.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename View1>
typename View1::iterator
replace_copy(View0& vw0, View1& vw1, typename View0::value_type old_value,
             typename View0::value_type new_value)
{
  replace_copy_if(vw0, vw1,
                  stapl::bind2nd(equal_to<typename View0::value_type>(),
                                 old_value),
                   new_value);
  return vw1.end();
}


//////////////////////////////////////////////////////////////////////
/// @brief Applies the given functor to all of the elements in the input.
/// @param vw0 One-dimensional view over the input.
/// @param func Unary functor to apply to the elements.
/// @return The functor that was passed as input.
/// @ingroup generatingAlgorithms
///
/// This algorithm will mutate the input view.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename Function>
Function
for_each(const View0& vw0, Function func)
{
  stapl::map_func(apply_functor<Function>(func), vw0);
  return func;
}


//////////////////////////////////////////////////////////////////////
/// @brief Applies the given function to the input, and stores the result in
///   the output.
/// @param vw0 One-dimensional view over the input.
/// @param vw1 One-dimensional view over the output.
/// @param func Unary function which is applied to all of the input elements.
/// @ingroup generatingAlgorithms
///
/// This algorithm mutates the output view only.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename View1, typename Function>
void transform(const View0& vw0, const View1& vw1, Function func)
{
  stapl::map_func(transform_assign<Function>(func), vw0, vw1);
}


//////////////////////////////////////////////////////////////////////
/// @brief Applies the given function to the inputs, and stores the result in
///   the output.
/// @param vw0 One-dimensional view over the first input.
/// @param vw1 One-dimensional view over the second input.
/// @param vw2 One-dimensional view over the output.
/// @param func Binary function which is applied to all of the input elements.
/// @ingroup generatingAlgorithms
///
/// This algorithm mutates the output view only.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename View1, typename View2, typename Function>
void transform(View0& vw0, View1& vw1, View2& vw2, Function func)
{
  stapl::map_func(transform_assign<Function>(func), vw0, vw1, vw2);
}


//////////////////////////////////////////////////////////////////////
/// @brief Computes whether the input view is sorted.
/// @param view One-dimensional view of the input.
/// @param comp Binary functor which implements the less operation.
/// @return True if the input is sorted, false otherwise.
/// @ingroup sortrelatedAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Comp>
bool is_sorted(View const& view, Comp comp)
{
 return stapl::map_reduce(algo_details::gen_comp_func<Comp>(comp),
                   logical_and<bool>(),
                   stapl::make_overlap_view(view, 1, 0, 1));
}


//////////////////////////////////////////////////////////////////////
/// @brief Computes whether the input view is sorted.
/// @param view One-dimensional view of the input.
/// @return True if the input is sorted, false otherwise.
/// @ingroup sortrelatedAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::less.
//////////////////////////////////////////////////////////////////////
template<typename View>
bool is_sorted(View const& view)
{
  return is_sorted(view,less<typename View::value_type>());
}


namespace algo_details {

//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref is_sorted_until(), which returns false if the
///   given elements are out of order, and true otherwise.
/// @tparam View Type of the input view.
/// @tparam Comp Binary functor implementing the less operation.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Comp>
class is_sorted_until_compare
{
  private:
    Comp m_comp;

  public:
    is_sorted_until_compare(Comp const& c)
      : m_comp(c)
    {}

  void define_type(typer& t)
  {
    t.member(m_comp);
  }

  typedef std::pair<bool, typename View::index_type>  pair_type;
  typedef pair_type result_type;

  template <typename Overlap_view>
  pair_type operator()(Overlap_view element) const
  {
    if (!m_comp(*(element.begin()+1),*(element.begin())))
      return pair_type(true,typename View::index_type());
    else
      return pair_type(false,index_of(*(element.begin())));
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function for @ref is_sorted_until(), which returns the first
///   argument if it is not sorted, and otherwise returns the second.
/// @tparam View Type of the input view.
//////////////////////////////////////////////////////////////////////
template <typename View>
struct is_sorted_until_reduce
{
  typedef std::pair<bool, typename View::index_type>  pair_type;
  typedef pair_type result_type;

  pair_type operator()(pair_type elem1, pair_type elem2)
  {
    if (!elem1.first)
    {
      return elem1;
    } else {
      return elem2;
    }
  }
};

}

//////////////////////////////////////////////////////////////////////
/// @brief Finds the range of elements in the input which are sorted.
/// @param v One-dimensional view of the input.
/// @param c Binary functor which implements the less operation.
/// @return A view over the sorted range.
/// @ingroup sortrelatedAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template <typename View, typename Comp>
View is_sorted_until(View const& v, Comp const& c)
{
  typedef std::pair<bool, typename View::index_type>  pair_type;
  typedef typename View::domain_type dom_t;

  pair_type result =
    stapl::map_reduce(algo_details::is_sorted_until_compare<View, Comp>(c),
                      algo_details::is_sorted_until_reduce<View>(),
                      stapl::make_overlap_view(v,1,0,1));

  if (result.first)
    return v;
  View view(v.container(),dom_t(0,result.second));
  return view;
}

//////////////////////////////////////////////////////////////////////
/// @brief Finds the range of elements in the input which are sorted.
/// @param v One-dimensional view of the input.
/// @return A view over the sorted range.
/// @ingroup sortrelatedAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::less.
//////////////////////////////////////////////////////////////////////
template <typename View>
View is_sorted_until(View const& v)
{
  less<typename View::value_type> comp;
  return is_sorted_until(v,comp);
}

//////////////////////////////////////////////////////////////////////
/// @brief Finds the first element in the input for which the predicate returns
///   true, or NULL if none exist.
/// @param view One-dimensional view of the input.
/// @param pred Functor used to test the elements.
/// @return A reference to the found element, or NULL if none found.
/// @ingroup searchAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Predicate>
typename View::reference
find_if(View const& view, Predicate const& pred)
{
  return stapl::map_reduce(algo_details::find_map<Predicate>(pred),
                    algo_details::find_reduce(),
                    view);
}

//////////////////////////////////////////////////////////////////////
/// @brief Finds the first element in the input for which the predicate returns
///   false, or NULL if none exist.
/// @param view One-dimensional view of the input.
/// @param pred Functor used to test the elements.
/// @return A reference to the found element, or NULL if none found.
/// @ingroup searchAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Predicate>
typename View::reference
find_if_not(View const& view, Predicate const& pred)
{
  stapl::unary_negate<Predicate> negative_pred(pred);
  return find_if(view, negative_pred);
}

//////////////////////////////////////////////////////////////////////
/// @brief Finds the first occurrence of the given value in the input, or NULL
///   if it is not found.
/// @param view One-dimensional view of the input.
/// @param value Value to find.
/// @return A reference to the found element, or NULL if none found.
/// @ingroup searchAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View, typename T>
typename View::reference
find(View const& view, T const& value)
{
  // FIXME - track down why std::bind2nd seems to be seeping into
  // this namespace (icc)
  //
  return find_if(view, stapl::bind2nd(equal_to<T>(), value));
}

//////////////////////////////////////////////////////////////////////
/// @brief Searches the input view for the given value using a binary
///   search, and returns true if that value exists in the input.
/// @param view One-dimensional view of the input.
/// @param value Value to search for in the input (of same type as inputs).
/// @param comp Binary functor which implements the less than operation.
/// @return True if the value is found in the input, false otherwise.
/// @ingroup binarysearchAlgorithms
///
/// This algorithm is non-mutating, and requires the input to be sorted.
//////////////////////////////////////////////////////////////////////
template < typename View, typename StrictWeakOrdering >
bool
binary_search(View const& view, typename View::value_type value,
              StrictWeakOrdering comp)
{
    typedef typename View::value_type value_t;
    typedef algo_details::binary_search_map_wf<value_t, StrictWeakOrdering>
              map_wf_t;

    return stapl::map_reduce(map_wf_t(value, comp),
                      stapl::logical_or<bool>(),
                      stapl::native_view(view));
}


//////////////////////////////////////////////////////////////////////
/// @brief Searches the input view for the given value using a binary
///   search, and returns true if that value exists in the input.
/// @param view One-dimensional view of the input.
/// @param value Value to search for in the input (of same type as inputs).
/// @return True if the value is found in the input, false otherwise.
/// @ingroup binarysearchAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::less.
//////////////////////////////////////////////////////////////////////
template < typename View >
bool
binary_search(View const& view, typename View::value_type value)
{
  return binary_search(view, value, stapl::less<typename View::value_type>());
}

//////////////////////////////////////////////////////////////////////
/// @brief Finds the position of the first element for which the functor returns
///   false, indicating the partition point.
/// @param pview One-dimensional view of the input, which must be partitioned.
/// @param predicate Unary functor used to partition the input.
/// @return A reference to the first element for which the functor returns
///   false.
/// @ingroup searchAlgorithms
///
/// This algorithm is non-mutating, but requires that the input view be
/// partitioned.
//////////////////////////////////////////////////////////////////////
template <typename View, typename Pred>
typename View::reference
partition_point(View const& pview, Pred predicate)
{
  return find_if_not(pview, predicate);
};


namespace algo_details {

//////////////////////////////////////////////////////////////////////
/// @brief Convert boolean return value of partition predicate passed to
///   @ref is_partitioned() to int, so that it is in the tribool form
///   expected by the @ref is_partitioned_reduce reduction operator
///   of the @ref stapl::map_reduce.
//////////////////////////////////////////////////////////////////////
template<typename WF>
class convert_to_int
{
private:
  WF m_wf;

public:
  convert_to_int(WF const& wf)
    : m_wf(wf)
  { }

  typedef int result_type;

  template<typename Reference>
  int operator()(Reference elem) const
  {
    return m_wf(elem);
  }

  void define_type(typer& t)
  {
    t.member(m_wf);
  }
}; // class convert_to_int


//////////////////////////////////////////////////////////////////////
/// @brief Work function used in @ref is_partitioned() to determine whether the
///   input is partitioned.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct is_partitioned_reduce
{
  typedef int result_type;

  template<typename Reference1, typename Reference2>
  int operator()(Reference1 elem1, Reference2 elem2) const
  {
    if (elem1 == 1 && elem2 == 1)
      return 1;

    if (elem1 == 1 && elem2 == 0)
      return 0;

    if (elem1 == 0 && elem2 == 0)
      return 0;

    if (elem1 == 0 && elem2 == 1)
      return 2;

    if (elem1 == 2 || elem2 == 2)
      return 2;

    // else
    return 2;
  }
}; // struct is_partitioned_reduce

} // namespace algo_details


//////////////////////////////////////////////////////////////////////
/// @brief Decides if the input view is partitioned according to the given
///   functor, in that all elements which return true precede all those that do
///   not.
/// @param pview One-dimensional view of the input.
/// @param predicate Unary functor used to check the partitioning.
/// @return True if the input is partitioned, false otherwise.
/// @ingroup summaryAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template <typename View, typename Predicate>
bool
is_partitioned(View const& pview, Predicate predicate)
{
  const int result = map_reduce(
    algo_details::convert_to_int<Predicate>(predicate),
    algo_details::is_partitioned_reduce<int>(),
    pview
  );

  if (result == 0 || result == 1)
    return true;

  // else
 return false;
}


//////////////////////////////////////////////////////////////////////
/// @brief Finds the first element in the input view which compares greater than
///   or equal to the given value.
/// @param view One-dimensional view of the input (which is sorted).
/// @param value Value to search for in the input (of same type as inputs).
/// @param comp Binary functor which implements the less than operation.
/// @return A reference to the first value in the input which is greater than
///   or equal to the given value.
/// @ingroup binarysearchAlgorithms
///
/// This algorithm is non-mutating, and requires the input view to be sorted.
//////////////////////////////////////////////////////////////////////
template<typename View, typename T, typename StrictWeakOrdering>
typename View::reference
lower_bound(View const& view, T const& value, StrictWeakOrdering comp)
{
    typedef typename View::value_type value_t;
    typedef algo_details::lower_bound_map_wf<value_t, StrictWeakOrdering>
              map_wf_t;

    return stapl::map_reduce(map_wf_t(value, comp),
                      algo_details::upper_lower_bound_reduce_wf(),
                      stapl::native_view(view));
}


//////////////////////////////////////////////////////////////////////
/// @brief Finds the first element in the input view which compares greater than
///   or equal to the given value.
/// @param view One-dimensional view of the input (which is sorted).
/// @param value Value to search for in the input (of same type as inputs).
/// @return A reference to the first value in the input which is greater than
///   or equal to the given value.
/// @ingroup binarysearchAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::less.
//////////////////////////////////////////////////////////////////////
template<typename View, typename T>
typename View::reference
lower_bound(View const& view, T const& value)
{
  return lower_bound(view, value, stapl::less<typename View::value_type>());
}


//////////////////////////////////////////////////////////////////////
/// @brief Finds the first element in the input view which compares greater than
///   the given value.
/// @param view One-dimensional view of the input (which is sorted).
/// @param value Value to search for in the input (of same type as inputs).
/// @param comp Binary functor which implements the less than operation.
/// @return A reference to the first value in the input which is greater than
///   the given value.
/// @ingroup binarysearchAlgorithms
///
/// This algorithm is non-mutating, and requires the input view to be sorted.
//////////////////////////////////////////////////////////////////////
template<typename View, typename T, typename StrictWeakOrdering>
typename View::reference
upper_bound(View const& view, T const& value, StrictWeakOrdering comp)
{
  typedef typename View::value_type value_t;
  typedef algo_details::upper_bound_map_wf<value_t, StrictWeakOrdering >
            map_wf_t;

  // the map_reduce call return type is native_view::reference::reference.
  // There's an assumption that this is convertible to View::reference,
  // which is reasonable, just saying...
  return stapl::map_reduce(map_wf_t(value, comp),
                    algo_details::upper_lower_bound_reduce_wf(),
                    stapl::native_view(view));
}


//////////////////////////////////////////////////////////////////////
/// @brief Finds the first element in the input view which compares greater than
///   the given value.
/// @param view One-dimensional view of the input (which is sorted).
/// @param value Value to search for in the input (of same type as inputs).
/// @return A reference to the first value in the input which is greater than
///   the given value.
/// @ingroup binarysearchAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::less.
//////////////////////////////////////////////////////////////////////
template<typename View, typename T>
typename View::reference
upper_bound(View const& view, T const& value)
{
  return upper_bound(view, value, stapl::less<typename View::value_type>());
}


//////////////////////////////////////////////////////////////////////
/// @brief Computes the range of elements which are equal to the given value.
/// @param view One-dimensional view of input (which is sorted).
/// @param value Value to search for in the input (of same type as inputs).
/// @param comp Functor which implements the less than operation.
/// @return A view over the elements which compare equal to the given value.
/// @ingroup binarysearchAlgorithms
///
/// This algorithm is non-mutating, and requires that the input view be sorted.
//////////////////////////////////////////////////////////////////////
template <typename View, typename StrictWeakOrdering>
View
equal_range(View const& view, typename View::value_type const& value,
            StrictWeakOrdering const& comp)
{
    typedef typename View::domain_type dom_t;
    typedef typename View::value_type value_t;
    typedef algo_details::equal_range_map_wf<value_t, StrictWeakOrdering, dom_t>
              map_wf_t;
    typedef algo_details::equal_range_reduce_wf<dom_t> red_wf_t;

    dom_t res =  map_reduce(map_wf_t(value, comp),
                            red_wf_t(),
                            view);

    return View(view.container(), res, view.mapfunc());
}


//////////////////////////////////////////////////////////////////////
/// @brief Computes the range of elements which are equal to the given value.
/// @param view One-dimensional view of input (which is sorted).
/// @param value Value to search for in the input (of same type as inputs).
/// @return A view over the elements which compare equal to the given value.
/// @ingroup binarysearchAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::less.
//////////////////////////////////////////////////////////////////////
template < typename View >
View
equal_range(View const& view, typename View::value_type const& value)
{
    return equal_range(view, value, stapl::less<typename View::value_type>());
}


//////////////////////////////////////////////////////////////////////
/// @brief Given two input views, returns the positions of the first elements
///   which do not match.
/// @param view1 One-dimensional view of input.
/// @param view2 One-dimensional view of input.
/// @param pred Functor which implements the equal operation.
/// @return A pair containing references to the first mismatched element in each
///   input view, or NULL if the views are equal.
/// @ingroup searchAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View1, typename View2, typename Predicate>
std::pair<typename View1::reference, typename View2::reference>
mismatch(View1 const& view1, View2 const& view2, Predicate pred)
{
  typedef typename View1::reference ref1;
  typedef typename View2::reference ref2;

  ref1 result = stapl::map_reduce(algo_details::mismatch_map<Predicate>(pred),
                                  algo_details::mismatch_reduce(), view1,
                                  view2);

  if (is_null_reference(result))
  {
    return std::make_pair(ref1(null_reference()), ref2(null_reference()));
  }
  else
  {
    return std::make_pair(result, view2[index_of(result)]);
  }
}

//////////////////////////////////////////////////////////////////////
/// @brief Given two input views, returns the positions of the first elements
///   which do not match.
/// @param view1 One-dimensional view of input.
/// @param view2 One-dimensional view of input.
/// @return A pair containing references to the first mismatched element in each
///   input view, or NULL if the views are equal.
/// @ingroup searchAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::equal_to.
//////////////////////////////////////////////////////////////////////
template<typename View1, typename View2>
std::pair<typename View1::reference, typename View2::reference>
mismatch(View1 const& view1, View2 const& view2)
{
  typedef typename View1::reference ref1;

  return mismatch(view1, view2, equal_to<ref1>());
}

//////////////////////////////////////////////////////////////////////
/// @brief Computes the number of elements in the input view for which the
///   given functor returns true.
/// @param view One-dimensional view over the input elements.
/// @param pred Functor which is used to test the elements for counting.
/// @return The number of elements for which the functor returned true.
/// @ingroup countingAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Predicate>
typename View::size_type
count_if(View const& view, Predicate pred)
{
  typedef typename View::size_type result_t;

  return stapl::map_reduce(pred, plus<result_t>(), view);
}


//////////////////////////////////////////////////////////////////////
/// @brief Computes the number of elements in the input view which compare
///   equal to the given value.
/// @param view One-dimensional view over the input elements.
/// @param value Value to count the occurrences of in the input.
/// @return The number of occurrences of the given element in the input.
/// @ingroup countingAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View, typename T>
typename View::size_type
count(View const& view, T const& value)
{
  // FIXME - track down why std::bind2nd seems to be seeping into
  // this namespace (icc)
  //
  return count_if(view, stapl::bind2nd(equal_to<T>(), value));
}

//////////////////////////////////////////////////////////////////////
/// @brief Finds the last occurrence of the given pattern in the input sequence.
/// @param sequence A one-dimensional view of the input.
/// @param pattern The pattern to search for in the input.
/// @param predicate Functor which implements the equal operation.
/// @return A reference to the first element of the last occurrence of the
///   pattern in the input, or null if the pattern is not found.
/// @ingroup searchAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View1, typename View2, typename Pred>
typename View1::reference
find_end(const View1 &sequence,const View2 &pattern, Pred const& predicate)
{

  return stapl::map_reduce(algo_details::find_end_map<Pred>(predicate),
                    algo_details::find_end_reduce(),
                    stapl::make_overlap_view(sequence,1,0,pattern.size()-1),
                    make_repeat_view(pattern));
}

//////////////////////////////////////////////////////////////////////
/// @brief Finds the last occurrence of the given pattern in the input sequence.
/// @param sequence A one-dimensional view of the input.
/// @param pattern The pattern to search for in the input.
/// @return A reference to the first element of the last occurrence of the
///   pattern in the input, or null if the pattern is not found.
/// @ingroup searchAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::equal_to.
//////////////////////////////////////////////////////////////////////
template<typename View1, typename View2>
typename View1::reference find_end(const View1 &sequence,const View2 &pattern)
{
  return find_end(sequence, pattern, equal_to<typename View1::reference>());
}


namespace algo_details {
//////////////////////////////////////////////////////////////////////
/// @brief Computes the next lexicographical permutation of the input view
///   according to the given predicate.
/// @param vw One-dimensional input view.
/// @param pred Functor which implements an ordering operation.
///
/// This function is used to implement the @ref prev_permutation() and
/// @ref next_permutation() operations.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Predicate>
bool lexicographical_permutation(View& vw, Predicate pred)
{

  typedef typename View::reference ref;
  typedef algo_details::adj_find_map<Predicate> map_t;
  typedef algo_details::find_end_reduce reduce_t;

  typedef typename View::domain_type dom_t;

  typedef static_array<int>  cont_t;
  typedef array_view<cont_t> view_t;

  if (vw.size() <= 1)
   {
    return false;
   }

  //"adjacent_find_last" :
  ref head_last = stapl::map_reduce(map_t(pred), reduce_t(),
                                    stapl::make_overlap_view(vw, 1, 0, 1));

  if (is_null_reference(head_last))
   {
    return false;
   }
  size_t tail_first = index_of(head_last);
  tail_first++;

  // view domain from tail_first to end
  View tailvw(vw.container(), dom_t(tail_first, vw.size() - 1));
  // reverse it
  // find_if > head_last

  cont_t c(tailvw.size());
  view_t rev_tail_copy(c);

  copy(reverse_view(tailvw), rev_tail_copy);

  ref ref_to_swap = find_if(rev_tail_copy, bind1st(pred, head_last));

  //swap
  if (get_location_id() == 0)
   {
    //std::swap(head_last,ref_to_swap);// <-- ERROR : Don't work
    typename View::value_type temp_val1 = ref_to_swap;
    ref_to_swap=head_last;
    head_last=temp_val1;

   }
  //copy again the tail :
  copy(rev_tail_copy, tailvw);

  return true;
}
}


//////////////////////////////////////////////////////////////////////
/// @brief Computes the next lexicographic ordering of the input view
///   (where the highest is sorted in decreasing order), or if input is already
///   in highest order, places it in the lowest permutation (increasing order).
/// @param vw One-dimensional input view.
/// @param pred Functor which implements the less-than operation.
/// @return True if the next permutation was computed, false if the input
///   was placed into the lowest permutation.
/// @ingroup permutingAlgorithms
//////////////////////////////////////////////////////////////////////
template<typename View, typename Predicate>
bool next_permutation(View& vw, Predicate pred)
{
  return algo_details::lexicographical_permutation(vw, pred);
}

//////////////////////////////////////////////////////////////////////
/// @brief Computes the next lexicographic ordering of the input view
///   (where the highest is sorted in decreasing order), or if input is already
///   in highest order, places it in the lowest permutation (increasing order).
/// @param vw One-dimensional input view.
/// @return True if the next permutation was computed, false if the input
///   was placed into the lowest permutation.
/// @ingroup permutingAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::less.
//////////////////////////////////////////////////////////////////////
template<typename View>
bool next_permutation(View& vw)
{
  return next_permutation(vw, stapl::less<typename View::value_type >());
}

//////////////////////////////////////////////////////////////////////
/// @brief Computes the previous lexicographic ordering of the input view
///   (where the lowest is sorted in increasing order), or if input is already
///   in lowest order, places it in the highest permutation (decreasing order).
/// @param vw One-dimensional input view.
/// @param pred Functor which implements the greater-than operation.
/// @return True if the previous permutation was computed, false if the input
///   was placed into the highest permutation.
/// @ingroup permutingAlgorithms
//////////////////////////////////////////////////////////////////////
template<typename View, typename Predicate>
bool prev_permutation(View& vw, Predicate pred)
{
  return algo_details::lexicographical_permutation(vw, pred);
}

//////////////////////////////////////////////////////////////////////
/// @brief Computes the previous lexicographic ordering of the input view
///   (where the lowest is sorted in increasing order), or if input is already
///   in lowest order, places it in the highest permutation (decreasing order).
/// @param vw One-dimensional input view.
/// @return True if the previous permutation was computed, false if the input
///   was placed into the highest permutation.
/// @ingroup permutingAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::greater.
//////////////////////////////////////////////////////////////////////
template<typename View>
bool prev_permutation(View& vw)
{
  return prev_permutation(vw, stapl::greater<typename View::value_type >());
}


//////////////////////////////////////////////////////////////////////
/// @brief Computes whether all the elements in the first view are contained in
///   the second view, even in a different order.
/// @param view1 One-dimensional input view.
/// @param view2 One-dimensional input view.
/// @param pred Functor which implements the equal operation.
/// @return True if the second view is a permutation of the first, else false.
/// @ingroup summaryAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View1, typename View2, typename Pred>
bool is_permutation(View1 const& view1, View2 const& view2, Pred pred)
{
  typedef typename View1::value_type                  value_type;
  typedef std::map<value_type, int>                   map_type;
  typedef static_array<map_type>                      sample_container_type;
  typedef array_view<sample_container_type>           sample_view_type;
  typedef stapl::hash<value_type>                     hash_type;

  sample_container_type samples1(get_num_locations());
  sample_container_type samples2(get_num_locations());
  sample_view_type samples_view1(samples1);
  sample_view_type samples_view2(samples2);

  std::pair<value_type,value_type> p1 =
    minmax_value(transform_view<View1, hash_type>(view1, hash_type()));
  std::pair<value_type,value_type> p2 =
    minmax_value(transform_view<View2, hash_type>(view2, hash_type()));

  if ((p1.first!=p2.first) || (p1.second!=p2.second))
    return false;

  int step = ((p1.second - p1.first) / get_num_locations()) + 1;

  map_func(algo_details::histogram<value_type>(&samples1,p1.first,step),view1);
  map_func(algo_details::histogram<value_type>(&samples2,p1.first,step),view2);

  return stapl::equal(samples_view1,samples_view2);
}


//////////////////////////////////////////////////////////////////////
/// @brief Computes whether all the elements in the first view are contained in
///   the second view, even in a different order.
/// @param view1 One-dimensional input view.
/// @param view2 One-dimensional input view.
/// @return True if the second view is a permutation of the first, else false.
/// @ingroup summaryAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::equal_to.
//////////////////////////////////////////////////////////////////////
template<typename View1, typename View2>
bool is_permutation(View1 const& view1, View2 const& view2)
{
  return is_permutation(view1, view2,
                        stapl::equal_to<typename View1::value_type>());
}


//////////////////////////////////////////////////////////////////////
/// @brief Compares the two input views and returns true if all of their
///   elements compare pairwise equal.
/// @param view0 One-dimensional input view.
/// @param view1 One-dimensional input view.
/// @param pred Binary functor which implements equality.
/// @return True if the elements in the views are equal, false otherwise.
/// @ingroup summaryAlgorithms
///
/// This algorithm is non-mutating.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename View1, typename Predicate>
bool equal(View0 const& view0, View1 const& view1, Predicate pred)
{
  return stapl::map_reduce(pred, logical_and<bool>(), view0, view1);
}

//////////////////////////////////////////////////////////////////////
/// @brief Compares the two input views and returns true if all of their
///   elements compare pairwise equal.
/// @param view0 One-dimensional input view.
/// @param view1 One-dimensional input view.
/// @ingroup summaryAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::equal_to.
//////////////////////////////////////////////////////////////////////
template<typename View0, typename View1>
bool equal(View0 const& view0, View1 const& view1)
{
  typedef typename View0::value_type value_t;
  return stapl::map_reduce(equal_to<value_t>(), logical_and<bool>(),
                           view0, view1);
}


//////////////////////////////////////////////////////////////////////
/// @brief Computes a random shuffle of elements in the input view.
///
/// @param view One-dimensional view of the input.
/// @param rng The random number generator to use.
/// @ingroup reorderingAlgorithms
///
/// This algorithm is mutating.
/////////////////////////////////////////////////////////////////////
template <typename View, typename RandomNumberGenerator>
void random_shuffle(View const& view, RandomNumberGenerator const& rng)
{
  typedef static_array<typename View::value_type>   tmp_container_t;
  typedef array_view<tmp_container_t>               tmp_view_t;
  typedef algo_details::get_input_view<
    View, typename View::index_type>                input_view_t;
  typedef typename stapl::result_of::native_view<
    typename input_view_t::view_type>::type         partitioner_t;

  if (view.size()==0) return;

  // Temporary result
  tmp_view_t result_view(new tmp_container_t(view.size()));

  partitioner_t pview = stapl::native_view(input_view_t()(view));
  stapl::map_func(algo_details::shuffle_wf<RandomNumberGenerator>(
                    pview.size(),rng), pview, make_repeat_view(result_view));
  copy(result_view,view);
}

//////////////////////////////////////////////////////////////////////
/// @brief Computes a random shuffle of elements in the input view
/// @param view One-dimensional view of the input.
/// @ingroup reorderingAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::algo_details::default_random_number_generator.
/// This algorithm is mutating.
//////////////////////////////////////////////////////////////////////
template <typename View>
void random_shuffle(View const& view)
{
  typedef typename View::const_iterator::difference_type difference_type;
  random_shuffle(view,
    algo_details::default_random_number_generator<difference_type>(
      view.get_location_id()));
}


//////////////////////////////////////////////////////////////////////
/// @brief Computes a random shuffle of elements in the input view, using the
///   given uniform random number generator.
/// @param view One-dimensional view of the input.
/// @param rng The random number generator to use.
/// @ingroup reorderingAlgorithms
///
/// This algorithm is mutating.
//////////////////////////////////////////////////////////////////////
template <typename View, typename UniformRandomNumberGenerator>
void shuffle(const View& view, const UniformRandomNumberGenerator& rng)
{
  algo_details::shuffle_random_number_generator<UniformRandomNumberGenerator>
    srg(rng);
  random_shuffle(view, srg);
}


//////////////////////////////////////////////////////////////////////
/// @brief Copies all of the elements from the source to the destination view,
///   except those that are consecutive duplicates (equal to the preceding
///   element), which are moved to the end of the destination view.
/// @param src_view One-dimensional view of the input elements.
/// @param dest_view One-dimensional view of the output container.
/// @param bin_predicate Binary functor which implements equality.
/// @return std::pair containing a view over the unique elements, and a view
///   over the remaining elements.
/// @ingroup removingAlgorithms
///
/// This algorithm is non-mutating to the input, and requires a view with
/// iterator support. Supplied predicate should be commutative. The
/// destination container will be updated.
//////////////////////////////////////////////////////////////////////
template <typename View, typename DestView, typename BinPredicate>
std::pair<DestView, DestView>
unique_copy( View& src_view, DestView& dest_view, BinPredicate bin_predicate )
{
  typedef static_array<algo_details::partition_pred_counter>  cnt_container_t;
  typedef array_view<cnt_container_t>                         cnt_view_t;
  typedef typename cnt_view_t::domain_type                    cnt_dom_t;

  typedef DestView                                            dest_view_t;
  typedef typename dest_view_t::domain_type                   dest_dom_t;
  typedef dest_dom_t                                          dom_t;

  unsigned int num_elements = src_view.size();
  cnt_container_t count_container(num_elements);
  cnt_view_t count_view(count_container);
  cnt_view_t count_view2(count_container,
    cnt_dom_t(count_view.domain().advance(count_view.domain().first(),1),
            count_view.domain().last(),count_view.domain()));

  dest_view_t dest_view2(dest_view.container(),
    dest_dom_t(dest_view.domain().advance(dest_view.domain().first(),1),
             dest_view.domain().last(),dest_view.domain()));

  count_view[0].set(true);
  *dest_view.begin() = *src_view.begin();

  map_func(algo_details::unique_count_matches<BinPredicate>(bin_predicate),
           stapl::make_overlap_view(src_view, 1, 1, 0), count_view2);
  algo_details::partition_pred_counter num_satisfy_elements =
    accumulate(count_view, algo_details::partition_pred_counter());
  partial_sum(count_view, count_view, false);
  map_func(algo_details::unique_fill_temp<BinPredicate>(
                            bin_predicate,num_satisfy_elements.m_is_match),
           stapl::make_overlap_view(src_view, 1, 1, 0),
           count_view2,
           make_repeat_view(dest_view2) );

  DestView v1(dest_view.container(),dom_t(dest_view.domain().first(),
    dest_view.domain().advance(dest_view.domain().first(),
             num_satisfy_elements.m_is_match-1),
              dest_view.domain()), dest_view.mapfunc() );
  DestView v2(dest_view.container(),
    dom_t(dest_view.domain().advance(dest_view.domain().first(),
          num_satisfy_elements.m_is_match),
          dest_view.domain().last(),dest_view.domain()),
    dest_view.mapfunc());
  return std::make_pair(v1,v2);
}


//////////////////////////////////////////////////////////////////////
/// @brief Copies all of the elements from the source to the destination view,
///   except those that are consecutive duplicates (equal to the preceding
///   element), which are moved to the end of the destination view.
/// @param src_view One-dimensional view of the input elements.
/// @param dest_view One-dimensional view of the output container.
/// @return std::pair containing a view over the unique elements, and a view
///   over the remaining elements.
/// @ingroup removingAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::equal_to.
//////////////////////////////////////////////////////////////////////
template <typename View0, typename View1>
std::pair<View1, View1>
unique_copy( View0 src_view, View1 dest_view)
{
  return unique_copy(src_view, dest_view,
          stapl::equal_to<typename View0::value_type>());
}


//////////////////////////////////////////////////////////////////////
/// @brief Remove all duplicate elements from the given view.
/// @param view One-dimensional view of the input elements.
/// @param bin_predicate Binary functor which implements the equal operation.
/// @return std::pair containing a view over the unique elements, and a view
///   over the remaining elements.
/// @ingroup removingAlgorithms
///
/// This algorithm is mutating, so the input view must be Read-Write and support
/// iterators. The underlying container is also modified.
//////////////////////////////////////////////////////////////////////
template <typename View, typename BinPredicate>
std::pair<View, View>
unique( View view, BinPredicate bin_predicate )
{
  typedef static_array<typename View::value_type>  tmp_container_t;
  typedef array_view<tmp_container_t>               tmp_view_t;

  typedef typename View::domain_type               dom_t;

  tmp_container_t temp_container(view.size());
  tmp_view_t temp_view(temp_container);

  std::pair<tmp_view_t, tmp_view_t> ret =
    unique_copy(view,temp_view,bin_predicate);
  copy(temp_view, view);
  dom_t dom1(view.domain().first(),
       view.domain().advance(view.domain().first(),ret.first.size()-1),
       view.domain());
  View v1(view.container(),dom1, view.mapfunc());
  dom_t dom2(view.domain().advance(view.domain().first(), ret.first.size()),
       view.domain().last(),
       view.domain());
  View v2(view.container(),dom2, view.mapfunc());

  return std::make_pair(v1,v2);
}


//////////////////////////////////////////////////////////////////////
/// @brief Remove all duplicate elements from the given view.
/// @param view One-dimensional view of the input elements.
/// @return std::pair containing a view over the unique elements, and a view
///   over the remaining elements.
/// @ingroup removingAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::equal_to.
//////////////////////////////////////////////////////////////////////
template <typename View>
std::pair<View, View>
unique( View view)
{
  return unique(view, stapl::equal_to<typename View::value_type>());
}


//////////////////////////////////////////////////////////////////////
/// @brief Return the position of the first adjacent pair of equal elements.
/// @param view One-dimensional view of the input elements.
/// @param bin_predicate Binary functor which implements the equal operation.
/// @return A reference to the first element of the pair, or NULL if none exist.
/// @ingroup searchAlgorithms
///
/// This algorithm is non-mutating, and requires iterator support and read-only.
//////////////////////////////////////////////////////////////////////
template <typename View , typename BinPredicate>
typename View::reference adjacent_find( View view, BinPredicate bin_predicate)
{
  typedef typename View::reference         ret_type;
  typedef algo_details::adj_find_map<BinPredicate>        map_t;
  typedef algo_details::find_reduce        reduce_t;

  ret_type result =  stapl::map_reduce(map_t(bin_predicate), reduce_t(),
                                       stapl::make_overlap_view(view,1,0,1));
  return result;
}


//////////////////////////////////////////////////////////////////////
/// @brief Return the position of the first adjacent pair of equal elements.
/// @param view One-dimensional view of the input elements.
/// @return A reference to the first element of the pair, or NULL if none exist.
/// @ingroup searchAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::equal_to.
//////////////////////////////////////////////////////////////////////
//Default predicate is stapl::equal
template <typename View>
typename View::reference adjacent_find( View view )
{
  return adjacent_find(view, stapl::equal_to<typename View::value_type>());
}


//////////////////////////////////////////////////////////////////////
/// @brief Return the position of the first occurrence of the given sequence
///   within the input, or NULL if it is not found.
/// @param v1 One-dimensional view of the input elements.
/// @param v2 One-dimensional view of the search sequence.
/// @param pred Functor which implements the equality operation.
/// @return A reference to the first element of the pair, or NULL if none exist.
/// @ingroup searchAlgorithms
///
/// This algorithm is non-mutating, and requires a view with iterator support.
//////////////////////////////////////////////////////////////////////
template <class View1,class View2,class Predicate>
typename View1::reference
search(View1 v1, View2 v2,Predicate pred)
{
  typedef typename algo_details::search_map_wf<Predicate> s_map;
  typedef typename View1::reference vtype;
  if (v2.size()==0 || v2.size()>v1.size())
    return vtype(null_reference());
  return stapl::map_reduce(s_map(pred),algo_details::find_reduce(),
                           stapl::make_overlap_view(v1,1,0,(v2.size()-1)),
                           make_repeat_view(v2));
}


//////////////////////////////////////////////////////////////////////
/// @brief Return the position of the first occurrence of the given sequence
///   within the input, or NULL if it is not found.
/// @param v1 One-dimensional view of the input elements.
/// @param v2 One-dimensional view of the search sequence.
/// @return A reference to the first element of the pair, or NULL if none exist.
/// @ingroup searchAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::equal_to.
//////////////////////////////////////////////////////////////////////
template <class View1,class View2>
typename View1::reference
search(View1 v1, View2 v2)
{
  return search(v1,v2,stapl::equal_to<typename View1::value_type>());
}


//////////////////////////////////////////////////////////////////////
/// @brief Return the position of the first occurrence of a sequence of the
///   given value which is of the given length, or NULL if none exists.
/// @param v1 One-dimensional view of input elements.
/// @param count Length of sequence of repeated elements to search.
/// @param value Value which is repeated in the searched sequence.
/// @param pred Functor which implements the equality operation.
/// @return A reference to the first element of the repeated sequence in the
///   input, or NULL if none exists.
/// @ingroup searchAlgorithms
///
/// This algorithm is non-mutating, and requires a view with iterator support.
//////////////////////////////////////////////////////////////////////
template <class View1,class Predicate>
typename View1::reference
search_n(View1 v1, size_t count,typename View1::value_type value,Predicate pred)
{
  typedef typename algo_details::search_n_map_wf<
    typename View1::value_type,Predicate>         s_map;
  typedef typename View1::reference retype;
  if (v1.size()==0)
    return retype(null_reference());
  return stapl::map_reduce(s_map(value,pred),algo_details::find_reduce(),
                           stapl::make_overlap_view(v1,1,0,(count-1)));
}


//////////////////////////////////////////////////////////////////////
/// @brief Return the position of the first occurrence of a sequence of the
///   given value which is of the given length, or NULL if none exists.
/// @param v1 One-dimensional view of input elements.
/// @param count Length of sequence of repeated elements to search.
/// @param value Value which is repeated in the searched sequence.
/// @return A reference to the first element of the repeated sequence in the
///   input, or NULL if none exists.
/// @ingroup searchAlgorithms
///
/// This version calls the predicated version with a default predicate of
/// stapl::equal_to.
//////////////////////////////////////////////////////////////////////
template <class View1>
typename View1::reference
search_n(View1 v1, size_t count,typename View1::value_type value)
{
  return search_n(v1,count,value,stapl::equal_to<typename View1::value_type>());
}


namespace functional {

//////////////////////////////////////////////////////////////////////
/// @brief Functor which computes in parallel whether the two input views
///   are equal.
//////////////////////////////////////////////////////////////////////
struct equal
{
  typedef bool result_type;

  template<typename View0, typename View1>
  bool
  operator()(View0 const& view0, View1 const& view1) const
  {
    typedef typename View0::value_type value_t;
    return stapl::map_reduce(equal_to<value_t>(), logical_and<bool>(),
                      view0, view1);
  }

  template<typename View0, typename View1, typename Predicate>
  bool
  operator()(View0 const& view0, View1 const& view1, Predicate pred) const
  {
    return stapl::map_reduce(pred, logical_and<bool>(), view0, view1);
  }
}; // struct equal

//////////////////////////////////////////////////////////////////////
/// @brief Functor which computes in parallel the minimum value present in the
///   input view.
/// @tparam Value The type of the minimum value.
//////////////////////////////////////////////////////////////////////
template<typename Value>
struct min_value
{
  typedef Value result_type;

  template<typename View>
  result_type
  operator()(View const& view) const
  {
    return stapl::map_reduce(identity<Value>(), min<Value>(), view);
  }

  template<typename View, typename Compare>
  result_type
  operator()(View const& view, Compare comp) const
  {
    return stapl::map_reduce(identity<Value>(), min_comp<Value, Compare>(comp),
                             view);
  }
};  //struct min_value

} // namespace functional


} //namespace stapl

#endif

