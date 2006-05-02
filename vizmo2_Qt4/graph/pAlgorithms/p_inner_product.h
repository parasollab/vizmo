#ifndef P_INNER_PRODUCT_H
#define P_INNER_PRODUCT_H

/* * * 
 * PARALLEL INNER_PRODUCT
 *
 * Functions defined in this file: 
 *     void p_inner_product(PRANGE& pr1, PRANGE& pr2)
 */
 
#include <algorithm>
#include <functional>
#include <vector>
#include "p_algobase.h"
#include "pRange.h"


template<class PRange, class  BinaryFunction1=std::plus<typename PRange::ContainerType::value_type> >
class _p_inner_product : public BasePObject {
  BinaryFunction1 adder;
  typedef typename PRange::ContainerType::value_type T;
  typedef _p_inner_product<PRange,BinaryFunction1> INNER;
  int threadID, nThreads;
  T local_result, global_result;
  
  public:
  _p_inner_product() { 
    nThreads = stapl::get_num_threads();
    this->register_this(this);
    stapl::rmi_fence();
  }  

  //Destructor
  ~_p_inner_product(){
    stapl::unregister_rmi_object(this->getHandle());
  }
  
  void sum(T* in, T* inout) { *inout = adder(*inout,*in); }
  
  T get_result(){ return global_result; }

  void reduce(T _local_result, BinaryFunction1 _adder = BinaryFunction1()) {
    adder = _adder;
    local_result = _local_result;
    threadID = stapl::get_thread_id();
    stapl::reduce_rmi(&local_result,&global_result, this->getHandle(),&INNER::sum, true);
  }
};


///@ingroup pAlgorithms  
///@name p_inner_product 
///@brief inner product using regular multiplication and addition
///@input PRange1, PRange2, T init
template<class PRange>
typename iterator_traits<typename PRange::iteratorType>::value_type 
p_inner_product(PRange& pr1,
		PRange& pr2, 
		typename iterator_traits<typename PRange::iteratorType>::value_type init){  

  //call STL locally
  typedef typename iterator_traits<typename PRange::iteratorType>::value_type T;
  T local_result;
  if (stapl::get_thread_id()==0){
    local_result = std::inner_product(pr1.local_begin(), 
				      pr1.local_end(),
				      pr2.local_begin(), 
				      init);
  }
  else{
    local_result = std::inner_product(pr1.local_begin(), 
				      pr1.local_end(),
				      pr2.local_begin(), 
				      (T)0);
  }
  stapl::rmi_fence();
  
  //get the final result
  if (stapl::get_num_threads() == 1){
    init = local_result;
  }
  else{
    _p_inner_product<PRange> _pinner;
    _pinner.reduce(local_result);
    init = _pinner.get_result();
  }
  return init;
}


///@ingroup pAlgorithms  
///@name p_inner_product 
///@brief inner product using regular multiplication and addition
///@input PRange1, PRange2, T init, BinaryFunction1, BinaryFunction2
template<class PRange, class BinaryFunction1, class BinaryFunction2>
typename iterator_traits<typename PRange::iteratorType>::value_type 
p_inner_product(PRange& pr1,
		PRange& pr2, 
		typename iterator_traits<typename PRange::iteratorType>::value_type init,
		BinaryFunction1 binary_op1, //addition
                BinaryFunction2 binary_op2){//multiplication  

  //call STL locally
  typedef typename iterator_traits<typename PRange::iteratorType>::value_type T;
  T local_result;
  if (stapl::get_thread_id()==0){
    local_result = std::inner_product(pr1.local_begin(), 
				      pr1.local_end(),
				      pr2.local_begin(), 
				      init,
				      binary_op1,
				      binary_op2);
  }
  else{
    local_result = std::inner_product(pr1.local_begin(), 
				      pr1.local_end(),
				      pr2.local_begin(), 
				      (T)(0),
				      binary_op1,
				      binary_op2);
  }
  stapl::rmi_fence();
  
  //get the final result
  if (stapl::get_num_threads() == 1){
    init = local_result;
  }
  else{
    _p_inner_product<PRange,BinaryFunction1> _pinner;
    _pinner.reduce(local_result,binary_op1);
    init = _pinner.get_result();
  }
  return init;
}

///@ingroup pAlgorithms  
///@name p_inner_product 
///@brief inner product of more than 2 PRanges using regular multiplication and addition
///@input vector<PRange>, T init
template<class PRange>
typename iterator_traits<typename PRange::iteratorType>::value_type 
p_inner_product(std::vector<PRange>& vec,
		typename iterator_traits<typename PRange::iteratorType>::value_type init){  
  int vec_size = vec.size();
  if (vec_size < 2){
    cout<<"Inner product is not defined on less than 2 pranges!!"<<endl;
  }
  else{
    if (vec_size == 2){
      init = p_inner_product(vec[0],vec[1],init);
    }
    else{
      //multiply locally and add
      typedef typename iterator_traits<typename PRange::iteratorType>::value_type T;
      typedef typename PRange::iteratorType ITER;
      T local_result = (T)0;
      
      vector<ITER> iter(vec_size);
      for (int i=0; i<vec_size; i++){
	iter[i] = vec[i].local_begin();
      }
      ITER eit = vec[0].local_end();
      
      while (iter[0] != eit){
	T temp = *iter[0];
	for (int i=1; i<vec_size; i++){
	  temp *= *iter[i];
	  ++iter[i];
	}
	local_result += temp;
	++iter[0];
      }
      stapl::rmi_fence();
      
      //add globally
      if (stapl::get_num_threads() == 1){
	init = local_result;
      }
      else{
	_p_inner_product<PRange> _pinner;
	_pinner.reduce(local_result);
	init += _pinner.get_result();
      }
    }
  }
  return init;
}


///@ingroup pAlgorithms  
///@name p_inner_product 
///@brief inner product of more than 2 PRanges using regular multiplication and addition
///@input vector<PRange>, T init
template<class PRange, class BinaryFunction1, class BinaryFunction2>
typename iterator_traits<typename PRange::iteratorType>::value_type 
p_inner_product(std::vector<PRange>& vec,
		typename iterator_traits<typename PRange::iteratorType>::value_type init,
		BinaryFunction1 binary_op1, //addition
                BinaryFunction2 binary_op2){//multiplication    
  int vec_size = vec.size();
  if (vec_size < 2){
    cout<<"Inner product is not defined on less than 2 pranges!!"<<endl;
  }
  else{
    if (vec_size == 2){
      init = p_inner_product(vec[0],vec[1],init,binary_op1,binary_op2);
    }
    else{
      //multiply locally and add
      typedef typename iterator_traits<typename PRange::iteratorType>::value_type T;
      typedef typename PRange::iteratorType ITER;
      T local_result = (T)0;
      
      vector<ITER> iter(vec_size);
      for (int i=0; i<vec_size; i++){
	iter[i] = vec[i].local_begin();
      }
      ITER eit = vec[0].local_end();
      
      while (iter[0] != eit){
	T temp = *iter[0];
	for (int i=1; i<vec_size; i++){
	  temp = binary_op2(temp,*iter[i]);
	  ++iter[i];
	}
	local_result = binary_op1(local_result,temp);
	++iter[0];
      }
      stapl::rmi_fence();
      
      //add globally
      if (stapl::get_num_threads() == 1){
	init = local_result;
      }
      else{
	_p_inner_product<PRange,BinaryFunction1> _pinner;
	_pinner.reduce(local_result,binary_op1);
	init = binary_op1(init,_pinner.get_result());
      }
    }
  }
  return init;
}

///@ingroup pAlgorithms  
///@name p_inner_product 
///@brief inner product of more than 2 PRanges using regular multiplication and addition
///@input vector<PRange*>, T init
template<class PRange>
typename iterator_traits<typename PRange::iteratorType>::value_type 
p_inner_product(std::vector<PRange*>& vec,
		typename iterator_traits<typename PRange::iteratorType>::value_type init){  
  int vec_size = vec.size();
  if (vec_size < 2){
    cout<<"Inner product is not defined on less than 2 pranges!!"<<endl;
  }
  else{
    if (vec_size == 2){
      init = p_inner_product(*vec[0],*vec[1],init);
    }
    else{
      //multiply locally and add
      typedef typename iterator_traits<typename PRange::iteratorType>::value_type T;
      typedef typename PRange::iteratorType ITER;
      T local_result = (T)0;
      
      vector<ITER> iter(vec_size);
      for (int i=0; i<vec_size; i++){
	iter[i] = vec[i]->local_begin();
      }
      ITER eit = vec[0]->local_end();
      
      while (iter[0] != eit){
	T temp = *iter[0];
	for (int i=1; i<vec_size; i++){
	  temp *= *iter[i];
	  ++iter[i];
	}
	local_result += temp;
	++iter[0];
      }
      stapl::rmi_fence();
      
      //add globally
      if (stapl::get_num_threads() == 1){
	init = local_result;
      }
      else{
	_p_inner_product<PRange> _pinner;
	_pinner.reduce(local_result);
	init += _pinner.get_result();
      }
    }
  }
  return init;
}

///@ingroup pAlgorithms  
///@name p_inner_product 
///@brief inner product of more than 2 PRanges using regular multiplication and addition
///@input vector<PRange*>, T init
template<class PRange, class BinaryFunction1, class BinaryFunction2>
typename iterator_traits<typename PRange::iteratorType>::value_type 
p_inner_product(std::vector<PRange*>& vec,
		typename iterator_traits<typename PRange::iteratorType>::value_type init,
		BinaryFunction1 binary_op1, //addition
                BinaryFunction2 binary_op2){//multiplication    
  int vec_size = vec.size();
  if (vec_size < 2){
    cout<<"Inner product is not defined on less than 2 pranges!!"<<endl;
  }
  else{
    if (vec_size == 2){
      init = p_inner_product(*vec[0],*vec[1],init,binary_op1,binary_op2);
    }
    else{
      //multiply locally and add
      typedef typename iterator_traits<typename PRange::iteratorType>::value_type T;
      typedef typename PRange::iteratorType ITER;
      T local_result = (T)0;
      
      vector<ITER> iter(vec_size);
      for (int i=0; i<vec_size; i++){
	iter[i] = vec[i]->local_begin();
      }
      ITER eit = vec[0]->local_end();
      
      while (iter[0] != eit){
	T temp = *iter[0];
	for (int i=1; i<vec_size; i++){
	  temp = binary_op2(temp,*iter[i]);
	  ++iter[i];
	}
	local_result = binary_op1(local_result,temp);
	++iter[0];
      }
      stapl::rmi_fence();
      
      //add globally
      if (stapl::get_num_threads() == 1){
	init = local_result;
      }
      else{
	_p_inner_product<PRange,BinaryFunction1> _pinner;
	_pinner.reduce(local_result,binary_op1);
	init = binary_op1(init,_pinner.get_result());
      }
    }
  }
  return init;
}


#endif
