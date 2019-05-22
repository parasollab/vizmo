/*
// Copyright (c) 2000-2010, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>

#include <stapl/skeletons/utility/tags.hpp>
#include <stapl/algorithms/algorithm.hpp>
#include <stapl/algorithms/numeric.hpp>
#include <stapl/containers/array/array.hpp>
#include <stapl/runtime/location_specific_storage.hpp>
#include <stapl/views/repeated_view.hpp>

// std
using std::size_t;
using std::cout;
using std::setw;
using std::setprecision;
using std::vector;

// container
using stapl::array;

// views
using stapl::array_view;
using stapl::make_array_view;
using stapl::make_repeat_view;

// Algorithms Used
using stapl::map_func;
using stapl::is_sorted;
using stapl::fill;
using stapl::partial_sum;

// Rank / Group Size info
using stapl::get_num_locations;
using stapl::get_location_id;


struct problem_traits
{
  size_t            total_keys;
  size_t            max_key_log_2;
  size_t            num_buckets_log_2;

  size_t            min_n_locs;
  size_t            max_n_locs;

  size_t            max_key;
  size_t            num_buckets;
  size_t            num_keys;

  size_t            size_of_buffers;

  problem_traits(size_t a, size_t b, size_t c,
                 size_t min_locs, size_t max_locs)
    : total_keys(1 << a),
      max_key_log_2(b),
      num_buckets_log_2(c),
      min_n_locs(min_locs),
      max_n_locs(max_locs),
      max_key(1 << max_key_log_2),
      num_buckets(1 << num_buckets_log_2),
      num_keys(total_keys / get_num_locations() * min_n_locs)
  {
    size_t n_locs = get_num_locations();

    if (n_locs < 256)
    {
      size_of_buffers = 3 * num_keys / 2;
      return;
    }

    if (n_locs < 512)
    {
      size_of_buffers = 5 * num_keys / 2;
      return;
    }

    if (n_locs < 1024)
    {
      size_of_buffers = 4 * num_keys;
      return;
    }

    size_of_buffers = 13 * num_keys / 2;
  }
};


typedef int  int_t;
typedef long int2_t;

const size_t test_array_size =  5;

const long S_test_index_array[test_array_size] = {48427,17148,23627,62548,4431};
const long S_test_rank_array[test_array_size]  = {0,18,346,64917,65463};

const long W_test_index_array[test_array_size] = {357773, 934767, 875723,
                                                  898999, 404505};
const long W_test_rank_array[test_array_size]  = {1249, 11698, 1039987,
                                                  1043896, 1048018};

const long A_test_index_array[test_array_size] = {2112377, 662041, 5336171,
                                                  3642833, 4250760};
const long A_test_rank_array[test_array_size]  = {104, 17523, 123928,
                                                  8288932, 8388264};

const long B_test_index_array[test_array_size] = {41869, 812306, 5102857,
                                                  18232239, 26860214};
const long B_test_rank_array[test_array_size]  = {33422937, 10244, 59149,
                                                  33135281,99};

const long C_test_index_array[test_array_size] = {44172927, 72999161, 74326391,
                                                  129606274, 21736814};
const long C_test_rank_array[test_array_size]  = {61147, 882988, 266290,
                                                  133997595, 133525895};

const long D_test_index_array[test_array_size] = {1317351170, 995930646,
                                                  1157283250, 1503301535,
                                                  1453734525};
const long D_test_rank_array[test_array_size]  = {1,36538729,
                                                  1978098519, 2145192618,
                                                  2147425337};

long test_index_array[test_array_size];
long test_rank_array[test_array_size];


void initialize_test_arrays(std::string problem_class)
{
  long const* index_array;
  long const* rank_array;

  bool val_set = false;

  if (problem_class == "s")
  {
    index_array = S_test_index_array;
    rank_array  = S_test_rank_array;
    val_set     = true;
  }

  if (problem_class == "a")
  {
    index_array = A_test_index_array;
    rank_array  = A_test_rank_array;
    val_set     = true;
  }

  if (problem_class == "w")
  {
    index_array = W_test_index_array;
    rank_array  = W_test_rank_array;
    val_set     = true;
  }

  if (problem_class == "b")
  {
    index_array = B_test_index_array;
    rank_array  = B_test_rank_array;
    val_set     = true;
  }

  if (problem_class == "c")
  {
    index_array = C_test_index_array;
    rank_array  = C_test_rank_array;
    val_set     = true;
  }

  if (problem_class == "d")
  {
    index_array = D_test_index_array;
    rank_array  = D_test_rank_array;
    val_set     = true;
  }

  if (!val_set)
    stapl::abort("problem class must be s, a, w, b, c, or d\n");

  for (size_t i = 0; i < test_array_size; ++i)
  {
    test_index_array[i] = index_array[i];
    test_rank_array[i]  = rank_array[i];
  }
}


problem_traits
compute_problem_traits(std::string problem_class)
{
  if (problem_class == "s")
    return problem_traits(16, 11,  9, 1,  128);

  stapl_assert(0, "IS only sample support ATM");

  if (problem_class == "w")
    return problem_traits(20, 16, 10, 1, 1024);

  if (problem_class == "a")
    return problem_traits(23, 19, 10, 1, 1024);

  if (problem_class == "b")
    return problem_traits(25, 21, 10, 1, 1024);

  if (problem_class == "c")
    return problem_traits(27, 23, 10, 1, 1024);

  if (problem_class == "d")
    return problem_traits(29, 27, 10, 4, 1024);

  stapl::abort("problem class must be s, w, a, b, c, or d.\n");

  return problem_traits(0, 0, 0, 0, 0);
}


double randlc(double *X, double *A)
{
  static int        KS=0;
  static double     R23, R46, T23, T46;
  double            T1, T2, T3, T4;
  double            A1;
  double            A2;
  double            X1;
  double            X2;
  double            Z;
  int               i, j;

  // ??? KS fixed to 0 above...
  if (KS == 0)
  {
    R23 = 1.0;
    R46 = 1.0;
    T23 = 1.0;
    T46 = 1.0;

    for (i=1; i<=23; i++)
    {
      R23 = 0.50 * R23;
      T23 = 2.0 * T23;
    }

   for (i=1; i<=46; i++)
    {
      R46 = 0.50 * R46;
      T46 = 2.0 * T46;
    }

    KS = 1;
  }

  //
  // Break A into two parts such that A = 2^23 * A1 + A2 and set X = N.
  //
  T1 = R23 * *A;
  j  = T1;
  A1 = j;
  A2 = *A - T23 * A1;

  // Break X into two parts such that X = 2^23 * X1 + X2, compute
  // Z = A1 * X2 + A2 * X1  (mod 2^23), and then
  // X = 2^23 * Z + A2 * X2  (mod 2^46).

  T1 = R23 * *X;
  j  = T1;
  X1 = j;
  X2 = *X - T23 * X1;
  T1 = A1 * X2 + A2 * X1;

  j  = R23 * T1;
  T2 = j;
  Z  = T1 - T23 * T2;
  T3 = T23 * Z + A2 * X2;
  j  = R46 * T3;
  T4 = j;
  *X = T3 - T46 * T4;

   return R46 * *X;
}


double find_my_seed(int kn,   // my processor rank, 0<=kn<=num procs
                    int np,   // np = num procs
                    long nn,  // total num of ran numbers, all procs
                    double s, // Ran num seed, for ex.: 314159265.00
                    double a) // Ran num gen mult, try 1220703125.00
{
  long i;
  double t1,t2,an;
  long mq,nq,kk,ik;

  nq = nn / np;

  for (mq=0; nq>1; mq++,nq/=2)
  { }

  t1 = a;

  for (i=1; i<=mq; i++)
  {
    t2 = randlc(&t1, &t1);
  }

  an = t1;
  kk = kn;
  t1 = s;
  t2 = an;

  for (i=1; i<=100; i++)
  {
    ik = kk / 2;

    if (2 * ik !=  kk)
    {
      randlc(&t1, &t2);
    }

    if ( ik == 0 )
    {
      break;
    }

    randlc(&t2, &t2);

    kk = ik;
  }

  return t1;
}


//////////////////////////////////////////////////////////////////////
/// @brief Generates random values specific to NAS benchmark.
///
/// @todo generate fix for static variables
//////////////////////////////////////////////////////////////////////
struct random_sequence_is_func
{
private:
  double       m_a;
  double       m_seed;
  const size_t m_k;

public:
  typedef void   result_type;

  random_sequence_is_func(double seed, size_t max_key)
    : m_a(1220703125.00),
      m_seed(seed),
      m_k(max_key/4)
  { }

  template <typename V>
  void operator()(V v)
  {
    for (auto&& e : v) {
      double x = randlc(&m_seed, &m_a);

      x += randlc(&m_seed, &m_a);
      x += randlc(&m_seed, &m_a);
      x += randlc(&m_seed, &m_a);

      e = m_k * x;
    }
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_a);
    t.member(m_seed);
    t.member(m_k);
  }
};


struct bucketize_func
{
  const int_t m_shift;

  typedef void result_type;

  bucketize_func(int_t shift)
    : m_shift(shift)
  { }

  void define_type(stapl::typer& t)
  {
    t.member(m_shift);
  }

  template<typename Reference, typename View>
  void operator()(Reference elem, View buckets) const
  {
    const size_t bucket_idx = elem >> m_shift;

    buckets[bucket_idx].push_back(elem);
  }
};


struct increment_ranks_func
{
  typedef void result_type;

  template<typename Reference, typename View>
  void operator()(Reference bucket, View ranks) const
  {
    // FIXME - for_each'ify - maybe use phoenix polymorphic lambdas...
    //
    // for_each(bucket, make_repeat_view(ranks), { ++_2[_1] })
    auto e_iter = bucket.end();

    for (auto iter = bucket.begin(); iter != e_iter; ++iter)
    {
      ++ranks[*iter];
    }
  }
};


struct sort_wf
{
  typedef void result_type;

  template<typename Reference, typename RankView, typename KeyView>
  void operator()(Reference bucket, RankView ranks, KeyView keys) const
  {
    // FIXME - for_each'ify - maybe use phoenix polymorphic lambdas...
    auto e_iter = bucket.end();

    for (auto iter = bucket.begin(); iter != e_iter; ++iter)
    {
       keys[--ranks[*iter]] = *iter;
    }
  }
};


struct clear_func
{
  typedef void result_type;

  template<typename Reference>
  void operator()(Reference bucket) const
  {
    bucket.clear();
  }
};


struct print_size_func
{
  template<typename Reference>
  void operator()(Reference bucket) const
  {
    std::cout << stapl::index_of(bucket) << ": " << bucket.size() << "\n";
  }
};


stapl::exit_code stapl_main(int argc, char* argv[])
{
  const size_t n_iters = 10;
  const size_t n_locs  = stapl::get_num_locations();
  const size_t loc_id  = stapl::get_location_id();

  if (argc < 2) {
    std::cerr << "Usage: mpiexec -n <#procs> "<< argv[0] << " <problem-class>\n"
              << "Problem classes are a, b, c, d, s1 or s2.\n";
    return EXIT_FAILURE;
  }

  const problem_traits traits = compute_problem_traits(argv[1]);

  stapl_assert(
    n_locs >= traits.min_n_locs && n_locs <= traits.max_n_locs,
    "Invalid processor count for this IS problem class.\n"
  );

  initialize_test_arrays(argv[1]);

  if (stapl::get_location_id() == 0)
    cout << "NAS IS Benchmark\n"
         << "Number of active processes: " << stapl::get_num_locations()
         << "\n";

  auto verify_cnts_ct = array<unsigned long>(n_locs);
  auto verify_cnts_vw = make_array_view(verify_cnts_ct);

  auto keys_ct        = array<int_t>(traits.total_keys);
  auto keys_vw        = make_array_view(keys_ct);

  auto buckets_ct     = array<vector<int_t> >(traits.num_buckets);
  auto buckets_vw     = make_array_view(buckets_ct);

  auto ranks_ct       = array<size_t>(traits.max_key);
  auto ranks_vw       = make_array_view(ranks_ct);

  auto ranks_part_vw  = balance_view(ranks_vw, traits.num_buckets);

  // FIXME - reserve bucket sizes in line with SIZE_OF_BUFFERS from benchmark
  const double seed =
    find_my_seed(
      loc_id,
      n_locs,
      4 * (long) traits.total_keys * traits.min_n_locs,
      314159265.00,
      1220703125.00
    );

  // Generate random sequence, as prescribed by NAS spec
  map_func<stapl::skeletons::tags::with_coarsened_wf>(
    random_sequence_is_func(seed, traits.max_key), keys_vw);

  // Start the Timer
  stapl::counter<stapl::default_timer> tmr;

  // barrier to get all the timers in sync.
  stapl::rmi_fence();

  tmr.start();

  //  The timed loop of successive rank operations
  for (size_t iteration=1; iteration <= n_iters; ++iteration)
  {
     // print iteration header and mutate 2 keys, per the spec
     if (loc_id == 0)
     {
       keys_vw[iteration]         = iteration;
       keys_vw[iteration+n_iters] = traits.max_key - iteration;
     }

    // assignments to key_vw should finish before the computation starts
    stapl::rmi_fence();

     // initialize / clear data structures
     fill(ranks_vw, 0);

     map_func(clear_func(), buckets_vw);

     // put keys into buckets
     const int_t shift = traits.max_key_log_2 - traits.num_buckets_log_2;

     map_func(bucketize_func(shift), keys_vw, make_repeat_view(buckets_vw));

     // scan each bucket and update value counts
     map_func(increment_ranks_func(), buckets_vw, ranks_part_vw);

     // partial sum to compute ranks
     partial_sum(ranks_vw, ranks_vw);

     // partial verification
     //
     // FIXME - no reason this can't be a palgo call...
     const int_t block_size   = (traits.num_buckets / n_locs);
     const int_t first_bucket = block_size * loc_id;
     const int_t last_bucket  = first_bucket + block_size - 1;
     const int_t min_key_val  = first_bucket << shift;
     const int_t max_key_val  = ((last_bucket + 1) << shift) - 1;

     for (size_t idx=0; idx < test_array_size; ++idx)
     {
       const int_t k = keys_vw[test_index_array[idx]];

       if (min_key_val <= k &&  k <= max_key_val)
       {
           auto correct_rank =
           idx <= 2 ? test_rank_array[idx]+iteration
                      : test_rank_array[idx]-iteration;

         if (correct_rank != ranks_vw[k-1]) {
           std::cerr << argv[0] << ": Partial validation failed!\n";
           return EXIT_FAILURE;
         }
       }
     }

    // Prevent other locations from starting the next iteration and
    // changing the key_view before the partial verification is done
    stapl::rmi_fence();
  }

  // Stop the Timer
  const double time = tmr.stop();

  // Full verification
  map_func(sort_wf(), buckets_vw, ranks_part_vw, make_repeat_view(keys_vw));

  is_sorted(keys_vw);

  const bool b_verified = is_sorted(keys_vw);

  if (loc_id == 0)
  {
    cout << "Benchmark completed\n";

   if (b_verified)
     cout << "VERIFICATION SUCCESSFUL\n";
   else
     cout << "VERIFICATION FAILED\n";

    cout << "IS Benchmark Results:\n"
         << "CPU Time " << setw(10) << setprecision(4) << time << "\n";
  }

  return EXIT_SUCCESS;
}
