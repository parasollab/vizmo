/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_ALGORITHMS_MATRIX_BLAS_UTILS_HPP
#define STAPL_ALGORITHMS_MATRIX_BLAS_UTILS_HPP

#include <functional>
#include <boost/mpl/logical.hpp>
#include <boost/type_traits/is_same.hpp>

namespace stapl {

namespace algorithm_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Type trait class to detect if a data type is single precision.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct is_single {
  typedef is_single<T>  type;
  typedef bool          value_type;
  static const bool value = false;
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for @ref is_single handling the float data type.
//////////////////////////////////////////////////////////////////////
template<>
struct is_single<float> {
  typedef is_single<float>  type;
  typedef bool          value_type;
  static const bool value = true;
};


//////////////////////////////////////////////////////////////////////
/// @brief Type trait class to detect if a data type is double precision.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct is_double {
  typedef is_double<T>  type;
  typedef bool          value_type;
  static const bool value = false;
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for @ref is_double handling the double data type.
//////////////////////////////////////////////////////////////////////
template<>
struct is_double<double> {
  typedef is_double<double>  type;
  typedef bool          value_type;
  static const bool value = true;
};


//////////////////////////////////////////////////////////////////////
/// @brief Type trait class to detect if a data type is complex precision.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct is_complex {
  typedef is_complex<T>  type;
  typedef bool          value_type;
  static const bool value = false;
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for @ref is_complex handling the std::complex<float>.
//////////////////////////////////////////////////////////////////////
template<>
struct is_complex<std::complex<float> > {
  typedef is_complex<std::complex<float> >  type;
  typedef bool          value_type;
  static const bool value = true;
};


//////////////////////////////////////////////////////////////////////
/// @brief Type trait class to detect if a data type is a double complex
///   precision.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct is_double_complex {
  typedef is_double_complex<T>  type;
  typedef bool          value_type;
  static const bool value = false;
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for @ref is_double_complex handling the
///   std::complex<double>.
//////////////////////////////////////////////////////////////////////
template<>
struct is_double_complex<std::complex<double> > {
  typedef is_double_complex<std::complex<double> >  type;
  typedef bool          value_type;
  static const bool value = true;
};


//////////////////////////////////////////////////////////////////////
/// @brief Determine if a view is a fast view.
///
/// This won't work for PBLAS.
//////////////////////////////////////////////////////////////////////
template<typename View>
struct is_fast_view
  : boost::true_type
{};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization of @ref is_fast_view to determine if a view is a fast
///   view.
/// @tparam U View type wrapped in an nfast_view instance.
///
/// This won't work for PBLAS.
//////////////////////////////////////////////////////////////////////
template<typename U>
struct is_fast_view<nfast_view<U> >
  : boost::true_type
{};


//////////////////////////////////////////////////////////////////////
/// @brief Check typedef defined in sequential component to determine if
///   contiguous storage is used.
/// @tparam View Targeted view.
///
/// This won't work for PBLAS.
//////////////////////////////////////////////////////////////////////
template<typename View>
struct uses_contiguous_storage
  : View::view_container_type::contiguous_storage
{};


//////////////////////////////////////////////////////////////////////
/// @brief Check if all three views passed to the matrix multiplication
///   algorithm are fast views backed by contiguous storage.
/// @tparam V0 Two-dimensional view over floating-point elements.
/// @tparam V1 Two-dimensional view over floating-point elements.
/// @tparam V2 Two-dimensional view over floating-point elements.
///
/// If returns true then the BLAS matrix multiplication algorithms can be
///   called on the views.
//////////////////////////////////////////////////////////////////////
template<typename V0, typename V1, typename V2>
struct blas_conformable_view_set
  : boost::mpl::and_<is_fast_view<V0>, uses_contiguous_storage<V0>,
                     is_fast_view<V1>, uses_contiguous_storage<V1>,
                     boost::mpl::and_<
                       is_fast_view<V2>, uses_contiguous_storage<V2> > >
{};


//////////////////////////////////////////////////////////////////////
/// @brief Identify instantiations of std::plus.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct is_plus
  : boost::false_type
{};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization of @ref is_plus using std::plus.
/// @tparam U Scalar numeric data type.
//////////////////////////////////////////////////////////////////////
template<typename U>
struct is_plus<std::plus<U> >
  : boost::true_type
{};


//////////////////////////////////////////////////////////////////////
/// @brief Identify instantiations of std::multiplies.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct is_multiplies
  : boost::false_type
{};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization of @ref is_multiplies using std::multiplies.
/// @tparam U Scalar numeric data type.
//////////////////////////////////////////////////////////////////////
template<typename U>
struct is_multiplies<std::multiplies<U> >
  : boost::true_type
{};


//////////////////////////////////////////////////////////////////////
/// @brief Identifies function objects that are equivalent to the operations
///   employed by BLAS.
/// @tparam T Scalar numeric data type.
//////////////////////////////////////////////////////////////////////
template<typename T>
struct
blas_conformable_operator
  : boost::mpl::or_<is_plus<T>, is_multiplies<T> >
{};


//////////////////////////////////////////////////////////////////////
/// @brief Checks to ensure views are over floating point elements stored
///   contiguously and the function objects provided are simple addition and
///   multiplication operators.
/// @tparam T0 Two-dimensional view over floating point elements.
/// @tparam T1 Two-dimensional view over floating point elements.
/// @tparam T2 Two-dimensional view over floating point elements.
///
/// Parameters are the desired precision, up to 3 views, and up to 2
///   BinaryOperator functors.
//////////////////////////////////////////////////////////////////////
template<typename T0, typename T1 = T0, typename T2 = T1>
struct blas_conformable
  : boost::mpl::and_<
      boost::mpl::or_<
        is_single<typename T0::value_type>,
        is_double<typename T0::value_type>,
        is_complex<typename T0::value_type>,
        is_double_complex<typename T0::value_type>
      >,
      boost::mpl::or_<
        is_single<typename T1::value_type>,
        is_double<typename T1::value_type>,
        is_complex<typename T1::value_type>,
        is_double_complex<typename T1::value_type>
      >,
      boost::mpl::or_<
        is_single<typename T2::value_type>,
        is_double<typename T2::value_type>,
        is_complex<typename T2::value_type>,
        is_double_complex<typename T2::value_type>
      >,
      blas_conformable_view_set<T0, T1, T2>
    >
{};


//////////////////////////////////////////////////////////////////////
/// @brief Wrapper for @ref blas_conformable_view_set with single precision data
///   types.
/// @tparam T0 Two-dimensional view over floating point elements.
/// @tparam T1 Two-dimensional view over floating point elements.
/// @tparam T2 Two-dimensional view over floating point elements.
//////////////////////////////////////////////////////////////////////
template<typename T0, typename T1 = T0, typename T2 = T1>
struct sblas_conformable
  : boost::mpl::and_<
      is_single<typename T0::value_type>,
      is_single<typename T1::value_type>,
      is_single<typename T2::value_type>,
      blas_conformable_view_set<T0, T1, T2>
    >
{};


//////////////////////////////////////////////////////////////////////
/// @brief Wrapper for @ref blas_conformable_view_set with double data types.
/// @tparam T0 Two-dimensional view over floating point elements.
/// @tparam T1 Two-dimensional view over floating point elements.
/// @tparam T2 Two-dimensional view over floating point elements.
//////////////////////////////////////////////////////////////////////
template<typename T0, typename T1 = T0, typename T2 = T1>
struct dblas_conformable
  : boost::mpl::and_<
      is_double<typename T0::value_type>,
      is_double<typename T1::value_type>,
      is_double<typename T2::value_type>,
      blas_conformable_view_set<T0, T1, T2>
    >
{};


//////////////////////////////////////////////////////////////////////
/// @brief Wrapper for @ref blas_conformable_view_set with complex data types.
/// @tparam T0 Two-dimensional view over floating point elements.
/// @tparam T1 Two-dimensional view over floating point elements.
/// @tparam T2 Two-dimensional view over floating point elements.
//////////////////////////////////////////////////////////////////////
template<typename T0, typename T1 = T0, typename T2 = T1>
struct cblas_conformable
  : boost::mpl::and_<
      is_complex<typename T0::value_type>,
      is_complex<typename T1::value_type>,
      is_complex<typename T2::value_type>,
      blas_conformable_view_set<T0, T1, T2>
    >
{};


//////////////////////////////////////////////////////////////////////
/// @brief Wrapper for @ref blas_conformable_view_set with doublecomplex data
///   types.
/// @tparam T0 Two-dimensional view over floating point elements.
/// @tparam T1 Two-dimensional view over floating point elements.
/// @tparam T2 Two-dimensional view over floating point elements.
//////////////////////////////////////////////////////////////////////
template<typename T0, typename T1 = T0, typename T2 = T1>
struct zblas_conformable
  : boost::mpl::and_<
      is_double_complex<typename T0::value_type>,
      is_double_complex<typename T1::value_type>,
      is_double_complex<typename T2::value_type>,
      blas_conformable_view_set<T0, T1, T2>
    >
{};


//////////////////////////////////////////////////////////////////////
/// @brief Check if a view is aligned as a row.
/// @tparam T0 Two-dimensional view over floating-point elements.
//////////////////////////////////////////////////////////////////////
template<typename T0>
struct is_row_view
  : boost::is_same<typename T0::distribution_type::domain_type::mm_type,
                   ROW_MAJOR>
{};


//////////////////////////////////////////////////////////////////////
/// @brief Check if a view is aligned as a column.
/// @tparam T0 Two-dimensional view over floating-point elements.
//////////////////////////////////////////////////////////////////////
template<typename T0>
struct is_column_view
  : boost::is_same<typename T0::distribution_type::domain_type::mm_type,
                   COLUMN_MAJOR>
{};


//////////////////////////////////////////////////////////////////////
/// @brief Check if a view is aligned as a row-major order.
/// @tparam T0 Two-dimensional view over floating-point elements.
//////////////////////////////////////////////////////////////////////
template<typename T0>
struct is_row_major
  : boost::is_same<typename T0::view_container_type::container_major_type,
                   ROW_MAJOR>
{};


//////////////////////////////////////////////////////////////////////
/// @brief Check if a view is aligned as a column-major order.
/// @tparam T0 Two-dimensional view over floating-point elements.
//////////////////////////////////////////////////////////////////////
template<typename T0>
struct is_column_major
  : boost::is_same<typename T0::view_container_type::container_major_type,
                   COLUMN_MAJOR>
{};


//////////////////////////////////////////////////////////////////////
/// @brief Check if three matrices have the same size.
/// @todo 'compatibility has been misspelled.
//////////////////////////////////////////////////////////////////////
struct is_matrix_size_compatability
{
  template<typename V0, typename V1, typename V2>
  bool operator()(const V0& view0, const V1& view1, const V2& view2)
  {
    // get block size of each matrix
    typename V0::size_type Abs = view0.get_data()->size_twod();
    typename V1::size_type Bbs = view1.get_data()->size_twod();
    typename V2::size_type Cbs = view2.get_data()->size_twod();
    return (Abs.y == Cbs.x) && (Bbs.y == Cbs.y) && (Abs.x == Bbs.x);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Check if three blocks have the same size.
/// @todo 'compatibility has been misspelled.
//////////////////////////////////////////////////////////////////////
struct is_block_size_compatability
{
  template<typename V0, typename V1, typename V2>
  bool operator()(const V0& view0, const V1& view1, const V2& view2)
  {
    // get block size of each matrix
    typename V0::size_type Abs =
      view0.get_p_container()->get_distribution().get_data()->get_block_size();
    typename V1::size_type Bbs =
      view1.get_p_container()->get_distribution().get_data()->get_block_size();
    typename V2::size_type Cbs =
      view2.get_p_container()->get_distribution().get_data()->get_block_size();
    return (Abs.y == Cbs.x) && (Bbs.y == Cbs.y) && (Abs.x == Bbs.x);
  }
};


} //algorithm_impl

} //end namespace stapl
#endif
