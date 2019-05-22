/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_GRAPH_TRAITS_CSR_GRAPH_TRAITS_HPP
#define STAPL_CONTAINERS_GRAPH_TRAITS_CSR_GRAPH_TRAITS_HPP

#include <stapl/containers/distribution/container_manager/container_manager_static_graph.hpp>
#include <stapl/containers/distribution/directory/static_registry.hpp>
#include <stapl/containers/distribution/directory/container_directory.hpp>
#include <stapl/containers/graph/distribution_static.hpp>
#include <stapl/containers/mapping/mapper.hpp>

#include <stapl/containers/graph/base_container/base_container.hpp>
#include <stapl/containers/graph/base_container/csr_storage.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Default traits for the @ref csr_graph 's base container.
/// @ingroup pgraphTraitsBaseContainer
/// @tparam D graph-attribute specifying Directedness (DIRECTED/UNDIRECTED).
/// @tparam M graph-attribute specifying Multiedge. (MULTIEDGES/NONMULTIEDGES).
/// @tparam VertexP type of property for the vertex.
/// @tparam EdgeP type of property for the edge.
/// @tparam Dom Domain type of the base container.
////////////////////////////////////////////////////////////////////////
template <graph_attributes D, graph_attributes M,
          typename VertexP, typename EdgeP, typename Dom>
struct csr_base_container_traits
{
  typedef csr_base_container_traits<D,M,VertexP,EdgeP, Dom> this_type;
public:

  typedef VertexP                                         raw_vertex_property;
  typedef typename define_value_type<VertexP>::type           stored_type;
  typedef stored_type                                         vertex_property;
  typedef EdgeP                                               edge_property;
  typedef size_t                                              vertex_descriptor;
  typedef size_t                                       simple_vertex_descriptor;
  typedef edge_descriptor_impl<vertex_descriptor>             edge_descriptor;
  typedef typename sequential::select_edge<vertex_descriptor,
                               edge_property,D>::type         edge_type;
  typedef csr_edgelist_impl<edge_type>                        edgelist_type;
  typedef vertex_csr_impl
          <vertex_descriptor,vertex_property,edgelist_type>   vertex_impl_type;
  typedef csr_vector_storage<this_type>            storage_type;
  typedef csr_model<this_type>                     container_type;
  typedef typename sequential::graph_type<this_type,D>::type  directness_type;
  typedef typename sequential::graph_type<this_type,M>::type  multiplicity_type;
  static const graph_attributes d_type = D; //directedness
  static const graph_attributes m_type = M; //edge multiplicity
  typedef Dom                                                 domain_type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Default traits for the @ref csr_graph container.
/// Specifies customizable type parameters that could be changed on a
/// per-container basis.
/// @ingroup pgraphTraitsPGraph
/// @tparam D graph-attribute specifying Directedness (DIRECTED/UNDIRECTED).
/// @tparam M graph-attribute specifying Multiedge. (MULTIEDGES/NONMULTIEDGES).
/// @tparam VertexP type of property for the vertex.
/// @tparam EdgeP type of property for the edge.
/// @tparam PS Partition strategy that defines how to partition
/// the original domain into subdomains.
/// @tparam Map Mapper that defines how to map the subdomains produced
/// by the partition to locations.
////////////////////////////////////////////////////////////////////////
template<graph_attributes D, graph_attributes M,
         typename VertexP, typename EdgeP,
         typename PS, typename Map>
struct csr_graph_traits
{
  typedef csr_graph_traits<D,M,VertexP,EdgeP,PS,Map>         this_type;
  typedef VertexP                                            vertex_property;
  typedef EdgeP                                              edge_property;
  typedef size_t                                             vertex_descriptor;
  typedef size_t                                      simple_vertex_descriptor;
  typedef vertex_descriptor                                  gid_type;
  typedef edge_descriptor_impl<vertex_descriptor>            edge_descriptor;
  typedef typename sequential::graph_type<this_type,D>::type directness_type;
  typedef typename sequential::graph_type<this_type,M>::type multiplicity_type;
  typedef PS                                                 partition_type;
  typedef Map                                                mapper_type;
  typedef typename PS::value_type                            domain_type;
  typedef typename domain_type::index_type                   index_type;
  typedef typename PS::index_type                            cid_type;
  typedef typename domain_type::size_type                    size_type;


  typedef graph_base_container<csr_base_container_traits<D,M,VertexP,EdgeP,
                                                         domain_type>
                               >                         base_container_type;

  typedef typename detail::construct_container_registry<
                     base_container_type, partition_type
                   >::type                             container_registry_type;

  typedef container_manager_static_graph<base_container_type,
    container_registry_type>
                                                         container_manager_type;


  typedef typename base_container_type::vertex_type      value_type;

private:
  typedef directory_impl::manager<partition_type, mapper_type> manager_type;

public:
  typedef container_directory<
            partition_type,
            mapper_type,
            manager_type,
            static_registry<manager_type>
          >                                                    directory_type;

  //////////////////////////////////////////////////////////////////////
  /// @brief Metafunction to compute the distribution type based on a
  /// container type.
  /// @tparam C Type of the container.
  //////////////////////////////////////////////////////////////////////
  template <typename C>
  struct construct_distribution
  {
    typedef graph_distribution_static<C>   type;
  };

};

} // namespace stapl

#endif
