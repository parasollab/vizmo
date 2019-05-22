#include <stapl/containers/vector/vector.hpp>
#include <stapl/containers/array/array.hpp>

#include <stapl/containers/matrix/matrix.hpp>
#include <stapl/containers/multiarray/traversals.hpp>
#include <stapl/views/matrix_view.hpp>
#include <stapl/containers/type_traits/default_traversal.hpp>
#include <stapl/algorithms/algorithm.hpp>
#include <stapl/algorithms/functional.hpp>

#include "viewhelp.hpp"
using namespace std;

#define CONF_INT_REP 32

typedef stapl::indexed_domain<size_t>                     vec_dom_tp;
typedef stapl::balanced_partition<vec_dom_tp>             bal_part_tp;
typedef stapl::tuple<size_t, size_t>                      gid_tp;

typedef stapl::row_major                                  trav2_row_major_tp;
typedef stapl::column_major                               trav2_col_major_tp;

typedef stapl::nd_partition<
          stapl::tuple<bal_part_tp, bal_part_tp>,
          trav2_row_major_tp>                             part2_tp;

typedef stapl::nd_partition<
          stapl::tuple<bal_part_tp, bal_part_tp>,
          trav2_row_major_tp>                             part2_row_major_tp;
typedef stapl::nd_partition<
          stapl::tuple<bal_part_tp, bal_part_tp>,
          trav2_col_major_tp>                             part2_col_major_tp;

stapl::exit_code experiment(int, int, const char *, double *, string *);
bool run_atom_test( size_t, size_t, double *, string *);
bool run_stl_test( size_t, size_t, double *, string *);
bool run_nest_test( size_t, size_t, double *, string *);

struct timeval tp;

char *opt_data = 0;
bool opt_list = false;
bool opt_quiet = false;
int  opt_test = -1;
bool opt_verbose = false;

///////////////////////////////////////////////////////////////////////////
// FEATURE: matrix view
///////////////////////////////////////////////////////////////////////////

stapl::exit_code stapl_main(int argc, char **argv) {

  double atom_times[6] = {0,0,0,0,0,0};
  double stl_times[6] =  {0,0,0,0,0,0};
  double nest_times[6] = {0,0,0,0,0,0};

  string atom_labels[6] = {"", "", "", "", "", ""};
  string stl_labels[6] =  {"", "", "", "", "", ""};
  string nest_labels[6] = {"", "", "", "", "", ""};

  for ( int argi = 1; argi < argc; ) {
    char * opt = argv[argi++];
    if ('-' == opt[0] ) {
      switch ( opt[1] ) {
      case 'h': cerr << "HELP\n";              break;
      case 'd': opt_data = argv[argi++];       break;
      case 'l': opt_list = true;               break;
      case 'q': opt_quiet = true;              break;
      case 't': opt_test = atoi(argv[argi++]); break;
      case 'v': opt_verbose = true;            break;
      }
    } else {
      cerr << "unknown command line argument " << opt << endl;
    }
  }

  int model = -1;
  switch ( opt_data[0] ) {
  case 't': model = 1;         break;
  case 's': model = 100;       break;
  case 'm': model = 10000;     break;
  case 'b': model = 1000000;   break;
  case 'h': model = 100000000; break;
  default:
    cerr << "opt_data " << opt_data << endl;
    break;
  }
  if (model == -1) {
    std::cerr << "usage: exe -data tiny/small/medium/big/huge\n";
    exit(1);
  }

  stapl::exit_code code1 = experiment(1, model, "matrix_vw atom",
                           atom_times, atom_labels);
  stapl::exit_code code2 = experiment(2, model, "matrix_vw stl",
                           stl_times, stl_labels);
#ifdef NESTED_ACTIVE
  stapl::exit_code code3 = experiment(3, model, "matrix_vw nest",
                           nest_times, nest_labels);
  if ( code1==EXIT_SUCCESS && code2==EXIT_SUCCESS && code3==EXIT_SUCCESS ) {
#else
  if ( code1==EXIT_SUCCESS && code2==EXIT_SUCCESS ) {
#endif
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}

///////////////////////////////////////////////////////////////////////////
// execute test enough times to achieve desired confidence interval
///////////////////////////////////////////////////////////////////////////

stapl::exit_code experiment(int test, int model, const char *test_name,
                            double *times, string *labels)
{
  counter_t timer;
  bool success = true;
  size_t total_size = 60000 * model;
  size_t atom_outer = total_size / 20;
  size_t atom_inner = total_size / 15;
  size_t stl_outer = total_size / 200;
  size_t stl_inner = total_size / 150;
  size_t nest_outer = total_size / 20000;
  size_t nest_inner = total_size / 15000;

  bool continue_iterating = true;
  int repeat = 0;
  confidence_interval_controller iter_control(CONF_INT_REP, 100, 0.05);
  while (continue_iterating && success) {
    repeat++;
    switch(test) {
    case 1:
      timer.reset();
      timer.start();
      success = run_atom_test(atom_outer, atom_inner, times, labels);
      iter_control.push_back(timer.stop());
      break;
    case 2:
      timer.reset();
      timer.start();
      success = run_stl_test(stl_outer, stl_inner, times, labels);
      iter_control.push_back(timer.stop());
      break;
#ifdef NESTED_ACTIVE
    case 3:
      timer.reset();
      timer.start();
      success = run_nest_test(nest_outer, nest_inner, times, labels);
      iter_control.push_back(timer.stop());
      break;
#endif
    }

    stapl::array<int> continue_ct(stapl::get_num_locations());
    stapl::array_view<stapl::array<int>> continue_vw(continue_ct);
    continue_vw[stapl::get_location_id()] = iter_control.iterate() ? 1 : 0;

    int iterate_sum = stapl::accumulate(continue_vw, (int) 0);

    continue_iterating = (iterate_sum != 0 ? true : false);
  }

  if ( !opt_quiet ) {
    iter_control.report(test_name);
  }
  if ( opt_verbose ) {
    stapl::do_once([&]() {
      int i=0;
      while( i<6 && labels[i].size() > 0 ) {
        double avg = times[i] / repeat;
        cout << labels[i] << " : " << avg << endl;
        i++;
      }
    });
  }

  if ( success ) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}

///////////////////////////////////////////////////////////////////////////
// PLOT: matrix of atoms
///////////////////////////////////////////////////////////////////////////

typedef int atom_tp;

typedef stapl::matrix< atom_tp, trav2_row_major_tp, part2_tp> mat_row_major_atom_tp;
typedef stapl::matrix_view<mat_row_major_atom_tp>      mat_row_major_atom_vw_tp;

typedef stapl::matrix< atom_tp, trav2_col_major_tp, part2_tp> mat_col_major_atom_tp;
typedef stapl::matrix_view<mat_col_major_atom_tp>      mat_col_major_atom_vw_tp;


typedef stapl::plus<atom_tp> add_atom_wf;


bool run_atom_test(size_t outer, size_t inner, double *times, string *labels ) {

  double time_start, time_end, time_delta;

  // construct container
  stapl::tuple<size_t,size_t> dims = stapl::make_tuple(outer,inner);
  mat_row_major_atom_tp a_ct(dims), b_ct(dims), c_ct(dims);
  mat_col_major_atom_tp u_ct(dims), v_ct(dims), w_ct(dims);

  seconds(time_start);

  // METRIC: construct view over container
  mat_row_major_atom_vw_tp a_vw(a_ct), b_vw(b_ct), c_vw(c_ct);
  mat_col_major_atom_vw_tp u_vw(u_ct), v_vw(v_ct), w_vw(w_ct);

#if STAPL_BUG
  auto a_lin_vw = stapl::linear_view(a_vw);
  auto b_lin_vw = stapl::linear_view(b_vw);
  auto u_lin_vw = stapl::linear_view(u_vw);
  auto v_lin_vw = stapl::linear_view(v_vw);
#endif

  seconds(time_end);
  time_delta = time_end - time_start;
  times[0] += time_delta;
  labels[0] = string("view: matrix");
  seconds(time_start);

  // METRIC: initialize containers in parallel
  int base = 0;
  int step = 10;
  typedef stapl::sequence<int> step_wf;
  int rep = 12;
  typedef stapl::block_sequence<int> repeat_wf;

#if STAPL_BUG
  stapl::generate(a_lin_vw, step_wf(base,step));
  stapl::generate(b_lin_vw, repeat_wf(base,rep));
  stapl::iota(u_lin_vw, 0);
  stapl::generate(v_lin_vw, step_wf(base,step));
#endif

  seconds(time_end);
  time_delta = time_end - time_start;
  times[1] += time_delta;
  labels[1] = string("view: matrix");
  seconds(time_start);

  // METRIC: process elements in parallel in row major
#if STAPL_BUG
  stapl::transform( a_vw, b_vw, c_vw, add_atom_wf() );
#endif

  seconds(time_end);
  time_delta = time_end - time_start;
  times[2] += time_delta;
  labels[2] = string("view: matrix");
  seconds(time_start);

  // METRIC: process elements in parallel in column major
#if STAPL_BUG
  stapl::transform( u_vw, v_vw, w_vw, add_atom_wf() );
#endif

  seconds(time_end);
  time_delta = time_end - time_start;
  times[3] += time_delta;
  labels[3] = string("view: matrix");

  return true;
}

///////////////////////////////////////////////////////////////////////////
// PLOT: matrix of STL vectors
///////////////////////////////////////////////////////////////////////////

typedef vector<atom_tp> stl_tp;

typedef stapl::matrix< stl_tp, trav2_row_major_tp, part2_tp> mat_row_major_stl_tp;
typedef stapl::matrix_view<mat_row_major_stl_tp>           mat_row_major_stl_vw_tp;

typedef stapl::matrix< stl_tp, trav2_col_major_tp, part2_tp> mat_col_major_stl_tp;
typedef stapl::matrix_view<mat_col_major_stl_tp>           mat_col_major_stl_vw_tp;


struct map_sz_wf
{
  typedef size_t result_type;
  template <typename Vec1>
  result_type operator()(Vec1 const v1) {
    return v1.size();
  }
};

typedef stapl::plus<size_t> add_sz_wf;


bool run_stl_test(size_t outer, size_t inner, double *times, string *labels ) {

  double time_start, time_end, time_delta;

  // construct container
  stapl::tuple<size_t,size_t> dims = stapl::make_tuple(outer,inner);
  mat_row_major_stl_tp a_ct(dims), b_ct(dims), c_ct(dims);
  mat_col_major_stl_tp u_ct(dims), v_ct(dims), w_ct(dims);

  seconds(time_start);

  // METRIC: construct view over container
  mat_row_major_stl_vw_tp a_vw(a_ct), b_vw(b_ct), c_vw(c_ct);
  mat_col_major_stl_vw_tp u_vw(u_ct), v_vw(v_ct), w_vw(w_ct);

#if STAPL_OR_TEST_BUG
  auto a_lin_vw = stapl::linear_view(a_vw);
  auto b_lin_vw = stapl::linear_view(b_vw);
  auto c_lin_vw = stapl::linear_view(c_vw);
  auto u_lin_vw = stapl::linear_view(u_vw);
  auto v_lin_vw = stapl::linear_view(v_vw);
  auto w_lin_vw = stapl::linear_view(w_vw);
#endif

  seconds(time_end);
  time_delta = time_end - time_start;
  times[0] += time_delta;
  labels[0] = string("view: matrix");
  seconds(time_start);

  // METRIC: initialize containers in parallel
  size_t size = outer * inner;
  ary_sz_tp len(size);
  ary_sz_vw_tp len_vw(len);
  stapl::map_func(roll_wf(), len_vw, stapl::make_repeat_view(inner));

#if STAPL_OR_TEST_BUG
  stapl::map_func( init_stl_vec_wf(), a_lin_vw, len_vw );
#endif

  seconds(time_end);
  time_delta = time_end - time_start;
  times[1] += time_delta;
  labels[1] = string("view: matrix");
  seconds(time_start);

  // METRIC: process elements in parallel
#if STAPL_OR_TEST_BUG
  atom_tp sum = stapl::map_reduce( map_sz_wf(), add_sz_wf(), a_vw );
#endif

  seconds(time_end);
  time_delta = time_end - time_start;
  times[2] += time_delta;
  labels[2] = string("view: matrix");
  seconds(time_start);

  // METRIC: process elements in parallel
#if STAPL_OR_TEST_BUG
  atom_tp sum = stapl::map_reduce( map_sz_wf(), add_sz_wf(), u_vw );
#endif

  seconds(time_end);
  time_delta = time_end - time_start;
  times[3] += time_delta;
  labels[3] = string("view: matrix");

  return true;
}

///////////////////////////////////////////////////////////////////////////
// plot: matrix of STAPL vectors
///////////////////////////////////////////////////////////////////////////

typedef stapl::vector<atom_tp> nest_tp;

typedef stapl::matrix< nest_tp, trav2_row_major_tp, part2_tp> mat_row_major_nest_tp;
typedef stapl::matrix_view<mat_row_major_nest_tp>           mat_row_major_nest_vw_tp;

typedef stapl::matrix< nest_tp, trav2_col_major_tp, part2_tp> mat_col_major_nest_tp;
typedef stapl::matrix_view<mat_col_major_nest_tp>           mat_col_major_nest_vw_tp;


#ifdef NESTED_ACTIVE
struct fill_wf
{
  typedef void result_type;
  template <typename Elem, typename View>
  result_type operator()(View const &vw, Elem count)
  {
    // CODE
  }
};

struct max_nest_inner_wf
{
  typedef void result_type;
  template <typename Elem1, typename Elem2, typename Elem3>
  result_type operator()(Elem1 v1, Elem2 v2, Elem3 &v3) {
    // CODE
  }
};

struct max_nest_outer_wf
{
  typedef void result_type;
  template <typename View1, typename View2, typename View3>
  result_type operator()(View1 const v1, View2 const v2, View3 const &v3) {
    stapl::map_func(max_nest_inner_wf(), v1, v2, v3 );
  }
};

bool run_nest_test(size_t outer, size_t inner, double *times, string *labels ) {

  double time_start, time_end, time_delta;

  // construct container
  ary_sz_tp len(outer);
  ary_sz_vw_tp len_vw(len);
  stapl::map_func(roll_wf(), len_vw, stapl::make_repeat_view(inner));

  stapl::tuple<size_t,size_t> dims = stapl::make_tuple(outer,inner);
  mat_row_major_nest_tp a_ct(dims), b_ct(dims), c_ct(dims);
  mat_col_major_nest_tp u_ct(dims), v_ct(dims), w_ct(dims);

  seconds(time_start);

  // metric: construct view over container
  mat_row_major_nest_vw_tp a_vw(a_ct), b_vw(b_ct), c_vw(c_ct);
  mat_col_major_nest_vw_tp u_vw(u_ct), v_vw(v_ct), w_vw(w_ct);

  seconds(time_end);
  time_delta = time_end - time_start;
  times[0] += time_delta;
  labels[0] = string("view: matrix");
  seconds(time_start);

  // metric: initialize containers in parallel
  stapl::map_func( fill_wf(), a_vw );

  seconds(time_end);
  time_delta = time_end - time_start;
  times[1] += time_delta;
  labels[1] = string("view: matrix");
  seconds(time_start);

  // metric: process elements in parallel
  stapl::map_func(max_nest_outer_wf(), a_vw, b_vw, c_vw);

  seconds(time_end);
  time_delta = time_end - time_start;
  times[2] += time_delta;
  labels[2] = string("view: matrix");
  seconds(time_start);

  // metric: process elements in parallel
  stapl::map_func(max_nest_outer_wf(), u_vw, v_vw, w_vw);

  seconds(time_end);
  time_delta = time_end - time_start;
  times[3] += time_delta;
  labels[3] = string("view: matrix");

  return true;
}
#endif

