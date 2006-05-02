#ifndef STAPL_BOOL_H
#define STAPL_BOOL_H

#include <stdlib.h>
#include <iostream>
#include <runtime.h>

class stapl_bool {
private:
  bool val;
  stapl::rmiHandle handle;

  void And(bool* in, bool* inout) { *inout &= *in; }

public:

  stapl_bool() { handle = stapl::register_rmi_object(this); }
  stapl_bool(bool v) : val(v) { handle = stapl::register_rmi_object(this); }
  ~stapl_bool() { stapl::unregister_rmi_object(handle); }

  inline bool value() { return val; }

  bool reduce() {
    bool result;
    stapl::reduce_rmi( &val, &result, handle, &stapl_bool::And, true);
    val = result;
    return val;
  }

  inline stapl_bool& operator= (const stapl_bool& x) {
    val = x.val;
    return *this;
  }

  inline stapl_bool& operator= (const bool& x) {
    val = x;
    return *this;
  }

};


class stapl_int {
private:
  int val;
  stapl::rmiHandle handle;

  void Sum(int* in, int* inout) { *inout += *in; }

public:

  stapl_int() { handle = stapl::register_rmi_object(this); }
  stapl_int(int v) : val(v) { handle = stapl::register_rmi_object(this); }
  ~stapl_int() { stapl::unregister_rmi_object(handle); }

  inline int value() { return val; }

  int reduce() {
    int result;
    stapl::reduce_rmi( &val, &result, handle, &stapl_int::Sum, true);
    val = result;
    return val;
  }

  inline stapl_int& operator= (const stapl_int& x) {
    val = x.val;
    return *this;
  }

  inline stapl_int& operator= (const int& x) {
    val = x;
    return *this;
  }

};
#endif
