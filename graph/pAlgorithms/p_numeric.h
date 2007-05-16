#ifndef P_ALGO_NUMERIC_H
#define P_ALGO_NUMERIC_H

/* functions defined in this file
 * -Generalized Numeric Algorithms <p_numeric.h>
 *    T p_accumulate(PRange1& pr1, T init)
 *    T p_accumulate(PRange1& pr1, T init, const BinaryFunc bf)
 *    T p_inner_product(PRange1& pr1, PRange2& pr2, T init)
 *    T p_inner_product(PRange1& pr1, PRange2& pr2, T init, 
 *                      BinaryFunc1 bf1, BinaryFunc2 bf2)
 *    PRange2 p_partial_sum(PRange1& pr1, PRange2& pr2)
 *    PRange2 p_partial_sum(PRange1& pr1, PRange2& pr2, BinaryFunc bf)
 *    PRange2 p_adjacent_difference(PRange1& pr1, PRange2& pr2)
 *    PRange2 p_adjacent_difference(PRange1& pr1, PRange2& pr2, BinaryFunc bf)
 */

#include <algorithm>
#include <list>

#include "p_algobase.h"
#include "stapl_num.h"

using std::list;
/**
 * @ingroup accumulate
 * pAccumulate
 */
template <class PRange>
typename iterator_traits<typename PRange::iteratorType>::value_type 
p_accumulate(PRange& pr1, 
	     typename iterator_traits<typename PRange::iteratorType>::value_type init = 0) {
  list<PRange*> subranges;

  get_subranges_no_deps(pr1, subranges);

  typename list<PRange*>::iterator iter = subranges.begin();
  stapl_num<typename iterator_traits<typename PRange::iteratorType>::value_type> sum;
		sum = 0;
  for ( ; iter != subranges.end(); ++iter) {
    sum = sum.value() + accumulate((**iter).get_boundary().start(),
                                   (**iter).get_boundary().finish(), init);
  }
  typename iterator_traits<typename PRange::iteratorType>::value_type result = sum.reduce();

  return result;
}

/*
template <class PRange, class BinaryFunc>
typename iterator_traits<typename PRange::iteratorType>::value_type 
p_accumulate(PRange& input_pr, BinaryFunc& func) {
  list<PRange*> subranges;

  get_subranges_no_deps(input_pr, subranges);

  typename list<PRange*>::iterator iter = subranges.begin();
  stapl_num<typename iterator_traits<typename PRange::iteratorType>::value_type> sum;
  for ( ; iter != subranges.end(); ++iter) {
    sum = func(sum, accumulate((**iter).get_boundary().start(),
                      (**iter).get_boundary().finish(), func));
  }
  typename iterator_traits<typename PRange::iteratorType>::value_type result = sum.reduce();

  return result;
}
*/
#endif
