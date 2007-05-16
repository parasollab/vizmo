#ifndef P_ALGO_DOUBLE_H
#define P_ALGO_DOUBLE_H

/* functions defined in this file
 * -Nonmodifying Sequence Operations <p_algorithm.h>
 *   BinaryFunc p_for_each(PRange1& pr1, PRange2& pr2, const BinaryFunc bf)
 *
 * -Modifying Sequence Operations <p_algorithm.h>
 *   void p_transform(PRange1& pr1, PRange2& pr2, const UnaryFunc uf)
 *   PRange2 p_swap_ranges(PRange1& pr1, PRange2& pr2)
 *   PRange2 p_replace_copy(PRange1& pr1, PRange2& pr2,
 *                          const T& old_value, const T& new_value)
 *   PRange2 p_replace_copy_if(PRange1& pr1, PRange2& pr2,
 *                             Predicate pred, const T& new_value)
 *   PRange2 p_rotate_copy(PRange1& pr1, typename Iterator1 middle, 
 *                         PRange2& res)
 */

#include <algorithm>
#include <list>

#include "p_algobase.h"

using std::transform;
using std::list;

/**
 * @ingroup transform
 * pTransform
 */
template <class PRange1, class PRange2, class UrnaryFunc>
void p_transform(PRange1& pr1, PRange2& pr2, const UrnaryFunc func) {
  list<typename PRange1::subrangeType*> input_subranges;
  list<typename PRange2::subrangeType*> output_subranges;

  get_subranges_no_deps(pr1, input_subranges);
  get_subranges_no_deps(pr2, output_subranges);

  typename list<typename PRange1::subrangeType*>::iterator input_iter = input_subranges.begin();
  typename list<typename PRange2::subrangeType*>::iterator output_iter = output_subranges.begin();

  for ( ; input_iter != input_subranges.end(); ++input_iter, ++output_iter) {
    transform((**input_iter).get_boundary().start(),
              (**input_iter).get_boundary().finish(),
              (**output_iter).get_boundary().start(), func);
  }
}


/**
 * @ingroup for_each
 * pForEach
 */
template <class PRange1, class PRange2, class BinaryFunc>
void p_for_each(PRange1& pr1, PRange2& pr2, BinaryFunc& func) {
  list<typename PRange1::subrangeType*> subranges1;
  list<typename PRange2::subrangeType*> subranges2;

  get_subranges_no_deps(pr1, subranges1);
  get_subranges_no_deps(pr2, subranges2);

  typename list<typename PRange1::subrangeType*>::iterator sr_it1 = subranges1.begin();
  typename list<typename PRange2::subrangeType*>::iterator sr_it2 = subranges2.begin();
  for ( ; sr_it1 != subranges1.end(); ++sr_it1, ++sr_it2) {
    typename PRange1::iteratorType el_it1 = (**sr_it1).get_boundary().start();
    typename PRange2::iteratorType el_it2 = (**sr_it2).get_boundary().start();
    for ( ; el_it1 != (**sr_it1).get_boundary().finish(); ++el_it1, ++el_it2) {
      func(*el_it1, *el_it2);
    }
  }
}
#endif
