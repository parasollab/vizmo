/*
 // Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
 // component of the Texas A&M University System.

 // All rights reserved.

 // The information and source code contained herein is the exclusive
 // property of TEES and may not be disclosed, examined or reproduced
 // in whole or in part without explicit written authorization from TEES.
 */

#ifndef STAPL_ALGORITHMS_MATRIX_SUMMA_WITH_COPY_HPP
#define STAPL_ALGORITHMS_MATRIX_SUMMA_WITH_COPY_HPP

/*
 * Matrix multiplication algorithm (SUMMA algorithm - adaptation)
 */
#include <stapl/paragraph/paragraph.hpp>
#include <stapl/paragraph/utility.hpp>
#include <stapl/utility/tuple.hpp>

#include <p_matrix.h>
#include "naive_matrix_multiply.hpp"

#include <views/partitions/matrix_block_partition.hpp>
#include "broadcast_matrix_factory.hpp"
#include "matrix_multiply_factory.hpp"
#include "matrix_mapping_functions.hpp"
#include "summa_work_functions.hpp"

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Summa matrix multiplication without the bulk data copying.
///
/// This summa implementation, copies parts of A and B using
/// @c set_element in the containers (setting each matrix element one
/// by one). In other words, it uses async_rmi(write) per each remote
/// element. This method uses a paragraph to perform copying and
/// multiplication.
///
/// @param view_a           matrix A
/// @param view_b           matrix B
/// @param view_c           matrix C
/// @param broadcast_view_a the matrix_view over broadcast matrix A,
///                         this view should be sent to this method
/// @param broadcast_view_b the matrix_view over broadcast matrix B,
///                         this view should be sent to this method
/// @param partition_dims   is used for creating native_segments for
///                         the views over the container
///
/// @see matrix_view
/// @see p_container_indexed::set_element
//////////////////////////////////////////////////////////////////////
template <typename ViewA, typename ViewB, typename ViewC>
void summa_matrix_multiply(ViewA& view_a, ViewB& view_b, ViewC& view_c,
                           ViewA& broadcast_view_a, ViewB& broadcast_view_b,
                           tuple<std::size_t, std::size_t> partition_dims)
{
  tuple<std::size_t, std::size_t> matrix_size = view_c.domain().size_twod();
  /// @todo consider A and B of the same size (square matrices)
  tuple<std::size_t, std::size_t> broadcast_matrix_size =
    broadcast_view_a.domain().size_twod();

  typedef new_dom1D<std::size_t>                             vec_dom_t;
  typedef new_matrix_part_balance<vec_dom_t>                 bal_part_t;
  typedef nd_partition<stapl::tuple<bal_part_t, bal_part_t>> part_t;
  typedef new_dom2D<std::size_t>                             matrix_dom_t;

  unsigned long partition_column_dim = get<0> (partition_dims);
  unsigned long partition_row_dim = get<1> (partition_dims);

  /// now we have to create segments with the corresponding domains
  /// for the block_matrix_partitions that we have to create above the
  /// normal matrix view
  bal_part_t p0(partition_row_dim);
  bal_part_t p1(partition_column_dim);
  part_t     matrix_partition(p0, p1);
  part_t     broadcast_matrix_part(p0, p1);

  /// creating the domain for the original matrices
  matrix_dom_t matrix_domain(vec_dom_t(0, get<0> (matrix_size) - 1),
                             vec_dom_t(0, get<1> (matrix_size) - 1));
  matrix_partition.update_domain(matrix_domain);

  /// creating the domain for broadcast matrices
  matrix_dom_t broadcast_matrix_domain(
                 vec_dom_t(0, get<0> (broadcast_matrix_size) - 1),
                 vec_dom_t(0, get<1> (broadcast_matrix_size) - 1));
  broadcast_matrix_part.update_domain(broadcast_matrix_domain);

  /// @todo why should we re-find-out the partition size in here? couldn't
  /// we get it from somewhere else?
  std::size_t partition_row_size =
    get<0> (matrix_size) / partition_row_dim +
    (get<0> (matrix_size) % partition_row_dim ? 1 : 0);
  std::size_t partition_column_size =
    get<1> (matrix_size) / partition_column_dim +
    (get<1> (matrix_size) % partition_column_dim ? 1 : 0);

  tuple<std::size_t, std::size_t> partition_size(partition_row_size,
                                                 partition_column_size);
  DEBUG_INFO("Partition size = " << partition_size <<
             " and partition dims = " << partition_dims);

  /// in our example, every block in the matrix has local domains for example
  ///                                 (0,0)--(0,5) (0,0)--(0,5)
  /// (0,0)--(0,5)--(0,9)               |xxxxxx|     |yyyyyy|
  ///   |xxxxxx|yyyyyy|                 |xxxxxx|     |yyyyyy|
  ///   |xxxxxx|yyyyyy|                 |xxxxxx|     |yyyyyy|
  ///   |xxxxxx|yyyyyy|   changes to  (5,0)--(5,5) (5,0)--(5,5)
  /// (5,0)--(5,5)--(5,9) ===========>
  ///   |zzzzzz|wwwwww|               (0,0)--(0,5) (0,0)--(0,5)
  ///   |zzzzzz|wwwwww|                 |zzzzzz|     |wwwwww|
  ///   |zzzzzz|wwwwww|                 |zzzzzz|     |wwwwww|
  /// (9,0)--(9,5)--(9,9)               |zzzzzz|     |wwwwww|
  ///                                 (5,0)--(5,5) (5,0)--(5,5)
  ///
  /// Therefore, we need to have a mapper which can convert our local
  /// indices to global indices and vice versa
  typedef map_fun_gen_1d_to_2d<
            f_global_index<
              matrix_dom_t::index_type> >             map_func_t;

  typedef typename ViewA::domain_type viewA_dom_t;
  typedef typename ViewB::domain_type viewB_dom_t;
  typedef typename ViewC::domain_type viewC_dom_t;
  viewA_dom_t view_a_domain = view_a.domain();
  viewB_dom_t view_b_domain = view_b.domain();
  viewC_dom_t view_c_domain = view_c.domain();
  //check if dimensions of A and B match for multiplication
  stapl_assert(get<1> (view_a_domain.size_twod()) ==
               get<0> (view_b_domain.size_twod()),
               "Matrix dimensions do not match");

  /// if all the matrices are the same this part can be simplified,
  /// otherwise we define a segmented view based on the map function
  /// that we defined and matrix_block_partition
  /// the goal in here is to make a segmented blockwise matrix view that
  /// can be used as a coarse view over the matrices
  /// if we have such a matrix we will be working with blocks in multiplication
  typedef segmented_view<
            ViewA,
            matrix_block_partition<part_t>,
            map_func_t>                               viewA_block_part_v_t;
  typedef segmented_view<
            ViewB,
            matrix_block_partition<part_t>,
            map_func_t>                               viewB_block_part_v_t;
  typedef segmented_view<
            ViewC,
            matrix_block_partition<part_t>,
            map_func_t>                               viewC_block_part_v_t;

  matrix_block_partition<part_t> matrix_native_partition(matrix_partition,
                                                         partition_row_dim,
                                                         partition_column_dim);
  map_func_t mapping_function(partition_size, partition_dims);

  /// and we create blockwise partitioned view over the matrices
  viewA_block_part_v_t view_a_partitioned(view_a, matrix_native_partition,
                                          mapping_function);
  viewB_block_part_v_t view_b_partitioned(view_b, matrix_native_partition,
                                          mapping_function);
  viewC_block_part_v_t view_c_partitioned(view_c, matrix_native_partition,
                                          mapping_function);

  /// now we have to create the blockwise segment for the broadcast matrices
  /// as well
  matrix_block_partition<part_t> broadcast_matrix_native_part(
                                   broadcast_matrix_part,
                                   partition_row_dim,
                                   partition_column_dim);

  /// the mapping function will be the same as for the original matrices
  viewA_block_part_v_t broadcast_viewA_part(broadcast_view_a,
                                            broadcast_matrix_native_part,
                                            mapping_function);
  viewB_block_part_v_t broadcast_viewB_part(broadcast_view_b,
                                            broadcast_matrix_native_part,
                                            mapping_function);

  /// and now we can start performing the multiplication
  for (std::size_t itr = 0; itr < partition_column_dim; ++itr) {
#ifdef SHOW_TIMING
    timer t;
    t.reset();
    t.start();
#endif
    /// broadcasting 'itr'th column of A

    make_paragraph(row_broadcast_factory(itr, partition_row_dim,
                                         partition_column_dim),
                   view_a_partitioned, broadcast_viewA_part)();

    /// broadcasting 'itr'th row of B
    make_paragraph(column_broadcast_factory(itr, partition_row_dim,
                                            partition_column_dim),
                   view_b_partitioned, broadcast_viewB_part)();

    /// map_func can be used instead of this too,
    typedef algo_details::summa_multiply_wf wf_t
    make_paragraph(matrix_multiply_factory<wf_t> (wf_t()),
                   broadcast_viewA_part, broadcast_viewB_part,
                   view_c_partitioned)();

#ifdef SHOW_TIMING
    std::cout << get_location_id() << " - time spent in " << itr
              <<"th iteration : " << t.stop() << std::endl;
#endif
  }

}
} //namespace stapl

#endif // STAPL_ALGORITHMS_MATRIX_SUMMA_WITH_COPY_HPP
