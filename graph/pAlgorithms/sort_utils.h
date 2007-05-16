/* Simplified form of the prefix sums (assumes one number per processor) */

#ifndef SORT_UTILS_H
#define SORT_UTILS_H

#include <runtime.h>
#include <vector>
#include <algorithm>


template<class T>
struct _prefix_sums : public BasePObject {
  T initial_value;
  T result;
  vector<T> indexarray;

  /* Constructor */
  _prefix_sums()
  :  indexarray(this->num_threads){
    this->register_this(this);
  } 
   
  /* Destructor */
  ~_prefix_sums(){
     stapl::unregister_rmi_object( this->getHandle());
  }

  /* combo holds the thread_id and the number that thread passes in */
  struct combo {
    int thread_id;
    T partsum;

    combo() : thread_id(stapl::get_thread_id()), partsum(0) {}

    void define_type(stapl::typer& t) { 
      t.local(thread_id);
      t.local(partsum); 
    }
  };
  
  void set_value(combo* in) {
    indexarray[in->thread_id] = in->partsum;
  }

  void set_result(T* val) {
    result = *val;
  }

  void operator()(T& in, T& out) {
    combo c;
    c.thread_id = this->thread_id;
    c.partsum = in;
    stapl::rmi_fence();
    
    //Each thread sends thread zero its number (of elements in the case of Sample Sort)  
    if (stapl::get_thread_id() != 0) {
      async_rmi(0, this->getHandle(), &_prefix_sums<T>::set_value, &c);
    } 
    else {
      indexarray[0] = c.partsum;
      result = 0;
    }
    stapl::rmi_fence();
    
    //Thread zero calculates the starting sums for all threads using stl's partial_sum
    if (stapl::get_thread_id()==0){
      partial_sum(indexarray.begin(), indexarray.end(), indexarray.begin());
      
      //Thread zero sends out to all threads their corresponding prefix sums
      for (int i = 1; i < stapl::get_num_threads(); i++){
	stapl::async_rmi(i, this->getHandle(), &_prefix_sums<T>::set_result, &indexarray[i-1]);
      }
    }
    stapl::rmi_fence();
    out = result;

  }//end of operator()
};

template <class T>
void prefix_sums(T& in, T& out){
  _prefix_sums<T> _ps;
  _ps(in,out);
}
#endif
