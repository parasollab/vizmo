/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_GRAPH_ALGORITHMS_PARADIGMS_LEVEL_SYNC_PARADIGM_HPP
#define STAPL_CONTAINERS_GRAPH_ALGORITHMS_PARADIGMS_LEVEL_SYNC_PARADIGM_HPP

#include <stapl/containers/graph/algorithms/paradigms/kla_paradigm.hpp>
#include <stapl/containers/graph/views/graph_view.hpp>
#include <stapl/algorithms/functional.hpp>
#include <stapl/paragraph/paragraph.hpp>
#include <stapl/views/repeated_view.hpp>
#include <stapl/runtime.hpp>

namespace stapl {

namespace level_sync_paradigm_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Work function to wrap the user provided vertex-operator.
/// The work function is applied on a vertex if the vertex is active.
/// Active vertices may perform some computation and update their values, and
/// may visit their neighboring vertices with the user provided
/// neighbor-operator.
/// Returns true if vertex was active (i.e. the user's work function returned
/// true), false otherwise.
/// @tparam GraphView Type of the input @ref graph_view.
/// @tparam WF Type of the user provided vertex-operator.
/// @ingroup pgraphAlgoDetails
/// @todo This work function stores a view to the input graph that is
/// needed for calling apply_set on target vertices inorder to apply
/// the neighbor-operator. This is currently passed in through the operator() as
/// a @ref repeat_view, with the pointer being set to the view received in
/// the operator(). However, this is slower than passing the view through
/// the constructor by ~7% on Hopper (using the g500 benchmark test). This
/// is documented in GForge to-do task #1207.
//////////////////////////////////////////////////////////////////////
template<typename GraphView, typename WF>
class level_sync_map_wf
{
  size_t     m_curr_level;
  WF         m_wf;
  GraphView* m_gvw;

  template<typename VertexDescriptor>
  location_type home_location(VertexDescriptor const& v) const
  { return m_gvw->container().locality(v).location(); }

public:
  typedef typename GraphView::vertex_descriptor descriptor_t;
  typedef typename GraphView::vertex_descriptor vertex_descriptor;
  typedef typename GraphView::vertex_property   vertex_property;
  typedef typename GraphView::edge_property     edge_property;

  typedef bool result_type;

  //////////////////////////////////////////////////////////////////////
  /// @param curr_level The current level of this visit.
  //////////////////////////////////////////////////////////////////////
  level_sync_map_wf(WF const& wf, size_t curr_level = 0)
    : m_curr_level(curr_level), m_wf(wf), m_gvw(0)
  { }

  template<typename Vertex, typename View>
  result_type operator()(Vertex v, View& g)
  {
    m_gvw = &g;
    return m_wf.template operator()<Vertex, level_sync_map_wf&>(v, *this);
  }

  //////////////////////////////////////////////////////////////////////
  /// Provides a visit() helper to user's work function
  /// so they can visit a neighboring vertex without exposing internals.
  /// @param target The descriptor of the target vertex to visit.
  /// @param uf The neighbor-operator to be applied on the target vertex.
  //////////////////////////////////////////////////////////////////////
  template<typename UpdateFunc>
  void visit(descriptor_t const& target, UpdateFunc const& uf)
  {
    stapl_assert(m_gvw != 0, "visit called with null graph view pointer");
    m_gvw->apply_set(target, uf);
  }


  //////////////////////////////////////////////////////////////////////
  /// Provides a visit_all_edges() helper to user's work function
  /// so they can visit all neighboring vertices without exposing internals.
  /// @param source The source vertex whose neighbors are being visited.
  /// @param uf The neighbor-operator to be applied on the target vertex.
  /// @todo Use aggregators here to allow aggregation even for non-contiguous
  /// edge-storage.
  //////////////////////////////////////////////////////////////////////
  template<typename Vertex, typename UpdateFunc>
  void visit_all_edges(Vertex source, UpdateFunc const& uf,
                       typename boost::disable_if_c<
                         boost::is_same<typename Vertex::adj_edge_iterator,
                         typename std::vector<
                           typename std::iterator_traits<
                             typename Vertex::adj_edge_iterator
                           >::value_type
                         >::iterator>::value>::type* =0)
  {
    auto it = source.begin();
    auto it_e = source.end();

    for (; it != it_e; ++it) {
      this->visit((*it).target(), uf);
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// Specialization of @ref visit_all_edges() for when the edges of the vertex
  /// are stored in contiguous (std::vector) storage.
  ///
  /// This version is significantly faster as it allows aggregation of message
  /// per vertex and avoids an extra copy in the runtime.
  /// Allows users to visit all neighboring vertices without exposing internals.
  /// @param source The source vertex whose neighbors are being visited.
  /// @param uf The neighbor-operator to be applied on the target vertex.
  //////////////////////////////////////////////////////////////////////
  template<typename Vertex, typename UpdateFunc>
  void visit_all_edges(Vertex source, UpdateFunc const& uf,
                       typename boost::enable_if_c<
                         boost::is_same<typename Vertex::adj_edge_iterator,
                         typename std::vector<
                           typename std::iterator_traits<
                             typename Vertex::adj_edge_iterator
                           >::value_type
                         >::iterator>::value>::type* =0)
  {
    stapl_assert(m_gvw != 0,
      "visit_all_edges called with null graph view pointer");

    typename Vertex::adj_edge_iterator it = source.begin();
    typename Vertex::adj_edge_iterator it_e = source.end();
    typename Vertex::adj_edge_iterator r_begin = it;

    if (it == it_e)
      return;

    location_type destination = home_location((*r_begin).target());

    for (it = r_begin+1; it != it_e; ++it) {
      const location_type n_dest = home_location((*it).target());
      if (n_dest != destination) {
        // Range aggregate...
        m_gvw->container().aggregate_apply_async(
          destination,
          make_range(r_begin, it),
          uf);
        r_begin = it;
        destination = n_dest;
      }
    }
    // Range aggregate...
    m_gvw->container().aggregate_apply_async(destination,
                                             make_range(r_begin, it_e),
                                             uf);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Visits neighbors of the provided source vertex that
  /// satisfy the given predicate (such that f(edge) is true) and applies
  /// the provided update functor on each of them.
  ///
  /// Adds a task on all neighboring vertices with the given neighbor-operator.
  /// @param source The vertex descriptor of the target vertex to visit.
  /// @param uf The neighbor-operator to apply on the target vertex.
  /// @param f The predicate to determine which edges to visit.
  /// @ingroup pgraphAlgoDetails
  //////////////////////////////////////////////////////////////////////
  template<class Vertex, class UpdateFunc, class Predicate>
  void visit_edge_if(Vertex source, UpdateFunc const& uf,
                     Predicate const& f) const
  {
    typename Vertex::adj_edge_iterator it = source.begin();
    typename Vertex::adj_edge_iterator it_e = source.end();
    for (; it != it_e; ++it) {
      if (f(*it))
        m_gvw->apply_set((*it).target(), uf);
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Adds an edge between the two given vertices with given property.
  /// The edge is added asynchronously and the method returns immediately.
  ///
  /// Edge is not guaranteed to be added until after current superstep ends.
  /// The edge may be added during the current superstep.
  /// @param source Descriptor of the source vertex.
  /// @param target Descriptor of the target vertex.
  /// @param ep Property of the edge.
  //////////////////////////////////////////////////////////////////////
  void add_edge(descriptor_t const& source, descriptor_t const& target,
                edge_property const& ep = edge_property())
  {
    m_gvw->add_edge_async(source, target);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Deletes the edge between the given source and target vertices.
  /// The edge is deleted asynchronously.
  ///
  /// The edge is not guaranteed to have been deleted until after current
  /// superstep ends, but may be deleted in the current superstep.
  /// @param source Descriptor of the source vertex.
  /// @param target Descriptor of the target vertex.
  //////////////////////////////////////////////////////////////////////
  void delete_edge(descriptor_t const& source, descriptor_t const& target)
  {
    m_gvw->delete_edge(source, target);
  }

  size_t level() const
  { return m_curr_level; }

  void increment_iteration()
  { ++m_curr_level; }

  void define_type(typer& t)
  {
    t.member(m_curr_level);
    t.member(m_wf);
    t.member(m_gvw);
  }
};

} // namespace level_sync_paradigm_impl


//////////////////////////////////////////////////////////////////////
/// @brief The Parallel Level-Synchronous (level_sync) Paradigm.
///
/// Implements the level-sync paradigm, which iteratively executes
/// BSP-Supersteps (SS). Each SS applies the user provided work function
/// over the vertices of the input graph.
/// Any communication generated within a SS is guaranteed to have finished
/// before the SS finishes.
/// The user provides a vertex-operator to express the computation to be
/// performed on each vertex, and a neighbor-operator that will be applied
/// to each neighbor that is visited.
/// The vertex-operator is passed in a vertex and a visit object. To visit a
/// neighboring vertex, the vertex-operator must call
/// visit(neighbor, neighbor-operator()).
/// The vertex-operator must return true if the vertex was active
/// (i.e. its value was updated), or false otherwise.
/// The neighbor-operator is passed in the target vertex. Neighbor-operators
/// may carry state, but must be immutable. They should return true if the
/// visit was successful (i.e. the target vertex will be activated after this
/// visit), or false otherwise.
/// Users may also provide additional functions to be executed before and after
/// each SS.
/// @tparam WF The type of the user provided vertex-operator expressing
/// computation to be performed over each vertex.
/// @tparam UF The type of the user provided neighbor-operator expressing
/// computation to be performed over neighboring vertices.
/// @param uwf Functor that implements the operation to be performed
/// on each vertex
/// @param post_execute Optional functor that will be executed on the
/// @ref graph_view at the end of each SS. This will be invoked with the
/// input @ref graph_view and the current SS ID (the ID of the SS that
/// just finished).
/// @param g The @ref graph_view over the input graph.
/// @return The number of iterations performed by the paradigm.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<typename WF, typename UF, typename PostExecute, typename GView>
size_t level_sync_paradigm(WF const& uwf, UF const&, PostExecute post_execute,
                           GView& g)
{
  // Sort all edges for each vertex according to their target home-locations.
  // This may help increase performance through aggregation of communication.
  g.sort_edges();

  typedef level_sync_paradigm_impl::level_sync_map_wf<GView, WF> wf_t;
  wf_t wf(uwf, 1);

  size_t iterations = 0;
  while (map_reduce(wf, plus<bool>(), g, make_repeat_view(g))) {
    post_execute(g, iterations);
    //now change the propagation trigger
    wf.increment_iteration();
    ++iterations;
  }
  return iterations;
}


//////////////////////////////////////////////////////////////////////
/// @brief The Parallel Level-Synchronous (Level_Sync) Paradigm.
///
/// Overloaded variant of @ref level_sync_paradigm with an empty post-execute.
//////////////////////////////////////////////////////////////////////
template<typename WF, typename UF, typename GView>
size_t level_sync_paradigm(WF const& uwf, UF const&, GView& g)
{
  return level_sync_paradigm(uwf, UF(),
                             kla_detail::empty_prepost_execute(), g);
}

} // namespace stapl

#endif
