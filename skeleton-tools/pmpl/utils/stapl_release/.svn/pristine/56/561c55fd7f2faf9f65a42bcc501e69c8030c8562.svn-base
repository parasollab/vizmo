/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <iostream>

#include <stapl/multiarray.hpp>
#include <stapl/views/repeated_view.hpp>
#include <stapl/algorithms/algorithm.hpp>
#include <stapl/skeletons/transformations/optimizers/zip.hpp>
#include <stapl/skeletons/transformations/nest/nest.hpp>
#include <stapl/skeletons/transformations/transform.hpp>
#include <stapl/skeletons/transformations/doall.hpp>
#include <stapl/skeletons/operators/elem.hpp>
#include <stapl/skeletons/functional/map.hpp>
#include <stapl/skeletons/functional/zip.hpp>
#include <stapl/skeletons/functional/wavefront.hpp>
#include <stapl/skeletons/executors/algorithm_executor.hpp>
#include <stapl/views/metadata/coarseners/null.hpp>
#include <stapl/utility/do_once.hpp>
#include "../test_report.hpp"

using namespace stapl;

namespace {

template <typename V>
bool validate(V&& v, int n)
{
  std::size_t nx, ny;
  std::tie(nx, ny) = v.domain().dimensions();
  std::size_t startx, starty;
  std::tie(startx, starty) = v.domain().first();

  std::function<int (int)> factorial = [&] (int i)
  {
    return (i == 1) ? 1 : i * factorial(i - 1);
  };

  for (std::size_t i = 0; i < nx; ++i)
  {
    for (std::size_t j = 0; j < ny; ++j)
    {
      if (v[make_tuple(startx+i, starty+j)] != factorial(n))
        return false;
    }
  }
  return true;
}

template <int i, typename T>
struct sum_op;

template <typename T>
struct sum_op<2, T>
{
  typedef T result_type;

  size_t m_idx;

public:

  void set_op_index(size_t idx)
  {
    m_idx = idx;
  }

  template <typename V1, typename V2, typename V3>
  result_type operator()(V1&& v1, V2&& v2, V3&& v3) const
  {
    v1 *= m_idx+1;
    return v1;
  }

  void define_type(typer& t)
  {
    t.member(m_idx);
  }
};

struct my_tag {};

} // namespace

namespace stapl {
namespace skeletons {
namespace transformations {

template <typename S,
          typename SkeletonTag,
          typename TransformTag>
struct transform;

template <typename S>
struct transform<S, my_tag, skeletons::tags::doall>
{
  using corners_t = typename S::skeleton_type::corners_type;
  using op_t      = typename S::skeleton_type::op_type;

  template <typename Index>
  static S call(S const& wavefront_sk,
                Index const& index)
  {
    return skeletons::wavefront(
             get_op(wavefront_sk, index),
             get_corners(index),
             wavefront_sk.get_filter());
  }

private:
  template <typename Index>
  static op_t get_op(S const& wavefront_sk, Index const& index)
  {
    auto op      = wavefront_sk.get_op();
    op.set_op_index(index);
    return op;
  }

  template <typename Index>
  static corners_t get_corners(Index const& index)
  {
    auto f = position::first;
    auto l = position::last;
    switch(index%4)
    {
      case 0:
        return {{f, f}};
      case 1:
        return {{f, l}};
      case 2:
        return {{l, f}};
      case 3:
        return {{l, l}};
      default:
        return {{f, f}};
    }
  }

};

} // namespace transformations
} // namespace skeletons
} // namespace stapl

namespace  {

void doall_wavefront(std::size_t n1, std::size_t n2)
{
  using value_type      = size_t;
  using multiarray_type = multiarray<2, value_type>;
  using dimensions_type = typename multiarray_type::dimensions_type;
  using corner_t        = skeletons::position;
  using corner_type     = std::array<corner_t, 2>;

  dimensions_type dims = dimensions_type(n1, n2);
  multiarray_type input(dims);

  auto input_view  = make_multiarray_view(input);

  auto index_view = counting_view<value_type>(n1);

  using namespace skeletons;
  using namespace wavefront_utils;
  using skeletons::tag;
  using skeletons::wrap;

  auto f = position::first;

  corner_type corners = {{f, f}};

  stapl::fill( input_view, value_type(1));

  auto wavefront_sk  = tag<my_tag>(
                         wavefront(sum_op<2, value_type>(), corners));

  algorithm_executor<null_coarsener>().execute(
    zip<4>(wrap<tags::doall_execution<>>(wavefront_sk)),
    index_view,
    make_repeat_view(input_view),
    make_repeat_view(input_view),
    make_repeat_view(input_view));

  STAPL_TEST_REPORT(validate(input_view, n1), "Testing doall-wavefront");

}

} // namespace

exit_code stapl_main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cout<< "usage: <exec> <n>" <<std::endl;
    exit(1);
  }

  std::size_t n = atol(argv[1]);
  doall_wavefront(n, n);

  return EXIT_SUCCESS;
}
