/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_ALGORITHMS_SORTING_HPP
#define STAPL_ALGORITHMS_SORTING_HPP

#include <vector>
#include <set>
#include <algorithm>
#include <functional>
#include <utility>
#include <numeric>
#include <sstream>

#include "functional.hpp"
#include <stapl/runtime.hpp>

#include <stapl/paragraph/paragraph.hpp>
#include <stapl/utility/do_once.hpp>
#include <stapl/utility/tuple.hpp>

#include <stapl/containers/array/array.hpp>
#include <stapl/containers/array/static_array.hpp>

#include <stapl/algorithms/algorithm.hpp>
#include <stapl/algorithms/numeric.hpp>
#include <stapl/algorithms/generator.hpp>
#include <stapl/algorithms/functional.hpp>

#include <stapl/views/strided_view.hpp>
#include <stapl/views/segmented_view.hpp>
#include <stapl/views/array_view.hpp>
#include <stapl/views/metadata/coarseners/null.hpp>

#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/skeletons/executors/algorithm_executor.hpp>
#include <stapl/skeletons/functional/butterfly.hpp>
#include <stapl/skeletons/functional/reduce_to_pow_two.hpp>
#include <stapl/skeletons/functional/expand_from_pow_two.hpp>
#include <stapl/skeletons/functional/sink.hpp>
#include <stapl/skeletons/spans.hpp>

#include <stapl/containers/partitions/splitter.hpp>


namespace stapl {

// @todo find a better spot or method for these (tags?)
enum {EVEN, SEMIRANDOM, RANDOM, BLOCK};


// forward declarations
template<typename View, typename T>
typename View::size_type count(View const&, T const&);


template<typename View, typename Compare>
typename View::value_type max_value(View const&, Compare);


template<typename View0, typename View1>
void copy(View0 const&, View1 const&);


template<typename InputView, typename SplittersView, typename Compare,
         typename Functor>
segmented_view<InputView,
                 splitter_partition<typename InputView::domain_type > >
n_partition(InputView, SplittersView, Compare, Functor);


template<typename View, typename Generator>
void generate(View const&, Generator);


namespace algo_details {

//////////////////////////////////////////////////////////////////////
/// @brief Compare wrapper used by n_partition to wrap the defined
///        comparator and use a std::pair to preserve the initial
///        position of the elements.
/// @tparam Compare the defined comparator for sorting.
//////////////////////////////////////////////////////////////////////
template <typename Compare>
struct comparator_wrapper
{
private:
  Compare m_comp;

public:
  comparator_wrapper(Compare comp)
    : m_comp(comp)
  { }

  template <typename LRef, typename RRef>
  bool operator () (LRef lhs, RRef rhs)
  {
    return m_comp(lhs.first, rhs.first) == m_comp(rhs.first, lhs.first)
              ? lhs.second < rhs.second
              : m_comp(lhs.first, rhs.first);
  }

  void define_type(typer& t)
  {
    t.member(m_comp);
  }

};

//////////////////////////////////////////////////////////////////////
/// @brief Work function that sorts elements, used by sample_sort() to sort each
///   partition of elements.
/// @tparam Compare Comparator used for sorting.
//////////////////////////////////////////////////////////////////////
template <typename Compare>
class serial_sort_wf
{
private:
  Compare m_compare;

public:
  typedef void result_type;

  explicit
  serial_sort_wf(Compare cmp)
    : m_compare(cmp)
  {}

  template<typename View0>
  void operator()(View0 view0) const
  {
    std::sort(view0.begin(), view0.end(), m_compare);
  }

  void define_type(typer& t)
  {
    t.member(m_compare);
  }
}; //class serial_sort_wf


//////////////////////////////////////////////////////////////////////
/// @brief Work function used to collect a set of samples from an input view.
///   Sampling methods : 0:even, 1: semi-random, 2: random, 3: block
//////////////////////////////////////////////////////////////////////
template <typename T>
class sample_wf
{
  int m_over_sampling_ratio;
  int m_num_components;
  int m_sampling_method;

public:
  typedef std::vector<T> result_type;

  sample_wf(int over_sampling, int num_comp, int sm)
    : m_over_sampling_ratio(over_sampling),
      m_num_components(num_comp),
      m_sampling_method(sm)
  {}

  template<typename View0>
  result_type operator()(View0 const& data) const
  {
    typedef typename View0::iterator data_iter;

    data_iter data_it  = data.begin();
    data_iter data_beg = data.begin();
    data_iter data_end = data.end();

    result_type result;

    size_t data_size = data.size();

    if (data_size == 0)
      return result;

    //Option 0: Evenly Spaced
    if (m_sampling_method == EVEN) {
      int step = (data_size-1)  / m_over_sampling_ratio;
      if (step==0)
        step=1;

      for (int i = 0; (i < m_over_sampling_ratio) && (data_it != data_end); ++i)
      {
        result.push_back(*data_it);
        data_it += step;
      }
      return result;
    }

    stapl_assert(0, "sampling method selected not tested "
                    "(hint chose evenly spaced :)");
    //Option 1: Semi-random step, semi-evenly spaced
    if (m_sampling_method == SEMIRANDOM) {
      typedef std::uniform_int_distribution<int> rng_dist_t;
      std::random_device rd;
      std::mt19937 gen(rd());

      int step = (data_size-1)/m_over_sampling_ratio;
      int rand_step = step;
      if (step == 0) {
        step = 1;
        rand_step = 1;
      }
      for (int i = 0; (i < m_over_sampling_ratio) && (data_it!=data_end); i++) {
        result.push_back(*data_it);
        data_it+=step;
        //Keep the step from going out of bounds
        step = rng_dist_t(1, rand_step - 1)(gen);
      }
      return result;
    }

    if (m_sampling_method == RANDOM) {
      //Option 2: random, not evenly spaced
      //set used to prevent duplicate indexes from being chosen
      typedef std::set<data_iter>             index_set;
      typedef typename index_set::iterator    iter;
      typedef std::uniform_int_distribution<int> rng_dist_t;
      std::random_device rd;
      std::mt19937 gen(rd());

      int step;// = rand() % data_size;

      index_set indexes;
      std::pair<iter, bool>  already_there;

      for (int i = 0; (i < m_over_sampling_ratio)
                      && (indexes.size()!=data_size); i++) {
        do {
          step = rng_dist_t(0, data_size - 1)(gen);
          data_it = data_beg;
          data_it += step;
          already_there = indexes.insert(data_it);
        } while (!already_there.second);
        result.push_back(*data_it);
      }
      return result;
    }

    if (m_sampling_method == BLOCK) {
      //Option 3: first block of size over_sampling_ratio
      //data elements
      for (int i = 0; (i <m_over_sampling_ratio) && (data_it!=data_end); i++) {
        result.push_back(*data_it);
        data_it++;
      }
      return result;
    }
    return result;
  }

  void define_type(typer& t)
  {
    t.member(m_over_sampling_ratio);
    t.member(m_num_components);
    t.member(m_sampling_method);
  }

}; //class sample_wf


//////////////////////////////////////////////////////////////////////
/// @brief Work function which does nothing, used by n_partition() to bypass
///   processing of the partitions.
//////////////////////////////////////////////////////////////////////
struct neutral_functor
{
  typedef void result_type;

  template<typename Bucket, typename Offset>
  result_type operator()(Bucket b, Offset o)
  {}
};


//////////////////////////////////////////////////////////////////////
/// @brief Work function which processes sample_sort() partitions.
/// @tparam Compare Comparator used for sorting.
//////////////////////////////////////////////////////////////////////
template<typename Compare>
struct sample_sort_pf
{
  typedef void result_type;

  Compare m_comp;

  sample_sort_pf(Compare c)
    : m_comp(c)
  {}

  template<typename Bucket, typename Offset>
  result_type operator()(Bucket b, Offset o)
  {
    std::sort(b.begin(), b.end(), m_comp);
  }

  void define_type(typer& t)
  {
    t.member(m_comp);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work function which processes nth_element() partitions.
/// @tparam Compare Comparator used for sorting.
//////////////////////////////////////////////////////////////////////
template<typename Compare>
struct nth_element_pf
{
  typedef void result_type;

  Compare m_comp;
  const std::size_t m_nth;

  nth_element_pf(Compare c, const std::size_t o = 0)
    : m_comp(c), m_nth(o)
  {}

  template<typename Bucket, typename Offset>
  result_type operator()(Bucket b, Offset o)
  {
    if (o + b.size() > m_nth && m_nth >= o)
      std::nth_element(b.begin(), b.begin() + (m_nth - o), b.end(), m_comp);
  }

  void define_type(typer& t)
  {
    t.member(m_comp);
    t.member(m_nth);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work function which processes partial_sort() partitions.
/// @tparam Compare Comparator used for sorting.
//////////////////////////////////////////////////////////////////////
template<typename Compare>
struct partial_sort_pf
{
  typedef void result_type;

  Compare m_comp;
  const std::size_t m_nth;

  partial_sort_pf(Compare c, const std::size_t o = 0)
    : m_comp(c), m_nth(o)
  {}

  template<typename Bucket, typename Offset>
  result_type operator()(Bucket b, Offset o)
  {
    if (o <= m_nth)
      std::sort(b.begin(), b.end(), m_comp);
  }

  void define_type(typer& t)
  {
    t.member(m_comp);
    t.member(m_nth);
  }
};

template<typename T>
struct reduce_vector_wf
{
  typedef std::vector<T> result_type;
  template<typename Element1, typename Element2>
  result_type operator()(Element1 const& elt1, Element2 const& elt2)
  {
    result_type result = elt1;
    std::copy(elt2.begin(), elt2.end(), std::back_inserter(result));
    return result;
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Implementation of the partial_sort() algorithm.
/// @param data              Reference to the input view to be sorted.
/// @param comp              Comparator used for sorting.
/// @param sample_method     Enumerated integer for selecting the sample method.
/// @param over_partitioning Number of buckets per location.
/// @param over_sampling     Number of samples taken per bucket.
/// @todo Replace the sequential sort of the samples call with another
/// sorting algorithm (p_merge_sort?) once one is implemented.
/// @note a sequential vector is used to collect the samples, resulting in
/// its replication across locations.
//////////////////////////////////////////////////////////////////////
template <typename View, typename Compare>
void sample_sort_impl(View& data, Compare comp,
                      size_t sample_method = EVEN,
                      size_t over_partitioning = 1,
                      size_t over_sampling = 128)
{
  stapl_assert(over_partitioning == 1,
               "sample sorting over_partitioning > 1 has not been tested\n");

  typedef typename View::value_type   value_t;

  if (over_sampling < over_partitioning)
    over_sampling = over_partitioning;

  const std::size_t num_buckets = get_num_locations();
  const size_t num_op_buckets = num_buckets * over_partitioning;

  if (num_buckets == 1 || data.size() < num_buckets) {
    do_once(std::bind(serial_sort_wf<Compare>(comp), data));
    return;
  }

  //Find Maximum Value in Data
  //Below When Selecting Splitters, Do not consider the maximum
  //value as a potential splitter if possible
  value_t maximum = max_value(data, comp);

  //Allocate Sample Container (Fill with Maximum Value of Data) and
  //Instantiate Sample View

  //Collect Samples
  sample_wf<value_t> sampler(over_sampling, num_buckets, sample_method);
  std::vector<value_t>
  samples_vect = map_reduce(sampler, reduce_vector_wf<value_t>(),
                            balance_view(data, num_buckets));

  //Sort Samples
  std::sort(samples_vect.begin(), samples_vect.end(), comp);

  //Find Splitters
  size_t splitter_size = num_op_buckets - 1;

  //  Filter Max Values
  size_t num_real_samples =
    samples_vect.size() - std::count(samples_vect.begin(), samples_vect.end(),
                                     maximum);
  if (num_real_samples < splitter_size)
    num_real_samples =  splitter_size;

  //  Calculate Step Size
  size_t step = num_real_samples / splitter_size;

  //Resize samples
  samples_vect.resize(num_real_samples);

  // Copy every stepth element into splitters
  std::vector<value_t> splitters;
  for (size_t index = 0; index < num_real_samples; index += step)
  {
    splitters.push_back(samples_vect[index]);
  }

  // Create partitions and sort each partition regarding to the given functor
  sample_sort_pf<comparator_wrapper<Compare> >
       pf((comparator_wrapper<Compare>(comp)));
  n_partition(data, splitters, comp, pf);
}


//////////////////////////////////////////////////////////////////////
/// @brief Copy the input data into the output container, starting at the given
///   position
//////////////////////////////////////////////////////////////////////
struct copy_back_wf
{
  typedef void result_type;

  template<typename InputRef, typename OffsetRef, typename OutputView>
  result_type operator()(InputRef in, OffsetRef pos, OutputView out)
  {
    std::size_t i = pos;
    for (typename InputRef::iterator it = in.begin(); it != in.end(); ++it, ++i)
      out.set_element(i, *it);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Copy split input back into the output container used by n_partition.
///        It differs from copy_back_wf in that this functor works with
///        the data that has the partition id added to each element.
//////////////////////////////////////////////////////////////////////
struct copy_to_input_wf
{
  template<typename InputRef, typename OffsetRef, typename OutputView>
  void operator () (InputRef in, OffsetRef pos, OutputView out)
  {
    std::size_t i = pos;
    for (typename InputRef::iterator it = in.begin();
         it != in.end();
         ++it, ++i)
    {
      out.set_element(i, (*it).first);
    }
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Compute buckets sizes after splitting data.
//////////////////////////////////////////////////////////////////////
struct buckets_sizes_wf
{
  typedef void result_type;

  template<typename InputRef, typename OutputRef>
  result_type operator()(InputRef in, OutputRef out)
  {
    out = in.size();
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Wrap a type in a std::vector<>.
/// @tparam T Value type to be wrapped.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct to_vector_wf
{
  typedef std::vector<T> result_type;

  template<typename Ref>
  result_type operator()(Ref in)
  {
    return result_type(1, in);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Concatenate two vectors.
/// @tparam T Value type of the containers.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct concat_vector_wf
{
  typedef std::vector<T> result_type;

  template<typename Ref1, typename Ref2>
  result_type operator()(Ref1 const& lhs, Ref2 const& rhs)
  {
    result_type dest(lhs);
    std::copy(rhs.begin(), rhs.end(), std::back_inserter(dest));
    return dest;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Fill local buckets with input data, used by n_partition().
/// @tparam Compare       Comparator used for sorting.
/// @tparam SplittersView Vector of splitters used to separate data.
//////////////////////////////////////////////////////////////////////
template<typename Compare, typename SplittersView>
struct fill_local_buckets
{
  typedef void result_type;

private:
  Compare m_comp;
  SplittersView m_splitters;

public:
  fill_local_buckets(Compare c, SplittersView s)
    : m_comp(c), m_splitters(s)
  {}

  template<typename DataView, typename BkRef>
  result_type operator()(DataView view, BkRef vref)
  {

    std::vector<std::vector<std::pair<typename DataView::value_type,
                                      unsigned long> > >
      tmp(m_splitters.size() + 1);

    unsigned long item_idx = 0;
    unsigned long offset = stapl::index_of(*view.begin());

    for (typename DataView::iterator el_it = view.begin(); el_it != view.end();
                                       ++el_it) {
      typename SplittersView::iterator
        spl_it = std::lower_bound(m_splitters.begin(), m_splitters.end(),
                                  *el_it, m_comp);
      std::size_t index = std::distance(m_splitters.begin(), spl_it);
      tmp[index].push_back(std::make_pair(*el_it, (offset + item_idx++)));
    }
    vref = tmp;
  }

  void define_type(typer& t)
  {
    t.member(m_comp);
    t.member(m_splitters);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Reduce two buckets in one bucket, used by n_partition().
/// @tparam T Inner data container.
///
/// @todo Making a copy - el2_copy - to avoid crashes due to the member_iterator
///   assuming the data it refers to is local.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct bucket_reduce_wf
{
  typedef std::vector<std::vector<std::pair<T, unsigned long > > >
          result_type;

  //////////////////////////////////////////////////////////////////////
  /// @todo Making a copy - el2_copy - to avoid crashes due to the
  ///   member_iterator assuming the data it refers to is local.
  //////////////////////////////////////////////////////////////////////
  template <typename Element1, typename Element2>
  result_type operator()(Element1 const& el1, Element2 const& el2) const
  {
    result_type result(el1);
    result_type el2_copy(el2);
    std::size_t i = 0;
    for (typename result_type::const_iterator el2_it  = el2_copy.begin();
                                              el2_it != el2_copy.end();
                                              ++el2_it, ++i)
      std::copy((*el2_it).begin(), (*el2_it).end(),
                std::back_inserter(result[i]));

    return result;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Split one bucket in two buckets, used by n_partition().
/// @tparam T Inner data container.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct split_bucket_wf
{
  typedef std::vector<std::vector< std::pair<T, unsigned long > > >
          result_type;

private:
  bool m_i;

public:
  split_bucket_wf(void)
    : m_i(false)
  { }

  void set_position(std::size_t index, bool)
  {
    m_i = (index % 2 != 0);
  }

  template<typename Element>
  result_type operator()(Element const& el) const
  {
    std::size_t start = (!m_i) ? 0 : ceil(el.size() / 2.);
    std::size_t sz = (!m_i) ? ceil(el.size() / 2.) : floor(el.size() / 2.);

    result_type result(sz);
    result_type el_copy(el);
    std::size_t i = 0;
    for (typename result_type::const_iterator it = el_copy.begin() + start;
                                     i != sz;
                                     ++it, ++i)
        std::copy((*it).begin(), (*it).end(), std::back_inserter(result[i]));

    return result;
  }

  void define_type(typer& t)
  {
    t.member(m_i);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work function given to the butterfly skeleton to merge the
/// partitioned data from different locations, used by n_partition().
/// @tparam T Inner data container.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct bucket_merge_wf
{
  typedef std::vector<std::vector<std::pair<T, unsigned long > > >
          result_type;

private:
  std::size_t m_butterfly_size;
  std::size_t m_index1;
  std::size_t m_index2;
  std::size_t m_nb_pairs;
  std::size_t m_nb_locs;

public:
  bucket_merge_wf(int const& locs)
    : m_butterfly_size(0), m_index1(0), m_index2(0), m_nb_locs(locs)
  {
    m_nb_pairs = locs - pow(2, floor(log(locs) / log(2)));
  }

  void set_position(std::size_t butterfly_size, std::size_t index1,
                    std::size_t index2, std::size_t /* ignored */)
  {
    m_butterfly_size = butterfly_size;
    m_index1 = index1;
    m_index2 = index2;
  }

  template <typename Element>
  result_type operator()(Element el1, Element el2) const
  {
    // # of pairs = limit offset between pairs and identities
    std::size_t L = m_nb_pairs;
    // number of btf at the current level of the graph
    std::size_t nbBtfNodes = m_butterfly_size * 2;
    // btf number
    std::size_t currentBtfNb = std::floor(m_index1 / nbBtfNodes);
    // btf median's offset
    std::size_t M = m_butterfly_size + currentBtfNb * nbBtfNodes;
    // left bound of the current btf
    std::size_t leftBound = M - nbBtfNodes / 2;
    // number of elements (nodes) on the left side of the current btf
    std::size_t N1 = ( L <= M )
                      ? ( ( leftBound < L )
                        ? 2 * ( L - leftBound ) + M - L
                        : M - leftBound )
                      : 2 * ( M - leftBound );
    // ... and on the right side
    std::size_t N2 = ( L <= M )
                      ? M - leftBound
                      : ( L >= M + ( M - leftBound ) )
                        ? 2 * ( M - leftBound )
                        : 2 * ( L - M ) + ( M + ( M - leftBound ) - L );

    std::size_t R = el1.size() - ( N1 + N2 ) * floor(el1.size() / ( N1 + N2) );
    std::size_t lim = floor(el1.size() / (N1 + N2)) * N1 + ((R >= N1) ? N1 : R);
    std::size_t start = (m_index1 < M) ? 0 : lim;
    std::size_t end = (m_index1 < M) ? lim : el1.size();

    // the result size is the number of buckets elements
    result_type result(end - start);

    for (std::size_t elmt = start; elmt != end ; ++elmt) {
      // extract the left input buckets
      result[elmt - start] = el2[elmt];
      // extract the right input buckets
      std::vector< std::pair< T, unsigned long > > el1e_copy(el1[elmt]);
      std::copy(el1e_copy.begin(), el1e_copy.end(),
        std::back_inserter(result[elmt - start]));
    }
    return result;
  }

  void define_type(typer& t)
  {
    t.member(m_butterfly_size);
    t.member(m_index1);
    t.member(m_index2);
    t.member(m_nb_pairs);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Implementation of the n_partition() algorithm.
/// @param input_v            The input view to be partitioned.
/// @param splitters_v        The splitters used to determine an element's
///   partition.
/// @param comp               The comparator used to compare elements to
///   splitters.
/// @param partition_functor  Work function which processes partitions.
///
/// @todo combine factory calls in the following lines into one skeleton
/// @todo The @ref map_reduce() should automatically handle views that have
///       less elements than the number of locations.
//////////////////////////////////////////////////////////////////////
template<typename InputView, typename SplittersView, typename Compare,
         typename Functor>
segmented_view<InputView,
                 splitter_partition<typename InputView::domain_type> >
n_partition_impl(InputView input_v, SplittersView splitters_v, Compare comp,
                 Functor partition_functor)
{
  // 1) compute split input
  // 1.a) local buckets
  typedef typename InputView::value_type           value_t;
  typedef std::vector<std::vector<std::pair<value_t, unsigned long> > >
          local_bk_value;
  typedef static_array<local_bk_value>             local_bk_t;
  typedef array_view<local_bk_t>                   local_bk_vt;

  const std::size_t nat_input_sz = stapl::native_view(input_v).size();

  local_bk_t local_bk_in(nat_input_sz);
  local_bk_vt local_bk_in_v(local_bk_in);

  map_func(fill_local_buckets<Compare, SplittersView>(comp, splitters_v),
           stapl::native_view(input_v), local_bk_in_v);

  // 1.b) use of reduce, butterfly merge & broadcast on our local_buckets
  // to split input more efficiently

  local_bk_t local_bk_out(nat_input_sz);
  local_bk_vt local_bk_out_v(local_bk_out);

  if (nat_input_sz > 1) {
    using namespace skeletons;
    //reduce + butterfly + broadcast
    typedef spans::nearest_pow_2<spans::balanced<>> bfly_span_t;
    algorithm_executor<null_coarsener>().execute(
      skeletons::sink<local_bk_value>(
        skeletons::compose(
          skeletons::reduce_to_pow_two(bucket_reduce_wf<value_t>()),
          skeletons::butterfly<true, use_default, bfly_span_t>
            (bucket_merge_wf<value_t>(local_bk_in_v.size())),
          skeletons::expand_from_pow_two<use_default,true>(
              split_bucket_wf<value_t>())
        )
      ),
      local_bk_in_v,
      local_bk_out_v);
  } else {
    copy(local_bk_in_v, local_bk_out_v);
  }

  // 1.c) reduce local buckets in split_input
  typedef static_array<std::size_t>                 num_local_bk_t;
  typedef array_view<num_local_bk_t>                num_local_bk_vt;
  typedef static_array<std::vector<std::pair<value_t, unsigned long > > >
          split_input_t;
  typedef array_view<split_input_t>                 split_input_vt;

  num_local_bk_t num_local_bk(nat_input_sz);
  num_local_bk_vt num_local_bk_v(num_local_bk);

  map_func(buckets_sizes_wf(), local_bk_out_v, num_local_bk_v);

  const std::size_t counted_num_buckets = accumulate(num_local_bk_v, 0);
  partial_sum(num_local_bk_v, num_local_bk_v, true);

  split_input_t split_input(counted_num_buckets > get_num_locations() ?
                            counted_num_buckets : get_num_locations());
  split_input_vt split_input_v(split_input);

  map_func(copy_back_wf(), local_bk_out_v, num_local_bk_v,
           make_repeat_view(split_input_v));

  // 2) compute buckets sizes
  typedef static_array<std::size_t> s_array_t;
  typedef array_view<s_array_t>     s_array_vt;

  s_array_t buckets_sizes(split_input_v.size());
  s_array_vt buckets_sizes_view(buckets_sizes);
  map_func(buckets_sizes_wf(), split_input_v, buckets_sizes_view);

  // 3) compute global offsets, with shifting
  partial_sum(buckets_sizes_view, buckets_sizes_view, true);

  // 4) use the functor to process the elements in each partition
  map_func(partition_functor, split_input_v, buckets_sizes_view);

  // 5) copy_back split input into the input InputView
  map_func(copy_to_input_wf(), split_input_v, buckets_sizes_view,
           make_repeat_view(input_v));

  // 6) reduce all the buckets sizes into the domain offsets
  typedef std::vector<std::size_t> offsets_t;

  offsets_t domain_offsets;

  if (buckets_sizes_view.size() < get_num_locations()) {
    // Remove this code when the @todo about map_reduce() is resolved.
    domain_offsets = map_reduce<skeletons::tags::no_coarsening>(
                        to_vector_wf<std::size_t>(),
                        concat_vector_wf<std::size_t>(),
                        buckets_sizes_view);
  }
  else {
    domain_offsets = map_reduce(to_vector_wf<std::size_t>(),
                                concat_vector_wf<std::size_t>(),
                                buckets_sizes_view);
  }

  // 7) create the segmented_view, allowing empties
  typedef typename InputView::domain_type           domain_t;
  typedef splitter_partition<domain_t>              split_part_t;
  typedef segmented_view<InputView, split_part_t> split_t;

  domain_offsets.erase(domain_offsets.begin());
  split_part_t split_part(input_v.domain(), domain_offsets, true);
  split_t split_pv(input_v, split_part);

  return split_pv;

}


//////////////////////////////////////////////////////////////////////
/// @brief Implementation of the partial_sort() algorithms.
/// @param input_v The input view to be sorted.
/// @param comp    The comparator used for sorting.
/// @param f       Work function which processes partitions.
/// @todo Replace the sequential sort of the splitters call with another
/// sorting algorithm (p_merge_sort?) once one is implemented.
/// @note a sequential vector is used to collect the splitters, resulting in
/// its replication across locations.
///
/// It is also used by the the nth_element() algorithm.
//////////////////////////////////////////////////////////////////////
template<typename InputView, typename Compare, typename Functor>
void partial_sort_impl(InputView input_v, Compare comp, Functor f)
{
  typedef typename InputView::value_type value_type;
  typedef std::vector<value_type>        s_array_t;

  // pick unique random splitters from input_v
  const std::size_t nb_locations = get_num_locations();

  std::size_t num_splitters = nb_locations == 1 ? 1 : nb_locations - 1;

  sample_wf<value_type> sampler(1, nb_locations, EVEN);
  s_array_t splitters = map_reduce(sampler, reduce_vector_wf<value_type>(),
                                   balance_view(input_v, num_splitters));

  // sort the splitters
  std::sort(splitters.begin(), splitters.end(), comp);

  // delete duplicates
  splitters.resize(std::distance(splitters.begin(),
                                 std::unique(splitters.begin(),
                                             splitters.end())));

  n_partition(input_v, splitters, comp, f);

  return;
}


//////////////////////////////////////////////////////////////////////
/// @brief Count the occurrences of each radix in the input view.
//////////////////////////////////////////////////////////////////////
class radix_sort_count_wf
{
private:
  int m_r;
  int m_pass;
  int m_mask;
  int m_two_to_r;

public:
  typedef void result_type;

  radix_sort_count_wf(int r0, int p, int m, int twotor)
    : m_r(r0), m_pass(p), m_mask(m), m_two_to_r(twotor)
  {}

  template<typename DataView, typename CountRef>
  result_type operator()(DataView dataView, CountRef count_ref)
  {
    typename CountRef::value_type cv(m_two_to_r, 0);
    typename DataView::value_type value;

    for (typename DataView::iterator it = dataView.begin();
                                     it != dataView.end();
                                     ++it) {
      value = ((*it)>>(m_r * m_pass)) & m_mask;
      ++cv[value];
    }
    count_ref = cv;
  }

  void define_type(typer& t)
  {
    t.member(m_r);
    t.member(m_pass);
    t.member(m_mask);
    t.member(m_two_to_r);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Binary function to add two vectors, used by radix_sort().
/// @tparam T The integral type used to count occurrences of each radix.
//////////////////////////////////////////////////////////////////////
template<typename T>
class radix_sort_vectorplus
  : public stapl::ro_binary_function<T, T, T>
{
public:
  typedef T result_type;

  result_type operator()(T const& v1, T const& v2) const
  {
    if (v1.size() == 0)
      return v2;

    int len = v1.size();
    result_type v(len);
    typename T::iterator it0 = v.begin();

    for (typename T::const_iterator it1 = v1.begin(), it2 = v2.begin();
                                    it1 != v1.end() && it2 != v2.end();
                                    ++it1, ++it2, ++it0) {
      *it0 = *it1 + *it2;
    }

    return v;
  }
};

} // namespace algo_details


template<typename T>
struct identity_value<algo_details::radix_sort_vectorplus<T>, T>
{
  static T value(void) { return T(); }
};


namespace algo_details {

//////////////////////////////////////////////////////////////////////
/// @brief Used by radix_sort() to copy values to their final positions in
///   the destination view.
/// @tparam T The data container type.
//////////////////////////////////////////////////////////////////////
template<typename T>
class radix_sort_pos_wf
{
private:
  typedef typename T::value_type    countvector_t;

  countvector_t m_totalsumsp;
  int m_r;
  int m_pass;
  int m_mask;

public:
  typedef void result_type;

  radix_sort_pos_wf(countvector_t totalsums, int r, int p, int m)
    : m_totalsumsp(totalsums), m_r(r), m_pass(p), m_mask(m)
  {}

  template<typename DataView, typename Dest, typename CountView>
  result_type operator()(DataView dataNatView, Dest dest, CountView countView)
  {
    // update the counts
    countvector_t counts = countView;
    int len = m_totalsumsp.size();
    for (int j=0; j<len; ++j) {
      counts[j] = m_totalsumsp[j] + counts[j];
    }

    // place data
    typename DataView::value_type j;
    for (typename DataView::iterator it = dataNatView.begin();
                                     it != dataNatView.end();
                                     ++it) {
      j = ((*it)>>(m_r * m_pass)) & m_mask;
      dest.set_element(counts[j], *it);

      ++counts[j];
    }
  }

  void define_type(typer& t)
  {
    t.member(m_totalsumsp);
    t.member(m_r);
    t.member(m_pass);
    t.member(m_mask);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Reset the first vectors of counts, used in radix_sort().
/// @tparam T The data container type.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct reset_counts_head_wf
{
  typedef void result_type;

  std::size_t m_len;

  reset_counts_head_wf(std::size_t l)
    : m_len(l)
  {}

  template<typename View>
  result_type operator()(View v)
  {
      T t = *(v.begin());
      t.assign(m_len, 0);
      *(v.begin()) = t;
  }

  void define_type(typer& t)
  {
    t.member(m_len);
  }
};
} //namespace algo_details


//////////////////////////////////////////////////////////////////////
/// @brief Sorts the elements of the input view according to the comparator
///   provided using a sample-based approach.
/// @param[in,out] view                    The view of elements to sort.
/// @param[in]     comp                    Comparison function which returns
///   true if the first argument is less than the second.
/// @param[in]     sampling_method         Method used for sampling
///   (0: even, 1: semi-random, 2:random, 3: block)
/// @param[in]     over_partitioning_ratio Number of buckets per location.
/// @param[in]     over_sampling_ratio     Number of samples taken per bucket.
/// @ingroup sortAlgorithms
///
/// The order of equal elements is not guaranteed to be preserved.
/// The given comparison function comp is used to compare the elements.
/// The given method sampling_method is used to select the sampling process.
/// The given ratio over_partitioning_ratio is used to determine the number
///   of buckets per location.
/// The given ratio over_sampling_ratio is used to determine the number of
///   samples per bucket.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Compare>
inline
void sample_sort(View& view,
                 Compare comp,
                 size_t sampling_method,
                 size_t over_partitioning_ratio = 1,
                 size_t over_sampling_ratio = 128)
{
  using algo_details::sample_sort_impl;

  sample_sort_impl(view, comp,
                   sampling_method, over_partitioning_ratio,
                   over_sampling_ratio);
}


//////////////////////////////////////////////////////////////////////
/// @brief Sorts the elements of the input view according to the comparator
///   provided using a sample-based approach.
/// @param[in,out] view                    The view of elements to sort.
///   true if the first argument is less than the second.
/// @param[in]     sampling_method         Method used for sampling
///   (0: even, 1: semi-random, 2:random, 3: block)
/// @param[in]     over_partitioning_ratio Number of buckets per location.
/// @param[in]     over_sampling_ratio     Number of samples taken per bucket.
/// @ingroup sortAlgorithms
///
/// The order of equal elements is not guaranteed to be preserved.
/// The default '<' comparison function is used to compare the elements.
/// The given method sampling_method is used to select the sampling process.
/// The given ratio over_partitioning_ratio is used to determine the number
///   of buckets per location.
/// The given ratio over_sampling_ratio is used to determine the number of
///   samples per bucket.
//////////////////////////////////////////////////////////////////////
template<typename View>
inline
void sample_sort(View& view,
                 size_t sampling_method,
                 size_t over_partitioning_ratio = 1,
                 size_t over_sampling_ratio = 128)
{
  using algo_details::sample_sort_impl;

  sample_sort_impl(view, std::less<typename View::value_type>(),
                   sampling_method, over_partitioning_ratio,
                   over_sampling_ratio);
}


//////////////////////////////////////////////////////////////////////
/// @brief Sorts the elements of the input view according to the comparator
///   provided using a sample-based approach.
/// @param[in,out] view                    The view of elements to sort.
/// @param[in]     comp                    Comparison function which returns
///   true if the first argument is less than the second.
/// @ingroup sortAlgorithms
///
/// The order of equal elements is not guaranteed to be preserved.
/// The given comparison function comp is used to compare the elements.
/// The default 'EVEN' sampling method is used.
/// The default '1' over partitioning ratio is used to determine the number
///   of buckets per location.
/// The default '128' over sampling ratio is used to determine the number of
///   samples per bucket.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Compare>
inline
void sample_sort(View& view, Compare comp)
{
  using algo_details::sample_sort_impl;
  sample_sort_impl(view, comp, EVEN, 1, 128);
}


//////////////////////////////////////////////////////////////////////
/// @brief Sorts the elements of the input view according to the comparator
///   provided using a sample-based approach.
/// @param[in,out] view                    The view of elements to sort.
/// @ingroup sortAlgorithms
///
/// The order of equal elements is not guaranteed to be preserved.
/// The default '<' comparison function is used to compare the elements.
/// The default 'EVEN' sampling method is used.
/// The default '1' over partitioning ratio is used to determine the number
///   of buckets per location.
/// The default '128' over sampling ratio is used to determine the number of
///   samples per bucket.
//////////////////////////////////////////////////////////////////////
template<typename View>
inline
void sample_sort(View& view)
{
  using algo_details::sample_sort_impl;
  typedef typename View::value_type value_type;

  sample_sort_impl(view, less<value_type>(), EVEN, 1, 128);
}


//////////////////////////////////////////////////////////////////////
/// @brief Sorts the elements of the input view according to the comparator
///   provided.
/// @param[in,out] view The view to be sorted.
/// @param[in]     comp Comparison function which returns true if the first
///   argument is less than the second.
/// @ingroup sortAlgorithms
///
/// sample_sort() is used for sorting.
/// The order of equal elements is not guaranteed to be preserved.
/// The given comparison function pred is used to compare the elements.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Comparator>
void sort(View& view, Comparator comp)
{
  sample_sort(view, comp);
}


//////////////////////////////////////////////////////////////////////
/// @brief Sorts the elements of the input view according to the comparator
///   provided.
/// @param[in,out] view The view to be sorted.
/// @ingroup sortAlgorithms
///
/// sample_sort() is used for sorting.
/// The order of equal elements is not guaranteed to be preserved.
/// The '<' comparison function is used to compare the elements.
//////////////////////////////////////////////////////////////////////
template<typename View>
void sort(View& view)
{
  sample_sort(view);
}


//////////////////////////////////////////////////////////////////////
/// @brief Sorts the elements of the input view according to the comparator
///        provided.
///        The order of equal elements is guaranteed to be preserved.
/// @param[in,out] view The view to be sorted.
/// @param[in]     comp Comparison function which returns true if the first
///   argument is less than the second.
/// @ingroup sortAlgorithms
///
/// sample_sort() is used for sorting.
/// The given comparison function pred is used to compare the elements.
//////////////////////////////////////////////////////////////////////
template<typename View, typename Comparator>
void stable_sort(View& view, Comparator comp)
{
   sample_sort(view, comp);
}

//////////////////////////////////////////////////////////////////////
/// @brief Sorts the elements of the input view according to the '<'
///        comparison function of the elements.
///        The order of equal elements is guaranteed to be preserved.
/// @param[in,out] view The view to be sorted.
/// @ingroup sortAlgorithms
///
/// sample_sort() is used for sorting.
//////////////////////////////////////////////////////////////////////
template<typename View>
void stable_sort(View& view)
{
  sample_sort(view);
}


//////////////////////////////////////////////////////////////////////
/// @brief Reorders the elements in the input view in such a way that all
///   elements for which the comparator returns true for a splitter s
///   - within the input splitters set - precede the elements for which the
///   compare function returns false.
///   Each set of partitioned elements is processed by the given
///     partition_functor function.
///   The relative ordering of the elements is not preserved.
/// @param[in,out] input_v         The input view to be partitioned.
/// @param[in]     splitters       The set of splitters to partition the input.
/// @param[in]     comp            The strict weak ordering comparison functor.
/// @param[in]     partition_functor Functor processing each partition.
/// @return segmented_view A view over the segments of elements.
/// @ingroup sortAlgorithms
/// @note a sequential vector is used to collect the splitters, resulting in
/// its replication across locations.
///
/// The set of splitters must be sorted with the same comparator prior to be
/// given to the algorithm.
///
/// @todo The @ref map_reduce() should automatically handle views that have
///       less elements than the number of locations.
//////////////////////////////////////////////////////////////////////
template<typename InputView, typename SplittersView, typename Compare,
         typename Functor>
segmented_view<InputView,
                 splitter_partition<typename InputView::domain_type> >
n_partition(InputView input_v,
            SplittersView splitters,
            Compare comp,
            Functor partition_functor)
{
  typedef typename SplittersView::value_type splitters_value_t;

  // share splitters
  typedef std::vector<splitters_value_t> splitters_t;

  splitters_t shared_splitters;

  if (splitters.size() < get_num_locations()) {
    // Remove this code when the @todo about map_reduce() is resolved.
    shared_splitters = map_reduce<skeletons::tags::no_coarsening>(
                            algo_details::to_vector_wf<splitters_value_t>(),
                            algo_details::concat_vector_wf<splitters_value_t>(),
                            splitters);
  }
  else {
    shared_splitters = map_reduce(
                            algo_details::to_vector_wf<splitters_value_t>(),
                            algo_details::concat_vector_wf<splitters_value_t>(),
                            splitters);
  }

  return algo_details::n_partition_impl(input_v, shared_splitters, comp,
                          partition_functor);
}


//////////////////////////////////////////////////////////////////////
/// @brief Reorders the elements in the input view in such a way that all
///   elements for which the comparator returns true for a splitter s
///   - within the input splitters set - precede the elements for which the
///   compare function returns false.
///   The relative ordering of the elements is not preserved.
/// @param[in,out] input_v         The input view to be partitioned.
/// @param[in]     splitters       The set of splitters to partition the input.
/// @param[in]     comp            The strict weak ordering comparison functor.
/// @return segmented_view A view over the segments of elements.
/// @ingroup sortAlgorithms
///
/// The set of splitters must be sorted with the same comparator prior to be
/// given to the algorithm.
//////////////////////////////////////////////////////////////////////
template<typename InputView, typename SplittersView, typename Compare>
segmented_view<InputView,
                 splitter_partition<typename InputView::domain_type> >
n_partition(InputView input_v, SplittersView splitters, Compare comp)
{
  algo_details::neutral_functor f;
  return n_partition(input_v, splitters, comp, f);
}


//////////////////////////////////////////////////////////////////////
/// @brief Reorders the elements in the input view in such a way that all
///   elements for which the default '<' comparison function returns
///   true for a splitter s - within the input splitters set - precede the
///   elements for which the compare function returns false.
///   The relative ordering of the elements is not preserved.
/// @param[in,out] input_v         The input view to be partitioned.
/// @param[in]     splitters       The set of splitters to partition the input.
/// @return segmented_view A view over the segments of elements.
/// @ingroup sortAlgorithms
///
/// The set of splitters must be sorted with the '<' operator.
//////////////////////////////////////////////////////////////////////
template<typename InputView, typename SplittersView>
segmented_view<InputView,
                 splitter_partition<typename InputView::domain_type> >
n_partition(InputView input_v, SplittersView splitters)
{
  return n_partition(input_v, splitters,
                     less<typename InputView::value_type>());
}


//////////////////////////////////////////////////////////////////////
/// @brief Reorders the elements in the input view in such a way that all
///   elements for which the comparator returns true for a splitter s
///   - within the input splitters set - precede the elements for which the
///   compare function returns false.
///   Each set of partitioned elements is processed by the given
///     partition_functor function.
///   The relative ordering of the elements is not preserved.
/// @param[in,out] input_v          The input view to be partitioned.
/// @param[in]     splitters        The set of splitters to partition the input.
/// @param[in]     comp             The strict weak ordering comparison functor.
/// @param[in]     partition_functor Functor processing each partition.
/// @return segmented_view A view over the segments of elements.
/// @ingroup sortAlgorithms
///
/// The set of splitters is std::vector<InputView::value_type>.
/// The set of splitters must be sorted with the same comparator prior to be
/// given to the algorithm.
//////////////////////////////////////////////////////////////////////
template<typename InputView, typename Compare, typename Functor>
segmented_view<InputView,
                 splitter_partition<typename InputView::domain_type> >
n_partition(InputView input_v,
            std::vector<typename InputView::value_type> splitters,
            Compare comp,
            Functor partition_functor)
{
  return
    algo_details::n_partition_impl(input_v, splitters, comp, partition_functor);
}


//////////////////////////////////////////////////////////////////////
/// @brief Performs a partial sort of the data in the input view using the
///   comparator provided such that all elements before the nth position are
///   less than the elements that follow the nth element, and the value stored
///   in the nth element is the same as if the input view were completely
///   sorted.
/// @param[in,out]  input_v The input view to be sorted.
/// @param[in]      nth     The iterator defining the sort partition point.
/// @param[in]      comp    Comparison function which returns true if the first
///   argument is less than the second.
///
/// The nth iterator must be the same type as InputView::iterator.
//////////////////////////////////////////////////////////////////////
template<typename InputView, typename Compare>
void nth_element(InputView input_v,
                 typename InputView::iterator nth,
                 Compare comp)
{
  if (nth < input_v.begin() || input_v.end() < nth)
    return;

  algo_details::nth_element_pf<Compare> pf(comp, nth - input_v.begin());
  return algo_details::partial_sort_impl(input_v, comp, pf);
}


//////////////////////////////////////////////////////////////////////
/// @brief Performs a partial sort of the data in the input view using the
///   comparator provided such that all elements before the nth position are
///   less than the elements that follow the nth element, and the value stored
///   in the nth element is the same as if the input view were completely/
///   sorted.
/// @param[in,out]  input_v The input view to be sorted.
/// @param[in]      nth     The iterator defining the sort partition point.
/// @param[in]      comp    Comparison function which returns true if the first
///   argument is less than the second.
///
/// The nth iterator must be the same type as InputView::iterator.
//////////////////////////////////////////////////////////////////////
template<typename InputView>
void nth_element(InputView input_v,
                 typename InputView::iterator nth)
{
  return nth_element(input_v, nth, less<typename InputView::value_type>());
}


//////////////////////////////////////////////////////////////////////
/// @brief Performs a partial sort of the data in the input view using the
///   comparator provided such that all elements before the nth position are
///   sorted using the comparator.
/// @param[in,out] input_v The input view to be sorted.
/// @param[in]     nth     The iterator to sort up to.
/// @param[in]     comp    Comparison function which returns true if the first
///   argument is less than the second.
/// @ingroup sortAlgorithms
///
/// The nth iterator must be the same type as InputView::iterator.
/// The order of equal elements is not guaranteed to be preserved.
//////////////////////////////////////////////////////////////////////
template<typename InputView, typename Compare>
void partial_sort(InputView input_v,
                  typename InputView::iterator nth,
                  Compare comp)
{
  if (nth < input_v.begin() || input_v.end() < nth)
    return;

  algo_details::partial_sort_pf<Compare> pf(comp, nth - input_v.begin());
  return algo_details::partial_sort_impl(input_v, comp, pf);
}


//////////////////////////////////////////////////////////////////////
/// @brief Performs a partial sort of the data in the input view using the
///   comparator provided such that all elements before the nth position are
///   sorted using the comparator.
/// @param[in,out] input_v The input view to be sorted.
/// @param[in]     nth     The iterator to sort up to.
/// @ingroup sortAlgorithms
///
/// The nth iterator must be the same type as InputView::iterator.
/// The order of equal elements is not guaranteed to be preserved.
/// The default '<' comparison function is used to compare the elements.
//////////////////////////////////////////////////////////////////////
template<typename InputView>
void partial_sort(InputView input_v,
                  typename InputView::iterator nth)
{
  return partial_sort(input_v, nth, less<typename InputView::value_type>());
}


//////////////////////////////////////////////////////////////////////
/// @brief Performs a partial sort of the input view data into the output view
///   using the comparator provided such that all elements before the nth
///   position are sorted using the comparator.
/// @param[in]  input_v  The input view to read from.
/// @param[out] output_v The output view to write to.
/// @param[in]  comp     Comparison function which returns true if the first
///   argument is less than the second.
/// @return Returns a view with the sorted elements.
/// @ingroup sortAlgorithms
///
/// The order of equal elements is not guaranteed to be preserved.
//////////////////////////////////////////////////////////////////////
template<typename InputView, typename OutputView, typename Compare>
OutputView partial_sort_copy(InputView input_v,
                             OutputView output_v,
                             Compare comp)
{
  typedef static_array<typename InputView::value_type> s_array_t;
  typedef array_view<s_array_t>                        s_array_vt;

  std::size_t N = std::min<std::size_t>(input_v.size(), output_v.size());

  // tmp will contain the whole set of elements to be sorted
  s_array_t tmp(input_v.size());
  s_array_vt tmp_v(tmp);
  copy(input_v, tmp_v);

  partial_sort(tmp_v, tmp_v.begin() + N, comp);

  copy(s_array_vt(tmp_v.container(),
                  typename s_array_vt::domain_type(0, N - 1)),
       OutputView(output_v.container(),
                  typename OutputView::domain_type(0, N - 1)));
  return OutputView(output_v.container(),
                    typename OutputView::domain_type(0, N - 1));
}


//////////////////////////////////////////////////////////////////////
/// @brief Performs a partial sort of the input view data into the output view
///   using the comparator provided such that all elements before the nth
///   position are sorted using the comparator.
/// @param[in]  input_v  The input view to read from.
/// @param[out] output_v The output view to write to.
/// @return Returns a view with the sorted elements.
/// @ingroup sortAlgorithms
///
/// The order of equal elements is not guaranteed to be preserved.
/// The default '<' comparison function is used to compare the elements.
//////////////////////////////////////////////////////////////////////
template<typename InputView, typename OutputView>
OutputView partial_sort_copy(InputView input_v,
                             OutputView output_v)
{
  return partial_sort_copy(input_v, output_v,
                           less<typename InputView::value_type>());
}


//////////////////////////////////////////////////////////////////////
/// @brief Sorts the element in the input view according to the radix-sort
///   algorithm.
/// @param[in,out] input_v The input view to be sorted.
/// @ingroup sortAlgorithms
///
/// Only accepts integers as data type.
/// @todo combine factory calls in the following lines into one pattern
//////////////////////////////////////////////////////////////////////
template<typename InputView>
void radix_sort(InputView input_v)
{
  typedef typename InputView::value_type                value_t;
  typedef typename InputView::iterator::difference_type count_t;
  typedef typename std::vector<count_t>                 countvector_t;

  // 1) find maximal value => nb of needed passes
  value_t global_max = max_element(input_v);
  int range = static_cast<int>
    (log10(static_cast<double>(global_max))/log10(2.0) + 1);
  int passes = range/16 + 1;
  if (range % passes != 0) {
    range = (range/passes + 1) * passes;
  }

  std::size_t bits     = range;
  std::size_t n        = passes;
  std::size_t r        = bits/n;    //consider r bits at a time
  std::size_t two_to_r = (std::size_t) (pow((float) (2), r));
  std::size_t mask     = two_to_r - 1;

  // 2) create a copy to manipulate data, then we'll copy back the data
  // PURPOSE: switch between this container and the input at each pass
  typedef array<value_t>        ipcont_t;
  typedef array_view<ipcont_t>  iview_t;
  ipcont_t temp_pcont(input_v.size());
  iview_t temp_v(temp_pcont);
  copy(input_v, temp_v);

  // 3) store counts of values
  typedef array<countvector_t> cpcont_t;
  typedef array_view<cpcont_t> cview_t;

  // construct native view to allow count_wf to process a range
  typedef typename
    stapl::result_of::native_view<InputView>::type native_input_vt;
  native_input_vt native_input_view = stapl::native_view(input_v);
  typedef typename stapl::result_of::native_view<iview_t>::type native_temp_vt;
  native_temp_vt native_temp_view = stapl::native_view(temp_v);

  // create the array to hold count results from each range
  const std::size_t nat_data_sz = native_input_view.size();
  cpcont_t count(nat_data_sz);
  cview_t count_v(count);

  // 4) For the ith block of r bits do: (going from LSB to MSB)
  for (std::size_t i=0; i<n; ++i) {
    // 4.a) count the # element per bucket on each location
    algo_details::radix_sort_count_wf RSC_wf(r, i, mask, two_to_r);
    if (i % 2 == 0) { // even: input view
      map_func(RSC_wf, native_input_view, count_v);
    } else { // odd: temp view
      map_func(RSC_wf, native_temp_view, count_v);
    }

    // 4.b) compute the offsets at which eachrange of data on a location should
     // be copy to in the destination container
    algo_details::radix_sort_vectorplus<countvector_t> vp;
    countvector_t totalsums = accumulate(count_v, countvector_t(), vp);
    stapl::partial_sum(count_v, count_v, vp, true);

    // then reset the first vector with {0}
    do_once(
      std::bind(algo_details::reset_counts_head_wf<countvector_t>(two_to_r),
                count_v));

    // 4.c) compute the prefix sums of total counts for every value, then shift
    std::partial_sum(totalsums.begin(), totalsums.end(), totalsums.begin());
    totalsums.insert(totalsums.begin(), 0);
    totalsums.resize(totalsums.size() - 1);

    // 4.d) put values at the right positions
    if (i % 2 == 0) { //even: input view
      algo_details::radix_sort_pos_wf<cview_t> RSP_wf(totalsums, r, i, mask);
      map_func(RSP_wf, native_input_view, make_repeat_view(temp_v), count_v);
    } else { // odd: temp view
      algo_details::radix_sort_pos_wf<cview_t> RSP_wf(totalsums, r, i, mask);
      map_func(RSP_wf, native_temp_view, make_repeat_view(input_v), count_v);
    }
  } //end for loop

  // 5) copy back temp sorted data into the input if #passes is odd
  if (n % 2 == 1){
    copy(temp_v, input_v);
  }

  return;
}

} //namespace stapl

#endif
