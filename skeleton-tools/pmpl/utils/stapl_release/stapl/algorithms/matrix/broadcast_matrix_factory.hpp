/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_ALGORITHMS_MATRIX_BROADCAST_MATRIX_FACTORY_HPP
#define STAPL_ALGORITHMS_MATRIX_BROADCAST_MATRIX_FACTORY_HPP

#include <stapl/paragraph/paragraph.hpp>
#include <stapl/paragraph/utility.hpp>
#include <stapl/views/localize_element.hpp>
#include <stapl/utility/tuple.hpp>

namespace stapl {

namespace bradcast_matrix_factory_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Copies data from a given matrix to another.
///
/// @note It is assumed that the source matrix has always dimensions
/// which are smaller than or equal to destination matrix. If the
/// source matrix is smaller than the destination, then the rest of
/// destination matrix is padded with zeros.
///
/// @code
/// | a b c |
/// | d e f |  => | a b c d e f g h i |
/// | g h i |
/// @endcode
//////////////////////////////////////////////////////////////////////
struct copy_wf
{
  typedef void result_type;

  template <typename SrcView, typename DestView>
  void operator()(SrcView src_view, DestView dest_view) const
  {
    /// @todo since we don't have 2D indices we should use the iterators.
    ///       For this reason we have to use linear_row method from matrix_view
    ///       which creates a linearized view over a matrix
    ///

    typedef typename SrcView::linear_row_type linear_t;
    typedef typename linear_t::iterator       linear_iter_t;
    typedef tuple<std::size_t, std::size_t>   size_type;

    linear_t src_linear = src_view.linear_row();
    linear_t dest_linear = dest_view.linear_row();

    size_type src_size = src_view.domain().size_twod();
    size_type dest_size = dest_view.domain().size_twod();

    linear_iter_t src_iterator = src_linear.begin();
    linear_iter_t dest_iterator = dest_linear.begin();

    bool dest_is_larger = (get<0> (dest_size) > get<0> (src_size)) ||
                          (get<1> (dest_size) > get<1> (src_size));
    //we have to devise something for the cases that the source
    size_t counter = 0;

#ifdef SHOW_TIMING
    timer t;
    t.reset();
    t.start();
#endif

    for (; dest_iterator != dest_linear.end(); ++dest_iterator, ++counter) {

      // it is not in the src rows
      // or it is in the columns that src does not have*/
      bool should_be_set_zero = dest_is_larger && (
      ((counter / get<1> (dest_size)) > (get<0> (src_size) - 1)) ||
      ((counter % get<1> (dest_size)) > (get<1> (src_size) - 1)));
      if (should_be_set_zero)
        *dest_iterator = 0;
      else {
        //the default case is to copy everything from src to dest
        *dest_iterator = *src_iterator;
        //src should be incremented here not in the for statement,
        //because otherwise if dest > src, then it will try to read garbage
        ++src_iterator;
      }

    }

#ifdef SHOW_TIMING
      std::cout << get_location_id() << " - time spent in copying : "
                << t.stop() << std::endl;
#endif
    }
};

} // broadcast_matrix_factory_impl namespace


//////////////////////////////////////////////////////////////////////
/// @brief This factory copies columns of a matrix to a temporary
/// matrix which would be used further in other matrix multiplications.
//////////////////////////////////////////////////////////////////////
class broadcast_factory
  : public task_factory_base
{
public:
  typedef bradcast_matrix_factory_impl::copy_wf     wf_t;
  typedef paragraph_impl::coarse_map_wf<wf_t>       coarse_map_wf_t;
  typedef paragraph_impl::coarse_map_wf<wf_t>       coarse_wf_t;
  typedef void                                      result_type;
private:
  size_t m_itr;
  /// @brief the user workfunction will receive a
  /// @li a row of A matrix
  /// @li the complete B matrix
  /// @li the corresponding row of C
  wf_t        m_wf;
  coarse_wf_t m_coarse_wf;
  /// @brief this field indicates the number of tasks that should be executed
  /// in the current invocation of this factory

  std::size_t m_tasks_per_call;
  /// @brief m_percent_per_call is used as the sliding window, not all tasks
  /// that a paragraph creates might be added as tasks at once. In this
  /// example only 20% of the total tasks created per invocation of the factory
  float       m_percent_per_call;

  std::size_t m_rows;
  std::size_t m_columns;

  broadcast_factory(std::size_t itr,
                    std::size_t rows,
                    std::size_t columns)
    : m_itr(itr),
      m_coarse_wf(m_wf),
      m_percent_per_call(0.2),
      m_tasks_per_call(0),
      m_rows(rows),
      m_columns(columns)
  { }
};


//////////////////////////////////////////////////////////////////////
/// @brief Makes a copy of matrix B by copying one column of the
/// matrix B to every column of the tempB matrix.
///
/// @code
///     | a b c d |                | a a a a |
///     | e f g h |  it=1          | e e e e |
/// B = | i j k l |  ====> tempB = | i i i i |
///     | m n o p |                | m m m m |
/// @endcode
//////////////////////////////////////////////////////////////////////
class column_broadcast_factory
  : public broadcast_factory
{
  public:
    column_broadcast_factory(std::size_t itr,
                             std::size_t rows,
                             std::size_t columns)
      : broadcast_factory(itr, rows, columns)
    { }

    template <typename View_B, typename View_B_Copy>
    void operator()(View_B& view_b, View_B_Copy& view_b_copy)
    {

      using std::make_pair;
      using paragraph_impl::subview_id_iterator_base;

      typedef subview_id_iterator<typename View_B::cid_type> id_iterator_b;

      if (this->m_subview_id_iterators.empty()) {
        //get the local ids and push them back on subview_id_iterators
        tuple<id_iterator_b> id_it = paragraph_impl::partition_id_set(view_b);

        this->m_subview_id_iterators.push_back(
                static_cast<subview_id_iterator_base*> (
                  new id_iterator_b(get<0> (id_it))));

        // assume partition is uniform to avoid global comm to find view size.
        m_tasks_per_call = std::max(
                             (int) (this->m_subview_id_iterators[0]->size()),
                             1);
      }

      id_iterator_b*
        i = static_cast<id_iterator_b*> (this->m_subview_id_iterators[0]);

      std::size_t task_cnt = 0;
      typedef typename View_B::gid_type gid_type;
      /// the partitioned view sends a 1D array here, so @c view_b is a 1D view
      /// of all blocks that the matrix has. In that case since the native
      /// partition is (get_num_locations() x 1), its size would be just 1
      for (; !i->at_end(); ++(*i), ++task_cnt) {
        gid_type current_row = (**i) / m_columns;
        gid_type current_column = (**i) % m_columns;
        if (current_row == itr) {
          /// what we should do is to copy the current_column element to every
          /// row. For example, if we are in the third row (current_row = 3)
          /// then we have to create a task for every row other than 3
          for (unsigned int j = 0; j < m_rows; j++) {
            add_task(m_coarse_wf,
                     make_view_pair(&view_b, **i),
                     make_view_pair(&view_b_copy,
                                    (j * m_columns) + current_column));
          }
        }

      }
      this->m_finished = true;
    }

};


//////////////////////////////////////////////////////////////////////
/// @brief Makes a copy of matrix A by copying one row of the matrix
/// A to every row of the tempA matrix.
///
/// @code
///      | a b c d |                | a b c d |
///      | e f g h |  it=1          | a b c d |
///  A = | i j k l |  ====> tempA = | a b c d |
///      | m n o p |                | a b c d |
/// @endcode
//////////////////////////////////////////////////////////////////////
class row_broadcast_factory
  : public broadcast_factory
{
  public:
    row_broadcast_factory(std::size_t itr,
                          std::size_t rows,
                          std::size_t columns)
      : broadcast_factory(itr, rows, columns)
    { }

    template <typename View_A, typename View_A_Copy>
    void operator()(View_A& view_a, View_A_Copy& view_a_copy)
    {
      using std::make_pair;
      using paragraph_impl::subview_id_iterator_base;

      typedef subview_id_iterator<typename View_A::cid_type> id_iter_a;

      if (this->m_subview_id_iterators.empty()) {
        //get the local ids and push them back on subview_id_iterators
        tuple<id_iter_a> id_it = paragraph_impl::partition_id_set(view_a);

        this->m_subview_id_iterators.push_back(
                static_cast<subview_id_iterator_base*> (
                  new id_iter_a(get<0> (id_it))));

        // assume partition is uniform to avoid global comm to find view size.
        m_tasks_per_call = std::max(
                             (int) (this->m_subview_id_iterators[0]->size()),
                             1);
      }

      id_iter_a* i = static_cast<id_iter_a*> (this->m_subview_id_iterators[0]);

      std::size_t task_cnt = 0;
      typedef typename View_A::gid_type gid_type;
      /// partitioned view sends a 1D array here, so @c view_a is a 1D view of
      /// all blocks that the matrix has. Since that native partition is of
      /// size (get_num_locations() x 1) so the size of the input in here is 1
      for (; !i->at_end(); ++(*i), ++task_cnt) {
        gid_type current_row = (**i) / m_columns;
        gid_type current_column = (**i) % m_columns;

        if (current_column == itr) {

          /// what we should do in here is to copy the current_column element
          /// to all the other columns. For example, if current column is the
          /// their column (current_column = 3), we have to create a task for
          /// all the other columns
          gid_type row_start_index = current_row * m_columns;
          //copy current part of A to the corresponding row in temp
          for (unsigned int j = 0; j < m_columns; j++) {
            add_task(m_coarse_wf,
                     make_view_pair(&view_a, **i),
                     make_view_pair(&view_a_copy, row_start_index + j));
          }
        }

      }
      this->m_finished = true;
    }
};

} // namespace stapl

#endif
