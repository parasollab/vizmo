#ifndef P_SORT_H
#define P_SORT_H

/* * * 
 * PARALLEL SORT
 *
 * Functions defined in this file: 
 *     void p_sort(PRANGE& pr)x
 */
 
#include <algorithm>
#include <functional>
#include <vector>
#include <utility>
#include <numeric>
#include <math.h>
#include "pRange.h"
#include "pMatrix.h"
#include "sample_sort.h"
#include "radix_sort.h"
#include "column_sort.h"
#include "bitonic_sort.h"
#include "psort_adaptivity.h"

using stapl::pArray;

template<class T, class Compare>
struct _compare_pair : public binary_function<T,T,bool> {
  Compare& comp;

  _compare_pair(Compare& c) : comp(c){}

  bool operator () (const T& a, const T& b) {
    return comp(a.second,b.second);
  }
};

template<class PRange, class Compare=std::less<typename PRange::ContainerType::value_type> >
class _p_sort : public BasePObject {
  int sort_to_use;
  Compare comp;
  typedef typename PRange::ContainerType::value_type T;
  typedef _p_sort<PRange,Compare> SORT;
  int threadID, nThreads;
  int local_sample_size, sample_size;
  vector<pair<int,T> > local_vec;
  vector<pair<int,T> > samples;

 public:
  _p_sort(int ss, Compare _comp = Compare()) : comp(_comp) { 
    nThreads = stapl::get_num_threads();
    this->register_this(this);
    sample_size = ss;
    local_sample_size = sample_size/nThreads;
    samples.resize(sample_size);
    local_vec.resize(local_sample_size);
    stapl::rmi_fence();
  }  

  // Destructor
  ~_p_sort(){
    stapl::unregister_rmi_object(this->getHandle());
  }

  void set_array(vector<pair<int,T> >* in){
    typename vector<pair<int,T> >::iterator it1 = in->begin();
    typename vector<pair<int,T> >::iterator it2 = in->end();
    typename vector<pair<int,T> >::iterator it3 = samples.begin() + it1->first;
    std::copy(it1,it2,it3);
  }

  void set_sort(int* local, int* in, const int count){
    sort_to_use = *in;
  }

  int get_sort(){
    return sort_to_use;
  }
  
  void operator()(typename PRange::subrangeType& pr) {
#ifdef _PHASES_TIMER
    sampling_phase=0;
    stime = stapl::start_timer(); 
#endif  

    threadID       = stapl::get_thread_id();
    int size       = pr.container_size();
    int local_size = pr.container_local_size();
    int start_gid  = local_size*threadID; 
    int spacing    = local_size/local_sample_size;
    int offset     = threadID*local_sample_size;
    
    //do local sampling
    vector<pair<int,T> > local_vec(local_sample_size);
    pair<int,T> p;
    for (int i=0; i<local_sample_size; i++){
      p.first = offset+i;
      p.second = pr.GetElement(start_gid+spacing*i);
      local_vec[i] = p;
    }
    stapl::rmi_fence();

    //send local_vec to thread 0, append to samples
    async_rmi(0, this->getHandle(),&SORT::set_array, &local_vec);
    stapl::rmi_fence();

    double dist_stddev, dist_normalized;
    //sample statistics on thead O
    if (threadID == 0){
      //use stl sort and comparison function
      typedef _compare_pair<pair<int,T>,Compare> comp_p;
      std::sort(samples.begin(),samples.end(),comp_p(comp));

      vector<double> indexes(sample_size);
      for (int i=0; i<sample_size; i++){
	indexes[i] = abs((double)samples[i].first - i)/sample_size;
      }
     
      double dist_sum = std::accumulate(indexes.begin(),indexes.end(),0.0);
      double dist_avg = dist_sum / (float) sample_size;
      double dist_sd_sum = 0.0;
      for (int i=0; i<sample_size; i++) {
	double temp = (double) indexes[i] - dist_avg;
	dist_sd_sum += temp*temp;
      }
      dist_stddev = (double) dist_sd_sum / (double) (sample_size-1);
      dist_stddev = sqrt(dist_stddev);
      dist_normalized = dist_sum/(float)sample_size;
      cout<<"dist_stddev="<<dist_stddev<<" dist_normalized="<<dist_normalized<<endl;
    }
    stapl::rmi_fence();

    if (threadID==0){
    //call the learning stuff
      
      int pre;
      if (dist_normalized == 0){
	pre = nearly;
      }
      else{
	if (dist_normalized == 0.5){
	  pre = revers;
	}
	else{
	  pre = random;
	}
      }
      sort_to_use = sort_adapt_algo(nThreads,
				    size,
				    typeid(T),
				    pre);
    }
    stapl::rmi_fence();

    //broadcast the sort to all threads
    for (int j=0; j<nThreads; j++){
      stapl::broadcast_rmi(&sort_to_use,1,this->getHandle(),&SORT::set_sort,j);
    }

#ifdef _PHASES_TIMER
    sampling_phase += stop_timer(stime);
    if (threadID==0){
      cout<<"sampling_phase "<<sampling_phase<<endl; 
    }
#endif        
  }
};

/* * *
 * pSort
 */

template<class PRANGE>
int p_sort(PRANGE& pr,int _sort){  
  int sample_size = stapl::get_num_threads();
  int param = 0;
  if (stapl::get_num_threads() == 1){
    std::sort(pr.local_begin(), pr.local_end());
  }
  else{
    if (pr.container_size() > 128){
      sample_size = 128;
    }
    if (pr.container_size() > 1000000){
      sample_size = 1024;
    }

    stapl::defaultScheduler s1;
    _p_sort<PRANGE> _psort(sample_size);
    p_for_all(pr, _psort, s1);

    if (_sort == 0){
      _sort = _psort.get_sort();
    }
    
    switch (_sort){
    case 4:
      param = 1;
      p_radix_sort(pr,param);
      break;
    case 1:
      param = sample_size;
      p_sample_sort(pr,param);
      break;
    case 2:
      p_column_sort(pr,param);
      break;
    case 3:
      p_bitonic_sort(pr,param);
      break;
    default: 
      cout<<"no such sort!!"<<endl;
      break;
    }
  }
  return _sort;
}

template<class PRANGE, class TYPE>
int p_sort(PRANGE& pr,int _sort){  
  int sample_size = stapl::get_num_threads();
  int param = 0;
  if (stapl::get_num_threads() == 1){
    std::sort(pr.local_begin(), pr.local_end());
  }
  else{
    int temp = pr.container_size()/100000 - (pr.container_size()/100000)%1024;
    if (temp > sample_size){
      sample_size = temp;
    }
    stapl::defaultScheduler s1;
    _p_sort<PRANGE> _psort(sample_size);
    p_for_all(pr, _psort, s1);
   
    if (_sort == 0){
      _sort = _psort.get_sort();
    }

    switch (_sort){
    case 1:
      param = sample_size;
      p_sample_sort(pr,param);
      break;
    case 2:
      p_column_sort(pr,param);
      break;
    case 3:
      p_bitonic_sort(pr,param);
      break;
    default: 
      cout<<"no such sort!!"<<endl;
      break;
    }
  }
 return _sort;
}

template<class PRANGE, class Compare>
  int p_sort(PRANGE& pr,int _sort,Compare comp){  
  int sample_size = stapl::get_num_threads();
  int param = 0;
  if (stapl::get_num_threads() == 1){
    std::sort(pr.local_begin(), pr.local_end(), comp);
  }
  else{
    int temp = pr.container_size()/100000 - (pr.container_size()/100000)%1024;
    if (temp > sample_size){
      sample_size = temp;
    }
    stapl::defaultScheduler s1;
    _p_sort<PRANGE,Compare> _psort(sample_size,comp);
    p_for_all(pr, _psort, s1);

    if (_sort == 0){
      _sort = _psort.get_sort();
    }

    switch (_sort){
    case 1:
      param = sample_size;
      p_sample_sort(pr,param,comp);
      break;
    case 2:
      p_column_sort(pr,param,comp);
      break;
    case 3:
      p_bitonic_sort(pr,param,comp);
      break;
    default: 
      cout<<"no such sort!!"<<endl;
      break;
    }
  }
  return _sort;
}


#endif
