#ifndef P_ALGO_TRIPLE_H
#define P_ALGO_TRIPLE_H

/* functions defined in this file
 * -Nonmodifying Sequence Operations <p_algorithm.h>
 *   TertiaryFunc p_for_each(PRange1& pr1, PRange2& pr2, PRange3& pr3, 
 *                           const TertiaryFunc& f)
 *
 *   -Modifying Sequence Operations <p_algorithm.h>
 *     void p_transform(PRange1& pr1, PRange2& pr2, PRange3& pr3, 
 *                      const BinaryFunc& f)
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
template <class PRange1, class PRange2, class PRange3, class BinaryFunc>
void p_transform(PRange1& pr1, PRange2& pr2, 
                 PRange3& pr3, const BinaryFunc func) {
  list<typename PRange1::subrangeType*> input1_subranges;
  list<typename PRange2::subrangeType*> input2_subranges;
  list<typename PRange3::subrangeType*> output_subranges;

  get_subranges_no_deps(pr1, input1_subranges);
  get_subranges_no_deps(pr2, input2_subranges);
  get_subranges_no_deps(pr3, output_subranges);

  typename list<typename PRange1::subrangeType*>::iterator input1_iter = input1_subranges.begin();
  typename list<typename PRange2::subrangeType*>::iterator input2_iter = input2_subranges.begin();
  typename list<typename PRange3::subrangeType*>::iterator output_iter = output_subranges.begin();

  for ( ; input1_iter != input1_subranges.end(); 
        ++input1_iter, ++input2_iter, ++output_iter) {
    transform((**input1_iter).get_boundary().start(),
              (**input1_iter).get_boundary().finish(),
              (**input2_iter).get_boundary().start(),
              (**output_iter).get_boundary().start(), func);
  }
}


/**
 * @ingroup for_each
 * pForEach
 */
template <class PRange1, class PRange2, class PRange3, class TertiaryFunc>
void p_for_each(PRange1& pr1, PRange2& pr2, PRange3& pr3, 
                TertiaryFunc func) {
  list<typename PRange1::subrangeType*> subranges1;
  list<typename PRange2::subrangeType*> subranges2;
  list<typename PRange3::subrangeType*> subranges3;

  get_subranges_no_deps(pr1, subranges1);
  get_subranges_no_deps(pr2, subranges2);
  get_subranges_no_deps(pr3, subranges3);

  typename list<typename PRange1::subrangeType*>::iterator sr_it1 = subranges1.begin();
  typename list<typename PRange2::subrangeType*>::iterator sr_it2 = subranges2.begin();
  typename list<typename PRange3::subrangeType*>::iterator sr_it3 = subranges3.begin();
  for ( ; sr_it1 != subranges1.end(); ++sr_it1, ++sr_it2, ++sr_it3) {
    typename PRange1::iteratorType el_it1 = (**sr_it1).get_boundary().start();
    typename PRange2::iteratorType el_it2 = (**sr_it2).get_boundary().start();
    typename PRange3::iteratorType el_it3 = (**sr_it3).get_boundary().start();
    for ( ; el_it1 != (**sr_it1).get_boundary().finish(); 
          ++el_it1, ++el_it2, ++el_it3) {
      func(*el_it1, *el_it2, *el_it3);
    }
  }
}
#endif
