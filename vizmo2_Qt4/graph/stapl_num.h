#ifndef STAPL_NUM_H
#define STAPL_NUM_H

#include <stdlib.h>
#include <iostream>
#include <runtime.h>

template<class T>
class stapl_num {
private:
  T val;
  stapl::rmiHandle handle;

  void sum(T* in, T* inout) { *inout += *in; }

public:

  stapl_num() { handle = stapl::register_rmi_object(this); }
  ~stapl_num() { stapl::unregister_rmi_object(handle); }

  T value() { return val; }

  T reduce() {
    T result;
    stapl::reduce_rmi( &val, &result, handle, &stapl_num<T>::sum, true);
    val = result;
    return val;
  }

  inline stapl_num& operator= (const stapl_num& x) {
    val = x.val;
    return *this;
  }

  inline T& operator= (const T& x) {
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
