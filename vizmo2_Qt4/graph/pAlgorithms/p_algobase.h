#ifndef P_ALGOBASE_H
#define P_ALGOBASE_H

/* Functions defined in this file
-Nonmodifying Sequence Operations
 pair<int thread_id, int offset> p_mismatch(PRange1& pr1, PRange2& pr2)
 pair<int thread_id, int offset> p_mismatch(PRange1& pr1, PRange2& pr2, BinaryPred bp)
 bool p_equal(PRange1& pr1, PRange2& pr2)
 bool p_equal(PRange1& pr1, PRange2& pr2, BinaryPred bp)

-Modifying Sequence Operations
 void p_copy(PRange1& pr1, PRange1& pr2)
 Iterator p_copy_backward(PRange1& pr1, Iterator res)
 PRange2 p_uninitialized_copy(PRange1& pr1, PRange2& pr2)
 void p_fill(PRange1& pr1, const T& v)
 PRange1 p_fill_n(PRange1& r, Size n, const T& value)
 void p_uninitialized_fill(PRange1& r, const T& x)
 PRange1 p_uninitialized_fill_n(PRange1& r, Size n, const T& x)
 bool p_lexicographical_compare(PRange1& pr1, PRange2& pr2)
 bool p_lexicographical_compare(PRange1& pr1, PRange2& pr2, BinaryPred bp)
*/

#include <algorithm>
#include <vector>
#include <list>
#include <utility>

namespace stapl {

using std::copy;
using std::equal;
using std::mismatch;
using std::vector;
using std::list;
using std::pair;
using std::fill;

template <class PRange1, class T>
void p_fill(PRange1& pr1, const T& value) {
  list<PRange1*> input_subranges;
  get_subranges_no_deps(pr1, input_subranges);

  typename list<PRange1*>::iterator input_iter = input_subranges.begin();

  for ( ; input_iter != input_subranges.end(); ++input_iter) {
    fill((**input_iter).get_boundary().start(), 
         (**input_iter).get_boundary().finish(), 
         value);
  }
}

/*
 * General function to extract a list of leaf subranges from a pRange.
 * This is useful when the algorithm doesn't use a DDG from the pRange.
 */
template <class PRange>
void get_subranges_no_deps(PRange& pr, list<typename PRange::subrangeType*>& subrange_list) {
  typedef typename PRange::subrangeType subrangeType;

  if (pr.get_num_subranges() != 0) {
    vector<subrangeType>& subranges = pr.get_subranges();
    for(typename vector<subrangeType>::iterator i  = subranges.begin();
                                          i != subranges.end(); 
                                        ++i) {
      subrange_list.push_back(&(*i));
    }
  }
}


/**
 * @ingroup copy
 * pCopy
 */
template <class PRange1, class PRange2>
void p_copy(PRange1& pr1, PRange2& pr2) {
  list<typename PRange1::subrangeType*> input_subranges;
  list<typename PRange2::subrangeType*> output_subranges;

  get_subranges_no_deps(pr1, input_subranges);
  get_subranges_no_deps(pr2, output_subranges);

  typename list<typename PRange1::subrangeType*>::iterator input_iter = input_subranges.begin();
  typename list<typename PRange2::subrangeType*>::iterator output_iter = output_subranges.begin();

  for ( ; input_iter != input_subranges.end(); ++input_iter, ++output_iter) {
    copy((**input_iter).get_boundary().start(), 
         (**input_iter).get_boundary().finish(), 
         (**output_iter).get_boundary().start());
  }
}



class stapl_bool {
private:
  bool val;
  stapl::rmiHandle handle;

  void And(bool* in, bool* inout) { *inout &= *in; }

public:

  stapl_bool() { handle = stapl::register_rmi_object(this); }
  ~stapl_bool() { stapl::unregister_rmi_object(handle); }

  inline bool value() { return val; }

  bool reduce() {
    bool result;
    stapl::reduce_rmi( &val, &result, handle, &stapl_bool::And, true);
    val = result;
    return val;
  }

  inline stapl_bool& operator= (const stapl_bool& x) {
    val = x.val;
    return *this;
  }

  inline stapl_bool& operator= (const bool& x) {
    val = x;
    return *this;
  }
};

/**
 * @ingroup equal     
 * pEqual
 */
template <class PRange1, class PRange2>
bool p_equal(PRange1& pr1, PRange2& pr2) {
  list<typename PRange1::subrangeType*> input_subranges;
  list<typename PRange2::subrangeType*> output_subranges;

  get_subranges_no_deps(pr1, input_subranges);
  get_subranges_no_deps(pr2, output_subranges);

  typename list<typename PRange1::subrangeType*>::iterator input_iter = input_subranges.begin();
  typename list<typename PRange2::subrangeType*>::iterator output_iter = output_subranges.begin();

  stapl_bool eq;
  eq = true;

  for ( ; input_iter != input_subranges.end(); ++input_iter, ++output_iter) {
    eq = eq.value() && equal((**input_iter).get_boundary().start(),
                (**input_iter).get_boundary().finish(),
                (**output_iter).get_boundary().start());
  }

  eq.reduce();
  return eq.value();
}



template<class T1, class T2>
class stapl_pair {
private:
  pair<T1, T2> val;
  stapl::rmiHandle handle;

  void min1(pair<T1, T2>* in, pair<T1, T2>* inout) { 
    if (in->first < inout->first) {
      inout->first = in->first;
      inout->second = inout->second;
    }
  }

  void min2(pair<T1, T2>* in, pair<T1, T2>* inout) { 
    if (in->second < inout->second) {
      inout->first = in->first;
      inout->second = inout->second;
    }
  }

  void max1(pair<T1, T2>* in, pair<T1, T2>* inout) { 
    if (in->first > inout->first) {
      inout->first = in->first;
      inout->second = inout->second;
    }
  }

  void max2(pair<T1, T2>* in, pair<T1, T2>* inout) { 
    if (in->second > inout->second) {
      inout->first = in->first;
      inout->second = inout->second;
    }
  }

public:

  stapl_pair() { handle = stapl::register_rmi_object(this); }
  ~stapl_pair() { stapl::unregister_rmi_object(handle); }

  inline pair<T1, T2>& value() { return val; }

  pair<T1, T2> min1() {
    pair<T1, T2> result;
    stapl::reduce_rmi( &val, &result, handle, &stapl_pair<T1, T2>::min1, true);
    val = result;
    return val;
  }

  pair<T1, T2> min2() {
    pair<T1, T2> result;
    stapl::reduce_rmi( &val, &result, handle, &stapl_pair<T1, T2>::min2, true);
    val = result;
    return val;
  }

  pair<T1, T2> max1() {
    pair<T1, T2> result;
    stapl::reduce_rmi( &val, &result, handle, &stapl_pair<T1, T2>::max1, true);
    val = result;
    return val;
  }

  pair<T1, T2> max2() {
    pair<T1, T2> result;
    stapl::reduce_rmi( &val, &result, handle, &stapl_pair<T1, T2>::max2, true);
    val = result;
    return val;
  }

  inline stapl_pair& operator= (const stapl_pair& x) {
    val = x.val;
    return *this;
  }

  inline stapl_pair& operator= (const pair<T1, T2>& x) {
    val = x;
    return *this;
  }
};

/**
 * @ingroup mismatch
 * pMismatch
 */
template <class PRange1, class PRange2>
pair<int, int> p_mismatch(PRange1& pr1, PRange2& pr2) {
  list<typename PRange1::subrangeType*> input_subranges;
  list<typename PRange2::subrangeType*> output_subranges;

  get_subranges_no_deps(pr1, input_subranges);
  get_subranges_no_deps(pr2, output_subranges);

  typename list<typename PRange1::subrangeType*>::iterator input_iter = input_subranges.begin();
  typename list<typename PRange2::subrangeType*>::iterator output_iter = output_subranges.begin();

  stapl_pair<int, int> result;
  result.value().first = get_num_threads();
  result.value().second = -1;
  bool mismatch_found = false;
  for ( ; input_iter != input_subranges.end(); ++input_iter, ++output_iter) {
    if (!mismatch_found) {
      pair<typename PRange1::iteratorType, typename PRange2::iteratorType> lres;
      lres = mismatch((**input_iter).get_boundary().start(),
                      (**input_iter).get_boundary().finish(),
                      (**output_iter).get_boundary().start());
      if (lres.first != (**input_iter).get_boundary().finish()) {
        mismatch_found = true;
        result.value().first = get_thread_id();
        result.value().second = distance((**input_iter).get_boundary().start(),
                                         lres.first);
      }
    }
  }

  return result.min1();
}

} // end namespace stapl
#endif
