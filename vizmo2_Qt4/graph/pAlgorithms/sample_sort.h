/// SAMPLE SORT CLASS
///@author Olga Tkachyshyn 
///@date 2004


#ifndef SAMPLE_SORT_H
#define SAMPLE_SORT_H

/* * * 
 * PARALLEL SAMPLE SORT
 *
 * Functions defined in this file: 
 *     void p_sample_sort(PRANGE& pr)
 *
 * NOTES:
 *     - none
 */

#include <algorithm>
#include <functional>
#include <pRange.h>
#include <pArray.h>
#include "pAlgorithms/sort_utils.h"

using stapl::pArray;

template<class T>
class assignx{
 public:
  assignx(){}
  
  int operator()(vector<T>& x,T val) { 
    x.push_back(val);
    return 1; 
  }
  void define_type(stapl::typer &t){}
}; 



template<class PRange, class Compare=std::less<typename PRange::ContainerType::value_type> >
  class _p_sample_sort {
  Compare comp;
  typedef typename PRange::ContainerType::value_type T;
  int threadID, nThreads;
  int sample_size, ss;

 public:
  _p_sample_sort(int ss, Compare _comp = Compare()) : comp(_comp) { 
    nThreads = stapl::get_num_threads();
    sample_size = ss/nThreads;
    stapl::rmi_fence();
  }  
  
  void operator()(typename PRange::subrangeType& pr) {

#ifdef _PHASES_TIMER
    stapl::timer stime;
    double initial_phase=0;
    stime = stapl::start_timer(); 
    double initial_splitters_phase=0;
    double sort_splitters_phase=0;
    double select_splitters_phase=0;
    double distribute_keys_phase=0;
    double sort_keys_phase=0;
    double copy_keys_phase=0;
#endif

    threadID       = stapl::get_thread_id();
    int local_size = pr.container_local_size();
  
    srand48((unsigned int) time(NULL));
      
    //create a pArray for the temporary splitters 
    assignx<T> ax;
    pArray<T> x(nThreads * sample_size);

#ifdef _PHASES_TIMER
    initial_phase = stop_timer(stime);
    stime = stapl::start_timer(); 
#endif

    // Select and store the splitters in the pArray x
    int step = local_size/sample_size;
    typename PRange::iteratorType first_split = pr.local_begin() + step/2;
    for (int i=0; i<sample_size; i++){
      x.SetElement(threadID*sample_size + i,*first_split);
      first_split += step;
    }
    stapl::rmi_fence();

#ifdef _PHASES_TIMER
    initial_splitters_phase = stop_timer(stime);
    stime = stapl::start_timer(); 
#endif

    // Sort the splitters are using buble_sort on the distributed pArray of splitters
    if (threadID == 0){
      for (int i=0; i<nThreads*sample_size-1; i++) {
	for (int j=0; j<nThreads*sample_size-1-i; j++){
	  if (comp(x[j+1],x[j])) { 
	    T tmp = x[j];         
	    x.SetElement(j,x[j+1]);
	    x.SetElement(j+1,tmp);
	  }
	}
      }
    }
    stapl::rmi_fence();

#ifdef _PHASES_TIMER
    sort_splitters_phase = stop_timer(stime);
    stime = stapl::start_timer(); 
#endif

    // Evenly select final splitters 
    std::vector<T> split(nThreads-1);
    for (int i=0; i<nThreads-1; i++){          
      split[i] = x[(i+1)*sample_size-1];
    }

#ifdef _PHASES_TIMER    
    select_splitters_phase = stop_timer(stime);
    stime = stapl::start_timer(); 
#endif

    // Create a temporary array of buckets
    pArray<vector<T> > tmp(nThreads);
    
    // Distribute the keys to the buckets
    typename PRange::iteratorType first1 = pr.local_begin();
    typename PRange::iteratorType last1 = pr.local_end();
    while (first1 != last1){
      int dest;	    
      T val = *first1;
      
      if (nThreads-1 > 1){//if at least two splitters
	T* d = std::upper_bound(&split[0],&split[nThreads-1],val,comp);
	dest = (int)(d-(&split[0]));
      }
      else{
	if (nThreads==2){//one splitter
	  if(comp(val,split[0]))
	    dest=0;
	  else
	    dest=1;
	}
	else{//no splitter, send to self
	  dest=0;
	}
      }	    
      tmp.UpdateElement(dest,ax,val);	
      ++first1;
    } 

    stapl::rmi_fence();

#ifdef _PHASES_TIMER     
    distribute_keys_phase = stop_timer(stime);
    stime = stapl::start_timer(); 
#endif

    // Sort the keys in the buckets using stl::sort()
    typename pArray<vector<T > >:: iterator it = tmp.local_begin();
    sort(it->begin(), it->end(), comp);
   
#ifdef _PHASES_TIMER
    sort_keys_phase = stop_timer(stime);
    stime = stapl::start_timer(); 
#endif

    //    cout<<"bucket size="<<it->size()<<endl;
    stapl::rmi_fence();

    // Copy sorted buckets (tmp) into the original prange
    int vector_size = tmp[threadID].size();
    int out;
    prefix_sums(vector_size,out);
    stapl::rmi_fence();
    
    typename pArray<vector<T > >:: iterator iterat = tmp.local_begin();
    typename vector<T>:: iterator iter = iterat->begin();
    typename vector<T>:: iterator iter_end = iterat->end();

    while (iter != iter_end){
      pr.SetElement(out,*iter);
      ++iter;
      out++;
    }  

#ifdef _PHASES_TIMER
    copy_keys_phase = stop_timer(stime);
    
    if (threadID==0){
      cout<<"initial_phase="<<initial_phase<<endl;
      cout<<"initial_splitters_phase="<<initial_splitters_phase<<endl;
      cout<<"sort_splitters_phase="<<sort_splitters_phase<<endl;
      cout<<"select_splitters_phase="<<select_splitters_phase<<endl;
      cout<<"distribute_keys_phase="<<distribute_keys_phase<<endl;
      cout<<"sort_keys_phase="<<sort_keys_phase<<endl;
      cout<<"copy_keys_phase="<<copy_keys_phase<<endl;
    }
#endif
  }
};


/* * *
 * Sample Sort
 */

template<class PRANGE>
void p_sample_sort(PRANGE& pr, int sample_size){  
    stapl::defaultScheduler s1;
    _p_sample_sort<PRANGE> _psort(sample_size);
    p_for_all(pr, _psort, s1);
}


template<class PRANGE, class Compare>
  void p_sample_sort(PRANGE& pr, int sample_size, Compare comp){  
    stapl::defaultScheduler s1;
    _p_sample_sort<PRANGE,Compare> _psort(sample_size,comp);
    p_for_all(pr, _psort, s1);
}

#endif


