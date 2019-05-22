/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_PROFILERS_VALUE_TYPE_HPP
#define STAPL_PROFILERS_VALUE_TYPE_HPP

#include <complex>
#include <iostream>
#include <stapl/algorithms/numeric.hpp>
#include <stapl/algorithms/identity_value.hpp>

////////////////////////////////////////////////////////////////////////////////
/// @brief A type that houses a container of elements and necessary operations.
///
/// @tparam The container type
/// @tparam N The size value type; int if unspecified
/// @tparam DL The time delay value type; int if unspecified
////////////////////////////////////////////////////////////////////////////////
template<class T, int N, int DL>
class my_variable
{
public:
  /// The container of elements
  T a[N];
  /// The counter value
  mutable size_t counter;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief Initializes the container's elements to 0.
  //////////////////////////////////////////////////////////////////////////////
  my_variable(void)
  {
    for (size_t i=0; i<N; ++i)
      a[i] = 0;
    counter = 0;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// @brief Initializes the container's elements to a value @p v.
  ///
  /// @param v The value to initialize the container's elements to
  //////////////////////////////////////////////////////////////////////////////
  my_variable(size_t v)
  {
    for (size_t i=0; i<N; ++i)
      a[i] = v;
    counter = 0;
  }

  my_variable& operator=(size_t v)
  {
    for (size_t i=0; i<N; ++i)
      a[i] = v;
    return *this;
  }

  bool operator==(my_variable const& other) const
  {
    bool eql=true;
    for (size_t i=0; i<N; ++i)
      if (a[i] != other.a[i])
        eql = false;
    return eql;
  }

  void operator+=(size_t v)
  {
    for (size_t i=0; i<N; ++i)
      a[i] += v;
  }

  my_variable operator+(size_t v) const
  {
    my_variable temp;
    for (size_t i=0; i<N; ++i)
      temp.a[i] = this->a[i] + v;
    return temp;
  }

  void operator+=(my_variable const& v)
  {
    for (size_t i=0; i<N; ++i)
      a[i] += v.a[i];
  }

  void define_type(stapl::typer& t)
  {
    t.member(a);
  }

  void delay() const
  {
    for (int i=0; i<DL; ++i)
      this->counter += (i / (lrand48()+1));
  }
};


namespace stapl {

////////////////////////////////////////////////////////////////////////////////
/// @brief A proxy for the my_variable type.
////////////////////////////////////////////////////////////////////////////////
template <class T, int N, int DL, typename Accessor>
class proxy<my_variable<T, N, DL>, Accessor>
  : public Accessor
{
private:
  friend class proxy_core_access;
  typedef my_variable<T, N, DL> target_t;

public:

  explicit proxy(Accessor const& acc)
  : Accessor(acc)
  { }

  operator target_t() const
  { return Accessor::read(); }

  proxy const& operator=(proxy const& rhs)
  { Accessor::write(rhs); return *this; }

  proxy const& operator=(target_t const& rhs)
  { Accessor::write(rhs); return *this;}

  void operator+=(size_t v)
  { Accessor::invoke(&target_t::operator+=, v); }

  void operator+=(target_t const& v)
  { Accessor::invoke(&target_t::operator+=, v); }

  target_t operator+(size_t v)
  { return Accessor::const_invoke(&target_t::operator+, v); }

}; //struct proxy
}

template<class T, int N, int DL>
std::ostream& operator<<(std::ostream& out, my_variable<T,N,DL> const& l)
{
  return out;
}

#endif
