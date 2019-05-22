/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_MATRIX_VIEW_HPP
#define STAPL_VIEWS_MATRIX_VIEW_HPP

#include <stapl/utility/utility.hpp>
#include <stapl/views/multiarray_view.hpp>

#include "rows_view.hpp"


namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Defines a matrix view (two-dimensional view).
///
/// Provides the operations that are commonly present in a two
/// dimensional array plus some helper methods
/// @tparam C Container type.
/// @tparam Dom Domain type. By default uses the same domain type
///             provided for the container.
/// @tparam Mapping function type
///    (default: identity mapping function)
/// @ingroup matrix_view
//////////////////////////////////////////////////////////////////////
template <typename C,
          typename Dom = typename container_traits<C>::domain_type,
          typename MapFunc = f_ident<typename container_traits<C>::gid_type>>
class matrix_view;


//////////////////////////////////////////////////////////////////////
/// @brief Specialization based on the multiarray specification
//////////////////////////////////////////////////////////////////////
template<typename... Params>
struct view_traits<matrix_view<Params...>>
  : view_traits<multiarray_view<Params...>>
{ };


template <typename C, typename Dom, typename MF>
class matrix_view
  : public multiarray_view<C, Dom, MF>
{
  typedef multiarray_view<C, Dom, MF>                            base_type;

public:
  typedef C                                                      container_type;
  typedef Dom                                                    domain_type;
  typedef typename domain_type::index_type                       index_type;
  typedef MF                                                     map_func_type;
  typedef map_func_type                                          map_function;

  // Includes base type constructors
  // @todo Uncomment the following statement when C++11 is complete
  //     valid for stapl
  //  using base_type::base_type;

  //////////////////////////////////////////////////////////////////////
  /// @copydoc stapl::multiarray_view::multiarray_view(view_container_type&,domain_type const&,map_func_type)
  //////////////////////////////////////////////////////////////////////
  matrix_view(container_type& vcont,
              domain_type const& dom,
              map_function mfunc = map_function())
    : base_type(vcont,dom,mfunc)
  { }

  matrix_view(container_type const& vcont,
              domain_type const& dom,
              map_function mfunc = map_function())
    : base_type(vcont,dom,mfunc)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc stapl::multiarray_view::multiarray_view(view_container_type*)
  //////////////////////////////////////////////////////////////////////
  matrix_view(container_type* vcont)
    : base_type(vcont, stapl::get_domain(*vcont))
  { }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc stapl::multiarray_view::multiarray_view(view_container_type&)
  //////////////////////////////////////////////////////////////////////
  matrix_view(container_type& vcont)
    : base_type(vcont, stapl::get_domain(vcont))
  { }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc stapl::multiarray_view::multiarray_view(view_container_type*,domain_type const&,map_func_type)
  //////////////////////////////////////////////////////////////////////
  matrix_view(container_type* vcont,
              domain_type const& dom,
              map_function mfunc = map_function())
    : base_type(vcont,dom,mfunc)
  { }

  matrix_view(container_type& vcont,
              domain_type const& dom,
              map_function mfunc,
              matrix_view const&)
    : base_type(vcont,dom,mfunc)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the domain associated with the row's dimension.
  //////////////////////////////////////////////////////////////////////
  auto row_domain() const
    STAPL_AUTO_RETURN(this->domain().template get_domain<0>())

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the domain associated with the column's dimension.
  //////////////////////////////////////////////////////////////////////
  auto col_domain() const
    STAPL_AUTO_RETURN(this->domain().template get_domain<1>())

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns a rows_view, where each element of the
  ///        view is a view over a row
  //////////////////////////////////////////////////////////////////////
  rows_view<matrix_view> rows() const
  {
    return make_rows_view(*this);
  }

};

} // namespace stapl

#endif /* STAPL_VIEWS_MATRIX_VIEW_HPP */
