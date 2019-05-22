/*
 // Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
 // component of the Texas A&M University System.

 // All rights reserved.

 // The information and source code contained herein is the exclusive
 // property of TEES and may not be disclosed, examined or reproduced
 // in whole or in part without explicit written authorization from TEES.
 */

#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>
#include <stapl/runtime.hpp>
#include <stapl/array.hpp>
#include <stapl/algorithms/functional.hpp>
#include <stapl/views/counting_view.hpp>
#include <stapl/views/functor_view.hpp>
#include <stapl/utility/do_once.hpp>
//includes from skeletons
#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/skeletons/executors/algorithm_executor.hpp>
#include <stapl/views/bitreversed_view.hpp>
#include <stapl/skeletons/functional/allreduce.hpp>
#include <stapl/skeletons/functional/broadcast.hpp>
#include <stapl/skeletons/functional/broadcast_to_locs.hpp>
#include <stapl/skeletons/functional/butterfly.hpp>
#include <stapl/skeletons/functional/copy.hpp>
#include <stapl/skeletons/functional/fft.hpp>
#include <stapl/skeletons/functional/inner_product.hpp>
#include <stapl/skeletons/functional/map.hpp>
#include <stapl/skeletons/functional/map_reduce.hpp>
#include <stapl/skeletons/functional/reduce.hpp>
#include <stapl/skeletons/functional/reduce_to_locs.hpp>
#include <stapl/skeletons/functional/scan.hpp>
#include <stapl/skeletons/functional/sink.hpp>
#include <stapl/skeletons/functional/sink_value.hpp>
#include <stapl/skeletons/functional/zip.hpp>
#include <stapl/skeletons/operators/do_while.hpp>
#include <stapl/skeletons/operators/repeat.hpp>
#include <stapl/skeletons/operators/compose.hpp>
#ifdef GRAPHVIZ_OUTPUT
#include <stapl/skeletons/environments/graphviz_env.hpp>
#endif
#include "../test_report.hpp"

STAPL_IS_BASIC_TYPE(std::complex<double>)

using namespace stapl;

namespace {

enum FFT_Direction{FORWARD_FFT, BACKWARD_FFT};

template <typename View>
void print_values(View& view, std::string title, std::ostream& o = std::cout)
{
#ifdef SHOW_RESULTS
  do_once([&view, &title, &o](void) {
    o << "\n" << title << "(size = " << view.size() << ")\n";
    std::copy(view.begin(), view.end(),
              std::ostream_iterator<typename View::value_type> (o, ", "));
    o << "\n";
  });
#endif
}

template <typename T>
void print_value(T& t, std::string title, std::ostream& o = std::cout)
{
#ifdef SHOW_RESULTS
  do_once([&t, &title, &o](void) {
    o << "\n" << title << ")\n";
    o << t << "\n";
  });
#endif
}

//////////////////////////////////////////////////////////////////////
/// @brief A generator for complex numbers which produces [0+0j, 1+0j,
/// ..., n-1+0j]
///
/// @tparam T the type of real part of the generated complex number
//////////////////////////////////////////////////////////////////////
template <typename T>
struct complex_seq
{
private:
  typedef std::complex<T> value_t;
  value_t m_init_v;
public:
  typedef std::size_t     index_type;
  typedef value_t         result_type;
  complex_seq(std::complex<T> init_v)
    : m_init_v(init_v)
  { }

  value_t operator()(index_type const& idx) const
  {
    return value_t(idx) + m_init_v;
  }

  void define_type(typer& t)
  {
    t.member(m_init_v);
  }
};


void test_fft(std::size_t n)
{
  using namespace skeletons;
  typedef double                           component_t;
  typedef std::complex<component_t>        value_t;
  typedef array<value_t>                   array_t;
  typedef array_view<array_t>              array_vt;
  typedef indexed_domain<size_t>           dom_t;
  typedef balanced_partition<dom_t>        partition_t;
  partition_t p_bal(dom_t(0, n-1), n);

  array_t  input_array(p_bal);
  array_t  output_array(p_bal);
  array_vt input_view(input_array);
  array_vt output_view(output_array);

  // create an incremental complex input (0 + 0j), (1 + 0j), ..., (n-1 + 0j)
  complex_seq<component_t> seq_gen(value_t(0, 0));

  algorithm_executor<null_coarsener> exec;

  exec.execute(skeletons::zip(stapl::assign<value_t>()),
               functor_view(n, seq_gen), input_view);

  print_values(input_view, "Input");

  // FFT DIF (Decimation In Frequency)
  exec.execute(sink<value_t>(
                 butterfly<true>(skeletons_impl::fft_dif_wf<component_t>())),
               input_view, bitreversed_view(output_view, n));

  print_values(output_view, "FFT DIF Result using bitreversed_view");

  // FFT DIT (Decimation In Time)
  exec.execute(sink<value_t>(
                 reverse_butterfly<true>(
                   skeletons_impl::fft_dit_wf<component_t>())),
               bitreversed_view(input_view, n), output_view);

  print_values(output_view, "FFT DIT Result using bitreversed_view");

  exec.execute(sink<value_t>(skeletons::fft<component_t, tags::dit>()),
               input_view, output_view);

  print_values(output_view, "FFT skeleton DIT Result");

  exec.execute(sink<value_t>(skeletons::fft<component_t,tags::dif>()),
               input_view, output_view);

  print_values(output_view, "FFT skeleton DIF Result");
}

template <typename T>
struct continutation_cond
{
  template <typename Element>
  bool operator()(Element & element) const
  {
#ifdef SHOW_RESULTS
    if (get_location_id() == 0)
      std::cout << element << " < 100 " << (element < 100) << std::endl;
#endif
    return element < 100;
  }
};

void test_all(std::size_t n)
{
  using namespace skeletons;
  using value_t     = int;
  using container_t = array<value_t>;
  using view_t      = array_view<container_t>;
  using ref_t       = view_t::reference;

  container_t input_array(n);
  container_t output_array(n);
  view_t      in_view(input_array);
  view_t      out_view(output_array);

  container_t n_to_p_results(get_num_locations());
  view_t n_to_p_results_view(n_to_p_results);

  // NOTICE:
  // Known fact. You might notice that when you call an in place algorithm
  // your algorithms works fine and correctly once and then the other time it
  // doesn't on more than one processor. This is due to the fact that each
  // processor will handle the tasks separately in STAPL. In fact, it might
  // happen that one processor starts reading input values later than another
  // processor writes its value back in the input.
  auto cv = counting_view<value_t> (n, value_t(0));

#ifdef GRAPHVIZ_OUTPUT
  typedef algorithm_executor<null_coarsener, graphviz_env> exec_t;
#else
  algorithm_executor<null_coarsener> exec;
#endif

  stapl::plus<value_t>     plus_i;

  using skeletons::map;
  using skeletons::map_reduce;
  using skeletons::scan;
  using skeletons::reduce;

  auto reduce_p         = reduce(plus_i);
  auto allreduce1_p     = allreduce(plus_i);
  auto allreduce2       = allreduce<tags::butterfly<>>(plus_i);
  auto allreduce3       = allreduce<tags::reverse_butterfly<>>(plus_i);
  auto allreduce4       = allreduce<tags::right_aligned>(plus_i);
  auto allreduce5       = allreduce<tags::left_aligned>(plus_i);
  auto inclusive_scan1  = scan(plus_i);
  auto inclusive_scan2  = scan<tags::hillis_steele>(plus_i);
  auto exclusive_scan1  = scan<tags::blelloch>(plus_i, 0);
  auto map_p            = map(stapl::bind1st(plus_i, 2));
  auto simple1_p        = repeat(allreduce1_p, lazysize(3));
  auto simple2_p        = compose(map_p, map_p);
  auto map_reduce_p     = map_reduce(stapl::bind1st(plus_i, 2), plus_i);
  auto map_reduce_to_p  = compose(map_reduce_p, broadcast_to_locs());
  auto red_to_locs_p    = reduce_to_locs(plus_i);
  auto do_while_pattern = do_while(map_p, red_to_locs_p,
                                   continutation_cond<value_t>());
  auto inner_product_p  = compose(
                            skeletons::inner_product<value_t>(), broadcast_to_locs());

#ifdef GRAPHVIZ_OUTPUT
  exec_t exec(graphviz_env("copy_" + std::to_string(n)));
#endif
  exec.execute(skeletons::copy<value_t>(), cv, in_view);
  print_values(in_view, "copy = elem(assign<T>()){in, counting_view}");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &cv](void) {
    bool is_valid = std::equal(in_view.begin(), in_view.end(), cv.begin());
    STAPL_TEST_REPORT(is_valid, "copy");
  });
#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("do_while_" + std::to_string(n)));
#endif
  exec.execute(sink<value_t>(do_while_pattern), in_view, out_view);
  print_values(
    out_view,
    "do_while = do_while(map(+2), reduce_to_p(+), continueif(< 100))");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &out_view, n](void) {
    std::vector<value_t> result(n);
    std::copy(in_view.begin(), in_view.end(), result.begin());
    value_t red_value;
    do {
      std::transform(result.begin(), result.end(), result.begin(),
                     boost::bind(std::plus<value_t>(), _1, 2));
      red_value = std::accumulate(result.begin(), result.end(), 0);
    } while (red_value < 100);
    bool is_valid = std::equal(out_view.begin(), out_view.end(),
                               result.begin());
    STAPL_TEST_REPORT(is_valid,
                      "do_while(map(+2), redce_to_p(+), continueif(< 100))");
  });
#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("reduce(+)_" + std::to_string(n)));
#endif
  exec.execute(reduce_p, in_view);
  print_values(out_view, "reduce(+) = binary_tree(+)");
  //there is no validation in this case since the value is reduced to the result
  //of a task, it is not propagated to other processors

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("allreduce1(+)_" + std::to_string(n)));
#endif
  exec.execute(sink<value_t>(allreduce1_p), in_view, out_view);
  print_values(out_view, "allreduce1(+) = "
                         "compose(reduce(+), broadcast(identity)");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &out_view](void) {
    value_t red_value = std::accumulate(in_view.begin(), in_view.end(), 0);
    bool is_valid = find_if(out_view.begin(), out_view.end(),
                            [red_value](ref_t element) {
                              return element != red_value;
                            }) == out_view.end();
    STAPL_TEST_REPORT(is_valid, "allreduce1(+)");
  });
#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("allreduce2(+)_" + std::to_string(n)));
#endif
  exec.execute(sink<value_t>(allreduce2), in_view, out_view);
  print_values(out_view, "allreduce2(+) = butterfly(+) = "
                         "repeat_log_2(elem(butterfly_pd(+)))");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &out_view](void) {
    value_t red_value = std::accumulate(in_view.begin(), in_view.end(), 0);
    bool is_valid = find_if(out_view.begin(), out_view.end(),
                            [red_value](ref_t element) {
                              return element != red_value;
                            }) == out_view.end();

    STAPL_TEST_REPORT(is_valid, "allreduce2(+)");
  });
#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("allreduce3(+)_" + std::to_string(n)));
#endif
  exec.execute(sink<value_t>(allreduce3), in_view, out_view);
  print_values(out_view, "allreduce3(+) = reverse_butterfly(+) = "
                         "repeat_log_2(elem(reverse_butterfly_pd(+)))");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &out_view](void) {
    value_t red_value = std::accumulate(in_view.begin(), in_view.end(), 0);
    bool is_valid = find_if(out_view.begin(), out_view.end(),
                            [red_value](ref_t element) {
                              return element != red_value;
                            }) == out_view.end();

    STAPL_TEST_REPORT(is_valid, "allreduce3(+)");
  });
#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("allreduce4(+)_" + std::to_string(n)));
#endif
  exec.execute(sink<value_t>(allreduce4), in_view, out_view);
  print_values(out_view, "allreduce4(+) = compose(reduce(+, right_aligned), "
                                             "broadcast(id, right_aligned))");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &out_view](void) {
    value_t red_value = std::accumulate(in_view.begin(), in_view.end(), 0);
    bool is_valid = find_if(out_view.begin(), out_view.end(),
                            [red_value](ref_t element) {
                              return element != red_value;
                            }) == out_view.end();

    STAPL_TEST_REPORT(is_valid, "allreduce4(+)");
  });
#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("allreduce5(+)_" + std::to_string(n)));
#endif
  exec.execute(sink<value_t>(allreduce5), in_view, out_view);
  print_values(out_view, "allreduce5(+) = compose(reduce(+, left_aligned), "
                                             "broadcast(id, left_aligned))");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &out_view](void) {
    value_t red_value = std::accumulate(in_view.begin(), in_view.end(), 0);
    bool is_valid = find_if(out_view.begin(), out_view.end(),
                            [red_value](ref_t element) {
                              return element != red_value;
                            }) == out_view.end();

    STAPL_TEST_REPORT(is_valid, "allreduce5(+)");
  });
#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("repeat(allredcue1(+))_" +
                                  std::to_string(n)));
#endif
  exec.execute(sink<value_t>(simple1_p), in_view, out_view);
  print_values(out_view, "repeat(allreduce1(+), 3)");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &out_view, n](void) {
    value_t red_value = std::accumulate(in_view.begin(), in_view.end(), 0);
    red_value = red_value * n * n;
    bool is_valid = find_if(out_view.begin(), out_view.end(),
                            [red_value](ref_t element) {
                              return element != red_value;
                            }) == out_view.end();
    STAPL_TEST_REPORT(is_valid, "repeat(allreduce1(+), 3)");
  });
#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("map(+2)_" + std::to_string(n)));
#endif
  exec.execute(sink<value_t>(map_p), in_view, out_view);
  print_values(out_view, "map(+2)");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &out_view](void) {
    bool is_valid = std::equal(out_view.begin(), out_view.end(),
                               in_view.begin(),
                               [](ref_t out, ref_t in) {
                                 return out == (in + 2);
                               });
    STAPL_TEST_REPORT(is_valid, "map(+2)");
  });
#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(
    graphviz_env("compose(map(+2),map(+2))_" + std::to_string(n)));
#endif
  exec.execute(sink<value_t>(simple2_p), in_view, out_view);
  print_values(out_view, "compose(map(+2), map(+2))");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &out_view](void) {
    bool is_valid = std::equal(out_view.begin(), out_view.end(),
                               in_view.begin(),
                               [](ref_t out, ref_t in) {
                                 return out == (in + 4);
                               });
    STAPL_TEST_REPORT(is_valid, "compose(map(+2), map(+2))");
  });
#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("map_reduce_to_p(+2,+)_" +
                                  std::to_string(n)));
#endif
  exec.execute(sink<value_t, spans::per_location>(map_reduce_to_p),
       in_view, n_to_p_results_view);
  print_values(out_view, "compose(map_reduce(+, +2), broadcast_to_locs(id))");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &n_to_p_results_view](void) {
    std::vector<value_t> result_test(n_to_p_results_view.size());
    value_t red_value = std::accumulate(in_view.begin(),
                                        in_view.end(),
                                        value_t(0),
                                        [](value_t in1, value_t in2) {
                                          return in1 + (in2 + 2);
                                        });
    bool is_valid = find_if(n_to_p_results_view.begin(),
                            n_to_p_results_view.end(),
                            [red_value](ref_t element) {
                               return element != red_value;
                            }) == n_to_p_results_view.end();
    STAPL_TEST_REPORT(is_valid,
                      "compose(map_reduce(+, +2), broadcast_to_locs(id))");
  });
#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("scan(+)_" + std::to_string(n)));
#endif
  exec.execute(sink<value_t>(inclusive_scan1), in_view, out_view);
  print_values(out_view, "scan(+)");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &out_view, n](void) {
    std::vector<value_t> result_test(n);
    std::partial_sum(in_view.begin(), in_view.end(), result_test.begin());
    bool is_valid = std::equal(out_view.begin(), out_view.end(),
                               result_test.begin());
    STAPL_TEST_REPORT(is_valid, "jaja_scan(+)");
  });
#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("hillis_steele_scan(+)_" +
                                  std::to_string(n)));
#endif
  exec.execute(sink<value_t>(inclusive_scan2), in_view, out_view);
  print_values(out_view, "hillis_steele_scan(+)");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &out_view, n](void) {
    std::vector<value_t> result_test(n);
    std::partial_sum(in_view.begin(), in_view.end(), result_test.begin());
    bool is_valid = std::equal(out_view.begin(), out_view.end(),
                               result_test.begin());
    STAPL_TEST_REPORT(is_valid, "hillis_steele_scan(+)");
  });
#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("blelloch_scan(+)_" + std::to_string(n)));
#endif
  exec.execute(sink<value_t>(exclusive_scan1), in_view, out_view);
  print_values(out_view, "blelloch_scan(+)");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &out_view, n ](void) {
    std::vector<value_t> result_test(n);
    std::partial_sum(in_view.begin(), in_view.end(), result_test.begin());
    bool is_valid = std::equal(++out_view.begin(), out_view.end(),
                               result_test.begin()) &&
                    (*out_view.begin() == 0);
    STAPL_TEST_REPORT(is_valid, "blelloch_scan(+)");
  });

#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("reduce_n_to_p(+)_" + std::to_string(n)));
#endif
  exec.execute(sink<value_t, spans::per_location>(red_to_locs_p),
       in_view, n_to_p_results_view);
  print_values(n_to_p_results,
               "reduce_n_to_p(+) = compose(reduce(+), broadcast<to_locations>)"
              );
#ifdef VALIDATE_RESULTS
  do_once([&in_view, &n_to_p_results_view](void) {
    value_t red_value = std::accumulate(in_view.begin(), in_view.end(), 0);
    bool is_valid = find_if(n_to_p_results_view.begin(),
                            n_to_p_results_view.end(),
                            [red_value](ref_t element) {
                              return element != red_value;
                            }) == n_to_p_results_view.end();
    STAPL_TEST_REPORT(is_valid, "red_n_to_p(+)");
  });
#endif

#ifdef GRAPHVIZ_OUTPUT
  exec.set_extra_env(graphviz_env("inner_product_" + std::to_string(n)));
#endif
  value_t red_value = exec.execute<value_t>(
                        sink_value<value_t>(inner_product_p),
                        in_view, in_view);
  print_value(red_value, "inner_product = zip_reduce(*, +)");
#ifdef VALIDATE_RESULTS
  do_once([&in_view, red_value](void) {
    bool is_valid = red_value ==
                    std::accumulate(
                      in_view.begin(),
                      in_view.end(),
                      (value_t) 0,
                      [](value_t in1, value_t in2) {
                        return in1 + in2*in2;
                      });
    STAPL_TEST_REPORT(is_valid, "inner_product()");
  });
#endif
}

} // namespace

stapl::exit_code stapl_main(int argc, char* argv[])
{
  if (argc < 2){
    std::cout << "<exec> <nElem>" << std::endl;
    exit(1);
  }
  size_t n = atol(argv[1]);
  test_all(n);
  test_fft(n);
  return EXIT_SUCCESS;
}
