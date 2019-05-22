/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <iostream>

#include <stapl/algorithms/functional.hpp>
#include <stapl/algorithms/algorithm.hpp>

#include <stapl/views/array_view.hpp>
#include <stapl/views/array_ro_view.hpp>
#include <stapl/views/list_view.hpp>
#include <stapl/views/counting_view.hpp>

#include <stapl/containers/array/array.hpp>
#include <stapl/list.hpp>

#include "../../test_report.hpp"

using namespace stapl;

struct rand_gen_cont
  : public p_object
{
  typedef int value_type;

  typedef value_type              reference;
  typedef const value_type        const_reference;
  typedef indexed_domain<size_t>  domain_type;
  typedef size_t                  gid_type;

  size_t m_size;

  rand_gen_cont(size_t size)
    : m_size(size)
  {
    srand(time(NULL)+(get_location_id()+100)*3+get_location_id());
  }

  const_reference operator[](size_t n) const
  {
    if ( (n==(m_size-1)) && (n== (m_size-2)))
      return 99;
    else
      return (rand()%10);
  }

  const_reference get_element(size_t n) const
  {
    if ( (n==(m_size-1)) && (n== (m_size-2)))
      return 99;
    else
      return (rand()%10);
  }

  template <typename Functor>
  const_reference apply_get(size_t idx, Functor const& f) const
  {
    return f(get_element(idx));
  }

  size_t size(void) const
  {
    return m_size;
  }

  domain_type domain(void) const
  {
    return domain_type(0, m_size-1);
  }

  void define_type(typer& t)
  {
    t.member(m_size);
  }

  size_t version(void) const
  {
    return 0;
  }

};


stapl::exit_code stapl_main(int argc, char* argv[])
{
  size_t n = 100;
  if (argc == 2) {
    n = atoi(argv[1]);
    if (n<2) {
      do_once([](void) {
        std::cout << "Problem size too small, should be greater than 2\n";
      });
      exit(0);
    }
  }

  typedef array<int>                            parray_t;
  typedef array_view<parray_t>                  view_t;
  typedef view_t::reference                     ref_t;
  typedef array_ro_view<rand_gen_cont>          randGenView;

  // p_array test
  parray_t pa(n);
  view_t view(pa);
  rand_gen_cont rgc(n);
  randGenView rgenv(rgc);
  copy(rgenv,view);

  ref_t ref1 = adjacent_find(view);
  bool result1 = !is_null_reference(ref1);

  copy(counting_view<int>(n),view);
  ref_t ref2 = adjacent_find(view);
  bool result2 = is_null_reference(ref2);


  STAPL_TEST_REPORT(result1 && result2, "Testing adjacent_find over p_array")

  //TODO: Uncomment this code when problems of promotion and plist is resolved
  // list test

  //typedef list<int>          list_t;
  //typedef list_view<list_t>  viewl_t;
  //typedef viewl_t::reference refl_t;

  //list_t pl(n);
  //viewl_t viewl(pl);
  //copy(rgenv,viewl);

  //do_once(std::cout << constant("Testing adjacent_find over list: "));
  //refl_t ref3 = adjacent_find(viewl);
  //bool result3 = !is_null_reference(ref3);

  //copy(counting_view<int>(n),viewl);
  //refl_t ref4 = adjacent_find(viewl);
  //bool result4 = is_null_reference(ref4);
  //
  //do_once(if_then_else(constant(result3 && result4),
  //       std::cout << constant(": PASSED\n"),
  //       std::cout << constant(": FAILED\n")
  //       )
  //  );

  return EXIT_SUCCESS;
}
