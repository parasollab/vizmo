#include <stapl/containers/vector/vector.hpp>
#include <stapl/containers/array/array.hpp>
#include <stapl/containers/matrix/matrix.hpp>
#include <stapl/containers/type_traits/default_traversal.hpp>
#include <stapl/containers/partitions/block_cyclic_partition.hpp>
#include <stapl/containers/partitions/blocked_partition.hpp>
#include <stapl/algorithms/algorithm.hpp>
#include <stapl/algorithms/functional.hpp>

#include "conthelp.hpp"
using namespace std;

#define CONF_INT_REP 32

typedef stapl::tuple<size_t, size_t> gid2_tp;

typedef stapl::indexed_domain<size_t>                  ndx_dom_tp;
typedef stapl::default_traversal<2>::type              trav2_tp;

typedef stapl::blk_cyclic_part<ndx_dom_tp>             blk_cyc_part_tp;
typedef stapl::nd_partition<
          stapl::tuple<blk_cyc_part_tp, blk_cyc_part_tp>,
          trav2_tp>                                    blk_cyc_2_part_tp;


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
// FEATURE: matrix container
///////////////////////////////////////////////////////////////////////////

stapl::exit_code stapl_main(int argc, char **argv) {

  double atom_times[6] = {0,0,0,0,0,0};
  double stl_times[6] =  {0,0,0,0,0.0};
  double nest_times[6] = {0,0,0,0,0.0};

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

  stapl::exit_code code1 = experiment(1, model, "matrix_ct atom",
                           atom_times, atom_labels);
  stapl::exit_code code2 = experiment(2, model, "matrix_ct stl",
                           stl_times, stl_labels);
#ifdef NESTED_ACTIVE
  stapl::exit_code code3 = experiment(3, model, "matrix_ct nest",
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
  size_t atom_rows = total_size / 15;
  size_t atom_cols = total_size / 12;
  size_t stl_rows = total_size / 150;
  size_t stl_cols = total_size / 120;
  size_t nest_rows = total_size / 15000;
  size_t nest_cols = total_size / 12000;

  bool continue_iterating = true;
  int repeat = 0;
  confidence_interval_controller iter_control(CONF_INT_REP, 100, 0.05);
  while (continue_iterating && success) {
    repeat++;
    switch(test) {
    case 1:
      timer.reset();
      timer.start();
      success = run_atom_test(atom_rows, atom_cols, times, labels);
      iter_control.push_back(timer.stop());
      break;
    case 2:
      timer.reset();
      timer.start();
      success = run_stl_test(stl_rows, stl_cols, times, labels);
      iter_control.push_back(timer.stop());
      break;
#ifdef NESTED_ACTIVE
    case 3:
      timer.reset();
      timer.start();
      success = run_nest_test(nest_rows, nest_cols, times, labels);
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
typedef stapl::matrix< atom_tp, trav2_tp,
                       blk_cyc_2_part_tp>           mat_atom_blk_cyc_tp;
typedef stapl::matrix<atom_tp> mat_atom_tp;

bool run_atom_test(size_t rows, size_t cols, double *times, string *labels ) {

  double time_start, time_end, time_delta;
  seconds(time_start);

  // METRIC: construct container
  stapl::tuple<size_t,size_t> dims = stapl::make_tuple(rows,cols);
  mat_atom_tp a_ct(dims);

  seconds(time_end);
  time_delta = time_end - time_start;
  times[0] += time_delta;
  labels[0] = string("container: matrix, defaults");
  seconds(time_start);

  // METRIC: construct container with non-default distribution
  size_t num_locs = a_ct.get_num_locations();
  blk_cyc_part_tp bc_p0(ndx_dom_tp(0, rows-1), num_locs);
  blk_cyc_part_tp bc_p1(ndx_dom_tp(0, cols-1), num_locs);
  blk_cyc_2_part_tp blk_cyc_part(bc_p0,bc_p1);
  mat_atom_blk_cyc_tp b_ct(blk_cyc_part);

  seconds(time_end);
  time_delta = time_end - time_start;
  times[1] += time_delta;
  labels[1] = string("container: matrix, non-default distribution");
  seconds(time_start);

  // METRIC: initialize container using sequential loop

  seconds(time_end);
  time_delta = time_end - time_start;
  times[2] += time_delta;
  labels[2] = string("container: matrix, initialize, sequential");
  seconds(time_start);

  // for p_objects on the stack
  stapl::rmi_fence();
  return times;
}

///////////////////////////////////////////////////////////////////////////
// PLOT: matrix of STL vectors
///////////////////////////////////////////////////////////////////////////

typedef vector<atom_tp> stl_tp;
typedef stapl::matrix< stl_tp, trav2_tp,
                       blk_cyc_2_part_tp>           mat_stl_blk_cyc_tp;
typedef stapl::matrix<stl_tp> mat_stl_tp;

bool run_stl_test(size_t rows, size_t cols, double *times, string *labels ) {

  double time_start, time_end, time_delta;
  seconds(time_start);

  // METRIC: construct container
#ifdef FINISH
#endif

  seconds(time_end);
  time_delta = time_end - time_start;
  times[0] += time_delta;
  labels[0] = string("container: matrix, defaults");
  seconds(time_start);

  // METRIC: construct container with non-default distribution
#ifdef FINISH
#endif

  seconds(time_end);
  time_delta = time_end - time_start;
  times[1] += time_delta;
  labels[1] = string("container: matrix, non-default distribution");
  seconds(time_start);

  // METRIC: initialize container using sequential loop
#ifdef FINISH
#endif

  seconds(time_end);
  time_delta = time_end - time_start;
  times[2] += time_delta;
  labels[2] = string("container: matrix, initialize, sequential");
  seconds(time_start);

  // for p_objects on the stack
  stapl::rmi_fence();
  return times;
}

///////////////////////////////////////////////////////////////////////////
// plot: matrix of STAPL vectors
///////////////////////////////////////////////////////////////////////////

typedef stapl::vector<int> nest_tp;
typedef stapl::matrix< nest_tp, trav2_tp,
                       blk_cyc_2_part_tp>           mat_nest_blk_cyc_tp;
typedef stapl::matrix<nest_tp> mat_nest_tp;

#ifdef NESTED_ACTIVE

bool run_nest_test(size_t rows, size_t cols, double *times, string *labels ) {

  double time_start, time_end, time_delta;
  seconds(time_start);

  // metric: construct container
  // CODE

  seconds(time_end);
  time_delta = time_end - time_start;
  times[0] += time_delta;
  labels[0] = string("container: matrix, defaults");
  seconds(time_start);

  // metric: construct container with non-default distribution
  // CODE

  seconds(time_end);
  time_delta = time_end - time_start;
  times[1] += time_delta;
  labels[1] = string("container: matrix, non-default distribution");
  seconds(time_start);

  // metric: initialize container using sequential loop
  // CODE

  seconds(time_end);
  time_delta = time_end - time_start;
  times[2] += time_delta;
  labels[2] = string("container: matrix, initialize, sequential");

  // for p_objects on the stack
  stapl::rmi_fence();
  return times;
}
#endif

