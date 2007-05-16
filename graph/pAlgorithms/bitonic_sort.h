#ifndef BITONIC_SORT_H
#define BITONIC_SORT_H

/* * * 
 * PARALLEL BITONIC SORT
 *
 * Functions defined in this file: 
 *     void p_bitonic_sort(PRANGE& pr)
 *
 * NOTES:
 *     - the number of processors executing must be a power of 2
 */
 
#include <algorithm>
#include <pRange.h>
#include <pArray.h>

using stapl::pArray;


template<class PRange, class Compare=std::less<typename PRange::ContainerType::value_type> >
  class _p_bitonic_sort {
  Compare comp;
  typedef typename PRange::ContainerType::value_type T;
  int threadID, nThreads;
  int param;

 public:
  _p_bitonic_sort(int param, Compare _comp = Compare()) : comp(_comp) { 
    nThreads = stapl::get_num_threads();
    stapl::rmi_fence();
  }  

#ifdef _PHASES_TIMER
    stapl::timer stime;
    stapl::timer btime;
    double local_sort_phase;
    double initial_phase;
    double copy_pranges_phase;
    double send_data_phase;
    double merge_phase;
    double total_bitonic_phase;
    double total_final_sort_phase;
#endif  

  /* * *
   * Function merge_and_split does exactly that: it merges parts from 2 processors
   * so that the result is split between the 2 processors, smaller elements on one
   * and larger on the other
   */
  void merge_and_split(typename PRange::subrangeType& pr, int i, int j){

#ifdef _PHASES_TIMER
    btime = stapl::start_timer(); 
#endif  

    threadID       = stapl::get_thread_id();
    int size       = pr.container_size();
    int local_size = pr.container_local_size();
    
    int dist;  //distance to the processor-coworker
    int dir;   //in which direction to sort

    if (threadID%(i*2) < i)     {dist = i; }  //current thread is the first one in the coworker pair
    else                        {dist = -i;}  //current thread is the second one in the coworker pair
    
    if (j == 1)                 {dir = 1;  }  //we are in the merge phase, so sort in ascending order
    else {if (threadID < (i*2)) {dir = 1;  }  //in the bitonic sequence phase, sort in ascending order
      else                      {dir = -1; }} //in the bitonic sequence phase, sort in descending order

#ifdef _PHASES_TIMER
    initial_phase += stop_timer(btime);
    btime = stapl::start_timer(); 
#endif     

    //temporary pranges with the same distribution and data
    typename PRange::ContainerType temp_cont1 = (pr.container_size());  
    PRange temp1(&temp_cont1);
    temp_cont1.get_prange(temp1);
    
    typename PRange::ContainerType temp_cont2 = (pr.container_size());  
    PRange temp2(&temp_cont2);
    temp_cont2.get_prange(temp2);

#ifdef _PHASES_TIMER
    copy_pranges_phase += stop_timer(btime);
    btime = stapl::start_timer(); 
#endif         

    //pass local elements to the coworker processor (remotely)
    typename PRange::ContainerType::iterator temp1_first = temp1.local_begin(); 
    typename PRange::ContainerType::iterator temp1_last = temp1.local_end(); 
    int offset = local_size*(threadID+dist);
    int num_sent = local_size;

    // Optimizing the merge phase: only send the necessary elements
    // 1. exchange max/min of the two processors
    // 2. using std::upperbound() find its position in local array
    // 3. appropriately adjust beginning and end
    
    // create temporary parrays for max/min 
    pArray<T> max_min(nThreads);    

    //if this is the processor that will keep smaller elements, send my max
    if (dist*dir >0){
      max_min.SetElement(threadID+dist,pr.GetElement(local_size*(threadID+1)-1));
    }
    //if this is the processor that will keep larger elements, send my min
    else{
      max_min.SetElement(threadID+dist,pr.GetElement(local_size*threadID));
    }
    stapl::rmi_fence();   //wait for the exchange to finish

    //find where other processor's min/max fits in my local elements
    typename PRange::ContainerType::iterator d = std::upper_bound(temp1_first, temp1_last, max_min.GetElement(threadID), comp);

    if (dist*dir >0){           //i am keeping smaller elements
      num_sent = temp1_last-d;  //decrement the number of elements sent
      temp1_first = d;          //increment the beginning of elements sent
    }
    else{                       //i am keeping larger elements
      num_sent = d-temp1_first; //decrement the number of elements sent
    }
    
    for (int i=0; i<num_sent; i++){//send elements needed by the coworker
      temp2.SetElement(offset+i, *temp1_first);
      ++temp1_first;
    }
    stapl::rmi_fence();         //wait for the exchange to be over
    
#ifdef _PHASES_TIMER
    send_data_phase += stop_timer(btime);
    btime = stapl::start_timer(); 
#endif       

    //now,depending on the distance, which one of the pair of processors i am, 
    //and sorting direction, merge beginning or end of temp1 and temp2 into pr
    if (dist*dir >0){  //merge the beginning of 2 sequences
      typename PRange::ContainerType::iterator destination = pr.local_begin();
      typename PRange::ContainerType::iterator feeder1 = temp1.local_begin();
      typename PRange::ContainerType::iterator feeder2 = temp2.local_begin();
      
      int i=0;
      while (i<num_sent){
	if (comp(*feeder1,*feeder2)){
	  *destination = *feeder1;
	  ++feeder1;
	}
	else{
	  *destination = *feeder2;
	  ++feeder2;
	  i++;
	}
	++destination;
      }
      while (i<local_size){
	*destination = *feeder1;
	++feeder1;
	++destination;
	i++;
      }
    }
    else{  //merge the ending of 2 sequences
      typename PRange::ContainerType::iterator destination = pr.local_end()-1;
      typename PRange::ContainerType::iterator feeder1 = temp1.local_end()-1;
      typename PRange::ContainerType::iterator feeder2 = temp2.local_end()-1;
      
      int i=0;
      while (i<num_sent){
	if (comp(*feeder2,*feeder1)){
	  *destination = *feeder1;
	  --feeder1;
	}
	else{
	  *destination = *feeder2;
	  --feeder2;
	  i++;
	}
	--destination;
      }
      while (i<local_size){
	*destination = *feeder1;
	--feeder1;
	--destination;
	i++;
      }
    }
#ifdef _PHASES_TIMER
    merge_phase += stop_timer(btime);
    btime = stapl::start_timer(); 
#endif  

  }
  
  /* * *
   * The operator function appropriately calls the merge_and_split function
   * first with the goal to obtain a bitonic sequence
   * then to perform a bitonic sort itself
   */
  void operator()(typename PRange::subrangeType& pr) {
    threadID       = stapl::get_thread_id();
    int size       = pr.container_size();
    int local_size = pr.container_local_size();

#ifdef _PHASES_TIMER
    local_sort_phase=0;
    initial_phase=0;
    copy_pranges_phase=0;
    send_data_phase=0;
    merge_phase=0;
    total_bitonic_phase=0;
    total_final_sort_phase=0;
#endif  

#ifdef _PHASES_TIMER
    stime = stapl::start_timer(); 
#endif    

    //sorting local elements
    sort(pr.local_begin(),pr.local_end(),comp);
    stapl::rmi_fence();

#ifdef _PHASES_TIMER
    local_sort_phase = stop_timer(stime);
    stime = stapl::start_timer(); 
#endif
    
    //forming the bitonic sequence phase
    for (int j=2; j<nThreads; j=j*2){
      for (int i=j; i>=1; i=i/2){
	merge_and_split(pr,i,0);
	stapl::rmi_fence();
      }
    }

#ifdef _PHASES_TIMER
    total_bitonic_phase = stop_timer(stime);
    stime = stapl::start_timer(); 
#endif 

    //the merge phase - final sorting
    for (int i=nThreads/2; i>=1; i=i/2){
      merge_and_split(pr,i,1);
      stapl::rmi_fence();
    }

#ifdef _PHASES_TIMER
    total_final_sort_phase = stop_timer(stime);
    
    if (threadID==0){
      cout<<"local_sort_phase"<<local_sort_phase<<endl; 
      cout<<"initial_phase"<<initial_phase<<endl; 
      cout<<"copy_pranges_phase"<<copy_pranges_phase<<endl; 
      cout<<"send_data_phase"<<send_data_phase<<endl; 
      cout<<"merge_phase"<<merge_phase<<endl; 
      cout<<"total_bitonic_phase"<<total_bitonic_phase<<endl; 
      cout<<"total_final_sort_phase"<<total_final_sort_phase<<endl; 
    }
#endif

  }
};


/* * *
 * Bitonic Sort
 */
template<class PRANGE>
void p_bitonic_sort(PRANGE& pr, int param){    
  if (stapl::get_num_threads() == 1){
    sort(pr.local_begin(), pr.local_end());
  }
  else{
    stapl::defaultScheduler s1;
    _p_bitonic_sort<PRANGE> _psort(param);
    p_for_all(pr, _psort, s1);
  }
}

template<class PRANGE, class Compare>
  void p_bitonic_sort(PRANGE& pr, int param, Compare comp){  
  if (stapl::get_num_threads() == 1){
    sort(pr.local_begin(), pr.local_end(),comp);
  }
  else{
    stapl::defaultScheduler s1;
    _p_bitonic_sort<PRANGE,Compare> _psort(param,comp);
    p_for_all(pr, _psort, s1);
  }
}

#endif
