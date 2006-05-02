#ifndef STAPL_VECTOR_H
#define STAPL_VECTOR_H

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <runtime.h>

template<class T>
class stapl_vector {
private:
  std::vector<T> val;
  stapl::rmiHandle handle;

  void sum(std::vector<T>* in, std::vector<T>* inout) { 
    typename std::vector<T>::iterator j = in->begin();
    for (typename std::vector<T>::iterator i  = inout->begin();
	 i != inout->end(); ++i, ++j) {
      *i += *j;
    }
  }

  void push(std::vector<T>* in, std::vector<T>* inout) { 
    for (typename std::vector<T>::iterator i = in->begin();
	 i != in->end(); ++i) {
      inout->push_back(*i);
    }
  }

public:

  stapl_vector() { handle = stapl::register_rmi_object(this); }

  stapl_vector(const int& size) : val(size) { 
    handle = stapl::register_rmi_object(this); 
  }

  stapl_vector(const int& size, const T& v) : val(size, v) { 
    handle = stapl::register_rmi_object(this); 
  }

  ~stapl_vector() { stapl::unregister_rmi_object(handle); }

  std::vector<T>& value() { return val; }

  std::vector<T> reduce(int rootThread = 0) {
    std::vector<T> result(val.size(),0);
    stapl::reduce_rmi( &val, &result, handle, &stapl_vector<T>::sum, true, rootThread);
    val = result;
    return val;
  }

  std::vector<T> combine(int rootThread = 0) {
    std::vector<T> result;
    stapl::reduce_rmi( &val, &result, handle, &stapl_vector<T>::push, true, rootThread);
    val = result;
    return val;
  }

  inline stapl_vector<T>& operator= (const stapl_vector<T>& x) {
    val = x.val;
    return *this;
  }

  inline std::vector<T>& operator= (const std::vector<T>& x) {
    val = x;
    return val;
  }
};

/*
void stapl_main(int argc, char **argv) {

  int id = stapl::get_thread_id();
  stapl_num<int> x;

  x = id;
  int red = x.reduce();
  std::cout << "sum of thread ids = " << red << "\n";

  id = x.value();
}
*/
#endif
