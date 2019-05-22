/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_PROFILERS_P_CONTAINER_PROFILER_HPP
#define STAPL_PROFILERS_P_CONTAINER_PROFILER_HPP

#include <stapl/profiler/base_profiler.hpp>

namespace stapl {

namespace p_container_profiler_detail {

////////////////////////////////////////////////////////////////////////////////
/// @brief In addition to time, other counters may print different information
/// such as l2 or l3 cache misses; these will be reported using a specialization
/// of this class.
///
/// @tparam Metric The type of value to be printed.
/// @tparam Profiler The profiler that will provide the value.
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////

template <class Metric, class Profiler>
struct print_extra_information
{
  static void print(Profiler&)
  { }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Extract size; some containers may not have size method and will have
/// to specialize the following.
///
/// @tparam Container The container type.
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template <class Container>
struct compute_size
{
  static size_t size(Container const* p)
  { return p->size(); }
};

}; //namespace p_container_profiler_detail

////////////////////////////////////////////////////////////////////////////////
/// @brief pContainer base profiler class that can be customized to extract a
/// metric from the value type of the counter.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template <class pC, class Counter>
class p_container_profiler
  : public base_profiler<Counter>
{
  typedef base_profiler<Counter> base_type;

protected:
  /// The container to test
  pC* m_pc;
  /// The individual test size
  size_t n_times;
  /// Dictates whether or not to coarsen results to a single invocation
  bool m_coarse;

public:

  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler with a single invocation.
  ///
  /// @param pc The container to test
  /// @param inname A string that represents the name of @p pc
  //////////////////////////////////////////////////////////////////////////////
  p_container_profiler(pC* pc, std::string inname, int argc=0, char **argv=NULL)
    : base_type(inname, argc, argv), m_pc(pc), m_coarse(false)
  {
    n_times = 1;
    for (int i = 1; i < argc; i++) {
      if (!strcmp("--ntimes", argv[i]))
        n_times = atoi(argv[++i]);
      if (!strcmp("--maxiterations", argv[i]))
        this->maxiterations = atoi(argv[++i]);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  /// @brief Print relevant data collected during testing.
  //////////////////////////////////////////////////////////////////////////////
  void report()
  {
    if (get_location_id() == 0)
    {
      std::cout << "Test:" << this->name << " P=" << get_num_locations()
                << "\n";
      std::cout << this->name << "_iterations=" << this->iterations << "\n";
      std::cout << this->name << "_num_invocations_per_iteration="
                << this->n_times << "\n";

      if (m_coarse) {
        std::cout << this->name << "=" << this->avg/this->n_times << "\n";
        std::cout << this->name << "_conf=" << this->confidenceinterval << "\n";
        std::cout << this->name << "_min=" << this->min/this->n_times << "\n";
        std::cout << this->name << "_max=" << this->max/this->n_times << "\n";
      }
      else {
        std::cout << this->name << "=" << this->avg << "\n";
        std::cout << this->name << "_conf=" << this->confidenceinterval << "\n";
        std::cout << this->name << "_min=" << this->min << "\n";
        std::cout << this->name << "_max=" << this->max << "\n";
      }
      p_container_profiler_detail::print_extra_information<
        typename Counter::value_type, p_container_profiler>::print(*this);
      std::cout << "\n";
    }
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiles the default constructor for an arbitrary container.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class default_constructor_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  /// a vector of containers of type @p pC
  std::vector<pC*> m_pcs;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls 100 invocations by default.
  ///
  /// @param pcname A string containing the name of the container to test
  /// @param pc The container
  //////////////////////////////////////////////////////////////////////////////
  default_constructor_profiler(std::string pcname, pC* pc,
                                 int argc=0, char **argv=NULL)
    : base_type(pc, pcname+"_default_constructor", argc, argv)
  {
    this->m_coarse = true;
    size_t npcs=100;
    for (int i = 1; i < argc; i++) {
      if (!strcmp("--pcntimes", argv[i]))
        npcs = atoi(argv[++i]);
    }
    this->n_times = npcs;
    m_pcs.resize(npcs);
  }

  void run()
  {
    for (size_t i=0;i < this->m_pcs.size(); ++i)
      this->m_pcs[i] = new pC();
  }

  void finalize_iteration()
  {
    for (size_t i=0; i < this->m_pcs.size(); ++i)
      delete this->m_pcs[i];
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiles the constructor of an arbitrary container with a given size.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class constructor_size_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  /// The size of the container(s)
  size_t           m_ne;
  /// A vector of containers of type @p pC
  std::vector<pC*> m_pcs;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Creates a profiler that calls 100 invocations by default.
  ///
  /// @param pcname A string containing the name of the container to test
  /// @param pc The container
  /// @param N The size value to construct the container with
  //////////////////////////////////////////////////////////////////////////////
  constructor_size_profiler(std::string pcname, pC* pc, size_t N,
                              int argc=0, char **argv=NULL)
    : base_type(pc, pcname+"_constructor_size", argc, argv), m_ne(N)
  {
    this->m_coarse = true;
    size_t npcs=100;
    for (int i = 1; i < argc; i++) {
      if (!strcmp("--pcntimes", argv[i]))
        npcs=atoi(argv[++i]);
    }
    this->n_times = npcs;
    m_pcs.resize(npcs);
  }

  void run()
  {
    for (size_t i=0;i < this->m_pcs.size(); ++i)
      this->m_pcs[i] = new pC(m_ne);
  }

  void finalize_iteration()
  {
    for (size_t i=0; i < this->m_pcs.size(); ++i)
      delete this->m_pcs[i];
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiles the constructor of an arbitrary container with a given size
/// and value.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class constructor_size_value_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  /// The size of the container(s)
  size_t           m_ne;
  /// A vector of containers of type @p pC
  std::vector<pC*> m_pcs;
  /// The value to initialize the container elements with
  typename pC::value_type t;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls 100 invocations by default.
  ///
  /// @param pcname A string containing the name of the container to test
  /// @param pc The container
  /// @param N The size value to construct the container with
  //////////////////////////////////////////////////////////////////////////////
  constructor_size_value_profiler(std::string pcname, pC* pc, size_t N,
                                    int argc=0, char **argv=NULL)
    : base_type(pc, pcname+"_constructor_size_value", argc, argv), m_ne(N)
  {
    this->m_coarse = true;
    size_t npcs=100;
    for (int i = 1; i < argc; i++) {
      if (!strcmp("--pcntimes", argv[i])) npcs=atoi(argv[++i]);
    }
    this->n_times = npcs;
    m_pcs.resize(npcs);
  }

  void run()
  {
    for (size_t i=0;i < this->m_pcs.size(); ++i)
      this->m_pcs[i] = new pC(m_ne, t);
  }

  void finalize_iteration()
  {
    this->m_pcs[0]->memory_size();
    for (size_t i=0; i < this->m_pcs.size(); ++i)
      delete this->m_pcs[i];
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief  Profiles the copy constructor of an arbitrary container.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class copy_constructor_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  /// A vector of containers of type @p pC
  std::vector<pC*> m_pcs;

public:

  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls 100 invocations by default.
  ///
  /// @param pcname A string containing the name of the container to test
  /// @param pc The container
  //////////////////////////////////////////////////////////////////////////////
  copy_constructor_profiler(std::string pcname, pC* pc,
                              int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_copy_constructor", argc, argv)
  {
    this->m_coarse = true;
    size_t npcs=100;
    for (int i = 1; i < argc; i++) {
      if (!strcmp("--pcntimes", argv[i])) npcs=atoi(argv[++i]);
    }
    m_pcs.resize(npcs);
  }

  void run()
  {
    for (size_t i=0; i < this->m_pcs.size(); ++i)
      m_pcs[i] = new pC(*this->m_pc);
  }

  void finalize_iteration()
  {
    for (size_t i=0; i < this->m_pcs.size(); ++i)
      delete this->m_pcs[i];
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the set_element() function of the array and static array
/// containers.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
/// @tparam vec_value_type The type of indices to be used in testing; set to
/// size_t type if unspecified.
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer>,
         typename vec_value_type = size_t>
class set_element_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;

  /// A vector of index values of type @p vec_value_type
  std::vector<vec_value_type> const& indices;

public:

  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function once for every index
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param idx The indices to use for testing
  //////////////////////////////////////////////////////////////////////////////
  set_element_profiler(std::string pcname, pC* pc,
                       std::vector<vec_value_type> const& idx,
                       int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_set_element", argc, argv), indices(idx)
  {
    this->n_times = indices.size();
  }

  void run()
  {
    size_t sz = indices.size();
    for (size_t i=0; i<sz; ++i)
      this->m_pc->set_element(indices[i], i);
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the get_element() function of the array and static array
/// containers.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
/// @tparam vec_value_type The type of indices to be used in testing; set to
/// size_t type if unspecified.
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer>,
         typename vec_value_type = size_t>
class get_element_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;

  /// Stores the result of the get_element() function
  typename pC::value_type t;
  /// The indices to be used to testing
  std::vector<vec_value_type> const& indices;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function once for every index
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param idx The indices to use for testing
  //////////////////////////////////////////////////////////////////////////////
  get_element_profiler(std::string pcname, pC* pc,
                       std::vector<vec_value_type> const& idx,
                       int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_get_element", argc, argv), indices(idx)
  {
    this->n_times = indices.size();
  }

  void run()
  {
    size_t sz = indices.size();
    for (size_t i=0; i<sz; ++i)
      t += this->m_pc->get_element(indices[i]);
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the get_element_split() function of the array and static
/// array containers.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
/// @tparam vec_value_type The type of indices to be used in testing; set to
/// size_t type if unspecified.
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer>,
         typename vec_value_type = size_t>
class get_element_split_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  /// The sum of handle values
  typename pC::value_type t;
  /// The indices to be used for testing
  std::vector<vec_value_type> const& indices;
  /// The number of handles
  size_t m_k;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function once for every index
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param idx The indices to use for testing
  //////////////////////////////////////////////////////////////////////////////
  get_element_split_profiler(std::string pcname, pC* pc,
                               std::vector<vec_value_type> const& idx,
                               int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_get_element_split", argc, argv), indices(idx)
  {
    this->n_times = indices.size();
    m_k = 1000;
    for (int i = 1; i < argc; i++)
      if (!strcmp("--nhandles", argv[i])) m_k = atoi(argv[++i]);
  }

  void run()
  {
    std::vector<future<typename pC::value_type> > handles;
    size_t fc = 0;
    size_t sz = indices.size();
    for (size_t i = 0; i < sz; ++i) {
      //<-- the magic line
      handles.push_back(this->m_pc->get_element_split( indices[i] ));
      if (fc == m_k-1) {
        for (size_t j = 0; j != m_k; ++j)
          t+=handles[j].get();
        fc = 0;
        handles.clear();
      }
      else {
        ++fc;
      }
    }
    for (size_t j = 0; j != fc; ++j)
      t+=handles[j].get();
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the apply_set() function of the array and static
/// array containers.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Func, typename Counter = counter<default_timer> >
class apply_set_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  /// The work function to be applied
  Func& m_f;
  // The indices to be used for testing
  std::vector<size_t> const& indices;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function once for every index
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param f The functor to apply to the elements
  /// @param idx The indices to use for testing
  //////////////////////////////////////////////////////////////////////////////
  apply_set_profiler(std::string pcname, pC* pc, Func& f,
                     std::vector<size_t> const& idx,
                     int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_apply_set", argc, argv), m_f(f), indices(idx)
  {
    this->n_times = indices.size();
  }

  void run()
  {
    size_t sz = indices.size();
    for (size_t i=0; i<sz; ++i)
      this->m_pc->apply_set(indices[i], m_f);
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the apply_get() function of the array and static
/// array containers.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Func, typename Counter = counter<default_timer> >
class apply_get_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  /// Stores the return value of the function
  typename pC::value_type t;
  /// The work function apply
  Func& m_f;
  /// The indices to be used for testing
  std::vector<size_t> const& indices;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function once for every index
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param f The functor to apply to the elements
  /// @param idx The indices to use for testing
  //////////////////////////////////////////////////////////////////////////////
  apply_get_profiler(std::string pcname, pC* pc, Func& f,
                     std::vector<size_t> const& idx,
                     int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_apply_get", argc, argv), m_f(f), indices(idx)
  {
    this->n_times = indices.size();
  }

  void run()
  {
    size_t sz = indices.size();
    for (size_t i=0; i<sz; ++i)
      t += this->m_pc->apply_get(indices[i], m_f);
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the left handed side of the bracket operator of the
/// array and static array containers.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
/// @tparam vec_value_type The type of index values to be used; set to size_t
/// if unspecified
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer>,
         typename vec_value_type = size_t>
class operator_square_bracket_lhs_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  /// The indices to be used for testing
  std::vector<vec_value_type> const& indices;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function once for every index
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param idx The indices to use for testing
  //////////////////////////////////////////////////////////////////////////////
  operator_square_bracket_lhs_profiler(std::string pcname, pC* pc,
                                       std::vector<vec_value_type> const& idx,
                                       int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_operator[]-lhs", argc, argv), indices(idx)
  {
    this->n_times = indices.size();
  }

  void run()
  {
    pC& pc = *this->m_pc;
    size_t sz = indices.size();
    for (size_t i=0; i<sz; ++i)
      pc[indices[i]] = i;
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the right handed side of the bracket operator of the
/// array and static array containers.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
/// @tparam vec_value_type The type of index values to be used; set to size_t
/// if unspecified
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer>,
         typename vec_value_type = size_t>
class operator_square_bracket_rhs_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  /// Stores the return value of the operator
  typename pC::value_type m_value;
  /// The indices to be used for testing
  std::vector<vec_value_type> const& indices;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function once for every index
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param idx The indices to use for testing
  //////////////////////////////////////////////////////////////////////////////
  operator_square_bracket_rhs_profiler(std::string pcname, pC* pc,
                                       std::vector<vec_value_type> const& idx,
                                       int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_operator[]-rhs", argc, argv), indices(idx)
  {
    this->n_times = indices.size();
  }

  void run()
  {
    pC& pc = *this->m_pc;
    size_t sz = indices.size();
    for (size_t i=0; i<sz; ++i)
      m_value += pc[indices[i]];
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the resize() function of the array and static array
/// containers.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class resize_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  /// The value type of the container
  typename pC::value_type m_value;
  /// The value to resize the container with
  size_t m_nsz;
  size_t m_osz;

public:

  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function once for every index
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param sz The new size to resize pc to
  //////////////////////////////////////////////////////////////////////////////
  resize_profiler(std::string pcname, pC* pc, size_t sz,
                  int argc=0, char **argv=NULL)
    : base_type(pc, pcname+"_resize", argc, argv), m_nsz(sz), m_osz(pc->size())
  { }

  void run(void)
  {
    // expensive operation; only one invocation is timed
    this->m_pc->resize(m_nsz);
  }

  void finalize_iteration(void)
  {
    this->m_pc->resize(m_osz);
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the set_partition() function of the array and static
/// array containers.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename PS, typename Counter = counter<default_timer> >
class set_ps_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  /// The value type of the container
  typename pC::value_type m_value;
  /// The partition strategy
  PS const& m_ps;

public:

  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function once for every index
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param ps The partition strategy to apply
  //////////////////////////////////////////////////////////////////////////////
  set_ps_profiler(std::string pcname, pC* pc, PS const& ps,
                  int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_set_ps", argc, argv), m_ps(ps)
  { }

  void run()
  {
    //expensive operation; only one invocations is timed
    this->m_pc->set_partition(m_ps);
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Returns a single value stored in the element of a container.
///
/// @tparam Val The type of value to be returned
////////////////////////////////////////////////////////////////////////////////
template <class Val>
struct extractor
{
  static Val const& extract(Val const& v)
  { return v;}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Returns a the first value of a pair stored in a container.
///
/// @tparam Ret The return value type
/// @tparam Val The type of the second value
////////////////////////////////////////////////////////////////////////////////
template <class Ret, class Val>
struct extractor<std::pair<Ret,Val> >
{
  static Ret const& extract(std::pair<Ret,Val> const& v)
  { return v.first;}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the insert() function of the associative containers.
///
/// @tparam pC The container type
/// @tparam Input The input container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Input,
         typename Counter = counter<default_timer> >
class assoc_insert_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  /// The key values to use for testing
  Input*   m_ic;
  /// Iterators for the input container
  typename Input::iterator it, it_end;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function once for every index
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param in The user-provided test values
  //////////////////////////////////////////////////////////////////////////////
  assoc_insert_profiler(std::string pcname, pC* pc, Input* in,
                        int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_insert", argc, argv), m_ic(in)
  {
    it = m_ic->begin(); it_end = m_ic->end();
    this->n_times = m_ic->size();
  }

  void run()
  {
    for (it = m_ic->begin(); it!=it_end; ++it)
      this->m_pc->insert(*it);
    rmi_fence();
  }

  void finalize_iteration()
  {
    this->m_pc->clear();
  }

};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the make_reference() function of the associative
/// containers.
///
/// @tparam pC The container type
/// @tparam Input The input container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Input,
         typename Counter = counter<default_timer> >
class assoc_find_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  /// Extracts the key from the container elements
  typedef  extractor<typename Input::value_type> ext;
  /// The input values
  Input*   m_ic;
  /// Iterators for the input container
  typename Input::iterator it, it_end;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function once for every index
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param in The user-provided test values
  //////////////////////////////////////////////////////////////////////////////
  assoc_find_profiler(std::string pcname, pC* pc, Input* in,
                      int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_find", argc, argv), m_ic(in)
  {
    it     = m_ic->begin(); it_end = m_ic->end();
    this->n_times = m_ic->size();
    this->m_pc->clear();
    for (; it!=it_end; ++it)
      this->m_pc->insert(*it);
    rmi_fence();
  }

  void run()
  {
    for (it=m_ic->begin(); it!=it_end; ++it)
      this->m_pc->make_reference(ext::extract(*it));
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the bracket operator of the associative containers.
///
/// @tparam pC The container type
/// @tparam Input The input container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Input,
         typename Counter = counter<default_timer> >
class assoc_find_val_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter>     base_type;
  typedef extractor<typename Input::value_type> ext;
  /// The input values
  Input*   m_ic;
  /// Stores the returned value
  int m_val;
  /// Iterators for the input container
  typename Input::iterator it, it_end;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function once for every index
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param in The user-provided test values
  //////////////////////////////////////////////////////////////////////////////
  assoc_find_val_profiler(std::string pcname, pC* pc, Input* in,
                          int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_find_val", argc, argv), m_ic(in)
  {
    it = m_ic->begin(); it_end = m_ic->end();
    this->n_times = m_ic->size();
    this->m_pc->clear();
    for (; it!=it_end; ++it)
      this->m_pc->insert(*it);
    rmi_fence();
  }

  void run()
  {
    for (it=m_ic->begin(); it!=it_end; ++it)
      m_val = (*(this->m_pc))[ext::extract(*it)];
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the find_split() function of the associative containers.
///
/// @tparam pC The container type
/// @tparam Input The input container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Input,
         typename Counter = counter<default_timer> >
class assoc_find_split_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  typedef typename pC::value_type value_type;
  typedef  extractor<typename Input::value_type> ext;
  /// The input values
  Input*   m_ic;
  /// Iterators for the input container
  typename Input::iterator it, it_end;
  /// The number of handles
  size_t m_k;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function once for every index
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param in The user-provided test values
  //////////////////////////////////////////////////////////////////////////////
  assoc_find_split_profiler(std::string pcname, pC* pc, Input* in,
                            int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_find_split", argc, argv), m_ic(in)
  {
    m_k = 1000; //set default; if provided as arg reset below
    for (int i = 1; i < argc; i++)
      if (!strcmp("--nhandles", argv[i])) m_k = atoi(argv[++i]);
    it = m_ic->begin(); it_end = m_ic->end();
    this->n_times = m_ic->size();
    this->m_pc->clear();
    for (; it!=it_end; ++it)
      this->m_pc->insert_async( value_type(*it) );
    rmi_fence();
  }

  void run()
  {
    std::vector<future<value_type> > handles;
    size_t fc = 0;
    for (it=m_ic->begin(); it!=it_end; ++it) {
      //<-- the magic line
      handles.push_back(this->m_pc->find_split( ext::extract(*it) ));
      if (fc == m_k - 1) {
        for (size_t j=0;j != m_k; ++j)
          handles[j].get();
        fc = 0;
        handles.clear();
      }
      else {
        ++fc;
      }
    }
    for (size_t j=0;j != fc; ++j)
      handles[j].get();
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the erase() function of the associative containers.
///
/// @tparam pC The container type
/// @tparam Input The input container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Input,
         typename Counter = counter<default_timer> >
class assoc_erase_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  typedef  extractor<typename Input::value_type> ext;
  /// The input values
  Input*   m_ic;
  /// Iterators for the input container
  typename Input::iterator it, it_end;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function once for every index
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param in The user-provided test values
  //////////////////////////////////////////////////////////////////////////////
  assoc_erase_profiler(std::string pcname, pC* pc, Input* in,
                       int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_erase", argc, argv), m_ic(in)
  {
    it = m_ic->begin(); it_end = m_ic->end();
    this->n_times = m_ic->size();
  }

  void initialize_iteration()
  {
    for (it=m_ic->begin(); it!=it_end; ++it)
      this->m_pc->insert(*it);
    rmi_fence();
  }

  void run()
  {
    for (it=m_ic->begin(); it!=it_end; ++it)
      this->m_pc->erase(ext::extract(*it));
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the add_vertex() function of the graph container.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class add_vertex_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function the specified number of
  /// times.
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param nt The number of times to call the function
  //////////////////////////////////////////////////////////////////////////////
  add_vertex_profiler(std::string pcname, pC* pc, size_t nt,
                      int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_add_vertex", argc, argv)
  {
    this->n_times = nt;
  }

  void run()
  {
    for (size_t i=0; i < this->n_times; ++i)
      this->m_pc->add_vertex();
    rmi_fence();
  }

  void finalize_iteration()
  {
    this->m_pc->clear();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the add_edge() function of the graph container.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class add_edge_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  typedef typename pC::vertex_descriptor vd_type;
  typedef std::vector<std::pair<vd_type,vd_type> > vedges_type;
  /// Iterators for the input container
  typename vedges_type::iterator it, it_end;
  // The input values
  vedges_type& m_edges;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function the specified number of
  /// times.
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param nt The number of times to call the function
  /// @param v The input values
  //////////////////////////////////////////////////////////////////////////////
  add_edge_profiler(std::string pcname, pC* pc, size_t nt, vedges_type& v,
                    int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_add_edge", argc, argv) , m_edges(v)
  {
    it_end = m_edges.end();
    this->n_times = nt;
  }

  void initialize_iteration()
  {
    this->m_pc->clear();
    for (size_t i=0; i < this->n_times; ++i)
      this->m_pc->add_vertex();
    rmi_fence();
  }

  void run()
  {
    for (it = m_edges.begin(); it != it_end; ++it)
      this->m_pc->add_edge(it->first, it->second);
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the add_edge_async() function of the graph container.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class add_edge_async_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  typedef typename pC::vertex_descriptor vd_type;
  typedef typename pC::edge_descriptor   ed_type;
  typedef std::vector<std::pair<vd_type,vd_type> > vedges_type;
  /// Iterators for the input container
  typename vedges_type::iterator it, it_end;
  /// The input values
  vedges_type& m_edges;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function the specified number of
  /// times.
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param nt The number of times to call the function
  /// @param v The input values
  //////////////////////////////////////////////////////////////////////////////
  add_edge_async_profiler(std::string pcname, pC* pc, size_t nt, vedges_type& v,
                          int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_add_edge_async", argc, argv) , m_edges(v)
  {
    it_end = m_edges.end();
    this->n_times = nt;
  }

  void initialize_iteration()
  {
    this->m_pc->clear();
    for (size_t i=0; i < this->n_times; ++i)
      this->m_pc->add_vertex();
    rmi_fence();
  }

  void run()
  {
    for (it = m_edges.begin(); it != it_end; ++it)
      this->m_pc->add_edge_async(ed_type(it->first, it->second));
    rmi_fence();
  }

  void finalize_iteration()
  {
    this->m_pc->clear();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the delete_edge() function of the graph container.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class delete_edge_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  typedef typename pC::vertex_descriptor vd_type;
  typedef typename pC::edge_descriptor   ed_type;
  typedef std::vector<std::pair<vd_type,vd_type> > vedges_type;
  /// Iterators for the input container
  typename vedges_type::iterator it, it_end;
  /// The input values
  vedges_type& m_edges;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function the specified number of
  /// times.
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param nt The number of times to call the function
  /// @param v The input values
  //////////////////////////////////////////////////////////////////////////////
  delete_edge_profiler(std::string pcname, pC* pc, size_t nt, vedges_type& v,
                       int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_delete_edge", argc, argv) , m_edges(v)
  {
    it_end = m_edges.end();
    this->n_times = nt;
  }

  void initialize_iteration()
  {
    for (it = m_edges.begin(); it != it_end; ++it)
      this->m_pc->add_edge(ed_type(it->first, it->second));
    rmi_fence();
  }

  void run()
  {
    for (it = m_edges.begin(); it != it_end; ++it)
      this->m_pc->delete_edge(it->first, it->second);
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the find_vertex() function of the graph container.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class find_vertex_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  typedef typename pC::vertex_descriptor  vd_type;
  typedef std::vector<vd_type>   verts_type;
  /// Iterators for the input container
  typename verts_type::iterator  it, it_end;
  /// The input values
  verts_type&                    m_verts;
  /// Iterators for the target container
  typename pC::vertex_iterator   vit, vit_end;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function the specified number of
  /// times.
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param nt The number of times to call the function
  /// @param v The input values
  //////////////////////////////////////////////////////////////////////////////
  find_vertex_profiler(std::string pcname, pC* pc, size_t nt, verts_type& v,
                      int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_find_vertex", argc, argv) , m_verts(v)
  {
    it_end = m_verts.end();
    vit_end = this->m_pc->end();
    this->n_times = nt;
  }

  void run()
  {
    for (it = m_verts.begin(); it != it_end; ++it) {
      vit = this->m_pc->find_vertex(*it);
      stapl_assert(vit != vit_end, "Failed find vertex");
    }
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the find_edge() function of the graph container.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class find_edge_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  typedef typename pC::edge_descriptor   ed_type;
  typedef typename pC::vertex_descriptor vd_type;
  typedef std::vector<std::pair<vd_type,vd_type> > vedges_type;
  /// Iterators for the input container
  typename vedges_type::iterator it, it_end;
  /// Iterator to the vertices of the container
  typename pC::vertex_iterator   vit;
  /// Iterator to the edges of the container
  typename pC::adj_edge_iterator eit;
  /// The input container
  vedges_type& m_edges;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function the specified number of
  /// times.
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param nt The number of times to call the function
  /// @param v The input values
  //////////////////////////////////////////////////////////////////////////////
  find_edge_profiler(std::string pcname, pC* pc, size_t nt, vedges_type& v,
                     int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_find_edge", argc, argv) , m_edges(v)
  {
    it_end = m_edges.end();
    this->n_times = nt;
  }

  void run()
  {
    for (it = m_edges.begin(); it != it_end; ++it) {
#ifndef STAPL_NDEBUG
      bool b = this->m_pc->find_edge(ed_type(it->first, it->second),vit,eit);
      stapl_assert(b, "Failed find edge");
#else
      this->m_pc->find_edge(ed_type(it->first, it->second),vit,eit);
#endif
    }
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the delete_vertex() function of the graph container.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class delete_vertex_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  typedef typename pC::vertex_descriptor  vd_type;
  typedef std::vector<vd_type>   verts_type;
  /// Iterators for the input container
  typename verts_type::iterator  it, it_end;
  // The input container
  verts_type&                    m_verts;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function the specified number of
  /// times.
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param nt The number of times to call the function
  /// @param v The input values
  //////////////////////////////////////////////////////////////////////////////
  delete_vertex_profiler(std::string pcname, pC* pc, size_t nt, verts_type& v,
                         int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_delete_vertex", argc, argv) , m_verts(v)
  {
    it     = m_verts.begin();
    it_end = m_verts.end();
    this->n_times = nt;
  }

  void run()
  {
    for (; it != it_end; ++it)
      this->m_pc->delete_vertex(*it);
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the push_back() function of the list container.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class push_back_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  typedef typename pC::value_type value_type;
  /// The input values
  std::vector<value_type> const& indices;
  /// The size of the input container
  const size_t sz;

public:

  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function the specified number of
  /// times.
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param idx The input values
  //////////////////////////////////////////////////////////////////////////////
  push_back_profiler(pC* pc, std::string pcname,
                     std::vector<value_type> const& idx,
                     int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_push_back", argc, argv), indices(idx), sz(idx.size())
  { }

  void run()
  {
    for (size_t i=0; i!=sz; ++i)
      this->m_pc->push_back(indices[i]);
    rmi_fence();
  }

  void finalize_iteration()
  {
    rmi_fence();
    this->m_pc->clear();
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the push_front() function of the list container.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class push_front_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  typedef typename pC::value_type value_type;
  /// The input values
  std::vector<value_type> const& indices;
  /// The size of the input container
  const size_t sz;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function the specified number of
  /// times.
  ///
  /// @param pcname A string containing the name of the container
  /// @param pc The container to be tested
  /// @param idx The input values
  //////////////////////////////////////////////////////////////////////////////
  push_front_profiler(pC* pc, std::string pcname,
                     std::vector<value_type> const& idx,
                     int argc=0, char **argv=NULL)
  : base_type(pc, pcname+"_push_front", argc, argv), indices(idx),
    sz(idx.size())
  { }

  void run()
  {
    for (size_t i=0; i!=sz; ++i)
      this->m_pc->push_front(indices[i]);
    rmi_fence();
  }

  void finalize_iteration()
  {
    rmi_fence();
    this->m_pc->clear();
    rmi_fence();
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Profiler for the push_anywhere() function of the list container.
///
/// @tparam pC The container type
/// @tparam Counter The timing construct
///
/// @ingroup performanceMonitor
////////////////////////////////////////////////////////////////////////////////
template<typename pC, typename Counter = counter<default_timer> >
class push_anywhere_profiler
  : public p_container_profiler<pC, Counter>
{
  typedef p_container_profiler<pC, Counter> base_type;
  typedef typename pC::value_type value_type;
  /// The input values
  std::vector<value_type> const& indices;
  /// The size of the input container
  const size_t sz;
  /// The total size of the target container
  const size_t nElements;

public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief Create a profiler that calls the function the specified number of
  /// times.
  ///
  /// @param pcname A string containing the name of the container
  /// @param idx The input values
  /// @param N The total size of the target container
  //////////////////////////////////////////////////////////////////////////////
  push_anywhere_profiler(std::string pcname, std::vector<value_type> const& idx,
                         size_t N, int argc=0, char **argv=NULL)
   : base_type(NULL, pcname+"_push_anywhere", argc, argv), indices(idx),
     sz(idx.size()), nElements(N)
  { }

  void initialize_iteration()
  {
    this->m_pc = new pC(this->get_num_locations());
  }

  void run()
  {
    for (size_t i=0; i!=sz; ++i)
      this->m_pc->push_anywhere(indices[i]);
    rmi_fence();
  }

  void finalize_iteration()
  {
    rmi_fence();
    delete this->m_pc;
    rmi_fence();
  }
};

} // namespace stapl

#endif
