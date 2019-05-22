/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <stapl/containers/multiarray/multiarray.hpp>
#include <stapl/containers/type_traits/default_traversal.hpp>
#include <stapl/views/multiarray_view.hpp>
#include <stapl/views/slices_view.hpp>
#include <stapl/utility/integer_sequence.hpp>
#include <stapl/algorithms/algorithm.hpp>

#include <boost/format.hpp>

#include "../test_report.hpp"

using namespace stapl;

//////////////////////////////////////////////////////////////////////
/// @brief Workfunction to increment a scalar
//////////////////////////////////////////////////////////////////////
struct increment
{
  typedef void result_type;

  template<typename T>
  void operator()(T x)
  {
    x += 1;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Workfunction that receives a slice of a view, and spawns
///        a nested PARAGRAPH to increment all elements of that slice.
//////////////////////////////////////////////////////////////////////
struct increment_slice
{
  typedef void result_type;

  template<typename View>
  void operator()(View const& slice)
  {
    typedef typename dimension_traits<View>::type     dim_t;
    typedef skeletons::spans::balanced<dim_t::value>  span_t;

    skeletons::algorithm_executor<null_coarsener>().execute(
      skeletons::zip<1, span_t>(increment()), slice
    );
  }
};


template<int D, int NumSlices,
         typename Indices = make_index_sequence<D>>
class test_slicer;


//////////////////////////////////////////////////////////////////////
/// @brief Test the slices view by creating a D-dimensional multiarray
///        and created a NumSlices-dimensional @see slices_view over it.
///        Each element of the slices view should be a @see SLICED_view,
///        and we increment all entries of the SLICED_view. The test passes
///        if all elements in the original multiarray is exactly 1.
//
/// @tparam D The number of dimensions of the multiarray
/// @tparam NumSlices The number of dimensions of the slices view
//////////////////////////////////////////////////////////////////////
template<int D, int NumSlices, std::size_t... Indices>
class test_slicer<D, NumSlices, index_sequence<Indices...>>
{
  typedef int                                         value_type;
  typedef multiarray<D, value_type>                   multiarray_type;
  typedef multiarray_view<multiarray_type>            view_type;
  typedef typename multiarray_type::traversal_type    traversal_type;
  typedef typename multiarray_type::dimensions_type   dimensions_type;
  typedef make_index_sequence<NumSlices>              slices_index_sequence_t;


  //////////////////////////////////////////////////////////////////////
  /// @brief Create a NumSlices-dimensional slices view over a D-dimensional
  ///        multiarray view, where the dimensions  0, 1, ..., NumSlices-1
  ///        are sliced off.
  //////////////////////////////////////////////////////////////////////
  template<typename View, std::size_t... SlicesIndices>
  static auto create_slices_view(View const& view,
                                 index_sequence<SlicesIndices...>)
    -> decltype(make_slices_view<SlicesIndices...>(view))
  {
    return make_slices_view<SlicesIndices...>(view);
  }

  typedef decltype(create_slices_view(
    std::declval<view_type>(), std::declval<slices_index_sequence_t>()
  )) slices_view_type;

  //////////////////////////////////////////////////////////////////////
  /// @brief Manually iterate over the domain of the slices view. Manually
  ///        extract the subview for each element, and then iterate over
  ///        the subview. For each inner-most element, increment it by 1.
  ///
  /// @param View The slices view to test
  //////////////////////////////////////////////////////////////////////
  template<typename View>
  static void iterate_domain(View const& view)
  {
    auto dom = view.domain();

    stapl::do_once([&](){
      domain_map(dom, [&](decltype(dom.first()) const& idx) {
        auto subview = view[idx];
        auto subdom = subview.domain();

        domain_map(subdom, [&](decltype(subdom.first()) const& i) {
          subview[i] += 1;
        });
      });
    });
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Invoke a (non-coarsened) PARAGRAPH over the slices view, where
  ///        the work function spawns a nested map_func over the inner
  ///        view that increments all values by 1.
  ///
  /// @param View The slices view to test
  //////////////////////////////////////////////////////////////////////
  template<typename View>
  static void invoke_paragraph(View const& view)
  {
    typedef typename dimension_traits<View>::type     dim_t;
    typedef skeletons::spans::balanced<dim_t::value>  span_t;

    skeletons::algorithm_executor<null_coarsener>().execute(
      skeletons::zip<1, span_t>(increment_slice()), view
    );
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a D-dimensional multiarray and NumSlices-dimensional
  ///        slices view. With the slices view, attempt to increment
  ///        all values by 1 either through a map_func or manual iteration
  ///        and test that all of the values are correctly set.
  ///
  /// @param Sizes A D-dimensional tuple of sizes for the multiarray
  /// @param F A function that encapsulates incrementing the values
  /// @return Boolean indicating whether the test passed or failed
  //////////////////////////////////////////////////////////////////////
  template<typename Sizes, typename F>
  static bool create_and_test(Sizes const& sizes, F&& f)
  {
    dimensions_type s = dimensions_type(get<Indices>(sizes)...);
    multiarray_type c(s, 0);

    view_type v(c);

    auto sv = create_slices_view(v, slices_index_sequence_t());

    f(sv);

    const bool passed =
      stapl::all_of(linear_view(v), boost::bind(stapl::equal_to<int>(), _1, 1));

    const std::string msg =
      str(boost::format("%dd slicing off %d dimensions") % D % NumSlices);

    STAPL_TEST_REPORT(passed, msg);

    return passed;
  }

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Test the slices view for map_func and manual iteration of
  ///        its domain, and the domains of its subviews.
  ///
  /// @param Sizes A D-dimensional tuple of sizes for the multiarray
  /// @return Boolean indicating whether the test passed or failed
  //////////////////////////////////////////////////////////////////////
  template<typename Sizes>
  static bool call(Sizes const& sizes)
  {
    // create a slices view and manually iterate over its domain
    bool b0 = create_and_test(sizes, [](slices_view_type const& view) {
      iterate_domain(view);
    });

    // create a slices view and invoke a PARAGRAPH on it
    bool b1 = create_and_test(sizes, [](slices_view_type const& view) {
      invoke_paragraph(view);
    });

    return b0 && b1;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief For a given dimension D, call the main test routine for each
///        possible value of the number of slices.
///
///        For example, for the 4D case, call the tester with slicing
///        1 dimension, 2 dimensions and 3 dimensions.
//////////////////////////////////////////////////////////////////////
template<int D, int NumSlices = 1>
struct test_all_slices_for_dimension
{
  template<typename Sizes>
  static bool call(Sizes const& sizes)
  {
    return test_slicer<D, NumSlices>::call(sizes) &&
      test_all_slices_for_dimension<D, NumSlices+1>::call(sizes);
  }
};

template<int D>
struct test_all_slices_for_dimension<D, D>
{
  template<typename Sizes>
  static bool call(Sizes const&)
  {
    return true;
  }
};

stapl::exit_code stapl_main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cout<< "usage: exe n" <<std::endl;
    exit(1);
  }

  const std::size_t n = atol(argv[1]);

  auto const sizes = std::make_tuple(n, n+1, n+2, n+3, n+4);

  test_all_slices_for_dimension<2>::call(sizes);
  test_all_slices_for_dimension<3>::call(sizes);
  test_all_slices_for_dimension<4>::call(sizes);

// 5 dimensions blows up compiler memory usage considerable
// test_all_slices_for_dimension<5>::call(sizes);

  return EXIT_SUCCESS;
}
