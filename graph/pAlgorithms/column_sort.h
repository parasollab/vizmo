#ifndef COLUMN_SORT_H
#define COLUMN_SORT_H

/* * * 
 * PARALLEL COLUMN SORT
 *
 * Functions defined in this file: 
 *     void p_column_sort(PRANGE& pr)
 *
 * NOTES:
 *     - declare a matrix of size r*s where s=nThreads,
 *       r is divisible by s, r>=2(s-1)^2
 */
 
#include <algorithm>
#include <functional>
#include <vector>
#include "pRange.h"
#include "pArray.h"
#include "pMatrix.h"
#include "mtl/mtl.h"
#include "mtl/utils.h"
#include "mtl/matrix.h"
#include "mtl/matrix_implementation.h"
#include "MatrixDistribution.h"

using stapl::pArray;
using stapl::pMatrix;


template<class PRange, class Compare=std::less<typename PRange::ContainerType::value_type> >
  class _p_column_sort {
  Compare comp;
  typedef typename PRange::ContainerType::value_type T;
  int threadID, nThreads;
  int param;

 public:
  _p_column_sort(int param, Compare _comp = Compare()) : comp(_comp) { 
    nThreads = stapl::get_num_threads();
    stapl::rmi_fence();
  }  

#ifdef _PHASES_TIMER
    stapl::timer stime;
    double copy_tomatrix_phase;
    double local_sort_phase;
    double transpose_reshape_phase;
    double reshape_transpose_phase;
    double shift_down_phase;
    double shift_up_phase;
    double copy_torange_phase;
#endif  
  
  /* * *
   * The operator function appropriately calls shift and sort_columns functions
   * first with the goal to obtain a bitonic sequence
   * then to perform a bitonic sort itself
   */
  void operator()(typename PRange::subrangeType& pr) {
#ifdef _PHASES_TIMER
    copy_tomatrix_phase=0;
    local_sort_phase=0;
    transpose_reshape_phase=0;
    reshape_transpose_phase=0;
    shift_down_phase=0;
    shift_up_phase=0;
    copy_torange_phase=0;
    stime = stapl::start_timer(); 
#endif  
    threadID       = stapl::get_thread_id();
    int size       = pr.container_size();
    int local_size = pr.container_local_size();

    //determine the size of matrix, r*s
    //let s be equal to the number of processors (1 column per processor)
    int s = nThreads;
    //let r be equal to size/s   !!! also assuming r is dividable by s !!!
    int r = size/s;

    //declare a column major matrix, as well as a temporary matrix of the same size/distribution
    typedef pMatrix<T,stapl_column_column_major> matrix_column_type;
    matrix_column_type M(r,s);
    stapl::rmi_fence();

    //copy elements from the pRange into the pMatrix
    //I am assuming that the elements are distributed equally between processors
    typename PRange::ContainerType::iterator pr_it = pr.local_begin();
    typename matrix_column_type::element_iterator m_it = (*(M.local_begin())).begin();
    for (int i=0; i<local_size; i++){
      *m_it = *pr_it;
      ++m_it;
      ++pr_it;
    }
    stapl::rmi_fence();
#ifdef _PHASES_TIMER
    copy_tomatrix_phase += stop_timer(stime);
    stime = stapl::start_timer(); 
#endif    

    //STEP 1: sort local elements
    sort((*(M.local_begin())).begin(),(*(M.local_begin())).end(),comp);
    stapl::rmi_fence();

#ifdef _PHASES_TIMER
    local_sort_phase += stop_timer(stime);
    stime = stapl::start_timer(); 
#endif    

    //STEP 2: transpose and reshape
    M.TransposeReshape();

#ifdef _PHASES_TIMER
    transpose_reshape_phase += stop_timer(stime);
    stime = stapl::start_timer(); 
#endif    

    //STEP 3: sort local elements
    sort((*(M.local_begin())).begin(),(*(M.local_begin())).end(),comp);
    stapl::rmi_fence();

#ifdef _PHASES_TIMER
    local_sort_phase += stop_timer(stime);
    stime = stapl::start_timer(); 
#endif    

    //STEP 4: reshape and transpose
    M.ReshapeTranspose();

#ifdef _PHASES_TIMER
    reshape_transpose_phase += stop_timer(stime);
    stime = stapl::start_timer(); 
#endif    

    //STEP 5: sort local elements
    sort((*(M.local_begin())).begin(),(*(M.local_begin())).end(),comp);
    stapl::rmi_fence();

#ifdef _PHASES_TIMER
    local_sort_phase += stop_timer(stime);
    stime = stapl::start_timer(); 
#endif    

    //STEP 6: cyclicly shift down by r/2
    M.ShiftCyclicDown(r/2);

#ifdef _PHASES_TIMER
    shift_down_phase += stop_timer(stime);
    stime = stapl::start_timer(); 
#endif    

    //STEP 7: sort local elements
    if (threadID==0){//sort 2 halves separately
      sort((*(M.local_begin())).begin(),(*(M.local_begin())).begin()+r/2,comp);
      sort((*(M.local_begin())).begin()+r/2,(*(M.local_begin())).end(),comp);
    }
    else{                   //sort normally
      sort((*(M.local_begin())).begin(),(*(M.local_begin())).end(),comp);
    }
    stapl::rmi_fence();

#ifdef _PHASES_TIMER
    local_sort_phase += stop_timer(stime);
    stime = stapl::start_timer(); 
#endif    

    //STEP 8: cyclicly shift up by r/2
    M.ShiftCyclicUp(r/2);

#ifdef _PHASES_TIMER
    shift_up_phase += stop_timer(stime);
    stime = stapl::start_timer(); 
#endif    

    //copy elements from the pMatrix back into the pRange
    pr_it = pr.local_begin(); 
    m_it = (*(M.local_begin())).begin();
    for (int i=0; i<local_size; i++){
      *pr_it = *m_it;
      ++pr_it;
      ++m_it;
    }
    stapl::rmi_fence();

#ifdef _PHASES_TIMER
    copy_torange_phase += stop_timer(stime);
    if (threadID==0){
      cout<<"copy_tomatrix_phase "<<copy_tomatrix_phase<<endl; 
      cout<<"local_sort_phase "<<local_sort_phase<<endl; 
      cout<<"transpose_reshape_phase "<<transpose_reshape_phase<<endl; 
      cout<<"reshape_transpose_phase "<<reshape_transpose_phase<<endl; 
      cout<<"shift_down_phase "<<shift_down_phase<<endl; 
      cout<<"shift_up_phase "<<shift_up_phase<<endl; 
      cout<<"copy_torange_phase "<<copy_torange_phase<<endl; 
    }
#endif        
  }
};


/* * *
 * Column Sort
 */
template<class PRANGE>
void p_column_sort(PRANGE& pr, int param){  
  
  if (stapl::get_num_threads() == 1){
    sort(pr.local_begin(), pr.local_end());
  }
  else{
    stapl::defaultScheduler s1;
    _p_column_sort<PRANGE> _psort(param);
    p_for_all(pr, _psort, s1);
  }
}

template<class PRANGE, class Compare>
  void p_column_sort(PRANGE& pr, int param, Compare comp){  
  if (stapl::get_num_threads() == 1){
    sort(pr.local_begin(), pr.local_end(), comp);
  }
  else{
    stapl::defaultScheduler s1;
    _p_column_sort<PRANGE,Compare> _psort(param,comp);
    p_for_all(pr, _psort, s1);
  }
}


#endif
