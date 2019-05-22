/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <stapl/containers/array/array.hpp>
#include <stapl/containers/partitions/blocked_partition.hpp>
#include "../p_container_profiler.hpp"
#include "../profiler_util.h"
#include "../value_type_util.h"

using namespace stapl;

////////////////////////////////////////////////////////////////////////////////
/// @brief Fills a container with indices with a specified percentage being
/// remote.
////////////////////////////////////////////////////////////////////////////////
class index_generator
{
public:

  //////////////////////////////////////////////////////////////////////////////
  /// @brief Fills a container with indices with a specified percentage being
  /// remote.
  ///
  /// @param indices The container of the resulting indices
  /// @param first The first index value for the location
  /// @param sz The number of indices to generate
  /// @param per_remote The percentage of indices that are remote
  /// @param next_only Restricts remote indices to neighbors if 1
  //////////////////////////////////////////////////////////////////////////////
  template <class V>
  static void generate(V& indices, size_t first, size_t sz,
                       size_t per_remote, size_t next_only)
  {
    // for next only the convention is if non local are on neighbours only (1)
    // else on all Ps(0)
    size_t nLocs=get_num_locations();
    indices.resize(sz);
    //generate first as if all are local
    for ( size_t i = 0; i < sz; ++i)
      indices[i] = first + (lrand48() % sz);

    if (per_remote == 0) //all indices local
      return;

    else
    { //there is a remote percentage
      size_t nremote = (per_remote*sz) / 100;
      size_t bs = sz/nremote;
      if (next_only == 1)
      { //remote only on neighbor
        bool left = false;
        for ( size_t i = 0; i < sz; i+=bs)
        {
          if (!left)
            indices[i] = (sz + first + ( lrand48() % sz)) % (nLocs * sz);

          else {
            if (first >= ( lrand48() % sz) )
              indices[i] = first - ( lrand48() % sz);
          }
          left = !left;
        }
      }
      else { //all over the place
        for (size_t i = 0; i < sz; i+=bs)
          indices[i] = lrand48() % (nLocs * sz) ;
      }
    }
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Calls the profilers for all of the functions of the array container.
///
/// @param name A string containing the name of the container
/// @param p The container to test
/// @param NElems The size of the container
////////////////////////////////////////////////////////////////////////////////
template <class pC>
void profile_methods(std::string name, pC& p, size_t NElems,
                     int argc, char** argv)
{
  typedef typename pC::value_type value_type;

  constructor_size_value_profiler<pC, counter_type>
    cep(name, NULL, NElems, argc, argv);
  cep.collect_profile();
  cep.report();

  size_t block = NElems / get_num_locations();
  size_t premote = 0;
  size_t next_only = 1;
  for (int i = 1; i < argc; i++) {
    if (!strcmp("--premote", argv[i])) {
      premote = atoi(argv[++i]);
    }
    if (!strcmp("--next_only", argv[i])) {
      next_only = atoi(argv[++i]);
    }
  }

  std::vector<size_t> indices;
  index_generator::generate(indices, block*get_location_id(), block,
                            premote,next_only);

  if (get_location_id() == 0) { //timing sequential array
    counter<default_timer> t;
    double elapsed;
    std::valarray<value_type>* v = new std::valarray<value_type>(block);
    t.reset();
    t.start();
    for (size_t i = 0; i < block; ++i) {
      (*v)[indices[i]] = i;
    }
    elapsed = t.stop();
    if (get_location_id() == 0)
      std::cout<<"Elapsed seq=" << elapsed <<"\n";

    value_type tot(0);
    t.reset();
    t.start();
    for (size_t i = 0; i < block; ++i) {
      tot += (*v)[indices[i]];
    }
    elapsed = t.stop();
    if (get_location_id() == 0)
      std::cout<<"Elapsed read plus seq=" << elapsed <<"\n";
    if (get_location_id() == 0)
      std::cout<<"Total=" << tot <<"\n";
  }

  set_element_profiler<pC, counter_type>
    sep(name, &p, indices, argc, argv);
  sep.collect_profile();
  sep.report();

  get_element_profiler<pC, counter_type>
    gep(name, &p, indices, argc, argv);
  gep.collect_profile();
  gep.report();

  get_element_split_profiler<pC, counter_type>
    gesp(name, &p, indices, argc, argv);
  gesp.collect_profile();
  gesp.report();

  operator_square_bracket_lhs_profiler<pC, counter_type>
    oplep(name, &p, indices, argc, argv);
  oplep.collect_profile();
  oplep.report();

  operator_square_bracket_rhs_profiler<pC, counter_type>
    oprep(name, &p, indices, argc, argv);
  oprep.collect_profile();
  oprep.report();

  sum_op<value_type> so;
  apply_set_profiler<pC, sum_op<value_type>, counter_type>
    asp(name, &p, so, indices, argc, argv);
  asp.collect_profile();
  asp.report();

  get_sum<value_type> gs;
  apply_get_profiler<pC, get_sum<value_type>, counter_type>
    agp(name, &p, gs, indices, argc, argv);
  agp.collect_profile();
  agp.report();
}


stapl::exit_code stapl_main(int argc, char** argv)
{
  stapl_print("pContainer Performance Evaluation\n");

  size_t case_id;
  size_t NElems;
  if (argc > 2) {
    case_id = atoi(argv[1]);
    NElems = atoi(argv[2]);
    srand(NElems + get_location_id());
    srand48(NElems + get_location_id());
  }
  else {
    stapl_print("Input size NElems required; Using 10 by default\n");
    NElems=10;
    case_id=0;
  }

  //here test individual array instances
  if (case_id==0)
  {  //default array
     array<int> p(NElems);
     profile_methods("array<int>", p, NElems, argc, argv);
  }

  if (case_id==1)
  {
    array<MVT> pdt(NElems);
    profile_methods("array<CV>", pdt, NElems, argc, argv);
  }

  //specific partition strategy
  std::string pcname;
  if (case_id==2)
  {
    pcname = "array<int,PSBalanced>";
    array<int> p(NElems);
    profile_methods(pcname, p, NElems, argc, argv);
    rmi_fence();
  }

  if (case_id==3)
  {
    pcname = "array<CV,PSBalanced>";
    array<MVT> p(NElems);
    profile_methods(pcname, p, NElems, argc, argv);
    rmi_fence();
  }

  const size_t NBS=4;
  size_t bs[NBS]={1024, 8192, 65536,0};
  bs[NBS-1] = NElems/get_num_locations();
  for (size_t i=0; i<NBS; ++i) {
    std::ostringstream o;
    o<<bs[i];
    if (case_id==4) {
      pcname = "array<int,PSB("+ o.str() +")";
      {
        typedef stapl::block_partitioner<stapl::indexed_domain<size_t>
          > blocked_type;
        stapl::indexed_domain<size_t> array_dom(0, NElems-1);
        array<int, blocked_type> p(NElems, blocked_type(array_dom, bs[i]));
        profile_methods(pcname, p, NElems, argc, argv);
        rmi_fence();
      }
    }
    if (case_id==5) {
      pcname = "array<CV,PSB("+o.str()+")";
      {
        typedef stapl::block_partitioner<stapl::indexed_domain<size_t>
          > blocked_type;
        stapl::indexed_domain<size_t> array_dom(0, NElems-1);
        array<MVT, blocked_type> p(NElems, blocked_type(array_dom, bs[i]));
        profile_methods(pcname, p, NElems, argc, argv);
        rmi_fence();
      }
    }
  }
  return EXIT_SUCCESS;
}
