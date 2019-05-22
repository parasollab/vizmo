/*
 // Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
 // component of the Texas A&M University System.

 // All rights reserved.

 // The information and source code contained herein is the exclusive
 // property of TEES and may not be disclosed, examined or reproduced
 // in whole or in part without explicit written authorization from TEES.
 */

#ifndef STAPL_ALGORITHMS_COLUMN_SORT_HPP
#define STAPL_ALGORITHMS_COLUMN_SORT_HPP

#include <stapl/domains/domain1D.hpp>
#include <stapl/domains/domain2D.hpp>
#include <stapl/algorithms/algorithm.hpp>
#include <stapl/views/balance_view.hpp>
#include <stapl/views/repeated_view.hpp>

#include <stapl/domains/domains.hpp>
#include <views/array_2D_view.hpp>
#include <pContainers/pmatrix/p_matrix.h>
#include <pContainers/pmatrix/p_matrix_partition.h>
#include <stapl/utility/tuple.hpp>
#include <stapl/algorithms/algorithm.hpp>
#include <stapl/algorithms/matrix/matrix_utils.hpp>
#include <pContainers/parray/p_array.h>


namespace stapl {
namespace algo_details {

//////////////////////////////////////////////////////////////////////
/// @brief This mapping is used to know the relation between the
/// reshaped and transposed matrix and the original matrix indices.
/// This knowledge is used further on for remote writes into a copy of
/// the matrix.
/// Using such indices makes all the writes async and makes the reads
/// local.
///
/// @par Reshape
/// Based on MATLAB documentation, reads from the <i>columns</i> and
/// writes to the <i>rows</i> without changing the dimensions of the
/// matrix.
/// @par Transpose
/// reads from columns and writes to the rows and swaps the dimensions
///
/// @code
/// | 1  2  3|                  | 1 10  8  6|              | 1  4  7|
/// | 4  5  6|  reshape(3, 4)   | 4  2 11  9|  transpose   |10  2  5|
/// | 7  8  9| ==============>  | 7  5  3 12| ===========> | 8 11  3|
/// |10 11 12|                                             | 6  9 12|
/// @endcode
///
/// @see MATLAB reshape operation documentation
/// @see MATLAB transpose operation documentation
//////////////////////////////////////////////////////////////////////
struct reshape_transpose_mapping
{
  typedef tuple<size_t, size_t>     result_type;
  typedef result_type               size_type;
  size_type m_size;

  reshape_transpose_mapping(size_type size)
    : m_size(size)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Receiving an 1D index from a linear column view, converts
  /// the index to a 2D index in the corresponding reshaped-transposed
  /// matrix
  ///
  /// @param index a 1D index
  //////////////////////////////////////////////////////////////////////
  result_type operator()(size_t index) const
  {
    return result_type(index / get<1> (m_size), index % get<1> (m_size));
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief This mapping is used to know the relation between the
/// transposed and reshaped matrix and the original matrix indices.
/// The difference between this mapping and @c reshape_transpose_mapping
/// is only in the order that the two transformations are applied
///
/// @code
/// | 1  2  3|              | 1  4  7 10|                 | 1  5  9|
/// | 4  5  6|  transpose   | 2  5  8 11|  reshape(4,3)   | 2  6 10|
/// | 7  8  9| ===========> | 3  6  9 12| ==============> | 3  7 11|
/// |10 11 12|                                            | 4  8 12|
/// @endcode
/// @see refer to MATLAB reshape operation
/// @see refer to MATLAB transpose operation
/// @see reshape_transpose_mapping
//////////////////////////////////////////////////////////////////////
struct transpose_reshape_mapping
{
  typedef tuple<size_t, size_t>     result_type;
  typedef result_type               size_type;
  size_type                         m_size;

  transpose_reshape_mapping(size_type size)
    : m_size(size)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Receiving an 1D index from a linear column view, converts
  /// the index to a 2D index in the corresponding transposed-reshaped
  /// matrix
  ///
  /// @param index a 1D index
  //////////////////////////////////////////////////////////////////////
  result_type operator()(size_t index) const
  {
    size_t row = get<0> (m_size);
    size_t col = get<1> (m_size);
    return result_type((((index % row) * col) + index / row) % row,
                       ((index % row) * col) / row);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief This generator behaves as a fake container, meaning that
/// receiving a 1D index it produces a 2D index by using the given
/// mapping function.
///
/// @note This generator does not return the actual data at an index
/// but returns a 2D index to a data.
///
/// @tparam F the mapping function to be used
/// @tparam T the type of the element to return
///
/// @see transpose_reshape_mapping
/// @see reshape_transpose_mapping
//////////////////////////////////////////////////////////////////////
template <typename F, typename T>
struct coordinate_gen_cont
{
  typedef T                              value_type;
  typedef value_type                     reference;
  typedef void                           iterator;
  typedef stapl::indexed_domain<size_t>  domain_type;
  typedef size_t                         gid_type;
  typedef tuple<size_t, size_t>          size_type;
  size_type                              m_size;
  F                                      m_f;

  coordinate_gen_cont(F f, size_type size)
    : m_size(size),
      m_f(f)
  { }

  reference operator[](size_t p) const
  {
    return m_f(p);
  }

  value_type get_element(size_t i) const
  {
    return this->operator[](i);
  }

  size_t size(void) const
  {
    return get<0> (m_size) * get<1> (m_size);
  }

  domain_type domain(void) const
  {
    return domain_type(0, this->size() - 1);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Simply sorts the given 1D view using @c std::sort
///
/// @tparam T type of the elements in the view
/// @tparam T the comparator to be used for sorting
//////////////////////////////////////////////////////////////////////
template <typename T, typename Comp>
struct sort_columns_sequential
{
  Comp   m_comp;
  size_t m_median;

  sort_columns_sequential(Comp comp = less<T>(), size_t median = 0)
    : m_comp(comp),
      m_median(median)
  { }

  void define_type(typer &t)
  {
    t.member(m_comp);
    t.member(m_median);
  }

  template <typename View>
  void operator()(View v) const
  {
    typename View::domain_type d = v.domain();
    typename View::iterator begin = v.begin();
    if (d.first() == 0)
    {
      std::advance(begin, m_median);
    }

    std::sort(begin, v.end(), m_comp);
  }
};


struct matrix_shape_modify_wf
{
  typedef void result_type;

  //////////////////////////////////////////////////////////////////////
  /// @brief Read from element referred to by @c c and write it to @c v
  /// at coordinate given by @c c
  ///
  /// @param r reference to the data to be read
  /// @param c coordinate in @c v to write to
  /// @param v the view that the asynchronous write is being called on
  //////////////////////////////////////////////////////////////////////
  template <typename Ref, typename Coord, typename View>
  void operator()(Ref r, Coord c, View v)
  {
    v.set_element(c, r);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Given a 1D index this mapping produces a 1D index with the
/// given offset
//////////////////////////////////////////////////////////////////////
template <typename T1, typename T2>
struct f_cyclic_offset
{
    typedef T2 gid_type;
    typedef T1 index_type;

    size_t m_size;
    size_t m_offset;

    f_cyclic_offset(size_t sz, size_t off = 0)
      : m_size(sz),
        m_offset(off)
    { }

    template <typename MF>
    f_cyclic_offset(MF const& mf)
      : m_size(mf.size),
        m_offset(mf.offset)
    { }

    gid_type operator()(index_type const& index) const
    {
      return gid_type((index + m_size + m_offset) % m_size);
    }
};


///////////////////////////////////////////////////////////////////////
/// @brief Reshape reads each column of the given matrix and puts the
/// data in rows of the result matrix without changing the size of the
/// column. Transpose then reads each row and puts the data in the
/// rows in columns of the same size.
///
/// @param view        the input matrix view
/// @param result_view the view to which the reshaped transposed matrix
///                    is going to be written
/// @param n           first computed dimension of the @c view (might
///                    not be the same as the real matrix dimension)
/// @param m           second computed dimension of the @c view (might
///                    not be the same as the real matrix dimension)
///////////////////////////////////////////////////////////////////////
template <typename IndexType, typename View, typename ResView>
void reshape_transpose(View& view, ResView& result_view,
                       std::size_t n, std::size_t m)
{
  typedef coordinate_gen_cont<
            reshape_transpose_mapping, IndexType> coord_gen_t;

  coord_gen_t resh_trans_coord(reshape_transpose_mapping(make_tuple(n, m)),
                               make_tuple(n, m));
  array_1D_ro_view<coord_gen_t> resh_trans_coord_view(resh_trans_coord);

  map_func(matrix_shape_modify_wf(),
           view.linear_column(),
           resh_trans_coord_view,
           make_repeat_view(result_view));
}

///////////////////////////////////////////////////////////////////////
/// @brief Transpose reads each row of the given matrix and puts the
/// data in columns of the result matrix without changing the size of
/// the row. Reshape then reads each column and puts the data in the
/// column in rows of the same size.
///
/// @param view        the input matrix view
/// @param result_view the view to which the transposed reshaped matrix
///                    is going to be written
/// @param n           first computed dimension of the @c view (might
///                    not be the same as the real matrix dimension)
/// @param m           second computed dimension of the @c view (might
///                    not be the same as the real matrix dimension)
///////////////////////////////////////////////////////////////////////
template <typename IndexType, typename View, typename ResView>
void transpose_reshape(View& view, ResView& result_view,
                       std::size_t n, std::size_t m)
{

  typedef coordinate_gen_cont<
          transpose_reshape_mapping, IndexType> rt_coord_gen_t;

  rt_coord_gen_t trans_resh_coord(transpose_reshape_mapping(make_tuple(n, m)),
                                  make_tuple(n, m));
  array_1D_ro_view<rt_coord_gen_t> trans_resh_coord_view(trans_resh_coord);

  map_func(matrix_shape_modify_wf(),
           view.linear_column(),
           trans_resh_coord_view,
           make_repeat_view(result_view));
}

///////////////////////////////////////////////////////////////////////
/// @brief Cyclic shift down move the elements in between the columns
/// of the matrix. This is a necessary step for column sort to
/// further on sort the whole array. For this to happen, we get a
/// matrix view, then we linearize the matrix by reading columns
/// of the matrix, and apply the cyclic shift on the linearized view.
///
/// @todo check if it is better to copy the elements to a different
/// container, because, now we are reading two half columns from two
/// different columns in sorting. The easier way is to create a view on
/// the linearized view over the original matrix.
///
/// @param view        the input matrix view
/// @param result_view the view to which the cyclically shifted down
///                    matrix is going to be written
/// @param n           first computed dimension of the @c view (might
///                    not be the same as the real matrix dimension)
/// @param m           second computed dimension of the @c view (might
///                    not be the same as the real matrix dimension)
///////////////////////////////////////////////////////////////////////
template <typename View, typename ResView>
void cyclic_shift_down(View& view, ResView& result_view,
                       std::size_t n, std::size_t m)
{
  typedef f_cyclic_offset<size_t, size_t>             cyc_mf_t;
  typedef coordinate_gen_cont<cyc_mf_t, size_t>       cyc_down_coord_gen_t;

  cyc_down_coord_gen_t csd_cgc(cyc_mf_t(n*m, floor(n / 2)), make_tuple(n, m));
  array_1D_ro_view<cyc_down_coord_gen_t> csd_cgv(csd_cgc);

  map_func(matrix_shape_modify_wf(),
           view.linear_column(),
           csd_cgv,
           make_repeat_view(result_view));
}

///////////////////////////////////////////////////////////////////////
/// @brief Cyclic shift down moves the elements in between the columns
/// of the matrix in the opposite direction of the shift
/// in @c cyclic_shift_down.
///
/// @param view        the input matrix view
/// @param result_view the view to which the cyclically shifted up
///                    matrix is going to be written
/// @param n           first computed dimension of the @c view (might
///                    not be the same as the real matrix dimension)
/// @param m           second computed dimension of the @c view (might
///                    not be the same as the real matrix dimension)
///
/// @see cyclic_shift_down
///////////////////////////////////////////////////////////////////////
template <typename View, typename CycDownView, typename ArrayView>
void cyclic_shift_up(View& view, CycDownView& cyc_down_view,
                     ArrayView& input_view,
                     std::size_t n, std::size_t m,
                     bool is_input_smaller)
{
  typedef f_cyclic_offset<size_t, size_t>             cyc_mf_t;
  typedef coordinate_gen_cont<cyc_mf_t, size_t>       cyc_up_coord_gen_t;

  cyc_up_coord_gen_t csu_cgc(cyc_mf_t(n*m, -floor(n / 2)), make_tuple(n, m));
  array_1D_ro_view<cyc_up_coord_gen_t> csu_cgv(csu_cgc);

  if (is_input_smaller) {
    typedef typename View::linear_column_type         linear_col_t;
    typedef array_1D_view<
              typename linear_col_t::view_container_type,
              indexed_domain<size_t>,
              typename linear_col_t::map_func_type>   smaller_view_t;
    typedef typename smaller_view_t::domain_type      smaller_view_dom_t;
    map_func(matrix_shape_modify_wf(),
             cyc_down_view,
             csu_cgv,
             make_repeat_view(view.linear_column()));
    smaller_view_t smaller_view(view.linear_column().container(),
                                smaller_view_dom_t(0, input_view.size() - 1),
                                view.linear_column().mapfunc());
    stapl::copy(smaller_view, input_view);
  }
  else
    map_func(matrix_shape_modify_wf(),
             cyc_down_view,
             csu_cgv,
             make_repeat_view(input_view));
}

///////////////////////////////////////////////////////////////////////
/// @brief This is the implementation for column-sort algorithm for
/// sorting array introduced by Tom Leighton. This algorithm first
/// copies the 1D input data to a matrix. Then it performs the
/// following operations as mentioned in the column-sort paper to sort
/// the input data, and in the end copies back the result to the input
/// @li <b>step 1 -</b> sorts each column of the matrix
/// @li <b>step 2 -</b> reshapes and transposes the matrix
/// @li <b>step 3 -</b> sorts each column of the matrix
/// @li <b>step 4 -</b> transposes and reshapes the matrix
/// @li <b>step 5 -</b> sorts each column of the matrix
/// @li <b>step 6 -</b> cyclic-shift-downs the matrix
/// @li <b>step 7 -</b> sorts each column of the matrix
/// @li <b>step 8 -</b> cyclic-shift-ups the matrix
///
/// @param view0 the input array to be sorted
/// @param comp  the comparator to be used for sorting
/// @see p_matrix
/// @see matriv_view
/// @see <b>Tight Bounds on the Complexity of Parallel Sorting -
///         Extened Abstract - Tom Leighton MIT</b>
///
/// @todo Should be ported to use new containers. There are issues
///       in defining @c array_view over p_matrix, and the current
///       matrix in the new containers framework does not support
///       non-square matrices.
///////////////////////////////////////////////////////////////////////
template <typename ArrayView, typename Comp>
void column_sort_impl(ArrayView& view0, Comp comp)
{

  size_t original_input_size = view0.size();

  /// Determine the dimensions of the r x s matrix.
  /// s = number of locations and r = number of elements per column

  /// to skip always one column in running in one location
  size_t s = get_num_locations();
  size_t r = original_input_size / s;

#if TEST_ORIGINAL_PAPER_EXAMPLE
  //use the dimensions used in Leighton paper
  r = 6;
  s = 3;
#else
  /// Increase the size of r until it's divisible by s and greater than
  /// or equal to 2(s-1)^2, AND so r*s is greater than or equal to the original
  /// # of elements.  r = the matrix column/location size.
  /// The column-sort algorithm requires these r x s dimension constraints.

  //to avoid

  while (r % s != 0 ||
         r < 2 * (s - 1) * (s - 1) ||
         r * s < original_input_size)
  { ++r; }

#endif

  size_t n = r;
  size_t m = s;
  //stapl_assert(original_input_size <= r * s,
  //             "Column matrix created out of the input array is too small!");

  typedef typename ArrayView::value_type                     value_t;
  typedef new_dom1D<size_t>                                  vec_dom_t;
  typedef new_dom2D<size_t>                                  mat_dom_t;
  typedef balanced_partition<vec_dom_t>                      bal_part_t;
  typedef nd_partition<stapl::tuple<bal_part_t, bal_part_t>> part_t;
  typedef p_matrix<value_t, COLUMN_MAJOR, part_t>            matrix_t;
  typedef matrix_view<matrix_t, mat_dom_t,
            f_ident<typename mat_dom_t::index_type> >        mat_view_t;
  typedef typename mat_view_t::domain_type                   view_dom_t;

  // set up column partition
  bal_part_t p0(1);
  bal_part_t p1(get_num_locations());
  part_t part(p0, p1);
  mat_dom_t mdom(vec_dom_t(0, n - 1), vec_dom_t(0, m - 1));
  part.update_domain(mdom);
  view_dom_t matrix_view_domain(n, m);
  // set up the matrix and view
  matrix_t matrix(n, m, part);
  mat_view_t view(matrix, matrix_view_domain);
  //when the matrix is transposed/reshaped we keep the new matrix in tmpmatrix
  matrix_t tmpmatrix(n, m, part);
  mat_view_t tmpview(tmpmatrix, matrix_view_domain);

  // fill the matrix with random data
  typedef typename mat_view_t::linear_column_type     linear_col_t;
  // create a column view
  typedef segmented_view<
            mat_view_t,
            columns_partition<view_dom_t, indexed_domain<size_t> > ,
            map_fun_gen1<frow_2d<size_t, typename view_dom_t::index_type> > ,
            typename mat_view_t::column_type>         col_view_t;

  columns_partition<view_dom_t, indexed_domain<size_t> > colsp(view.domain());

  col_view_t col_view(view, colsp);
  linear_col_t linear_column = view.linear_column();
#if TEST_ORIGINAL_PAPER_EXAMPLE
  value_t a[] = {6, 14, 10, 3, 17, 5, 15, 4, 1, 16, 8, 11, 12, 7, 13, 9, 2, 0};
  if (get_location_id () == 0) {
    for (unsigned int i = 0; i < 18; ++i)
    linear_column[i] = a[i];
  }
#else
  initialize_matrix_with_constant(view, std::numeric_limits<value_t>::max());
  typedef array_1D_view<
            typename linear_col_t::view_container_type,
            indexed_domain<size_t>,
            typename linear_col_t::map_func_type>     smaller_view_t;
  typedef typename smaller_view_t::domain_type        smaller_view_dom_t;

  /// with the following view we can sort in-place
  if (r * s > original_input_size)
  {
    smaller_view_t smaller_view(linear_column.container(),
                                smaller_view_dom_t(0, original_input_size - 1),
                                linear_column.mapfunc());
    stapl::copy(view0, smaller_view);
  }
  else
    stapl::copy(view0, view.linear_column());
#endif
  /// STEP 1 - sorting the columns
  stapl::for_each(col_view, sort_columns_sequential<value_t, Comp> (comp));

  /// STEP 2 - reshape/transpose
  reshape_transpose<typename mat_dom_t::index_type>(view, tmpview, n, m);

  /// STEP 3 - sorting the columns of the reshaped/transposed matrix
  col_view_t tmp_col_view(tmpview, colsp);
  stapl::for_each(tmp_col_view, sort_columns_sequential<value_t, Comp> (comp));

  /// STEP 4 - now doing transpose/reshape and putting the results in the
  /// original matrix
  transpose_reshape<typename mat_dom_t::index_type>(tmpview, view, n, m);

  /// STEP 5 - sorting the columns of the original matrix
  stapl::for_each(col_view, sort_columns_sequential<value_t, Comp> (comp));

  /// STEP 6 -  cyclic shift-down half - the matrix is shifted down in columns
  /// by the amount of floor(n/2)
  size_t N = n * m;
  typedef p_array<value_t>                            cyc_down_t;
  typedef array_1D_view<cyc_down_t>                   cyc_down_view_t;
  cyc_down_t      tmp_cont(N);
  cyc_down_view_t cycled_down_view(tmp_cont);

  cyclic_shift_down(view, cycled_down_view, n, m);


  ///STEP 7 - yet another column sort, we have to sort the columns again -
  /// this time it is different, the items are in an array
  /// instantiate a part_balanced_view to do a local sort on the parray
  /// @todo: Antal, solve the problem in coarse_partition_locality in the cases
  /// that the mapping function is not identity
  typename result_of::balance_view<cyc_down_view_t>::type
    bal_vw = balance_view(cycled_down_view, m);
  stapl::for_each(bal_vw, sort_columns_sequential<value_t, Comp> (comp, n/2));

  /// STEP 8 - cycling shift half up, shift the matrix in column by the amount
  /// of floor(n/2), going back to the original matrix
  bool is_input_smaller = r * s > original_input_size;
  cyclic_shift_up(view, cycled_down_view, view0, n, m, is_input_smaller);
}

} // namespace algo_details

//////////////////////////////////////////////////////////////////////
/// @brief Column sort is a sorting algorithm for arrays introduced
/// by <b>Tom Leighton</b>, which sorts an array given a comparator.
///
/// @see column_sort_impl
/// @see <b>Tight Bounds on the Complexity of Parallel Sorting -
///         Extended Abstract - Tom Leighton MIT</b>
//////////////////////////////////////////////////////////////////////
template <typename View, typename Compare>
void column_sort(View& vw, Compare comp)
{
  algo_details::column_sort_impl(vw, comp);
}

} // namespace stapl

#endif

