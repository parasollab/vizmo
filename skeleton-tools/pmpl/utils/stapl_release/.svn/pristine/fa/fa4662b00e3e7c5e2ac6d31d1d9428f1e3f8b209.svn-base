/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <stapl/containers/matrix/matrix.hpp>
#include <stapl/containers/multiarray/traversals.hpp>
#include <stapl/views/native_view.hpp>
#include <test/algorithms/test_utils.h>
#include <test/algorithms/test_utils.h>

#include "../../test_report.hpp"

using namespace stapl;


template<typename Container>
void test_container(Container& c, size_t n, size_t m)
{
  typedef typename Container::gid_type gid_type;

  bool passed = c.size() == n*m;

  STAPL_TEST_REPORT(passed, "Testing size")

  do_once([&](void) {
    for (size_t i = 0; i < n; ++i) {
      for (size_t j = 0; j < m; ++j) {
        gid_type g(i,j);
        c.set_element(g, i*n+j);
      }
    }
  });

  STAPL_TEST_REPORT(true, "Testing get_element")

  passed = true;

  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < m; ++j) {
      gid_type g(i,j);
      if (static_cast<size_t>(c.get_element(g)) != i*n+j)
        passed = false;
    }
  }

  // check if set_element test passed on all locations
  stapl_bool res(passed);

  bool all_passed = res.reduce();

  STAPL_TEST_REPORT(all_passed, "Testing get_element")

  passed = true;
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < m; ++j) {
      gid_type g(i,j);
      if (static_cast<size_t>(c[g]) != i*n+j)
        passed = false;
    }
  }

  // check if operator[] test passed on all locations
  res = stapl_bool(passed);
  all_passed = res.reduce();

  STAPL_TEST_REPORT(all_passed, "Testing operator[]")
};


void test_dense(size_t n, size_t m)
{
  typedef int                                                value_type;
  typedef indexed_domain<size_t>                             vector_domain_type;
  typedef balanced_partition<vector_domain_type>             balanced_type;
  typedef stapl::row_major                                   traversal_type;
  typedef nd_partition<
            stapl::tuple<balanced_type, balanced_type>,
            traversal_type>                                  partition_type;
  typedef stapl::tuple<size_t, size_t>                       gid_type;

  // ===================================================================

  STAPL_TEST_MESSAGE("Testing size_type, Partition constructor")
  typedef matrix<value_type, traversal_type, partition_type> matrix_type;

  balanced_type p0(vector_domain_type(0, n-1), get_num_locations());
  balanced_type p1(vector_domain_type(0, m-1), get_num_locations());
  partition_type part(p0, p1);

  matrix_type f(gid_type(n,m), part);
  test_container(f, n, m);

  // ===================================================================

  STAPL_TEST_MESSAGE("Testing size_type constructor")
  typedef matrix<value_type, traversal_type> matrix_type2;
  matrix_type2 g(gid_type(n,m));
  test_container(g, n, m);

  // ===================================================================

  STAPL_TEST_MESSAGE("Testing size_type, Mapper constructor")
  typedef matrix_type2::domain_type domain_type;
  typedef matrix_type2::partition_type partition_type2;
  typedef matrix_type2::mapper_type mapper_type;

  domain_type dom(gid_type(n,m));
  partition_type2 part2(dom,
  stapl::multiarray_impl::make_multiarray_size<2>()(get_num_locations()));
  mapper_type mz(part2);

  matrix_type2 dd(gid_type(n,m), mz);
  test_container(dd, n, m);

  // ===================================================================

  STAPL_TEST_REPORT(true,"Testing Partition, Mapper constructor")
  matrix_type2 rs(part2, mz);
  test_container(rs, n, m);

  // ===================================================================

  STAPL_TEST_MESSAGE("Testing size_type, value_type constructor")
  matrix_type2 rr(gid_type(n,m), 5);
  test_container(rr, n, m);

  // ===================================================================

  STAPL_TEST_MESSAGE("Testing Partition Constructor")
  matrix_type d(part);
  test_container(d, n, m);
}


void test_sparse(size_t n, size_t m)
{
  typedef int                                                value_type;
  typedef indexed_domain<size_t>                             vector_domain_type;
  typedef balanced_partition<vector_domain_type>             balanced_type;
  typedef stapl::row_major                                   traversal_type;
  typedef nd_partition<
            stapl::tuple<balanced_type, balanced_type>,
            traversal_type>                                  partition_type;
  typedef stapl::tuple<size_t, size_t>                       gid_type;

  // ===================================================================

  STAPL_TEST_MESSAGE("Testing sparse size_type, Partition constructor")
  typedef sparse_matrix<value_type, traversal_type, partition_type> matrix_type;

  balanced_type p0(vector_domain_type(0, n-1), get_num_locations());
  balanced_type p1(vector_domain_type(0, m-1), get_num_locations());
  partition_type part(p0, p1);

  matrix_type f(gid_type(n,m), part);
  test_container(f, n, m);

   // ===================================================================

   STAPL_TEST_MESSAGE("Testing sparse size_type constructor")
   typedef sparse_matrix<value_type> matrix_type2;
   matrix_type2 g(gid_type(n,m));
   test_container(g, n, m);

   // ===================================================================

   STAPL_TEST_MESSAGE("Testing sparse size_type, Mapper constructor")
   typedef matrix_type2::domain_type domain_type;
   typedef matrix_type2::partition_type partition_type2;
   typedef matrix_type2::mapper_type mapper_type;

   domain_type dom(gid_type(n,m));
   partition_type2 part2(dom,
   stapl::multiarray_impl::make_multiarray_size<2>()(get_num_locations()));
   mapper_type mz(part2);

   matrix_type2 dd(gid_type(n,m), mz);
   test_container(dd, n, m);

   // ===================================================================

   STAPL_TEST_REPORT(true,"Testing sparse Partition, Mapper constructor")
   matrix_type2 rs(part2, mz);
   test_container(rs, n, m);

   // ===================================================================

   STAPL_TEST_MESSAGE("Testing sparse Partition Constructor")
   matrix_type d(part);
   test_container(d, n, m);
}


void test_sparse_external(size_t n, size_t m)
{
  STAPL_TEST_MESSAGE("Testing sparse with external storage")

  typedef int                                 value_type;
  typedef sparse_matrix_external<value_type>  matrix_type;
  typedef stapl::tuple<size_t, size_t>        gid_type;

  matrix_type mat(gid_type(n, m));

  const int nz = 5;

  // Non Zero Element Values
  std::vector<value_type> a {1, 2, 3, 4, 5};

  // Column Indices for Non Zero Elements;
  std::vector<int> colidx {1, 2, 1, 2, 1};

  // Row start indices, place one element on first a.size() rows.
  std::vector<int> rowstr {1, 2, 3, 4, 5};

  for (int i = 0; i < (int) (n - nz); ++i)
    rowstr.push_back(nz+1);

  for (auto&& bc : mat.distribution().container_manager())
  {
    bc.set_storage(get<0>(bc.dimensions()), get<1>(bc.dimensions()),
                   nz, &(*a.begin()), &(*rowstr.begin()), &(*colidx.begin()));
  }

  rmi_fence();

  int sum = 0;

  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < m; ++j) {
      gid_type g(i,j);
      sum += mat.get_element(g);
    }
  }

  bool passed = sum == (int) get_num_locations() * nz * (nz + 1) / 2;

  stapl_bool res(passed);

  bool all_passed = res.reduce();

  STAPL_TEST_REPORT(all_passed, "Testing get_element")
}


stapl::exit_code stapl_main(int argc, char* argv[])
{
  if (argc < 3) {
    std::cout<< "usage: exe n m" <<std::endl;
    exit(1);
  }

  size_t n = atoi(argv[1]);
  size_t m = atoi(argv[2]);

  test_dense(n, m);
  test_sparse(n, m);
  test_sparse_external(n, m);

  return EXIT_SUCCESS;
}
