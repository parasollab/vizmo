
#ifndef LINEARRECURRENCE_H
#define LINEARRECURRENCE_H

/** 
 * \file p_Recurrence.h
 * \author Tao Huang
 * \date 01/19/2005
 * \*ingroup stapl
 * \brief First Order Linear Recurrence pAlgorithm.
 *
 * Solving first-order linear recurrence relation:
 * x[i]= x[i+/-1] '+' x[i]	..... case (I)
 * A more general form is:   
 * x[i]= a[i] '*' x[i+/-1] '+' x[i] .. case (II)
 * 	(+/- depends on the recurrence direction)
 * where a[i] and x[i] are of compatible types.
 */

#include <runtime.h>
#include <rmitools.h>
#include "SimplePointerJumping.h"
#include "runtime/timer.h"

namespace stapl {

//the recurrence direction
//Fix me!
//
//There should be a method in pRange to decide whether it is 
// a Forwarding pRange or reverse. 
//Since there is no such way now, we need a boolean direction
// to specifically show the direction of computation, which is
// actually the same as the direction (normal pRange or reverse)
// of the input pRange for the pAlgorithm.

#ifndef Forward 
#define Forward true
#endif

#ifndef Backward
#define Backward false
#endif

/* used in case(I)'s step1: work function to compute accumulated value */
template <class PRANGEX, class PRANGEY, class VALTYPE, class ASSOC_PLUS>
class __accumulate1 { 
  ASSOC_PLUS* _assoc_plus;
public: 
  __accumulate1(ASSOC_PLUS& assoc_plus) : _assoc_plus(&assoc_plus) {}

  void operator()(PRANGEX& pr, PRANGEY& pry){
    typename PRANGEX::iteratorType it = pr.get_boundary().start();
    if (pr.get_boundary().finish()==it ) { 
#ifdef STAPL_DEBUG
      cout<<"The input pRange pr is empty."<<endl; 
#endif
      return;
    }
    typename PRANGEY::iteratorType ity = pry.get_boundary().start();
    if (pry.get_boundary().finish()==ity ) { 
#ifdef STAPL_DEBUG
      cout<<"The input pRange pry is empty."<<endl; 
#endif
      return;
    }
    VALTYPE __accum = *it; 
    ++it;
    while (it != pr.get_boundary().finish() ) {
      __accum = (*_assoc_plus)(__accum, *it);  
      ++it;
    }
    *ity = __accum;
  }
};

/* used in case(II)'s step1: work function to compute accumulated value */
template <class PRANGEX, class PRANGEY, class VALTYPE, class ASSOC_PLUS, class ASSOC_TIMES>
class __accumulate2 { 
  ASSOC_PLUS* _assoc_plus;
  ASSOC_TIMES* _assoc_times;
public: 
  __accumulate2(ASSOC_PLUS& assoc_plus, ASSOC_TIMES& assoc_times) : _assoc_plus(&assoc_plus), _assoc_times(&assoc_times) {}

  void operator()(PRANGEX& pr, PRANGEY& pry){
    typename PRANGEX::iteratorType it = pr.get_boundary().start();
    if (pr.get_boundary().finish()==it ) { 
#ifdef STAPL_DEBUG
      cout<<"The input pRange pr is empty."<<endl; 
#endif
      return;
    }
    typename PRANGEY::iteratorType ity = pry.get_boundary().start();
    if (pry.get_boundary().finish()==ity ) { 
#ifdef STAPL_DEBUG
      cout<<"The input pRange pry is empty."<<endl; 
#endif
      return;
    }
    VALTYPE __accum = *it; 
    ++it;
    while (it != pr.get_boundary().finish() ) {
      __accum.first = (*_assoc_plus)( (*_assoc_times)((*it).second, __accum.first), (*it).first );
      __accum.second = (*_assoc_times)( (*it).second, __accum.second );
      ++it;
    }
    *ity = __accum;
  }
};

/* used in case(I)'s step2: TODOFunc1 is a simple wrapper for ASSOC_PLUS;  
 * the reason to do so is to provide SimplePointerJumping pAlgo. a simple 
 * binary workfunction computing on 2 elements (in this case, they are simple x[i-1] and x[i] */
template <class ELEMENT, class ASSOC_PLUS>
class TODOFunc1 {
    ASSOC_PLUS* _assoc_plus;
  public:
    TODOFunc1(ASSOC_PLUS& assoc_plus): _assoc_plus(&assoc_plus){}
    ELEMENT identity() { return (*_assoc_plus).identity();}
    ELEMENT operator() (ELEMENT accum1, ELEMENT accum2) { return (*_assoc_plus)(accum1, accum2); }
};


/* used in case(II)'s step2: TODOFunc2 is a wrapper for ASSOC_PLUS and ASSOC_TIMES
 * the reason to do so is to provide SimplePointerJumping pAlgo. a simple binary 
 * workfunction computing on 2 elements (in this case, they are 2 (x,a) pairs: 
 *     (x[i-1], a[i-1]) and (x[i], a[i]) (Assume the direction is Forward.)
 * Then the binary operation is as following:
 * newpair.x = pair2.second * pair1.first + pair2.first
 * newpair.a = pair2.second * pair1.second 
 */
template <class ELEMENT, class ASSOC_PLUS, class ASSOC_TIMES>
class TODOFunc2 {
    ASSOC_PLUS* _assoc_plus;
    ASSOC_TIMES* _assoc_times;
  public:
    TODOFunc2(ASSOC_PLUS& assoc_plus, ASSOC_TIMES& assoc_times): _assoc_plus(&assoc_plus), _assoc_times(&assoc_times) {}
    ELEMENT identity() { return ELEMENT( (*_assoc_plus).identity(),(*_assoc_times).identity() ); }
    ELEMENT operator() (ELEMENT pair1, ELEMENT pair2) {
      return ELEMENT( (*_assoc_plus)((*_assoc_times)(pair2.second, pair1.first), pair2.first), (*_assoc_times)(pair2.second, pair1.second) );
    }
};

/* used in case(I)'s step3: work function update elements based on 'y' in step2 */
template<class PRANGEX, class PRANGEY, class VALTYPE, class ASSOC_PLUS>
class __update1 {
  ASSOC_PLUS* _assoc_plus;
public: 
  __update1(ASSOC_PLUS& assoc_plus) : _assoc_plus(&assoc_plus) {}

  void operator()(PRANGEX& pr, PRANGEY& pry){
    typename PRANGEX::iteratorType it = pr.get_boundary().start();
    typename PRANGEY::iteratorType ity = pry.get_boundary().start();
    if (pr.get_boundary().finish()==it ) cout<<"pr finish"<<endl;
    if (pry.get_boundary().finish()==ity ) cout<<"pry finish"<<endl;
    VALTYPE __temp = *ity;
    while (it!= pr.get_boundary().finish() ) {
      __temp = (*_assoc_plus)(__temp, *it);
      *it = __temp;  
      ++it;
    }
  }
};

/* used in case(II)'s step3: work function update elements based on 'y' in step2 */
template<class PRANGEX, class PRANGEY, class VALTYPE, class ASSOC_PLUS, class ASSOC_TIMES>
class __update2 {
  ASSOC_PLUS* _assoc_plus;
  ASSOC_TIMES* _assoc_times;
public: 
  __update2(ASSOC_PLUS& assoc_plus, ASSOC_TIMES& assoc_times) : _assoc_plus(&assoc_plus), _assoc_times(&assoc_times) {}

  void operator()(PRANGEX& pr, PRANGEY& pry){
    typename PRANGEX::iteratorType it = pr.get_boundary().start();
    typename PRANGEY::iteratorType ity = pry.get_boundary().start();
    if (pr.get_boundary().finish()==it ) cout<<"pr finish"<<endl;
    if (pry.get_boundary().finish()==ity ) cout<<"pry finish"<<endl;
    VALTYPE __temp = *ity;
    while (it!= pr.get_boundary().finish() ) {
      __temp.first = (*_assoc_plus)( (*_assoc_times)((*it).second, __temp.first), (*it).first);
      *it = __temp;  
      ++it;
    }
  }
};


  //------------------------------------------------------------------------------------------
  // Main function 
  //
  //  step1. Run __accumulate on each component so that each component gets its own 'accum' value
  //  step2. call __SimplePointerJumping to transmit values among linked components
  //  step3. Run __update on each component so that each component updates its all elements
  //------------------------------------------------------------------------------------------

  template<class PRANGE, class ASSOC_PLUS>
  void Recurrence1(PRANGE& pr, ASSOC_PLUS& assoc_plus, bool direction=Forward) {
    timer tv; double elapsed;
    typedef typename PRANGE::ContainerType PCONT;
    typedef typename PCONT::value_type VALTYPE;
    typedef stapl::defaultScheduler scheduler;
    scheduler s1, s2;

    VALTYPE init = assoc_plus.identity();
    PCONT y(pr, false);
    for (PARTID partid = 0; partid<pr.get_num_subranges(); partid++) {
      *((*(y.GetPart(partid))).begin()) = init;
    } 
    stapl::rmi_fence();

    PRANGE pry(&y);
    y.get_prange(pry);
    stapl::rmi_fence();

    // step1: accumulate 
    tv = start_timer();
    typedef typename PRANGE::subrangeType subrangeType;
    __accumulate1<subrangeType, subrangeType, VALTYPE, ASSOC_PLUS> accumulator(assoc_plus);
    p_for_all<PRANGE, PRANGE, __accumulate1<subrangeType, subrangeType, VALTYPE, ASSOC_PLUS>, scheduler > (pr, pry, accumulator, s1);
    stapl::rmi_fence();
    elapsed = stop_timer(tv);
#ifdef STAPL_DEBUG
    if (stapl::get_thread_id() == 0)  
      cout<<"accumulate takes time "<<elapsed<<endl;
#endif

    // step2: SimplePointerJumping to accumulate values across parts
    tv = start_timer();
    typedef TODOFunc1<VALTYPE, ASSOC_PLUS> TODOFUNC;
    TODOFUNC todofunc(assoc_plus);
    stapl::rmi_fence();
    SimplePointerJumping<PRANGE, TODOFUNC>(pry, todofunc, direction, true);
    elapsed = stop_timer(tv);
#ifdef STAPL_DEBUG
    if (stapl::get_thread_id() == 0)  
      cout<<"pointer jumping takes time "<<elapsed<<endl;
#endif

    // step3: update 
    tv = start_timer();
    pr.get_ddg().reset();
    __update1<subrangeType, subrangeType, VALTYPE, ASSOC_PLUS> updator(assoc_plus);
    p_for_all<PRANGE, PRANGE, __update1<subrangeType, subrangeType, VALTYPE, ASSOC_PLUS>, scheduler> (pr, pry, updator, s2);
    stapl::rmi_fence();
    elapsed = stop_timer(tv);
#ifdef STAPL_DEBUG
    if (stapl::get_thread_id() == 0)  
      cout<<"update takes time "<<elapsed<<endl;
#endif
    stapl::rmi_fence();
  }

  template<class PRANGE, class ASSOC_PLUS, class ASSOC_TIMES>
  void Recurrence2(PRANGE& pr, ASSOC_PLUS& assoc_plus, ASSOC_TIMES& assoc_times, bool direction=Forward) {
    timer tv; double elapsed;
    typedef typename PRANGE::ContainerType PCONT;
    typedef typename PCONT::value_type VALTYPE;
    typedef stapl::defaultScheduler scheduler;
    scheduler s1, s2;

    VALTYPE init = VALTYPE(assoc_plus.identity(), assoc_times.identity() );
    PCONT y(pr, false);
    for (PARTID partid = 0; partid<pr.get_num_subranges(); partid++) {
      *((*(y.GetPart(partid))).begin()) = init;
    } 
    stapl::rmi_fence();
    PRANGE pry(&y);
    y.get_prange(pry);
    stapl::rmi_fence();

    // step1: accumulate 
    tv = start_timer();
    typedef typename PRANGE::subrangeType subrangeType;
    __accumulate2<subrangeType, subrangeType, VALTYPE, ASSOC_PLUS, ASSOC_TIMES> accumulator(assoc_plus, assoc_times);
    p_for_all<PRANGE, PRANGE, __accumulate2<subrangeType, subrangeType, VALTYPE, ASSOC_PLUS, ASSOC_TIMES>, scheduler > (pr, pry, accumulator, s1);
    stapl::rmi_fence();
    elapsed = stop_timer(tv);
#ifdef STAPL_DEBUG
    if (stapl::get_thread_id() == 0)  
      cout<<"accumulate takes time "<<elapsed<<endl;
#endif

    // step2: SimplePointerJumping to accumulate values across parts
    tv = start_timer();
    typedef TODOFunc2<VALTYPE, ASSOC_PLUS, ASSOC_TIMES> TODOFUNC;
    TODOFUNC todofunc(assoc_plus, assoc_times);
    stapl::rmi_fence();
    SimplePointerJumping<PRANGE, TODOFUNC>(pry, todofunc, direction, true);
    elapsed = stop_timer(tv);
#ifdef STAPL_DEBUG
    if (stapl::get_thread_id() == 0)  
      cout<<"pointer jumping takes time "<<elapsed<<endl;
#endif

    // step3: update 
    tv = start_timer();
    pr.get_ddg().reset();
    __update2<subrangeType, subrangeType, VALTYPE, ASSOC_PLUS, ASSOC_TIMES> updator(assoc_plus, assoc_times);
    p_for_all<PRANGE, PRANGE, __update2<subrangeType, subrangeType, VALTYPE, ASSOC_PLUS, ASSOC_TIMES>, scheduler> (pr, pry, updator, s2);
    stapl::rmi_fence();
    elapsed = stop_timer(tv);
#ifdef STAPL_DEBUG
    if (stapl::get_thread_id() == 0)  
      cout<<"update takes time "<<elapsed<<endl;
#endif
  }
}

#endif
