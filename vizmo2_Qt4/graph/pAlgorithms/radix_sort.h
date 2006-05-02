#ifndef RADIX_SORT_H
#define RADIX_SORT_H

/* * * 
 * PARALLEL RADIX SORT
 *
 * Functions defined in this file: 
 *     void p_radix_sort(PRANGE& pr)
 *
 * NOTES:
 *     - works only for integers
 */

#include <math.h>
#include <algorithm>
#include "pRange.h"
#include <pArray.h>
#include "../runtime.h"
#include "p_algobase.h"

using stapl::pArray;

template<class PRange>
class _p_radix_sort {
  typedef typename PRange::ContainerType::value_type T;
  int threadID, nThreads, passes;
  
 public:
  _p_radix_sort(int p) { 
    nThreads = stapl::get_num_threads();
    passes = p;
    stapl::rmi_fence();
  }  
  
  void operator()(typename PRange::subrangeType& pr) {

#ifdef _PHASES_TIMER
    stapl::timer stime;
    double initial_phase=0;
    stime = stapl::start_timer();
#endif

    threadID = stapl::get_thread_id();
    
    //optimization:  sort only the range of elements necessary
    //get local max, pick global max
    pArray<T> max(nThreads);
    typename PRange::iteratorType m_first = pr.local_begin();
    typename PRange::iteratorType m_last = pr.local_end();
    max.SetElement(threadID, *max_element(m_first,m_last)); 
    stapl::rmi_fence();  //wait for all threads to find the max and the min
    T global_max = max[0];
    T temp_max;
    for (int p=1; p<nThreads; p++){
      temp_max = max[p];
      if (temp_max > global_max){
	global_max = temp_max;
      }
    }
    int range = (int)(log10((float)global_max)/log10((float)2)) + 1;

    //prange with the same distribution and data
    typename PRange::ContainerType temp_cont(pr.container_size());
    PRange temp(&temp_cont);
    temp_cont.get_prange(temp);
    
    typename PRange::iteratorType g1 = pr.local_begin();
    typename PRange::iteratorType g2 = pr.local_end();
    typename PRange::iteratorType g3 = temp.local_begin();
    copy(g1,g2,g3);
    
    int local_size = pr.container_local_size();

    if (range%passes != 0){
      range = range + range%passes;
    }
      int bits = range;    
      int n    = passes;
      int r    = bits/n;    //consider r bits at a time

    int two_to_r = (int) (pow((float) (2),r));
 
#ifdef _PHASES_TIMER       
    double count_phase[n];
    double prefix_phase[n];
    double previous_phase[n];
    double final_phase[n];
    double copy_phase[n];
#endif

    //& with mask to get the least significant bits only
    int mask = 0;
    for (int j=1; j<two_to_r; j*=2){
      mask += j;
    }
    
    //pArrays to store local arrays
    int size = nThreads * two_to_r;
    int local_begin = threadID * two_to_r;
    int local_end = local_begin + two_to_r;
        
    pArray<T> count(size);     //1. number of elements with each value
                               //2. position of elements (previous + prefix in step 2)
    pArray<T> previous(size);  //1. number of elements with each value on previous threads
    pArray<T> prefix(size);    //1. prefix sum of count array in step 1
                               //2. total of prefix sums in prefix array in step 1
    stapl::rmi_fence();

#ifdef _PHASES_TIMER
    initial_phase = stop_timer(stime);
#endif

    //For the ith block of r bits do: (going from least significant to most significant)
    for(int i=0; i<n; i++){	

#ifdef _PHASES_TIMER
      stime = stapl::start_timer();
#endif
     
      //initially set all counts to zero
      typename PRange::ContainerType::iterator first1 = count.local_begin();
      typename PRange::ContainerType::iterator last1 = count.local_end();
      while(first1 != last1){         
	*first1 = 0;
	++first1;
      }

      stapl::rmi_fence();
      
      //Each thread counts the number of its n/p elements with value v for 0<=v<2^r-1 
      //Each thread stores the counts in the local count array C of size 2^r (local computation)
      if (i%2 == 0){
	typename PRange::iteratorType first2 = pr.local_begin();
	typename PRange::iteratorType last2 = pr.local_end();
	int index;
	while (first2 != last2){
	  index = local_begin + (((*first2)>>(r*i)) & mask);
	  count.SetElement(index, count[index]+1);
	  ++first2;
	}
      }
      else{
	typename PRange::iteratorType first2 = temp.local_begin();
	typename PRange::iteratorType last2 = temp.local_end();
	int index;
	while (first2 != last2){
	  index = local_begin + (((*first2)>>(r*i)) & mask);
	  count.SetElement(index, count[index]+1);
	  ++first2;
	}
      }
      stapl::rmi_fence();

#ifdef _PHASES_TIMER
      count_phase[i] = stop_timer(stime);
      stime = stapl::start_timer();
#endif
      
      //Each thread computes the prefix sums for the count array C (local computation) 
      typename pArray<T>::iterator count_begin = count.local_begin();
      typename pArray<T>::iterator count_end = count.local_end();
      typename pArray<T>::iterator prefix_begin = prefix.local_begin();
      partial_sum(count_begin,count_end,prefix_begin);
      stapl::rmi_fence();

#ifdef _PHASES_TIMER
      prefix_phase[i] = stop_timer(stime);
      stime = stapl::start_timer();
#endif
	
      //Each thread is assigned 2^r/p values of [0,2^r) 
      //store the number of elements with value on threads zero through threadID-1 in array previous 
      //store the total number of elements with value on all threads in array prefix (sum of all local prefixes)
      int assigned = two_to_r/nThreads;	  
      for (int j=threadID*assigned; j<(threadID+1)*assigned; j++){
	int tmp = 0;
	for (int k=0; k<nThreads; k++){	
	  int current_index = k*two_to_r + j;
	  tmp += prefix[current_index];
	  if (k==0){
	    previous.SetElement(current_index, 0);
	  }
	  else{
	    int previous_index = (k-1)*two_to_r + j; 
	    previous.SetElement(current_index, (previous[previous_index] + count[previous_index]));
	  }
	}
      
	
	for (int k=0; k<nThreads; k++){
	  int current_index = k*two_to_r + j;	
	  prefix.SetElement(current_index, tmp);
	}
      }
      stapl::rmi_fence();

#ifdef _PHASES_TIMER
      previous_phase[i] = stop_timer(stime);
      stime = stapl::start_timer();
#endif      

      //each thread calculates the final positions for its own elements
      for(int j=local_begin; j<local_end; j++){        
	if (j == local_begin){                //first element on the processor
	  if (j == 0){                        //smallest element on the first processor
	    count.SetElement(0,0);            //therefore nothing precedes it 
	  }
	  else{                               //smallest element not on the first processor
	    count.SetElement(j,previous[j]);  //the only elements preceding it are on previous processors
	  }
	}
	else{                                 //all the rest elements, previous[j] + prefix[j-1]  
	  count.SetElement(j, previous[j]+prefix[j-1]);
	}
      }
      stapl::rmi_fence();

#ifdef _PHASES_TIMER     
      final_phase[i] = stop_timer(stime);
      stime = stapl::start_timer();
#endif    

      //each thread places its elements into the output
      if (i%2 == 0){ 
	typename PRange::iteratorType pr_start = pr.local_begin();
	typename PRange::iteratorType pr_end = pr.local_end();
	while (pr_start != pr_end){
	  T elem = *pr_start;
	  int k = local_begin + ((elem>>(r*i)) & mask);
	  temp.SetElement(count[k], elem);
	  count.SetElement(k, count[k]+1);
	  ++pr_start;
	}
      }
      else{
	typename PRange::iteratorType pr_start = temp.local_begin();
	typename PRange::iteratorType pr_end = temp.local_end();
	while (pr_start != pr_end){
	  T elem = *pr_start;
	  int k = local_begin + ((elem>>(r*i)) & mask);
	  pr.SetElement(count[k], elem);
	  count.SetElement(k, count[k]+1);
	  ++pr_start;
	}
      }
      stapl::rmi_fence();

#ifdef _PHASES_TIMER
      copy_phase[i] = stop_timer(stime);
#endif
    }

    //if there is an odd number of passes, final copy phase is required
    if (n%2 == 1){     
      typename PRange::iteratorType f1 = temp.local_begin();
      typename PRange::iteratorType f2 = temp.local_end();
      typename PRange::iteratorType f3 = pr.local_begin();
      copy(f1,f2,f3);
    }
   
#ifdef _PHASES_TIMER
    if (threadID==0){
      cout<<"initial phase                "<<initial_phase<<endl;
      for (int i=0; i<n; i++){
	
	  cout<<"i="<<i<<", count phase     "<<count_phase[i]<<endl;
	  cout<<"i="<<i<<", prefix phase    "<<prefix_phase[i]<<endl;
	  cout<<"i="<<i<<", previous phase  "<<previous_phase[i]<<endl;
	  cout<<"i="<<i<<", final phase     "<<final_phase[i]<<endl;
	  cout<<"i="<<i<<", copy phase      "<<copy_phase[i]<<endl;
      }
    }//end of phases counter
#endif
  } 
}; 


/* * *
 * Radix Sort
 */
template<class PRANGE>
void p_radix_sort(PRANGE& pr, int passes){  
    stapl::defaultScheduler s1;
    _p_radix_sort<PRANGE> _psort(passes);
    p_for_all(pr, _psort, s1);
}

#endif
