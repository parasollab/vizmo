/*
 // Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
 // component of the Texas A&M University System.

 // All rights reserved.

 // The information and source code contained herein is the exclusive
 // property of TEES and may not be disclosed, examined or reproduced
 // in whole or in part without explicit written authorization from TEES.
 */

#ifndef MATRIX_MAPPING_FUNCTIONS_HPP_
#define MATRIX_MAPPING_FUNCTIONS_HPP_

#include <stapl/utility/tuple.hpp>

//////////////////////////////////////////////////////////////////////
/// @file matrix_mapping_functions.hpp
///
/// @todo Verify if this mapping functions are need after the matrix
///       algorithms (stapl/algorithms/matrix/summa_with_copy.hpp and
///       stapl/algorithms/matrix/summa_bulk_copy.hpp) are updated to
///       use multi-dimensional array.
//////////////////////////////////////////////////////////////////////

namespace stapl {

template <typename T>
struct f_local_index;

//////////////////////////////////////////////////////////////////////
/// @brief Functor to translate 2-dimensional local indexes into
///        2-dimensional global indexes.
//////////////////////////////////////////////////////////////////////
template <typename T>
struct f_global_index
{
    typedef T index_type;
    typedef T gid_type;
    typedef f_local_index<T> inverse;

    T m_offset;

    f_global_index(T offset, T partition_size)
      : m_offset(T(get<0> (offset) * get<0> (partition_size),
                   get<1> (offset) * get<1> (partition_size)))
    { }

    f_global_index(inverse const& mf)
      : m_offset(mf.m_offset)
    { }

    gid_type operator()(index_type x) const
    {
      return index_type(get<0> (m_offset) + get<0> (x),
                        get<1> (m_offset) + get<1> (x));
    }

    void define_type(typer& t)
    {
      t.member(m_offset);
    }
};


//////////////////////////////////////////////////////////////////////
/// @brief Functor to translate 2-dimensional global indexes into
///        2-dimensional local indexes.
//////////////////////////////////////////////////////////////////////
template <typename T>
struct f_local_index
{
    typedef T index_type;
    typedef T gid_type;
    typedef f_global_index<T> inverse;

    T m_offset;

    /*
     * ---------------      (0,0)--(0,5)--(0,9)
     * |      |      |        |      |      |
     * |(0,0) |(0,1) |        |      |      |
     * |      |      |        |      |      |
     * |-------------| =>   (5,0)--(5,5)--(5,9)
     * |      |      |        |      |      |
     * |(1,0) |(1,1) |        |      |      |
     * |      |      |        |      |      |
     * ---------------      (9,0)--(9,5)--(9,9)
     *
     */
    f_local_index(T offset, T partition_size)
      : m_offset(T(get<0> (offset) * get<0> (partition_size),
                   get<1> (offset) * get<1> (partition_size)))
    { }

    f_local_index(inverse mf)
      : m_offset(mf.m_offset)
    { }

    gid_type operator()(index_type x) const
    {
      return index_type(get<0> (m_offset) - get<0> (x),
                        get<1> (m_offset) - get<1> (x));
    }

    void define_type(typer& t)
    {
      t.member(m_offset);
    }

};

//////////////////////////////////////////////////////////////////////
/// @brief Map function generator where every mapping function
///        generated is constructed passing the index as input
///        argument.
/// @tparam MF Type of the mapping function to return.
//////////////////////////////////////////////////////////////////////
template <typename MF>
struct map_fun_gen_1d_to_2d
{
    typedef tuple<size_t, size_t> partition_size_type;
    partition_size_type m_partition_size;
    partition_size_type m_partition_dims;
    typedef MF mapfunc_type;

    map_fun_gen_1d_to_2d(partition_size_type partition_size,
                         partition_size_type partition_dims)
      : m_partition_size(partition_size), m_partition_dims(partition_dims)
    { }

    mapfunc_type operator[](size_t i) const
    {
      size_t partition_column_dim = get<1> (m_partition_dims);
      return mapfunc_type(tuple<size_t, size_t>(i / partition_column_dim,
                                                i % partition_column_dim),
                          m_partition_size);
    }

};

} // stapl namespace

#endif /* MATRIX_MAPPING_FUNCTIONS_HPP_ */
