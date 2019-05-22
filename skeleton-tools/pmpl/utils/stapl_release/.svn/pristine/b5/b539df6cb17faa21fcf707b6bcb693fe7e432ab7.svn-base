/*
 // Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
 // component of the Texas A&M University System.

 // All rights reserved.

 // The information and source code contained herein is the exclusive
 // property of TEES and may not be disclosed, examined or reproduced
 // in whole or in part without explicit written authorization from TEES.
 */

#ifndef STAPL_ALGORITHMS_MATRIX_SUMMA_BULK_COPY_HPP
#define STAPL_ALGORITHMS_MATRIX_SUMMA_BULK_COPY_HPP

#include <stapl/paragraph/paragraph.hpp>
#include <stapl/paragraph/utility.hpp>

#include "summa_work_functions.hpp"
#include "matrix_mapping_functions.hpp"
#include <views/partitions/matrix_block_partition.hpp>
namespace stapl {
namespace algo_details{

//////////////////////////////////////////////////////////////////////
/// @brief Copying values to components given by @c to_components list
/// This method invokes set_elements for each component. This can
/// be replaced by a multicast which can be called from the containers.
///
/// @param matrix        the matrix to which the to_components belong
/// @param values        the values (should be iterable) which are
///                      going to be copied to each component
/// @param to_components the list of component CIDs to which values are
///                      going to be be copied
///
/// @see p_container_indexed::set_elements(cid_type, values_type)
//////////////////////////////////////////////////////////////////////
template <typename Matrix, typename Data, typename CIDs>
void copy_components(Matrix matrix, Data& values, CIDs& to_components)
{
  //now we wrap our data in a wrapper
  for (unsigned int i = 0; i < to_components.size(); ++i) {
    matrix->set_elements(to_components[i], values);
  }

}


//////////////////////////////////////////////////////////////////////
/// @brief Puts the data in a component in a new content-holder.
///
/// The main reason behind this copying is that sometimes we need to
/// pad the components contents in order to match the other component
/// sizes. Consider a matrix in SUMMA algorithm which is segmented
/// to non-uniform components. In that case the padding is necessary,
/// otherwise the SUMMA algorithm cannot be used.
///
/// This is an example for non-uniform sized components:
/// Consider a 5x5 matrix and that is divided into 2x2 components<br/>
/// @code
/// |x x x a a|
/// |y y y b b|
/// |z z z c c|
/// |t t t d d|
/// |w w w e e|
/// @endcode
///
/// the components will look like the following, and would be of
/// non-uniform size
/// @code
/// |x x x| |a a|
/// |y y y| |b b|
/// |z z z| |c c|
///
/// |t t t| |d d|
/// |w w w| |e e|
/// @endcode
///
/// so if we want to send data from component 2 to for example
/// component 1 we have to pad it as
/// @code
/// |a a 0|
/// |b b 0|
/// |c c 0|
/// @endcode
///
/// @param component    the source from which the values should be
///                     copied to values
/// @param values the   place holder to keep the padded contents of
///                     the component
/// @param values_sizes is used to determine when to pad with zeros
/// @see copy_components(Matrix, Data&, CIDs&)
//////////////////////////////////////////////////////////////////////
template <typename Component, typename Data, typename Size>
void prepare_data(Component* component, Data& values, Size& values_sizes)
{

  typename Component::size_type component_size = component->size_twod();

  bool dest_is_larger = (get<0> (values_sizes) > get<0> (component_size)) ||
                        (get<1> (values_sizes) > get<1> (component_size));
  typedef typename Component::iterator comp_iter_t;

  comp_iter_t it = component->begin();
  comp_iter_t it_end = component->end();

  unsigned int counter = 0;
  for (; it != it_end; ++counter) {

    // if it is not in the src rows or
    // or it is in the columns that src does not have
    bool should_be_set_zero = dest_is_larger && (
    ((counter / get<1> (values_sizes)) > (get<0> (component_size) - 1)) ||
    ((counter % get<1> (values_sizes)) > (get<1> (component_size) - 1)));
    if (should_be_set_zero)
      values[counter] = 0;
    else {
      //the default case is to copy everything from src to dest
      //      std::cout << *it << " " << std::flush;
      values[counter] = *it;
      //src should be incremented here not in the for statement,
      //because otherwise if dest > src, then it will try to read garbage
      ++it;
    }

  }
}
}//namespace algo_details

//////////////////////////////////////////////////////////////////////
/// @brief This is the Summa matrix multiplication algorithm that
/// benefits from aggregate data copying.
///
/// This implementation copies parts of A and B using @c set_elements
/// function in @c p_container instead of @c set_element. In addition,
/// it sets the elements by directly using component IDs and avoiding
/// the @c PARAGRAPH. Therefore, it is a faster implementation than
/// the one with single element copies.
///
/// @param view_a matrix A
/// @param view_b matrix B
/// @param view_c matrix C (C = A x B)
/// @param broadcast_view_a the matrix_view over broadcast matrix A,
///                         this view should be sent to this method
/// @param broadcast_view_b the matrix_view over broadcast matrix A,
///                         this view should be sent to this method
/// @param partition_dims   the segment information used for creating
///                         native segments over the container
///
/// @see stapl::matrix_view
/// @see stapl::p_container_indexed::set_element(gid_type const&, value_type const&)
/// @see stapl::p_container_indexed::set_elements(cid_type const&, Container const&)
///
/// @todo needs to be changed for A and B with different dimensions
///////////////////////////////////////////////////////////////////////
template <typename ViewA, typename ViewB, typename ViewC>
void summa_matrix_multiply(ViewA& view_a, ViewB& view_b, ViewC& view_c,
                           ViewA& broadcast_view_a, ViewB& broadcast_view_b,
                           tuple<std::size_t, std::size_t> partition_dims)
{
  /// we use a different type of segmenting for the segmented matrix

  tuple<std::size_t, std::size_t> matrix_size = view_c.domain().size_twod();
  tuple<std::size_t, std::size_t>
    broadcast_matrix_size = broadcast_view_a.domain().size_twod();

  typedef new_dom1D<std::size_t>                             vec_dom_t;
  typedef new_matrix_part_balance<vec_dom_t>                 bal_part_t;
  typedef nd_partition<stapl::tuple<bal_part_t, bal_part_t>> part_t;
  typedef new_dom2D<std::size_t>                             matrix_dom_t;

  unsigned long partition_column_dim = get<0> (partition_dims);
  unsigned long partition_row_dim = get<1> (partition_dims);

  /// now we have to create segments with the corresponding domains for
  /// the block_matrix_segments that we have to create over the original
  /// matrix view
  bal_part_t p0(partition_row_dim);
  bal_part_t p1(partition_column_dim);
  part_t     matrix_partition(p0, p1);
  part_t     broadcast_matrix_part(p0, p1);

  /// create the domain of the original matrices
  matrix_dom_t matrix_domain(vec_dom_t(0, get<0> (matrix_size) - 1),
                             vec_dom_t(0, get<1> (matrix_size) - 1));
  matrix_partition.update_domain(matrix_domain);

  /// create the domain for broadcast matrices
  matrix_dom_t broadcast_matrix_domain(
                 vec_dom_t(0, get<0> (broadcast_matrix_size) - 1),
                 vec_dom_t(0, get<1> (broadcast_matrix_size) - 1));
  broadcast_matrix_part.update_domain(broadcast_matrix_domain);

  /// @todo why should we re-find-out the partition size in here?
  /// couldn't we get it from somewhere else?
  std::size_t partition_row_size =
    get<0> (matrix_size) / partition_row_dim +
    (get<0> (matrix_size) % partition_row_dim ? 1 : 0);

  std::size_t partition_column_size =
    get<1> (matrix_size) / partition_column_dim +
    (get<1> (matrix_size) % partition_column_dim ? 1 : 0);
  tuple<std::size_t, std::size_t> partition_size(partition_row_size,
                                                 partition_column_size);

  /// Here every block in the matrix has local domains for example
  ///                                (0,0)--(0,5) (0,0)--(0,5)
  ///(0,0)--(0,5)--(0,9)               |xxxxxx|     |yyyyyy|
  ///  |xxxxxx|yyyyyy|                 |xxxxxx|     |yyyyyy|
  ///  |xxxxxx|yyyyyy|                 |xxxxxx|     |yyyyyy|
  ///  |xxxxxx|yyyyyy|   changes to  (5,0)--(5,5) (5,0)--(5,5)
  ///(5,0)--(5,5)--(5,9) ===========>
  ///  |zzzzzz|wwwwww|               (0,0)--(0,5) (0,0)--(0,5)
  ///  |zzzzzz|wwwwww|                 |zzzzzz|     |wwwwww|
  ///  |zzzzzz|wwwwww|                 |zzzzzz|     |wwwwww|
  ///(9,0)--(9,5)--(9,9)               |zzzzzz|     |wwwwww|
  ///                                (5,0)--(5,5) (5,0)--(5,5)
  ///
  /// Therefore, we need to have a mapper which can convert our local indices
  /// to global indices and vice versa
  typedef map_fun_gen_1d_to_2d<
           f_global_index<matrix_dom_t::index_type> > map_func_t;

  /// check if dimensions of A and B match for multiplication
  stapl_assert(get<1> (view_a.domain().size_twod()) ==
               get<0> (view_b.domain().size_twod()),
               "Matrix dimensions do not match");

  /// if all the matrices are the same this part can be simplified, otherwise
  /// we need to define a segmented view based on the mapping function that
  /// we defined and matrix_block_partition/
  /// The goal in here is to make a segmented matrix view that can be used
  /// as a coarse view over the matrices
  /// if we have such a matrix we will can work with blocks in multiplication
  typedef segmented_view<
            ViewA,
            matrix_block_partition<part_t>,
            map_func_t>                               viewA_block_part_v;
  typedef segmented_view<
            ViewB,
            matrix_block_partition<part_t>,
            map_func_t>                               viewB_block_part_v;
  typedef segmented_view<
            ViewC,
            matrix_block_partition<part_t>,
            map_func_t>                               viewC_block_part_v;

  /// the native segment for a view corresponds to the segmenting that
  /// is used by the underlying container
  matrix_block_partition<part_t> matrix_native_part(matrix_partition,
                                                         partition_row_dim,
                                                         partition_column_dim);
  map_func_t mapping_function(partition_size, partition_dims);

  /// and we create blockwise segmented view over the matrices
  viewA_block_part_v viewA_part(view_a, matrix_native_part, mapping_function);
  viewB_block_part_v viewB_part(view_b, matrix_native_part, mapping_function);
  viewC_block_part_v viewC_part(view_c, matrix_native_part, mapping_function);

  /// now we have to create the blockwise segment for the broadcast matrices
  /// as well
  matrix_block_partition<part_t> broadcast_matrix_native_part(
                                   broadcast_matrix_part,
                                   partition_row_dim,
                                   partition_column_dim);
  /// the mapping function will be the same as for the original matrices
  viewA_block_part_v broadcast_viewA_part(broadcast_view_a,
                                          broadcast_matrix_native_part,
                                          mapping_function);
  viewB_block_part_v broadcast_viewB_part(broadcast_view_b,
                                          broadcast_matrix_native_part,
                                          mapping_function);

  /// and now we can start performing the multiplication
  typedef typename ViewA::view_container_type::cid_type cid_type;
  std::vector<cid_type>
    a_local_cids = view_a.get_container()->get_distribution()->get_local_cids();
  std::vector<cid_type>
    b_local_cids = view_b.get_container()->get_distribution()->get_local_cids();

  for (std::size_t itr = 0; itr < partition_column_dim; ++itr) {
#ifdef SHOW_TIMING
    timer t;
    t.reset();
    t.start();
#endif

    /// for all components that this location has and are in the `itr`th
    /// column of A broadcast them in the row
    typedef tuple<unsigned int, unsigned int> tuple_type;
    for (unsigned int i = 0; i < a_local_cids.size(); i++) {
      if (get<1> (a_local_cids[i].cid()) == itr) {
        typename cid_type::internal_cid_type current_cid =
          a_local_cids[i].cid();
        /// we want to copy the whole component to everybody in the same row

        /// what do we need here
        /// 1. on the broadcast-a matrix
        /// 2. data to be sent (we need to create a temporary storage for that
        ///   because we might want to pad it if it is something like this
        ///   |x x|   |x x - -|
        ///   |y y|   |y y - -|
        ///   |z z| =>|z z - -|
        ///   |t t|   |t t - -|
        /// 3. where to send - to all in the same row

        /// @todo be more wise and copy directly to broadcast-a
        /// @todo change to matrix data type
        std::vector<double> values(partition_column_size * partition_row_size);
        tuple_type values_size(partition_row_size, partition_column_size);

        //what is a component data
        typedef typename
          ViewA::view_container_type::component_type component_type;
        component_type* component = view_a.get_container()->
                                             get_location_manager()->
                                               get_component(current_cid);
        algo_details::prepare_data(component, values, values_size);

        std::vector<cid_type> cids(partition_column_dim);
        for (unsigned int j = 0; j < cids.size(); ++j)
          cids[j] = cid_type(tuple_type(get<0> (current_cid), j));

        algo_details::copy_components(broadcast_view_a.get_container(),
                                      values, cids);
      }
    }

    rmi_fence();

    /// for all components that this location has and are in the `itr`th row of
    /// B broadcast them in the column
    for (unsigned int i = 0; i < b_local_cids.size(); i++) {
      if (get<0> (b_local_cids[i].cid()) == itr) {
        typename cid_type::internal_cid_type
          current_cid = b_local_cids[i].cid();
        //we want to copy the whole component to everybody in the same row

        /// what do we need here
        /// 1. on the broadcast-a matrix
        /// 2. data to be sent (we need to create a temporary storage for that
        ///   because we might want to pad it if it is something like this
        ///   |x x|   |x x - -|
        ///   |y y|   |y y - -|
        ///   |z z| =>|z z - -|
        ///   |t t|   |t t - -|
        /// 3. where to send - to all in the same row

        std::vector<double> values(partition_column_size * partition_row_size);

        tuple_type values_size(partition_row_size, partition_column_size);

        //what is a component data
        typedef typename
          ViewB::view_container_type::component_type component_type;
        component_type* component = view_b.get_container()->
                                             get_location_manager()->
                                               get_component(current_cid);
        algo_details::prepare_data(component, values, values_size);

        std::vector<cid_type> cids(partition_row_dim);
        for (unsigned int j = 0; j < cids.size(); ++j)
          cids[j] = cid_type(tuple_type(j, get<1> (current_cid)));

        algo_details::copy_components(broadcast_view_b.get_container(),
                                      values, cids);

      }
    }
    rmi_fence();
    map_func(algo_details::summa_multiply_wf(),
             broadcast_viewA_part, broadcast_viewB_part, viewC_part);
#ifdef SHOW_TIMING
    std::cout << get_location_id() << " - time spent in " << itr
              <<"th iteration : " << t.stop() << std::endl;
#endif
  }

}

} // namespace stapl

#endif // STAPL_ALGORITHMS_MATRIX_SUMMA_BULK_COPY_HPP
