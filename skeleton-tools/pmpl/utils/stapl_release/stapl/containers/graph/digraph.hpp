/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_DIGRAPH_HPP
#define STAPL_CONTAINERS_DIGRAPH_HPP

#include <stapl/containers/graph/graph.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Creates a DIRECTED, NONMULTIEDGES graph.
/// @ingroup pgraphSpecial
///
/// Static graphs do not allow addition or deletion of vertices. The number of
/// vertices must be known at construction. Edges may be added/deleted.
/// @tparam VertexP type of property for the vertex. Default is no_property.
/// Must be default assignable, copyable and assignable.
/// @tparam EdgeP type of property for the edge. Default is no_property.
/// Must be default assignable, copyable and assignable.
/// @tparam PS Partition strategy that defines how to partition
/// the original domain into subdomains. The default partition is
/// @ref balanced_partition.
/// @tparam Map Mapper that defines how to map the subdomains produced
/// by the partition to locations. The default mapper is @ref mapper.
/// @tparam Traits A traits class that defines customizable components
/// of graph, such as the domain type, base container type, storage, etc. The
/// default traits class is @ref static_graph_traits.
//////////////////////////////////////////////////////////////////////
template <typename ...OptionalParams>
class digraph
  : public graph<DIRECTED, NONMULTIEDGES, OptionalParams...>
{
  typedef graph<DIRECTED, NONMULTIEDGES, OptionalParams...> base_type;

public:

  /// @name Constructors
  /// @{

  //////////////////////////////////////////////////////////////////////
  /// @copydoc graph::graph(size_t const&, vertex_property const&)
  //////////////////////////////////////////////////////////////////////
  digraph(size_t const& n=0,
          typename base_type::vertex_property const& default_value
            =typename base_type::vertex_property())
    : base_type(n, default_value)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc graph::graph(partition_type const&, vertex_property const&)
  //////////////////////////////////////////////////////////////////////
  digraph(typename base_type::partition_type const& ps,
          typename base_type::vertex_property const& default_value
            =typename base_type::vertex_property())
    : base_type(ps, default_value)
  { }

  /// @}
};

} // stapl namespace


#endif /* STAPL_CONTAINERS_DIGRAPH_HPP */
