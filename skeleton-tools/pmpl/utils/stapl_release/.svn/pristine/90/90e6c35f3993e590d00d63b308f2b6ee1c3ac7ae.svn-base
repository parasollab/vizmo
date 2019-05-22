/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <cstdlib>
#include <string>
#include <iostream>
#include <stapl/array.hpp>
#include <stapl/views/array_view.hpp>
#include <stapl/algorithms/algorithm.hpp>
#include <stapl/views/overlap_view.hpp>
#include <stapl/runtime.hpp>
#include <boost/random/uniform_int_distribution.hpp>

using namespace std;

typedef stapl::array<int>                                 ar_int;
typedef stapl::array_view<ar_int>                         vw_ar_int;
typedef stapl::overlap_view<vw_ar_int>                    ov_vw_ar_int;

template<typename Value>
struct msg_val
{
private:
  string m_txt;
  Value m_val;
public:
  msg_val(string text, Value val)
    : m_txt(text), m_val(val)
  { }

  typedef void result_type;
  result_type operator()()
  {
    cout << m_txt << " " << m_val << endl;
  }
  void define_type(stapl::typer& t)
  {
    t.member(m_txt);
    t.member(m_val);
  }
};

struct rand_gen
{
  boost::random::mt19937 m_rng;
  typedef boost::random::uniform_int_distribution<size_t> rng_dist_t;

  rand_gen(unsigned int seed = 42)
    : m_rng(seed)
  { }

  size_t rand(size_t min, size_t max)
  { return rng_dist_t(min, max)(m_rng); }
};

struct fill_arr
{
  typedef void result_type;
  template<typename Ref>
  void operator()(Ref arr)
  {
    std::random_device gen;

    rand_gen r(gen());
    arr = r.rand(0,9);
  }
};

struct comp_prod
{
  typedef void result_type;
  template<typename Ref1, typename Ref2>
  void operator()(Ref1 inview, Ref2 outview)
  {
    int temp = 1;

    for (auto x : inview)
      temp *= x;

    outview = temp;
  }
};

stapl::exit_code stapl_main(int argc, char *argv[])
{
  if (argc < 3)
  {
    cout << "Usage: " << argv[0] << " n m";
    exit(1);
  }

  int n = atol(argv[1]);
  int m = atol(argv[2]);

  srand (time(NULL));

  stapl::counter<stapl::default_timer> ctr_tot;
  ctr_tot.start();

  ar_int arr = stapl::array<int>(n);
  vw_ar_int arr_view = stapl::array_view<ar_int>(arr);
  ar_int arr_res = stapl::array<int>(n - m + 1);
  vw_ar_int arr_res_view = stapl::array_view<ar_int>(arr_res);

  stapl::map_func(fill_arr(), arr_view);

  ov_vw_ar_int ov_arr_view
    = stapl::make_overlap_view(arr_view, 1, 0, m - 1);

  stapl::map_func(comp_prod(), ov_arr_view, arr_res_view);
  int max = stapl::max_value(arr_res_view);

  float exec_time = ctr_tot.stop();

  stapl::do_once(msg_val<int>("Maximum Product: ", max));
  stapl::do_once(msg_val<int>("N Value: ", n));
  stapl::do_once(msg_val<int>("M Value: ", m));
  stapl::do_once(msg_val<float>("Execution Time: ", exec_time));

  return EXIT_SUCCESS;
}
