#include <functional>
#include <Kripke/SubTVec.h>
#include <stapl/multiarray.hpp>
#include <stapl/views/slices_view.hpp>
#include <stapl/containers/multiarray/base_container.hpp>
#include <stapl/containers/type_traits/default_traversal.hpp>
#include <stapl/runtime/counter/default_counters.hpp>
#include <stapl/utility/tuple.hpp>
#include <stapl/utility/integer_sequence.hpp>
#include <confint.hpp>
#include <boost/multi_array.hpp>

#ifdef KOKKOS_DEFINED
#include <Kokkos_Core.hpp>

typedef Kokkos::View<double *****, Kokkos::LayoutStride, Kokkos::Serial>
          kokkos_arr_t;
#endif

typedef stapl::counter<stapl::default_timer>  counter_t;
typedef boost::multi_array<double, 5>         boost_arr_t;
typedef boost::general_storage_order<5>       storage_t;

struct Bench_User_Data
{
  Nesting_Order nest;
  int           nx, ny, nz;
  int           dirs, d_ds;
  int           grps, g_gs;
  int           nm;

  typedef std::tuple<size_t, size_t, size_t, size_t, size_t> index_type;

  Bench_User_Data(int argc, char** argv)
    : nest(nestingFromString(argv[1])),
      nx(atoi(argv[2])), ny(atoi(argv[3])), nz(atoi(argv[4])),
      dirs(atoi(argv[5])), d_ds(atoi(argv[8])),
      grps(atoi(argv[6])), g_gs(atoi(argv[9])), nm(atoi(argv[7]))
  { }

  index_type first(void) const
  { return index_type(0, 0, 0, 0, 0); }

  // psi and rhs have dimensions (nx, ny, nz, d_ds, g_gs)
  index_type psi_last(void) const
  { return index_type(nx-1, ny-1, nz-1, d_ds-1, g_gs-1); }

  // sigt has dimensions (nx, ny, nz, 1, g_gs)
  index_type sigt_last(void) const
  { return index_type(nx-1, ny-1, nz-1, 0, g_gs-1); }

  // phi and phi_out have dimensions (nx, ny, nz, nm, g)
  index_type phi_last(void) const
  { return index_type(nx-1, ny-1, nz-1, nm-1, grps-1); }

  // ell and ell_plus have dimensions (1, 1, 1, d, nm)
  index_type ell_last(void) const
  { return index_type(0, 0, 0, dirs-1, nm-1); }

  void print(void)
  {
    std::cerr << "Nesting: " << nestingString(nest)
              << ", nx " << nx << " ny " << ny << " nz " << nz
              << ", dirs " << dirs << " dirs/ds " << d_ds
              << ", grps " << grps << " grps/gs " << g_gs
              << ", nmts " << nm << std::endl;
  }
};

template<typename Traversal>
struct compute_bc_type;

//////////////////////////////////////////////////////////////////////
/// @brief Compute the base container type given the traversal order.
///
/// @todo Add separate types for phi and ell base containers to match what
/// Kripke SubTVec setup does to evaluate performance difference
//////////////////////////////////////////////////////////////////////
template<std::size_t... TraversalIndices>
struct compute_bc_type<stapl::index_sequence<TraversalIndices...>>
{
  typedef std::tuple<boost::mpl::int_<TraversalIndices>... > traversal_t;
  typedef stapl::indexed_domain<size_t, 5, traversal_t> domain_t;
  typedef typename domain_t::index_type                 cid_t;
  typedef stapl::multiarray_base_container_traits<double, 5, traversal_t>
            traits_t;
  typedef stapl::multiarray_base_container<double, domain_t, cid_t, traits_t>
            container_t;
};

template<Nesting_Order>
struct base_container_traits;

template<>
struct base_container_traits<NEST_ZDG>
  : public compute_bc_type<stapl::index_sequence<2, 3, 4, 1, 0>>
{ };

template<>
struct base_container_traits<NEST_ZGD>
  : public compute_bc_type<stapl::index_sequence<2, 3, 4, 0, 1>>
{ };

template<>
struct base_container_traits<NEST_GZD>
  : public compute_bc_type<stapl::index_sequence<1, 2, 3, 0, 4>>
{ };

template<>
struct base_container_traits<NEST_GDZ>
  : public compute_bc_type<stapl::index_sequence<0, 1, 2, 3, 4>>
{ };

template<>
struct base_container_traits<NEST_DGZ>
  : public compute_bc_type<stapl::index_sequence<0, 1, 2, 4, 3>>
{ };

template<>
struct base_container_traits<NEST_DZG>
  : public compute_bc_type<stapl::index_sequence<1, 2, 3, 4, 0>>
{ };

template <typename Traversal>
struct compute_multiarray_type
{
  typedef typename stapl::default_traversal<5, Traversal>::type   traversal_t;
  typedef typename stapl::multiarray<5, double, traversal_t>      container_t;
};

template<Nesting_Order>
struct multiarray_traits;

template<>
struct multiarray_traits<NEST_ZDG>
  : public compute_multiarray_type<stapl::index_sequence<2, 1, 0, 3, 4>>
{ };

template<>
struct multiarray_traits<NEST_ZGD>
  : public compute_multiarray_type<stapl::index_sequence<2, 1, 0, 4, 3>>
{ };

template<>
struct multiarray_traits<NEST_DZG>
  : public compute_multiarray_type<stapl::index_sequence<3, 2, 1, 0, 4>>
{ };

template<>
struct multiarray_traits<NEST_GZD>
  : public compute_multiarray_type<stapl::index_sequence<3, 2, 1, 4, 0>>
{ };

template<>
struct multiarray_traits<NEST_DGZ>
  : public compute_multiarray_type<stapl::index_sequence<4, 3, 2, 0, 1>>
{ };

template<>
struct multiarray_traits<NEST_GDZ>
  : public compute_multiarray_type<stapl::index_sequence<4, 3, 2, 1, 0>>
{ };

// populate function used by Boost, SubTVec, and multiarray base container
void populate(double* ptr, size_t size)
{
  srand48(31415926530);
  for (size_t i = 0; i != size; ++ptr, ++i)
    *ptr = drand48();
}

// populate function used by stapl::multiarray
template <typename Multiarray>
void populate(Multiarray* cont,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  srand48(31415926530);

  for (size_t d = ds; d != dl; ++d)
    for (size_t g = gs; g != gl; ++g)
      for (size_t k = ks; k != kl; ++k)
        for (size_t j = js; j != jl; ++j)
          for (size_t i = is; i != il; ++i)
            (*cont)(i, j, k, d, g) = drand48();
}

#ifdef KOKKOS_DEFINED
// populate function used by Kokkos multiarray
void populate(kokkos_arr_t* ptr)
{
  srand48(31415926530);

  for (size_t i = 0; i != ptr->dimension(0); ++i)
    for (size_t j = 0; j != ptr->dimension(1); ++j)
      for (size_t k = 0; k != ptr->dimension(2); ++k)
        for (size_t d = 0; d != ptr->dimension(3); ++d)
          for (size_t g = 0; g != ptr->dimension(4); ++g)
            (*ptr)(i, j, k, d, g) = drand48();
}

template<Nesting_Order>
void traverse(kokkos_arr_t*, int, int, int, int, int)
{
  std::cerr << "Nesting order unrecognized for Kokkos array traversal.\n";
  exit(1);
}

template<>
void traverse<NEST_ZDG>(kokkos_arr_t* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int k = 0; k != b5; ++k)
    for (int j = 0; j != b4; ++j)
      for (int i = 0; i != b3; ++i)
        for (int d = 0; d != b2; ++d)
          for (int g = 0; g != b1; ++g)
            (*bcontainer)(i, j, k, d, g) *= 1.4142135624;
}

template<>
void traverse<NEST_ZGD>(kokkos_arr_t* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int k = 0; k != b5; ++k)
    for (int j = 0; j != b4; ++j)
      for (int i = 0; i != b3; ++i)
        for (int g = 0; g != b1; ++g)
          for (int d = 0; d != b2; ++d)
            (*bcontainer)(i, j, k, d, g) *= 1.4142135624;
}

template<>
void traverse<NEST_GZD>(kokkos_arr_t* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int g = 0; g != b1; ++g)
    for (int k = 0; k != b5; ++k)
      for (int j = 0; j != b4; ++j)
        for (int i = 0; i != b3; ++i)
          for (int d = 0; d != b2; ++d)
            (*bcontainer)(i, j, k, d, g) *= 1.4142135624;
}

template<>
void traverse<NEST_GDZ>(kokkos_arr_t* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int g = 0; g != b1; ++g)
    for (int d = 0; d != b2; ++d)
      for (int k = 0; k != b5; ++k)
        for (int j = 0; j != b4; ++j)
          for (int i = 0; i != b3; ++i)
            (*bcontainer)(i, j, k, d, g) *= 1.4142135624;
}

template<>
void traverse<NEST_DGZ>(kokkos_arr_t* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int d = 0; d != b2; ++d)
    for (int g = 0; g != b1; ++g)
      for (int k = 0; k != b5; ++k)
        for (int j = 0; j != b4; ++j)
          for (int i = 0; i != b3; ++i)
            (*bcontainer)(i, j, k, d, g) *= 1.4142135624;
}

template<>
void traverse<NEST_DZG>(kokkos_arr_t* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int d = 0; d != b2; ++d)
    for (int k = 0; k != b5; ++k)
      for (int j = 0; j != b4; ++j)
        for (int i = 0; i != b3; ++i)
          for (int g = 0; g != b1; ++g)
            (*bcontainer)(i, j, k, d, g) *= 1.4142135624;
}

template<Nesting_Order Nest>
std::tuple<std::string, double, double, double, double>
run_kokkos(Nesting_Order actual_nest, int size1, int size2,
           int size3, int size4, int size5)
{
  int layout[5];
  size_t dims[] = {unsigned(size3), unsigned(size4),
    unsigned(size5), unsigned(size2), unsigned(size1)};

  switch(actual_nest)
  {
    case NEST_DGZ:
      layout[0] = 0;
      layout[1] = 1;
      layout[2] = 2;
      layout[3] = 4;
      layout[4] = 3;
      break;
    case NEST_DZG:
      layout[0] = 4;
      layout[1] = 0;
      layout[2] = 1;
      layout[3] = 2;
      layout[4] = 3;
      break;
    case NEST_GDZ:
      layout[0] = 0;
      layout[1] = 1;
      layout[2] = 2;
      layout[3] = 3;
      layout[4] = 4;
      break;
    case NEST_GZD:
      layout[0] = 3;
      layout[1] = 0;
      layout[2] = 1;
      layout[3] = 2;
      layout[4] = 4;
      break;
    case NEST_ZDG:
      layout[0] = 4;
      layout[1] = 3;
      layout[2] = 0;
      layout[3] = 1;
      layout[4] = 2;
      break;
    case NEST_ZGD:
      layout[0] = 3;
      layout[1] = 4;
      layout[2] = 0;
      layout[3] = 1;
      layout[4] = 2;
      break;
  }

  Kokkos::LayoutStride stride
    = Kokkos::LayoutStride::order_dimensions(5, layout, dims);

  counter_t timer;
  timer.reset();
  std::tuple<std::string, double, double, double, double> times;
  std::get<0>(times) = nestingString(actual_nest);

  // Construction
  timer.start();
  kokkos_arr_t* bc = new kokkos_arr_t("kokko_arr", stride);
  std::get<1>(times) = timer.stop();
  timer.reset();

  // Initialization over raw data
  timer.start();
  populate(bc);
  std::get<2>(times) = timer.stop();
  timer.reset();

  // Traversal using index operators
  timer.start();
  traverse<Nest>(bc, size1, size2, size3, size4, size5);
  std::get<3>(times) = timer.stop();
  timer.reset();

  // Destruct
  timer.start();
  delete bc;
  std::get<4>(times) = timer.stop();

  return times;
}
#endif

template<Nesting_Order>
void traverse(boost_arr_t*, int, int, int, int, int)
{
  std::cerr << "Nesting order unrecognized for Boost array traversal.\n";
  exit(1);
}

template<>
void traverse<NEST_ZDG>(boost_arr_t* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int k = 0; k != b5; ++k)
    for (int j = 0; j != b4; ++j)
      for (int i = 0; i != b3; ++i)
        for (int d = 0; d != b2; ++d)
          for (int g = 0; g != b1; ++g)
            (*bcontainer)[i][j][k][d][g] *= 1.4142135624;
}

template<>
void traverse<NEST_ZGD>(boost_arr_t* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int k = 0; k != b5; ++k)
    for (int j = 0; j != b4; ++j)
      for (int i = 0; i != b3; ++i)
        for (int g = 0; g != b1; ++g)
          for (int d = 0; d != b2; ++d)
            (*bcontainer)[i][j][k][d][g] *= 1.4142135624;
}

template<>
void traverse<NEST_GZD>(boost_arr_t* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int g = 0; g != b1; ++g)
    for (int k = 0; k != b5; ++k)
      for (int j = 0; j != b4; ++j)
        for (int i = 0; i != b3; ++i)
          for (int d = 0; d != b2; ++d)
            (*bcontainer)[i][j][k][d][g] *= 1.4142135624;
}

template<>
void traverse<NEST_GDZ>(boost_arr_t* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int g = 0; g != b1; ++g)
    for (int d = 0; d != b2; ++d)
      for (int k = 0; k != b5; ++k)
        for (int j = 0; j != b4; ++j)
          for (int i = 0; i != b3; ++i)
            (*bcontainer)[i][j][k][d][g] *= 1.4142135624;
}

template<>
void traverse<NEST_DGZ>(boost_arr_t* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int d = 0; d != b2; ++d)
    for (int g = 0; g != b1; ++g)
      for (int k = 0; k != b5; ++k)
        for (int j = 0; j != b4; ++j)
          for (int i = 0; i != b3; ++i)
            (*bcontainer)[i][j][k][d][g] *= 1.4142135624;
}

template<>
void traverse<NEST_DZG>(boost_arr_t* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int d = 0; d != b2; ++d)
    for (int k = 0; k != b5; ++k)
      for (int j = 0; j != b4; ++j)
        for (int i = 0; i != b3; ++i)
          for (int g = 0; g != b1; ++g)
            (*bcontainer)[i][j][k][d][g] *= 1.4142135624;
}

template<Nesting_Order Nest>
std::tuple<std::string, double, double, double, double>
run_boost(int size1, int size2, int size3, int size4, int size5)
{
  boost_arr_t::size_type layout[5];
  bool ordering[] = {true, true, true, true, true};

  switch(Nest)
  {
    case NEST_DGZ:
      layout[0] = 0;
      layout[1] = 1;
      layout[2] = 2;
      layout[3] = 4;
      layout[4] = 3;
      break;
    case NEST_DZG:
      layout[0] = 4;
      layout[1] = 0;
      layout[2] = 1;
      layout[3] = 2;
      layout[4] = 3;
      break;
    case NEST_GDZ:
      layout[0] = 0;
      layout[1] = 1;
      layout[2] = 2;
      layout[3] = 3;
      layout[4] = 4;
      break;
    case NEST_GZD:
      layout[0] = 3;
      layout[1] = 0;
      layout[2] = 1;
      layout[3] = 2;
      layout[4] = 4;
      break;
    case NEST_ZDG:
      layout[0] = 4;
      layout[1] = 3;
      layout[2] = 0;
      layout[3] = 1;
      layout[4] = 2;
      break;
    case NEST_ZGD:
      layout[0] = 3;
      layout[1] = 4;
      layout[2] = 0;
      layout[3] = 1;
      layout[4] = 2;
      break;
  }

  counter_t timer;
  timer.reset();
  std::tuple<std::string, double, double, double, double> times;
  std::get<0>(times) = nestingString(Nest);

  // Construction
  timer.start();
  boost_arr_t* bc
    = new boost_arr_t(boost::extents[size3][size4][size5][size2][size1],
        storage_t(layout, ordering));
  std::get<1>(times) = timer.stop();
  timer.reset();

  // Initialization over raw data
  timer.start();
  populate(bc->origin(), size1*size2*size3*size4*size5);
  std::get<2>(times) = timer.stop();
  timer.reset();

  // Traversal using index operators
  timer.start();
  traverse<Nest>(bc, size1, size2, size3, size4, size5);
  std::get<3>(times) = timer.stop();
  timer.reset();

  // Destruct
  timer.start();
  delete bc;
  std::get<4>(times) = timer.stop();

  return times;
}


// Traverse a SubTVec instance given the nesting order.
// Note: restrict keyword not used at this point.
template<Nesting_Order>
void traverse(SubTVec*, int, int, int, int, int)
{
  std::cerr << "Nesting order unrecognized for SubTVec traversal.\n";
  exit(1);
}

template<>
void traverse<NEST_ZDG>(SubTVec* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int k = 0; k != b5; ++k)
  {
    for (int j = 0; j != b4; ++j)
    {
      for (int i = 0; i != b3; ++i)
      {
        int z = i + j * b3 + k * b3 * b4;
        for (int d = 0; d != b2; ++d)
        {
          double* bc_z_d = bcontainer->ptr(0, d, z);
          for (int g = 0; g != b1; ++g)
          {
            bc_z_d[g] *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<>
void traverse<NEST_ZGD>(SubTVec* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int k = 0; k != b5; ++k)
  {
    for (int j = 0; j != b4; ++j)
    {
      for (int i = 0; i != b3; ++i)
      {
        int z = i + j * b3 + k * b3 * b4;
        for (int g = 0; g != b1; ++g)
        {
          double* bc_z_g = bcontainer->ptr(g, 0, z);
          for (int d = 0; d != b2; ++d)
          {
            bc_z_g[d] *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<>
void traverse<NEST_GZD>(SubTVec* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int g = 0; g != b1; ++g)
  {
    for (int k = 0; k != b5; ++k)
    {
      for (int j = 0; j != b4; ++j)
      {
        for (int i = 0; i != b3; ++i)
        {
          int z = i + j * b3 + k * b3 * b4;
          double* bc_g_z = bcontainer->ptr(g, 0, z);
          for (int d = 0; d != b2; ++d)
          {
            bc_g_z[d] *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<>
void traverse<NEST_GDZ>(SubTVec* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int g = 0; g != b1; ++g)
  {
    for (int d = 0; d != b2; ++d)
    {
      double* bc_g_d = bcontainer->ptr(g, d, 0);
      for (int k = 0; k != b5; ++k)
      {
        for (int j = 0; j != b4; ++j)
        {
          int z = j * b3 + k * b3 * b4;
          for (int i = 0; i != b3; ++i, ++z)
          {
            bc_g_d[z] *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<>
void traverse<NEST_DGZ>(SubTVec* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int d = 0; d != b2; ++d)
  {
    for (int g = 0; g != b1; ++g)
    {
      double* bc_d_g = bcontainer->ptr(g, d, 0);
      for (int k = 0; k != b5; ++k)
      {
        for (int j = 0; j != b4; ++j)
        {
          int z = j * b3 + k * b3 * b4;
          for (int i = 0; i != b3; ++i, ++z)
          {
            bc_d_g[z] *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<>
void traverse<NEST_DZG>(SubTVec* bcontainer,
                        int b1, int b2, int b3, int b4, int b5)
{
  for (int d = 0; d != b2; ++d)
  {
    for (int k = 0; k != b5; ++k)
    {
      for (int j = 0; j != b4; ++j)
      {
        for (int i = 0; i != b3; ++i)
        {
          int z = i + j * b3 + k * b3 * b4;
          double* bc_d_z = bcontainer->ptr(0, d, z);
          for (int g = 0; g != b1; ++g)
          {
            bc_d_z[g] *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<Nesting_Order Nest>
std::tuple<std::string, double, double, double, double>
run_subtvec(Nesting_Order actual_nest, int size1, int size2,
            int size3, int size4, int size5)
{
  counter_t timer;
  timer.reset();
  std::tuple<std::string, double, double, double, double> times;
  std::get<0>(times) = nestingString(actual_nest);

  // Construction
  timer.start();
  SubTVec* bc = new SubTVec(Nest, size1, size2, size3*size4*size5);
  std::get<1>(times) = timer.stop();
  timer.reset();

  // Initialization over raw data
  timer.start();
  populate(bc->ptr(), size1*size2*size3*size4*size5);
  std::get<2>(times) = timer.stop();
  timer.reset();

  // Traversal using index operators
  timer.start();
  traverse<Nest>(bc, size1, size2, size3, size4, size5);
  std::get<3>(times) = timer.stop();
  timer.reset();

  // Destruct
  timer.start();
  delete bc;
  std::get<4>(times) = timer.stop();

  return times;
}


void traverse(base_container_traits<NEST_ZDG>::container_t* bc,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t k = ks; k != kl; ++k)
  {
    for (size_t j = js; j != jl; ++j)
    {
      for (size_t i = is; i != il; ++i)
      {
        for (size_t d = ds; d != dl; ++d)
        {
          for (size_t g = gs; g != gl; ++g)
          {
            (*bc)(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(base_container_traits<NEST_ZGD>::container_t* bc,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t k = ks; k != kl; ++k)
  {
    for (size_t j = js; j != jl; ++j)
    {
      for (size_t i = is; i != il; ++i)
      {
        for (size_t g = gs; g != gl; ++g)
        {
          for (size_t d = ds; d != dl; ++d)
          {
            (*bc)(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(base_container_traits<NEST_GZD>::container_t* bc,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t g = gs; g != gl; ++g)
  {
    for (size_t k = ks; k != kl; ++k)
    {
      for (size_t j = js; j != jl; ++j)
      {
        for (size_t i = is; i != il; ++i)
        {
          for (size_t d = ds; d != dl; ++d)
          {
            (*bc)(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(base_container_traits<NEST_GDZ>::container_t* bc,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t g = gs; g != gl; ++g)
  {
    for (size_t d = ds; d != dl; ++d)
    {
      for (size_t k = ks; k != kl; ++k)
      {
        for (size_t j = js; j != jl; ++j)
        {
          for (size_t i = is; i != il; ++i)
          {
            (*bc)(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(base_container_traits<NEST_DGZ>::container_t* bc,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t d = ds; d != dl; ++d)
  {
    for (size_t g = gs; g != gl; ++g)
    {
      for (size_t k = ks; k != kl; ++k)
      {
        for (size_t j = js; j != jl; ++j)
        {
          for (size_t i = is; i != il; ++i)
          {
            (*bc)(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(base_container_traits<NEST_DZG>::container_t* bc,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t d = ds; d != dl; ++d)
  {
    for (size_t k = ks; k != kl; ++k)
    {
      for (size_t j = js; j != jl; ++j)
      {
        for (size_t i = is; i != il; ++i)
        {
          for (size_t g = gs; g != gl; ++g)
          {
            (*bc)(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<Nesting_Order Nest>
std::tuple<std::string, double, double, double, double>
run_base_container(Bench_User_Data::index_type const& first,
                   Bench_User_Data::index_type const& last)
{
  typedef base_container_traits<Nest> traits_t;
  typedef typename traits_t::container_t container_t;
  typedef typename traits_t::domain_t    domain_t;
  typedef typename traits_t::cid_t       index_t;

  index_t cid(0,0,0,0,0);

  counter_t timer;
  timer.reset();
  std::tuple<std::string, double, double, double, double> times;
  std::get<0>(times) = nestingString(Nest);

  // Construction
  timer.start();
  container_t* bc = new container_t(domain_t(first, last, true), cid);
  std::get<1>(times) = timer.stop();
  timer.reset();

  // Initialization over raw data
  timer.start();
  populate(bc->container().data(), bc->size());
  std::get<2>(times) = timer.stop();
  timer.reset();

  // Traversal using index operators
  timer.start();
  traverse(bc, first, last);
  std::get<3>(times) = timer.stop();
  timer.reset();

  // Destruct
  timer.start();
  delete bc;
  std::get<4>(times) = timer.stop();

  return times;
}


void traverse(stapl::multiarray_view<
                base_container_traits<NEST_ZDG>::container_t> ma_vw,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t k = ks; k != kl; ++k)
  {
    for (size_t j = js; j != jl; ++j)
    {
      for (size_t i = is; i != il; ++i)
      {
        for (size_t d = ds; d != dl; ++d)
        {
          for (size_t g = gs; g != gl; ++g)
          {
            ma_vw(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(stapl::multiarray_view<
                base_container_traits<NEST_ZGD>::container_t> ma_vw,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t k = ks; k != kl; ++k)
  {
    for (size_t j = js; j != jl; ++j)
    {
      for (size_t i = is; i != il; ++i)
      {
        for (size_t g = gs; g != gl; ++g)
        {
          for (size_t d = ds; d != dl; ++d)
          {
            ma_vw(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(stapl::multiarray_view<
                base_container_traits<NEST_GZD>::container_t> ma_vw,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t g = gs; g != gl; ++g)
  {
    for (size_t k = ks; k != kl; ++k)
    {
      for (size_t j = js; j != jl; ++j)
      {
        for (size_t i = is; i != il; ++i)
        {
          for (size_t d = ds; d != dl; ++d)
          {
            ma_vw(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(stapl::multiarray_view<
                base_container_traits<NEST_GDZ>::container_t> ma_vw,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t g = gs; g != gl; ++g)
  {
    for (size_t d = ds; d != dl; ++d)
    {
      for (size_t k = ks; k != kl; ++k)
      {
        for (size_t j = js; j != jl; ++j)
        {
          for (size_t i = is; i != il; ++i)
          {
            ma_vw(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(stapl::multiarray_view<
                base_container_traits<NEST_DGZ>::container_t> ma_vw,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t d = ds; d != dl; ++d)
  {
    for (size_t g = gs; g != gl; ++g)
    {
      for (size_t k = ks; k != kl; ++k)
      {
        for (size_t j = js; j != jl; ++j)
        {
          for (size_t i = is; i != il; ++i)
          {
            ma_vw(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(stapl::multiarray_view<
                base_container_traits<NEST_DZG>::container_t> ma_vw,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t d = ds; d != dl; ++d)
  {
    for (size_t k = ks; k != kl; ++k)
    {
      for (size_t j = js; j != jl; ++j)
      {
        for (size_t i = is; i != il; ++i)
        {
          for (size_t g = gs; g != gl; ++g)
          {
            ma_vw(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<Nesting_Order Nest>
std::tuple<std::string, double, double, double, double>
run_bc_multiarray_view(Bench_User_Data::index_type const& first,
                       Bench_User_Data::index_type const& last)
{
  typedef base_container_traits<Nest> traits_t;
  typedef typename traits_t::container_t container_t;
  typedef typename traits_t::domain_t    domain_t;
  typedef typename traits_t::cid_t       index_t;

  index_t cid(0,0,0,0,0);

  counter_t timer;
  timer.reset();
  std::tuple<std::string, double, double, double, double> times;
  std::get<0>(times) = nestingString(Nest);

  // Construction
  timer.start();
  container_t* bc = new container_t(domain_t(first, last, true), cid);
  auto&& ma_vw = make_multiarray_view(*bc);
  std::get<1>(times) = timer.stop();
  timer.reset();

  // Initialization over raw data
  timer.start();
  populate(bc->container().data(), bc->size());
  std::get<2>(times) = timer.stop();
  timer.reset();

  // Traversal using index operators
  timer.start();
  traverse(ma_vw, first, last);
  std::get<3>(times) = timer.stop();
  timer.reset();

  // Destruct
  timer.start();
  delete bc;
  std::get<4>(times) = timer.stop();

  return times;
}

template<Nesting_Order Nest>
struct Traversal{ };

template<typename SlicesView>
void traverse(SlicesView sl_vw,
              Traversal<NEST_ZDG> traversal,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t k = ks; k != kl; ++k)
  {
    for (size_t j = js; j != jl; ++j)
    {
      for (size_t i = is; i != il; ++i)
      {
        auto&& slice_z = sl_vw(i, j, k);

        for (size_t d = ds; d != dl; ++d)
        {
          auto&& slice_z_d = slice_z[d];

          for (size_t g = gs; g != gl; ++g)
          {
            slice_z_d(g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<typename SlicesView>
void traverse(SlicesView sl_vw,
              Traversal<NEST_ZGD> traversal,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t k = ks; k != kl; ++k)
  {
    for (size_t j = js; j != jl; ++j)
    {
      for (size_t i = is; i != il; ++i)
      {
        auto&& slice_z = sl_vw(i, j, k);

        for (size_t g = gs; g != gl; ++g)
        {
          auto&& slice_z_g = slice_z[g];

          for (size_t d = ds; d != dl; ++d)
          {
            slice_z_g(d) *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<typename SlicesView>
void traverse(SlicesView sl_vw,
              Traversal<NEST_GZD> traversal,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t g = gs; g != gl; ++g)
  {
    auto&& slice_g = sl_vw[g];

    for (size_t k = ks; k != kl; ++k)
    {
      for (size_t j = js; j != jl; ++j)
      {
        for (size_t i = is; i != il; ++i)
        {
          auto&& slice_g_z = slice_g(i, j, k);

          for (size_t d = ds; d != dl; ++d)
          {
            slice_g_z(d) *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<typename SlicesView>
void traverse(SlicesView sl_vw,
              Traversal<NEST_GDZ> traversal,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t g = gs; g != gl; ++g)
  {
    auto&& slice_g = sl_vw[g];

    for (size_t d = ds; d != dl; ++d)
    {
      auto&& slice_g_d = slice_g[d];

      for (size_t k = ks; k != kl; ++k)
      {
        for (size_t j = js; j != jl; ++j)
        {
          for (size_t i = is; i != il; ++i)
          {
            slice_g_d(i, j, k) *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<typename SlicesView>
void traverse(SlicesView sl_vw,
              Traversal<NEST_DGZ> traversal,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t d = ds; d != dl; ++d)
  {
    auto&& slice_d = sl_vw[d];

    for (size_t g = gs; g != gl; ++g)
    {
      auto&& slice_d_g = slice_d[g];

      for (size_t k = ks; k != kl; ++k)
      {
        for (size_t j = js; j != jl; ++j)
        {
          for (size_t i = is; i != il; ++i)
          {
            slice_d_g(i, j, k) *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<typename SlicesView>
void traverse(SlicesView sl_vw,
              Traversal<NEST_DZG> traversal,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t d = ds; d != dl; ++d)
  {
    auto&& slice_d = sl_vw[d];

    for (size_t k = ks; k != kl; ++k)
    {
      for (size_t j = js; j != jl; ++j)
      {
        for (size_t i = is; i != il; ++i)
        {
          auto&& slice_d_z = slice_d(i, j, k);

          for (size_t g = gs; g != gl; ++g)
          {
            slice_d_z(g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<Nesting_Order Nest>
std::tuple<std::string, double, double, double, double>
run_bc_slices_view(Bench_User_Data::index_type const& first,
                   Bench_User_Data::index_type const& last)
{
  typedef base_container_traits<Nest>    traits_t;
  typedef typename traits_t::container_t container_t;
  typedef typename traits_t::domain_t    domain_t;
  typedef typename traits_t::cid_t       index_t;

  index_t cid(0,0,0,0,0);

  counter_t timer;
  timer.reset();
  std::tuple<std::string, double, double, double, double> times;
  std::get<0>(times) = nestingString(Nest);

  // Construction
  timer.start();
  container_t* bc = new container_t(domain_t(first, last, true), cid);
  auto&& ma_vw = make_multiarray_view(*bc);
  std::get<1>(times) = timer.stop();
  timer.reset();

  // Initialization over raw data
  timer.start();
  populate(bc->container().data(), bc->size());
  std::get<2>(times) = timer.stop();
  timer.reset();

  // Traversal using index operators
  switch (Nest)
  {
    case NEST_DGZ:
      {
        timer.start();
        auto tmp_slice = stapl::make_slices_view<3, 4>(ma_vw);
        auto slice_vw = stapl::make_slices_view<0>(tmp_slice);
        std::get<1>(times) += timer.stop();
        timer.reset();

        timer.start();
        traverse(slice_vw, Traversal<NEST_DGZ>(), first, last);
        std::get<3>(times) = timer.stop();
        timer.reset();
      }
      break;
    case NEST_DZG:
      {
        timer.start();
        auto tmp_slice = stapl::make_slices_view<0, 1, 2, 3>(ma_vw);
        auto slice_vw = stapl::make_slices_view<3>(tmp_slice);
        std::get<1>(times) += timer.stop();
        timer.reset();

        timer.start();
        traverse(slice_vw, Traversal<NEST_DZG>(), first, last);
        std::get<3>(times) = timer.stop();
        timer.reset();
      }
      break;
    case NEST_GDZ:
      {
        timer.start();
        auto tmp_slice = stapl::make_slices_view<3, 4>(ma_vw);
        auto slice_vw = stapl::make_slices_view<1>(tmp_slice);
        std::get<1>(times) += timer.stop();
        timer.reset();

        timer.start();
        traverse(slice_vw, Traversal<NEST_GDZ>(), first, last);
        std::get<3>(times) = timer.stop();
        timer.reset();
      }
      break;
    case NEST_GZD:
      {
        timer.start();
        auto tmp_slice = stapl::make_slices_view<0, 1, 2, 4>(ma_vw);
        auto slice_vw = stapl::make_slices_view<3>(tmp_slice);
        std::get<1>(times) += timer.stop();
        timer.reset();

        timer.start();
        traverse(slice_vw, Traversal<NEST_GZD>(), first, last);
        std::get<3>(times) = timer.stop();
        timer.reset();
      }
      break;
    case NEST_ZDG:
      {
        timer.start();
        auto tmp_slice = stapl::make_slices_view<0, 1, 2, 3>(ma_vw);
        auto slice_vw = stapl::make_slices_view<0, 1, 2>(tmp_slice);
        std::get<1>(times) += timer.stop();
        timer.reset();

        timer.start();
        traverse(slice_vw, Traversal<NEST_ZDG>(), first, last);
        std::get<3>(times) = timer.stop();
        timer.reset();
      }
      break;
    case NEST_ZGD:
      {
        timer.start();
        auto tmp_slice = stapl::make_slices_view<0, 1, 2, 4>(ma_vw);
        auto slice_vw = stapl::make_slices_view<0, 1, 2>(tmp_slice);
        std::get<1>(times) += timer.stop();
        timer.reset();

        timer.start();
        traverse(slice_vw, Traversal<NEST_ZGD>(), first, last);
        std::get<3>(times) = timer.stop();
        timer.reset();
      }
      break;
  }

  // Destruct
  timer.start();
  delete bc;
  std::get<4>(times) = timer.stop();

  return times;
}

void traverse(multiarray_traits<NEST_ZDG>::container_t* cont,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t k = ks; k != kl; ++k)
  {
    for (size_t j = js; j != jl; ++j)
    {
      for (size_t i = is; i != il; ++i)
      {
        for (size_t d = ds; d != dl; ++d)
        {
          for (size_t g = gs; g != gl; ++g)
          {
            (*cont)(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(multiarray_traits<NEST_ZGD>::container_t* cont,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t k = ks; k != kl; ++k)
  {
    for (size_t j = js; j != jl; ++j)
    {
      for (size_t i = is; i != il; ++i)
      {
        for (size_t g = gs; g != gl; ++g)
        {
          for (size_t d = ds; d != dl; ++d)
          {
            (*cont)(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(multiarray_traits<NEST_GZD>::container_t* cont,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t g = gs; g != gl; ++g)
  {
    for (size_t k = ks; k != kl; ++k)
    {
      for (size_t j = js; j != jl; ++j)
      {
        for (size_t i = is; i != il; ++i)
        {
          for (size_t d = ds; d != dl; ++d)
          {
            (*cont)(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(multiarray_traits<NEST_GDZ>::container_t* cont,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t g = gs; g != gl; ++g)
  {
    for (size_t d = ds; d != dl; ++d)
    {
      for (size_t k = ks; k != kl; ++k)
      {
        for (size_t j = js; j != jl; ++j)
        {
          for (size_t i = is; i != il; ++i)
          {
            (*cont)(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(multiarray_traits<NEST_DGZ>::container_t* cont,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t d = ds; d != dl; ++d)
  {
    for (size_t g = gs; g != gl; ++g)
    {
      for (size_t k = ks; k != kl; ++k)
      {
        for (size_t j = js; j != jl; ++j)
        {
          for (size_t i = is; i != il; ++i)
          {
            (*cont)(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(multiarray_traits<NEST_DZG>::container_t* cont,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t d = ds; d != dl; ++d)
  {
    for (size_t k = ks; k != kl; ++k)
    {
      for (size_t j = js; j != jl; ++j)
      {
        for (size_t i = is; i != il; ++i)
        {
          for (size_t g = gs; g != gl; ++g)
          {
            (*cont)(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<Nesting_Order Nest>
std::tuple<std::string, double, double, double, double>
run_multiarray(Bench_User_Data::index_type const& first,
               Bench_User_Data::index_type const& last)
{
  typedef multiarray_traits<Nest>                   traits_t;
  typedef typename traits_t::container_t            container_t;
  typedef typename container_t::dimensions_type     dim_t;

  size_t i(std::get<0>(last) + 1 - std::get<0>(first));
  size_t j(std::get<1>(last) + 1 - std::get<1>(first));
  size_t k(std::get<2>(last) + 1 - std::get<2>(first));
  size_t d(std::get<3>(last) + 1 - std::get<3>(first));
  size_t g(std::get<4>(last) + 1 - std::get<4>(first));

  dim_t dims = dim_t(i, j, k, d, g);

  counter_t timer;
  timer.reset();
  std::tuple<std::string, double, double, double, double> times;
  std::get<0>(times) = nestingString(Nest);

  // Construction
  timer.start();
  container_t* cont = new container_t(dims);
  std::get<1>(times) = timer.stop();
  timer.reset();

  // Initialization over raw data
  timer.start();
  populate(cont, first, last);
  std::get<2>(times) = timer.stop();
  timer.reset();

  // Traversal using index operators
  timer.start();
  traverse(cont, first, last);
  std::get<3>(times) = timer.stop();
  timer.reset();

  // Destruct
  timer.start();
  delete cont;
  std::get<4>(times) = timer.stop();

  return times;
}

void traverse(stapl::multiarray_view<
                multiarray_traits<NEST_ZDG>::container_t> ma_vw,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t k = ks; k != kl; ++k)
  {
    for (size_t j = js; j != jl; ++j)
    {
      for (size_t i = is; i != il; ++i)
      {
        for (size_t d = ds; d != dl; ++d)
        {
          for (size_t g = gs; g != gl; ++g)
          {
            ma_vw(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(stapl::multiarray_view<
                multiarray_traits<NEST_ZGD>::container_t> ma_vw,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t k = ks; k != kl; ++k)
  {
    for (size_t j = js; j != jl; ++j)
    {
      for (size_t i = is; i != il; ++i)
      {
        for (size_t g = gs; g != gl; ++g)
        {
          for (size_t d = ds; d != dl; ++d)
          {
            ma_vw(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(stapl::multiarray_view<
                multiarray_traits<NEST_GZD>::container_t> ma_vw,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t g = gs; g != gl; ++g)
  {
    for (size_t k = ks; k != kl; ++k)
    {
      for (size_t j = js; j != jl; ++j)
      {
        for (size_t i = is; i != il; ++i)
        {
          for (size_t d = ds; d != dl; ++d)
          {
            ma_vw(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(stapl::multiarray_view<
                multiarray_traits<NEST_GDZ>::container_t> ma_vw,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t g = gs; g != gl; ++g)
  {
    for (size_t d = ds; d != dl; ++d)
    {
      for (size_t k = ks; k != kl; ++k)
      {
        for (size_t j = js; j != jl; ++j)
        {
          for (size_t i = is; i != il; ++i)
          {
            ma_vw(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(stapl::multiarray_view<
                multiarray_traits<NEST_DGZ>::container_t> ma_vw,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t d = ds; d != dl; ++d)
  {
    for (size_t g = gs; g != gl; ++g)
    {
      for (size_t k = ks; k != kl; ++k)
      {
        for (size_t j = js; j != jl; ++j)
        {
          for (size_t i = is; i != il; ++i)
          {
            ma_vw(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

void traverse(stapl::multiarray_view<
                multiarray_traits<NEST_DZG>::container_t> ma_vw,
              Bench_User_Data::index_type const& first,
              Bench_User_Data::index_type const& last)
{
  size_t is(std::get<0>(first)), il(std::get<0>(last)+1);
  size_t js(std::get<1>(first)), jl(std::get<1>(last)+1);
  size_t ks(std::get<2>(first)), kl(std::get<2>(last)+1);
  size_t ds(std::get<3>(first)), dl(std::get<3>(last)+1);
  size_t gs(std::get<4>(first)), gl(std::get<4>(last)+1);

  for (size_t d = ds; d != dl; ++d)
  {
    for (size_t k = ks; k != kl; ++k)
    {
      for (size_t j = js; j != jl; ++j)
      {
        for (size_t i = is; i != il; ++i)
        {
          for (size_t g = gs; g != gl; ++g)
          {
            ma_vw(i, j, k, d, g) *= 1.4142135624;
          }
        }
      }
    }
  }
}

template<Nesting_Order Nest>
std::tuple<std::string, double, double, double, double>
run_multiarray_view(Bench_User_Data::index_type const& first,
                    Bench_User_Data::index_type const& last)
{
  typedef multiarray_traits<Nest>                   traits_t;
  typedef typename traits_t::container_t            container_t;
  typedef typename container_t::dimensions_type     dim_t;

  size_t i(std::get<0>(last) + 1 - std::get<0>(first));
  size_t j(std::get<1>(last) + 1 - std::get<1>(first));
  size_t k(std::get<2>(last) + 1 - std::get<2>(first));
  size_t d(std::get<3>(last) + 1 - std::get<3>(first));
  size_t g(std::get<4>(last) + 1 - std::get<4>(first));

  dim_t dims = dim_t(i, j, k, d, g);

  counter_t timer;
  timer.reset();
  std::tuple<std::string, double, double, double, double> times;
  std::get<0>(times) = nestingString(Nest);

  // Construction
  timer.start();
  container_t* cont = new container_t(dims);
  auto&& ma_vw = make_multiarray_view(*cont);
  std::get<1>(times) = timer.stop();
  timer.reset();

  // Initialization over raw data
  timer.start();
  populate(cont, first, last);
  std::get<2>(times) = timer.stop();
  timer.reset();

  // Traversal using index operators
  timer.start();
  traverse(ma_vw, first, last);
  std::get<3>(times) = timer.stop();
  timer.reset();

  // Destruct
  timer.start();
  delete cont;
  std::get<4>(times) = timer.stop();

  return times;
}

template<Nesting_Order Nest>
std::tuple<std::string, double, double, double, double>
run_slices_view(Bench_User_Data::index_type const& first,
                Bench_User_Data::index_type const& last)
{
  typedef multiarray_traits<Nest>                   traits_t;
  typedef typename traits_t::container_t            container_t;
  typedef typename container_t::dimensions_type     dim_t;

  size_t i(std::get<0>(last) + 1 - std::get<0>(first));
  size_t j(std::get<1>(last) + 1 - std::get<1>(first));
  size_t k(std::get<2>(last) + 1 - std::get<2>(first));
  size_t d(std::get<3>(last) + 1 - std::get<3>(first));
  size_t g(std::get<4>(last) + 1 - std::get<4>(first));

  dim_t dims = dim_t(i, j, k, d, g);

  counter_t timer;
  timer.reset();
  std::tuple<std::string, double, double, double, double> times;
  std::get<0>(times) = nestingString(Nest);

  // Construction
  timer.start();
  container_t* cont = new container_t(dims);
  auto&& ma_vw = make_multiarray_view(*cont);
  std::get<1>(times) = timer.stop();
  timer.reset();

  // Initialization over raw data
  timer.start();
  populate(cont, first, last);
  std::get<2>(times) = timer.stop();
  timer.reset();

  // Traversal using index operators
  switch (Nest)
  {
    case NEST_DGZ:
      {
        timer.start();
        auto tmp_slice = stapl::make_slices_view<3, 4>(ma_vw);
        auto slice_vw = stapl::make_slices_view<0>(tmp_slice);
        std::get<1>(times) += timer.stop();
        timer.reset();

        timer.start();
        traverse(slice_vw, Traversal<NEST_DGZ>(), first, last);
        std::get<3>(times) = timer.stop();
        timer.reset();
      }
      break;
    case NEST_DZG:
      {
        timer.start();
        auto tmp_slice = stapl::make_slices_view<0, 1, 2, 3>(ma_vw);
        auto slice_vw = stapl::make_slices_view<3>(tmp_slice);
        std::get<1>(times) += timer.stop();
        timer.reset();

        timer.start();
        traverse(slice_vw, Traversal<NEST_DZG>(), first, last);
        std::get<3>(times) = timer.stop();
        timer.reset();
      }
      break;
    case NEST_GDZ:
      {
        timer.start();
        auto tmp_slice = stapl::make_slices_view<3, 4>(ma_vw);
        auto slice_vw = stapl::make_slices_view<1>(tmp_slice);
        std::get<1>(times) += timer.stop();
        timer.reset();

        timer.start();
        traverse(slice_vw, Traversal<NEST_GDZ>(), first, last);
        std::get<3>(times) = timer.stop();
        timer.reset();
      }
      break;
    case NEST_GZD:
      {
        timer.start();
        auto tmp_slice = stapl::make_slices_view<0, 1, 2, 4>(ma_vw);
        auto slice_vw = stapl::make_slices_view<3>(tmp_slice);
        std::get<1>(times) += timer.stop();
        timer.reset();

        timer.start();
        traverse(slice_vw, Traversal<NEST_GZD>(), first, last);
        std::get<3>(times) = timer.stop();
        timer.reset();
      }
      break;
    case NEST_ZDG:
      {
        timer.start();
        auto tmp_slice = stapl::make_slices_view<0, 1, 2, 3>(ma_vw);
        auto slice_vw = stapl::make_slices_view<0, 1, 2>(tmp_slice);
        std::get<1>(times) += timer.stop();
        timer.reset();

        timer.start();
        traverse(slice_vw, Traversal<NEST_ZDG>(), first, last);
        std::get<3>(times) = timer.stop();
        timer.reset();
      }
      break;
    case NEST_ZGD:
      {
        timer.start();
        auto tmp_slice = stapl::make_slices_view<0, 1, 2, 4>(ma_vw);
        auto slice_vw = stapl::make_slices_view<0, 1, 2>(tmp_slice);
        std::get<1>(times) += timer.stop();
        timer.reset();

        timer.start();
        traverse(slice_vw, Traversal<NEST_ZGD>(), first, last);
        std::get<3>(times) = timer.stop();
        timer.reset();
      }
      break;
  }

  // Destruct
  timer.start();
  delete cont;
  std::get<4>(times) = timer.stop();

  return times;
}

void report_results(std::string const& name, std::string const& version,
                    std::vector<double> const& ctor_samples,
                    std::vector<double> const& pop_samples,
                    std::vector<double> const& del_samples,
                    confidence_interval_controller const& controller)
{
  stapl::do_once([&] {
  stat_t trav_res = controller.stats();
#if 0
  // enable to report timing data on container construction, initialization, and
  // deletion.
  stat_t ctor_res = compute_stats(ctor_samples);
  stat_t pop_res  = compute_stats(pop_samples);
  stat_t del_res  = compute_stats(del_samples);
#endif

#if 0
  // enable to report timing data on container construction and initialization
  std::cerr << "Test : " << name << " ctor\n";
  std::cerr << "Version : " << version << "\n";
  std::cerr << "Time : " << ctor_res.avg << "\n";
  std::cerr << "Notes : (ci, min, max, stddev, samples) ";
  std::cerr << ctor_res.conf_interval << " " << ctor_res.min << " "
            << ctor_res.max << " " << ctor_res.stddev << " "
            << ctor_res.num_samples << "\n";

  std::cerr << "Test : " << name << " populate\n";
  std::cerr << "Version : " << version << "\n";
  std::cerr << "Time : " << pop_res.avg << "\n";
  std::cerr << "Notes : (ci, min, max, stddev, samples) ";
  std::cerr << pop_res.conf_interval << " " << pop_res.min << " "
            << pop_res.max << " " << pop_res.stddev << " "
            << pop_res.num_samples << "\n";
#endif

  std::cerr << "Test : " << name << " traverse\n";
  std::cerr << "Version : " << version << "\n";
  std::cerr << "Time : " << trav_res.avg << "\n";
  std::cerr << "Notes : (ci, min, max, stddev, samples) ";
  std::cerr << trav_res.conf_interval << " " << trav_res.min << " "
            << trav_res.max << " " << trav_res.stddev << " "
            << trav_res.num_samples << "\n";

#if 0
  // enable to report timing data on container deletion
  std::cerr << "Test : " << name << " delete\n";
  std::cerr << "Version : " << version << "\n";
  std::cerr << "Time : " << del_res.avg << "\n";
  std::cerr << "Notes : (ci, min, max, stddev, samples) ";
  std::cerr << del_res.conf_interval << " " << del_res.min << " "
            << del_res.max << " " << del_res.stddev << " "
            << del_res.num_samples << "\n";
#endif
  });
}


template<Nesting_Order Nest>
void run_nest(Bench_User_Data const& input)
{
  //SubTVec
  std::tuple<std::string, double, double, double, double> stt;
  std::vector<double> stt_ctor, stt_pop, stt_del;
  std::string stt_name;

  confidence_interval_controller stt_psi_controller(5, 32);

  // Base Container and view layers
  std::tuple<std::string, double, double, double, double> bct;
  std::vector<double> bct_ctor, bct_pop, bct_del;
  std::string bct_name;

  confidence_interval_controller bct_psi_controller(5, 32);

  std::tuple<std::string, double, double, double, double> bmt;
  std::vector<double> bmt_ctor, bmt_pop, bmt_del;
  std::string bmt_name;

  confidence_interval_controller bmt_psi_controller(5, 32);

  std::tuple<std::string, double, double, double, double> bst;
  std::vector<double> bst_ctor, bst_pop, bst_del;
  std::string bst_name;

  confidence_interval_controller bst_psi_controller(5, 32);

  // Multiarray and view layers
  std::tuple<std::string, double, double, double, double> mat;
  std::vector<double> mat_ctor, mat_pop, mat_del;
  std::string mat_name;

  confidence_interval_controller mat_psi_controller(5, 32);

  std::tuple<std::string, double, double, double, double> mmt;
  std::vector<double> mmt_ctor, mmt_pop, mmt_del;
  std::string mmt_name;

  confidence_interval_controller mmt_psi_controller(5, 32);

  std::tuple<std::string, double, double, double, double> mst;
  std::vector<double> mst_ctor, mst_pop, mst_del;
  std::string mst_name;

  confidence_interval_controller mst_psi_controller(5, 32);

  // Boost multiarray
  std::tuple<std::string, double, double, double, double> btt;
  std::vector<double> btt_ctor, btt_pop, btt_del;
  std::string btt_name;

  confidence_interval_controller btt_psi_controller(5, 32);

  bool collect = stt_psi_controller.iterate() || bct_psi_controller.iterate()
    || bmt_psi_controller.iterate() || bst_psi_controller.iterate()
    || mat_psi_controller.iterate() || mmt_psi_controller.iterate()
    || mst_psi_controller.iterate() || btt_psi_controller.iterate();

#ifdef KOKKOS_DEFINED
  // Kokkos multiarray
  std::tuple<std::string, double, double, double, double> kkt;
  std::vector<double> kkt_ctor, kkt_pop, kkt_del;
  std::string kkt_name;

  confidence_interval_controller kkt_psi_controller(5, 32);

  collect |= kkt_psi_controller.iterate();
#endif

  // Evaluate psi and rhs size containers
  while (collect)
  {
    bct = run_base_container<Nest>(input.first(), input.psi_last());

    bct_name = std::get<0>(bct);
    bct_ctor.push_back(std::get<1>(bct));
    bct_pop.push_back(std::get<2>(bct));
    bct_psi_controller.push_back(std::get<3>(bct));
    bct_del.push_back(std::get<4>(bct));

    bmt = run_bc_multiarray_view<Nest>(input.first(), input.psi_last());

    bmt_name = std::get<0>(bmt);
    bmt_ctor.push_back(std::get<1>(bmt));
    bmt_pop.push_back(std::get<2>(bmt));
    bmt_psi_controller.push_back(std::get<3>(bmt));
    bmt_del.push_back(std::get<4>(bmt));

    bst = run_bc_slices_view<Nest>(input.first(), input.psi_last());

    bst_name = std::get<0>(bst);
    bst_ctor.push_back(std::get<1>(bst));
    bst_pop.push_back(std::get<2>(bst));
    bst_psi_controller.push_back(std::get<3>(bst));
    bst_del.push_back(std::get<4>(bst));

    mat = run_multiarray<Nest>(input.first(), input.psi_last());

    mat_name = std::get<0>(mat);
    mat_ctor.push_back(std::get<1>(mat));
    mat_pop.push_back(std::get<2>(mat));
    mat_psi_controller.push_back(std::get<3>(mat));
    mat_del.push_back(std::get<4>(mat));

    mmt = run_multiarray_view<Nest>(input.first(), input.psi_last());

    mmt_name = std::get<0>(mmt);
    mmt_ctor.push_back(std::get<1>(mmt));
    mmt_pop.push_back(std::get<2>(mmt));
    mmt_psi_controller.push_back(std::get<3>(mmt));
    mmt_del.push_back(std::get<4>(mmt));

    mst = run_slices_view<Nest>(input.first(), input.psi_last());

    mst_name = std::get<0>(mst);
    mst_ctor.push_back(std::get<1>(mst));
    mst_pop.push_back(std::get<2>(mst));
    mst_psi_controller.push_back(std::get<3>(mst));
    mst_del.push_back(std::get<4>(mst));

    stt = run_subtvec<Nest>(Nest,
        input.g_gs, input.d_ds, input.nx, input.ny, input.nz);

    stt_name = std::get<0>(stt);
    stt_ctor.push_back(std::get<1>(stt));
    stt_pop.push_back(std::get<2>(stt));
    stt_psi_controller.push_back(std::get<3>(stt));
    stt_del.push_back(std::get<4>(stt));

    btt = run_boost<Nest>(input.g_gs, input.d_ds, input.nx,
        input.ny, input.nz);

    btt_name = std::get<0>(btt);
    btt_ctor.push_back(std::get<1>(btt));
    btt_pop.push_back(std::get<2>(btt));
    btt_psi_controller.push_back(std::get<3>(btt));
    btt_del.push_back(std::get<4>(btt));

    collect = stt_psi_controller.iterate() || bct_psi_controller.iterate()
      || bmt_psi_controller.iterate() || bst_psi_controller.iterate()
      || mat_psi_controller.iterate() || mmt_psi_controller.iterate()
      || mst_psi_controller.iterate() || btt_psi_controller.iterate();

#ifdef KOKKOS_DEFINED
    kkt = run_kokkos<Nest>(Nest,
        input.g_gs, input.d_ds, input.nx, input.ny, input.nz);

    kkt_name = std::get<0>(kkt);
    kkt_ctor.push_back(std::get<1>(kkt));
    kkt_pop.push_back(std::get<2>(kkt));
    kkt_psi_controller.push_back(std::get<3>(kkt));
    kkt_del.push_back(std::get<4>(kkt));

    collect |= kkt_psi_controller.iterate();
#endif
  }
  report_results(bct_name, "psi_multiarray_base_container",
                 bct_ctor, bct_pop, bct_del, bct_psi_controller);
  bct_ctor.clear(); bct_pop.clear(); bct_del.clear();

  report_results(bmt_name, "psi_bc_multiarray_view",
                 bmt_ctor, bmt_pop, bmt_del, bmt_psi_controller);
  bmt_ctor.clear(); bmt_pop.clear(); bmt_del.clear();

  report_results(bst_name, "psi_bc_slices_view",
                 bst_ctor, bst_pop, bst_del, bst_psi_controller);
  bst_ctor.clear(); bst_pop.clear(); bst_del.clear();

  report_results(mat_name, "psi_multiarray",
                 mat_ctor, mat_pop, mat_del, mat_psi_controller);
  mat_ctor.clear(); mat_pop.clear(); mat_del.clear();

  report_results(mmt_name, "psi_multiarray_view",
                 mmt_ctor, mmt_pop, mmt_del, mmt_psi_controller);
  mmt_ctor.clear(); mmt_pop.clear(); mmt_del.clear();

  report_results(mst_name, "psi_slices_view",
                 mst_ctor, mst_pop, mst_del, mst_psi_controller);
  mst_ctor.clear(); mst_pop.clear(); mst_del.clear();

  report_results(stt_name, "psi_SubTVec",
                 stt_ctor, stt_pop, stt_del, stt_psi_controller);
  stt_ctor.clear(); stt_pop.clear(); stt_del.clear();

  report_results(btt_name, "psi_boost",
                 btt_ctor, btt_pop, btt_del, btt_psi_controller);
  btt_ctor.clear(); btt_pop.clear(); btt_del.clear();

#ifdef KOKKOS_DEFINED
  report_results(kkt_name, "psi_Kokkos",
                 kkt_ctor, kkt_pop, kkt_del, kkt_psi_controller);
  kkt_ctor.clear(); kkt_pop.clear(); kkt_del.clear();
#endif

  // Evaluate sigt size container
  confidence_interval_controller stt_sigt_controller(5, 32);
  confidence_interval_controller bct_sigt_controller(5, 32);
  confidence_interval_controller bmt_sigt_controller(5, 32);
  confidence_interval_controller bst_sigt_controller(5, 32);
  confidence_interval_controller mat_sigt_controller(5, 32);
  confidence_interval_controller mmt_sigt_controller(5, 32);
  confidence_interval_controller mst_sigt_controller(5, 32);
  confidence_interval_controller btt_sigt_controller(5, 32);

  collect = stt_sigt_controller.iterate() || bct_sigt_controller.iterate()
    || bmt_sigt_controller.iterate() || bst_sigt_controller.iterate()
    || mat_sigt_controller.iterate() || mmt_sigt_controller.iterate()
    || mst_sigt_controller.iterate() || btt_sigt_controller.iterate();

#ifdef KOKKOS_DEFINED
  confidence_interval_controller kkt_sigt_controller(5, 32);

  collect |= kkt_sigt_controller.iterate();
#endif

  while (collect)
  {
    bct = run_base_container<Nest>(input.first(), input.sigt_last());

    bct_name = std::get<0>(bct);
    bct_ctor.push_back(std::get<1>(bct));
    bct_pop.push_back(std::get<2>(bct));
    bct_sigt_controller.push_back(std::get<3>(bct));
    bct_del.push_back(std::get<4>(bct));

    bmt = run_bc_multiarray_view<Nest>(input.first(), input.sigt_last());

    bmt_name = std::get<0>(bmt);
    bmt_ctor.push_back(std::get<1>(bmt));
    bmt_pop.push_back(std::get<2>(bmt));
    bmt_sigt_controller.push_back(std::get<3>(bmt));
    bmt_del.push_back(std::get<4>(bmt));

    bst = run_bc_slices_view<Nest>(input.first(), input.sigt_last());

    bst_name = std::get<0>(bst);
    bst_ctor.push_back(std::get<1>(bst));
    bst_pop.push_back(std::get<2>(bst));
    bst_sigt_controller.push_back(std::get<3>(bst));
    bst_del.push_back(std::get<4>(bst));

    mat = run_multiarray<Nest>(input.first(), input.sigt_last());

    mat_name = std::get<0>(mat);
    mat_ctor.push_back(std::get<1>(mat));
    mat_pop.push_back(std::get<2>(mat));
    mat_sigt_controller.push_back(std::get<3>(mat));
    mat_del.push_back(std::get<4>(mat));

    mmt = run_multiarray_view<Nest>(input.first(), input.sigt_last());

    mmt_name = std::get<0>(mmt);
    mmt_ctor.push_back(std::get<1>(mmt));
    mmt_pop.push_back(std::get<2>(mmt));
    mmt_sigt_controller.push_back(std::get<3>(mmt));
    mmt_del.push_back(std::get<4>(mmt));

    mst = run_slices_view<Nest>(input.first(), input.sigt_last());

    mst_name = std::get<0>(mst);
    mst_ctor.push_back(std::get<1>(mst));
    mst_pop.push_back(std::get<2>(mst));
    mst_sigt_controller.push_back(std::get<3>(mst));
    mst_del.push_back(std::get<4>(mst));

    if (Nest == NEST_GDZ || Nest == NEST_DGZ || Nest == NEST_GZD)
      stt = run_subtvec<NEST_DGZ>(Nest,
          input.grps, 1, input.nx, input.ny, input.nz);
    else
      stt = run_subtvec<NEST_DZG>(Nest,
          input.grps, 1, input.nx, input.ny, input.nz);

    stt_name = std::get<0>(stt);
    stt_ctor.push_back(std::get<1>(stt));
    stt_pop.push_back(std::get<2>(stt));
    stt_sigt_controller.push_back(std::get<3>(stt));
    stt_del.push_back(std::get<4>(stt));

    btt = run_boost<Nest>(input.grps, 1, input.nx, input.ny, input.nz);

    btt_name = std::get<0>(btt);
    btt_ctor.push_back(std::get<1>(btt));
    btt_pop.push_back(std::get<2>(btt));
    btt_sigt_controller.push_back(std::get<3>(btt));
    btt_del.push_back(std::get<4>(btt));

    collect = stt_sigt_controller.iterate() || bct_sigt_controller.iterate()
      || bmt_sigt_controller.iterate() || bst_sigt_controller.iterate()
      || mat_sigt_controller.iterate() || mmt_sigt_controller.iterate()
      || mst_sigt_controller.iterate() || btt_sigt_controller.iterate();

#ifdef KOKKOS_DEFINED
    kkt = run_kokkos<Nest>(Nest, input.grps, 1, input.nx, input.ny, input.nz);

    kkt_name = std::get<0>(kkt);
    kkt_ctor.push_back(std::get<1>(kkt));
    kkt_pop.push_back(std::get<2>(kkt));
    kkt_sigt_controller.push_back(std::get<3>(kkt));
    kkt_del.push_back(std::get<4>(kkt));

    collect |= kkt_sigt_controller.iterate();
#endif
  }
  report_results(bct_name, "sigt_multiarray_base_container",
                 bct_ctor, bct_pop, bct_del, bct_sigt_controller);
  bct_ctor.clear(); bct_pop.clear(); bct_del.clear();

  report_results(bmt_name, "sigt_bc_multiarray_view",
                 bmt_ctor, bmt_pop, bmt_del, bmt_sigt_controller);
  bmt_ctor.clear(); bmt_pop.clear(); bmt_del.clear();

  report_results(bst_name, "sigt_bc_slices_view",
                 bst_ctor, bst_pop, bst_del, bst_sigt_controller);
  bst_ctor.clear(); bst_pop.clear(); bst_del.clear();

  report_results(mat_name, "sigt_multiarray",
                 mat_ctor, mat_pop, mat_del, mat_sigt_controller);
  mat_ctor.clear(); mat_pop.clear(); mat_del.clear();

  report_results(mmt_name, "sigt_multiarray_view",
                 mmt_ctor, mmt_pop, mmt_del, mmt_sigt_controller);
  mmt_ctor.clear(); mmt_pop.clear(); mmt_del.clear();

  report_results(mst_name, "sigt_slices_view",
                 mst_ctor, mst_pop, mst_del, mst_sigt_controller);
  mst_ctor.clear(); mst_pop.clear(); mst_del.clear();

  report_results(stt_name, "sigt_SubTVec",
                 stt_ctor, stt_pop, stt_del, stt_sigt_controller);
  stt_ctor.clear(); stt_pop.clear(); stt_del.clear();

  report_results(btt_name, "sigt_boost",
                 btt_ctor, btt_pop, btt_del, btt_sigt_controller);
  btt_ctor.clear(); btt_pop.clear(); btt_del.clear();

#ifdef KOKKOS_DEFINED
  report_results(kkt_name, "sigt_Kokkos",
                 kkt_ctor, kkt_pop, kkt_del, kkt_sigt_controller);
  kkt_ctor.clear(); kkt_pop.clear(); kkt_del.clear();
#endif


  // Evaluate phi and phi_out size containers
  confidence_interval_controller stt_phi_controller(5, 32);
  confidence_interval_controller bct_phi_controller(5, 32);
  confidence_interval_controller bmt_phi_controller(5, 32);
  confidence_interval_controller bst_phi_controller(5, 32);
  confidence_interval_controller mat_phi_controller(5, 32);
  confidence_interval_controller mmt_phi_controller(5, 32);
  confidence_interval_controller mst_phi_controller(5, 32);
  confidence_interval_controller btt_phi_controller(5, 32);

  collect = stt_phi_controller.iterate() || bct_phi_controller.iterate()
    || bmt_phi_controller.iterate() || bst_phi_controller.iterate()
    || mat_phi_controller.iterate() || mmt_phi_controller.iterate()
    || mst_phi_controller.iterate() || btt_phi_controller.iterate();

#ifdef KOKKOS_DEFINED
  confidence_interval_controller kkt_phi_controller(5, 32);

  collect |= kkt_phi_controller.iterate();
#endif
  while (collect)
  {
    bct = run_base_container<Nest>(input.first(), input.phi_last());

    bct_name = std::get<0>(bct);
    bct_ctor.push_back(std::get<1>(bct));
    bct_pop.push_back(std::get<2>(bct));
    bct_phi_controller.push_back(std::get<3>(bct));
    bct_del.push_back(std::get<4>(bct));

    bmt = run_bc_multiarray_view<Nest>(input.first(), input.phi_last());

    bmt_name = std::get<0>(bmt);
    bmt_ctor.push_back(std::get<1>(bmt));
    bmt_pop.push_back(std::get<2>(bmt));
    bmt_phi_controller.push_back(std::get<3>(bmt));
    bmt_del.push_back(std::get<4>(bmt));

    bst = run_bc_slices_view<Nest>(input.first(), input.phi_last());

    bst_name = std::get<0>(bst);
    bst_ctor.push_back(std::get<1>(bst));
    bst_pop.push_back(std::get<2>(bst));
    bst_phi_controller.push_back(std::get<3>(bst));
    bst_del.push_back(std::get<4>(bst));

    mat = run_multiarray<Nest>(input.first(), input.phi_last());

    mat_name = std::get<0>(mat);
    mat_ctor.push_back(std::get<1>(mat));
    mat_pop.push_back(std::get<2>(mat));
    mat_phi_controller.push_back(std::get<3>(mat));
    mat_del.push_back(std::get<4>(mat));

    mmt = run_multiarray_view<Nest>(input.first(), input.phi_last());

    mmt_name = std::get<0>(mmt);
    mmt_ctor.push_back(std::get<1>(mmt));
    mmt_pop.push_back(std::get<2>(mmt));
    mmt_phi_controller.push_back(std::get<3>(mmt));
    mmt_del.push_back(std::get<4>(mmt));

    mst = run_slices_view<Nest>(input.first(), input.phi_last());

    mst_name = std::get<0>(mst);
    mst_ctor.push_back(std::get<1>(mst));
    mst_pop.push_back(std::get<2>(mst));
    mst_phi_controller.push_back(std::get<3>(mst));
    mst_del.push_back(std::get<4>(mst));

    stt = run_subtvec<Nest>(Nest,
        input.grps, input.nm, input.nx, input.ny, input.nz);

    stt_name = std::get<0>(stt);
    stt_ctor.push_back(std::get<1>(stt));
    stt_pop.push_back(std::get<2>(stt));
    stt_phi_controller.push_back(std::get<3>(stt));
    stt_del.push_back(std::get<4>(stt));

    btt = run_boost<Nest>(input.grps, input.nm, input.nx, input.ny, input.nz);

    btt_name = std::get<0>(btt);
    btt_ctor.push_back(std::get<1>(btt));
    btt_pop.push_back(std::get<2>(btt));
    btt_phi_controller.push_back(std::get<3>(btt));
    btt_del.push_back(std::get<4>(btt));

    collect = stt_phi_controller.iterate() || bct_phi_controller.iterate()
      || bmt_phi_controller.iterate() || bst_phi_controller.iterate()
      || mat_phi_controller.iterate() || mmt_phi_controller.iterate()
      || mst_phi_controller.iterate() || btt_phi_controller.iterate();

#ifdef KOKKOS_DEFINED
    kkt = run_kokkos<Nest>(Nest,
        input.grps, input.nm, input.nx, input.ny, input.nz);

    kkt_name = std::get<0>(kkt);
    kkt_ctor.push_back(std::get<1>(kkt));
    kkt_pop.push_back(std::get<2>(kkt));
    kkt_phi_controller.push_back(std::get<3>(kkt));
    kkt_del.push_back(std::get<4>(kkt));

    collect |= kkt_phi_controller.iterate();
#endif
  }
  report_results(bct_name, "phi_multiarray_base_container",
                 bct_ctor, bct_pop, bct_del, bct_phi_controller);
  bct_ctor.clear(); bct_pop.clear(); bct_del.clear();

  report_results(bmt_name, "phi_bc_multiarray_view",
                 bmt_ctor, bmt_pop, bmt_del, bmt_phi_controller);
  bmt_ctor.clear(); bmt_pop.clear(); bmt_del.clear();

  report_results(bst_name, "phi_bc_slices_view",
                 bst_ctor, bst_pop, bst_del, bst_phi_controller);
  bst_ctor.clear(); bst_pop.clear(); bst_del.clear();

  report_results(mat_name, "phi_multiarray",
                 mat_ctor, mat_pop, mat_del, mat_phi_controller);
  mat_ctor.clear(); mat_pop.clear(); mat_del.clear();

  report_results(mmt_name, "phi_multiarray_view",
                 mmt_ctor, mmt_pop, mmt_del, mmt_phi_controller);
  mmt_ctor.clear(); mmt_pop.clear(); mmt_del.clear();

  report_results(mst_name, "phi_slices_view",
                 mst_ctor, mst_pop, mst_del, mst_phi_controller);
  mst_ctor.clear(); mst_pop.clear(); mst_del.clear();

  report_results(stt_name, "phi_SubTVec",
                 stt_ctor, stt_pop, stt_del, stt_phi_controller);
  stt_ctor.clear(); stt_pop.clear(); stt_del.clear();

  report_results(btt_name, "phi_boost",
                 btt_ctor, btt_pop, btt_del, btt_phi_controller);
  btt_ctor.clear(); btt_pop.clear(); btt_del.clear();

#ifdef KOKKOS_DEFINED
  report_results(kkt_name, "phi_Kokkos",
                 kkt_ctor, kkt_pop, kkt_del, kkt_phi_controller);
  kkt_ctor.clear(); kkt_pop.clear(); kkt_del.clear();
#endif

  // Evaluate ell and ell_plus size containers
  confidence_interval_controller stt_ell_controller(5, 32);
  confidence_interval_controller bct_ell_controller(5, 32);
  confidence_interval_controller bmt_ell_controller(5, 32);
  confidence_interval_controller bst_ell_controller(5, 32);
  confidence_interval_controller mat_ell_controller(5, 32);
  confidence_interval_controller mmt_ell_controller(5, 32);
  confidence_interval_controller mst_ell_controller(5, 32);
  confidence_interval_controller btt_ell_controller(5, 32);

  collect = stt_ell_controller.iterate() || bct_ell_controller.iterate()
    || bmt_ell_controller.iterate() || bst_ell_controller.iterate()
    || mat_ell_controller.iterate() || mmt_ell_controller.iterate()
    || mst_ell_controller.iterate() || btt_ell_controller.iterate();

#ifdef KOKKOS_DEFINED
  confidence_interval_controller kkt_ell_controller(5, 32);

  collect |= kkt_ell_controller.iterate();
#endif
  while (collect)
  {
    bct = run_base_container<Nest>(input.first(), input.ell_last());

    bct_name = std::get<0>(bct);
    bct_ctor.push_back(std::get<1>(bct));
    bct_pop.push_back(std::get<2>(bct));
    bct_ell_controller.push_back(std::get<3>(bct));
    bct_del.push_back(std::get<4>(bct));

    bmt = run_bc_multiarray_view<Nest>(input.first(), input.ell_last());

    bmt_name = std::get<0>(bmt);
    bmt_ctor.push_back(std::get<1>(bmt));
    bmt_pop.push_back(std::get<2>(bmt));
    bmt_ell_controller.push_back(std::get<3>(bmt));
    bmt_del.push_back(std::get<4>(bmt));

    bst = run_bc_slices_view<Nest>(input.first(), input.ell_last());

    bst_name = std::get<0>(bst);
    bst_ctor.push_back(std::get<1>(bst));
    bst_pop.push_back(std::get<2>(bst));
    bst_ell_controller.push_back(std::get<3>(bst));
    bst_del.push_back(std::get<4>(bst));

    mat = run_multiarray<Nest>(input.first(), input.ell_last());

    mat_name = std::get<0>(mat);
    mat_ctor.push_back(std::get<1>(mat));
    mat_pop.push_back(std::get<2>(mat));
    mat_ell_controller.push_back(std::get<3>(mat));
    mat_del.push_back(std::get<4>(mat));

    mmt = run_multiarray_view<Nest>(input.first(), input.ell_last());

    mmt_name = std::get<0>(mmt);
    mmt_ctor.push_back(std::get<1>(mmt));
    mmt_pop.push_back(std::get<2>(mmt));
    mmt_ell_controller.push_back(std::get<3>(mmt));
    mmt_del.push_back(std::get<4>(mmt));

    mst = run_slices_view<Nest>(input.first(), input.ell_last());

    mst_name = std::get<0>(mst);
    mst_ctor.push_back(std::get<1>(mst));
    mst_pop.push_back(std::get<2>(mst));
    mst_ell_controller.push_back(std::get<3>(mst));
    mst_del.push_back(std::get<4>(mst));

    if (Nest == NEST_GDZ || Nest == NEST_DZG || Nest == NEST_DGZ)
      stt = run_subtvec<NEST_ZGD>(Nest, input.nm, input.dirs, 1, 1, 1);
    else
      stt = run_subtvec<NEST_ZDG>(Nest, input.nm, input.dirs, 1, 1, 1);

    stt_name = std::get<0>(stt);
    stt_ctor.push_back(std::get<1>(stt));
    stt_pop.push_back(std::get<2>(stt));
    stt_ell_controller.push_back(std::get<3>(stt));
    stt_del.push_back(std::get<4>(stt));

    btt = run_boost<Nest>(input.nm, input.dirs, 1, 1, 1);

    btt_name = std::get<0>(btt);
    btt_ctor.push_back(std::get<1>(btt));
    btt_pop.push_back(std::get<2>(btt));
    btt_ell_controller.push_back(std::get<3>(btt));
    btt_del.push_back(std::get<4>(btt));

    collect = stt_ell_controller.iterate() || bct_ell_controller.iterate()
      || bmt_ell_controller.iterate() || bst_ell_controller.iterate()
      || mat_ell_controller.iterate() || mmt_ell_controller.iterate()
      || mst_ell_controller.iterate() || btt_ell_controller.iterate();

#ifdef KOKKOS_DEFINED
    kkt = run_kokkos<Nest>(Nest, input.nm, input.dirs, 1, 1, 1);

    kkt_name = std::get<0>(kkt);
    kkt_ctor.push_back(std::get<1>(kkt));
    kkt_pop.push_back(std::get<2>(kkt));
    kkt_ell_controller.push_back(std::get<3>(kkt));
    kkt_del.push_back(std::get<4>(kkt));

    collect |= kkt_ell_controller.iterate();
#endif
  }
  report_results(bct_name, "ell_multiarray_base_container",
                 bct_ctor, bct_pop, bct_del, bct_ell_controller);
  bct_ctor.clear(); bct_pop.clear(); bct_del.clear();

  report_results(bmt_name, "ell_bc_multiarray_view",
                 bmt_ctor, bmt_pop, bmt_del, bmt_ell_controller);
  bmt_ctor.clear(); bmt_pop.clear(); bmt_del.clear();

  report_results(bst_name, "ell_bc_slices_view",
                 bst_ctor, bst_pop, bst_del, bst_ell_controller);
  bst_ctor.clear(); bst_pop.clear(); bst_del.clear();

  report_results(mat_name, "ell_multiarray",
                 mat_ctor, mat_pop, mat_del, mat_ell_controller);
  mat_ctor.clear(); mat_pop.clear(); mat_del.clear();

  report_results(mmt_name, "ell_multiarray_view",
                 mmt_ctor, mmt_pop, mmt_del, mmt_ell_controller);
  mmt_ctor.clear(); mmt_pop.clear(); mmt_del.clear();

  report_results(mst_name, "ell_slices_view",
                 mst_ctor, mst_pop, mst_del, mst_ell_controller);
  mst_ctor.clear(); mst_pop.clear(); mst_del.clear();

  report_results(stt_name, "ell_SubTVec",
                 stt_ctor, stt_pop, stt_del, stt_ell_controller);
  stt_ctor.clear(); stt_pop.clear(); stt_del.clear();

  report_results(btt_name, "ell_boost",
                 btt_ctor, btt_pop, btt_del, btt_ell_controller);
  btt_ctor.clear(); btt_pop.clear(); btt_del.clear();

#ifdef KOKKOS_DEFINED
  report_results(kkt_name, "ell_Kokkos",
      kkt_ctor, kkt_pop, kkt_del, kkt_ell_controller);
  kkt_ctor.clear(); kkt_pop.clear(); kkt_del.clear();
#endif
}


stapl::exit_code stapl_main(int argc, char** argv)
{
  if (argc < 10)
  {
    std::cerr << "./multiarray_test NEST nx ny nz dirs grps nmom d/ds g/gs"
              << std::endl;
    return EXIT_FAILURE;
  }

  Bench_User_Data input(argc, argv);

#ifdef KOKKOS_DEFINED
  Kokkos::initialize(argc,argv);
#endif

  switch (input.nest)
  {
    case NEST_DGZ:
      run_nest<NEST_DGZ>(input);
      break;
    case NEST_DZG:
      run_nest<NEST_DZG>(input);
      break;
    case NEST_GDZ:
      run_nest<NEST_GDZ>(input);
      break;
    case NEST_GZD:
      run_nest<NEST_GZD>(input);
      break;
    case NEST_ZDG:
      run_nest<NEST_ZDG>(input);
      break;
    case NEST_ZGD:
      run_nest<NEST_ZGD>(input);
  }

#ifdef KOKKOS_DEFINED
  Kokkos::finalize();
#endif

  return EXIT_SUCCESS;
}
