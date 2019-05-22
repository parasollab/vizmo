/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_CONTAINERS_GRAPH_ALGORITHMS_CREATE_LEVEL_HUBS_HPP
#define STAPL_CONTAINERS_GRAPH_ALGORITHMS_CREATE_LEVEL_HUBS_HPP

#include <stapl/containers/graph/digraph.hpp>
#include <stapl/containers/graph/algorithms/aggregator.hpp>
#include <stapl/containers/graph/views/graph_view.hpp>
#include <stapl/containers/graph/views/lazy_graph_view.hpp>
#include <stapl/domains/interval.hpp>
#include <boost/serialization/unordered_set.hpp>

namespace stapl {

namespace create_level_hubs_detail {

//////////////////////////////////////////////////////////////////////
/// @brief Vertex stub class for the @ref create_level_hubs algorithm.
///
/// Used as vertex property for the hubs supergraph and behaves like
/// a hub vertex. This provides functionality needed to support a
/// hierarchy of hubs.
///
/// @tparam VD The type of the vertex descriptor.
/// @tparam VP The type of the vertex property.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<typename VD, typename VP>
struct vertex_stub
{
  typedef VD  vertex_descriptor;
  typedef VP  property_type;
  typedef VP& property_reference;

  property_type m_property;
  bool          m_active;

  vertex_stub(void)
    : m_property(), m_active(false)
  { }

  property_reference property(void)
  { return m_property; }

  void reset(void)
  {
    m_property = property_type();
    m_active = false;
  }

  bool is_active(void) const
  { return m_active; }

  void set_active(bool b)
  { m_active = b; }

  void define_type(typer& t)
  { t.member(m_property); }
};


//////////////////////////////////////////////////////////////////////
/// @brief Vertex property class for the @ref create_level_hubs algorithm.
/// @tparam GraphView The type of the lower-level input @ref graph_view.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<typename GraphView>
struct super_vertex_hub_property
{
  typedef typename GraphView::vertex_descriptor vd_type;
  typedef typename GraphView::vertex_property   vp_type;

  typedef vertex_stub<vd_type, vp_type> vertex_type;

  /// Maps a source (child) vertex to its hubs.
  typedef boost::unordered_map<vd_type,
                               std::vector<vertex_type*> >  child_cont_type;

  /// @brief Maps a hub (child) descriptor to its reducer-vertex and a bool
  /// indicating if it was updated.
  typedef boost::unordered_map<vd_type, vertex_type>  hub_reducer_cont_type;

  template<typename VPR>
  struct property_set_f
  {
    VPR m_vpr;
    vp_type m_property;

    property_set_f(VPR const& vpr, vp_type const& p)
      : m_vpr(vpr), m_property(p)
    { }

    template<typename V>
    void operator()(V& v) const
    { m_vpr(v.property(), m_property); }

    void define_type(typer& t)
    {
      t.member(m_vpr);
      t.member(m_property);
    }
  };

  /// @brief Stores the vertex descriptors of children vertices,
  /// as well as their mapping to hub vertices.
  child_cont_type  m_children;

  /// @brief Stores the reduction of properties of hub vertices.
  /// These must be flushed-out to the actual hubs.
  hub_reducer_cont_type  m_hubs;

  /// Stores the user vertex property.
  GraphView*       m_lower_level;

  super_vertex_hub_property(void)
    : m_lower_level(nullptr)
  { }

  super_vertex_hub_property(GraphView& g)
    : m_lower_level(&g)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Applies the provided functor to reduced properties of hubs
  /// connected to the given source vertex.
  /// @param source The descriptor of the source vertex (from lower-level
  /// graph).
  /// @param f The functor to be applied to the hub targets.
  //////////////////////////////////////////////////////////////////////
  template<typename Func>
  void apply_updates_to_hubs(vd_type const& source, Func&& f)
  {
    auto it = m_children.find(source);
    if (it != m_children.end()) {
      for (auto const& hub_target : it->second) {
        f.template operator()<vertex_type&>(*hub_target);
        hub_target->set_active(true);
      }
    }
  }

  template<typename VPR>
  void flush_updates_to_hubs(VPR const& vpr)
  {
    for (auto& h : m_hubs) {
      if (h.second.is_active()) {
        m_lower_level->container().apply_set(h.first,
          property_set_f<VPR>(vpr, h.second.property()));
        h.second.reset();
      }
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Adds a child vertex to this supervertex.
  /// @param source The descriptor of the source vertex (from lower graph).
  /// @param hub The descriptor of the hub vertex (from lower graph).
  //////////////////////////////////////////////////////////////////////
  void add_child(vd_type const& source, vd_type const& hub)
  { m_children[source].push_back(&m_hubs[hub]); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Adds the provided hub vertices to this supervertex.
  /// @param hubs Container specifying the descriptors of hub vertices
  /// (from lower graph).
  //////////////////////////////////////////////////////////////////////
  template<typename HubsIDCont>
  void hubs(HubsIDCont const& hubs_cont)
  {
    for (auto const& h : hubs_cont)
      m_hubs.emplace(h, vertex_type());
  }

  void define_type(typer& t)
  {
    t.member(m_children);
    t.member(m_lower_level);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Functor to initialize hub-vertices.
/// @tparam HubCont The type of the container of hub-descriptors.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<typename HubCont>
struct init_sv_hubs
{
  HubCont m_hubs;

  init_sv_hubs(HubCont const& hubs)
    : m_hubs(hubs)
  { }

  typedef void result_type;

  template<typename Vertex>
  result_type operator()(Vertex v) const
  { v.property().hubs(m_hubs); }

  void define_type(typer& t)
  { t.member(m_hubs); }
};


//////////////////////////////////////////////////////////////////////
/// @brief Functor to find hub-vertices.
///
/// Selects all vertices with out-degree greater than or equal to a
/// given threshold as hub vertices.
/// @tparam VD The type of the vertex descriptors.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<typename VD>
struct find_hubs_wf
{
  size_t m_k;

  find_hubs_wf(size_t k)
    : m_k(k)
  { }

  typedef boost::unordered_set<VD> result_type;

  template<typename Vertex>
  result_type operator()(Vertex v) const
  {
    result_type r;
    if (v.size() >= m_k)
      r.insert(v.descriptor());
    return r;
  }

  void define_type(typer& t)
  { t.member(m_k); }
};


//////////////////////////////////////////////////////////////////////
/// @brief Reduce functor to find hub-vertices.
///
/// Selects all vertices with out-degree greater than a given threshold
/// as hub vertices.
/// @tparam VD The type of the vertex descriptors.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<typename VD>
struct reduce_hubs_wf
{
  typedef boost::unordered_set<VD> result_type;

  template<typename T>
  result_type operator()(T const& t1, T const& t2) const
  {
    result_type r(t1);
    result_type temp(t2);
    r.insert(temp.begin(), temp.end());
    return r;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work-function to add a child source vertex and its target to
/// a supervertex.
/// @tparam VD Type of the vertex-descriptor.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
template <typename VD>
struct add_child_to_sv_wf
{
  typedef boost::unordered_set<VD> hubs_id_t;
  VD m_child_source;
  hubs_id_t m_hub_targets;

  add_child_to_sv_wf(VD const& child_source, hubs_id_t& hub_targets)
    : m_child_source(child_source), m_hub_targets(hub_targets)
  { }

  typedef void result_type;

  template<typename SuperVertexProperty>
  result_type operator() (SuperVertexProperty& svp) const
  {
    for (auto const& h : m_hub_targets)
      svp.add_child(m_child_source, h);
  }

  void define_type(typer& t)
  {
    t.member(m_child_source);
    t.member(m_hub_targets);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work-function enabling children inform their supervertex
/// about themselves (new child) and their adjacent edges and properties.
/// @tparam G The type of the graph container of the next level (same
/// type for all levels).
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
template<typename G>
struct children_inform_supervertex_wf
{
  typedef typename G::vertex_descriptor                  vertex_desc_t;
  typedef typename G::edge_descriptor                    edge_desc_t;
  typedef boost::unordered_set<vertex_desc_t>              hubs_id_t;

  hubs_id_t     m_hubs;
  G*            m_g;
  location_type m_location_id;
  bool          m_delete_edges;

  struct eq_target
  {
    hubs_id_t& m_vds;
    hubs_id_t& m_hub_targets;

    eq_target(hubs_id_t& vds, hubs_id_t& hub_targets)
      : m_vds(vds), m_hub_targets(hub_targets)
    { }

    template <class Edge>
    bool operator()(Edge const& e)
    {
      if (m_vds.count(e.target()) != 0) {
        m_hub_targets.insert(e.target());
        return true;
      }
      return false;
    }
  };

  struct comp_target
  {
    template <class Edge1, class Edge2>
    bool operator()(Edge1 const& e1, Edge2 const& e2) const
    { return e1.target() < e2.target(); }
  };

  struct same_target
  {
    template <class Edge1, class Edge2>
    bool operator()(Edge1 const& e1, Edge2 const& e2) const
    { return e1.target() == e2.target(); }
  };


  typedef void result_type;

  children_inform_supervertex_wf(hubs_id_t const& hubs,
                                 G* g, bool delete_edges)
    : m_hubs(hubs), m_g(g), m_location_id(g->get_location_id()),
      m_delete_edges(delete_edges)
  { }

  template<typename Vertex>
  result_type operator()(Vertex v)
  {
    const vertex_desc_t vd = v.descriptor();

    // Add/inform my supervertex's edge(s) of its children edges.
    // Remove all edges to hubs.
    auto begin = v.begin();
    std::sort(begin, v.end(), comp_target());
    auto new_end = std::unique(begin, v.end(), same_target());

    hubs_id_t target_hubs;

    new_end = remove_if(begin, new_end, eq_target(m_hubs, target_hubs));

    m_g->vp_apply_async(m_location_id,
                        add_child_to_sv_wf<vertex_desc_t>(vd, target_hubs));

    if (m_delete_edges)
      v.erase(new_end-v.begin());
  }

  void define_type(typer& t)
  {
    t.member(m_hubs);
    t.member(m_g);
    t.member(m_delete_edges);
  }
};


struct second_select
{
  template<typename H>
  bool operator()(H const& h1, H const& h2) const
  { return h1.second < h2.second; }
};

} // namespace create_level_hubs_detail;


STAPL_PROXY_HEADER_TEMPLATE(
  create_level_hubs_detail::super_vertex_hub_property, GraphView)
{
  STAPL_PROXY_DEFINES(
    create_level_hubs_detail::super_vertex_hub_property<GraphView>)
  STAPL_PROXY_REFLECT_TYPE(vd_type)
  STAPL_PROXY_REFLECT_TYPE(child_cont_type)
  STAPL_PROXY_REFLECT_TYPE(hub_reducer_cont_type)
  STAPL_PROXY_METHOD(add_child, vd_type, vd_type)

  template<typename Func>
  void apply_updates_to_hubs(vd_type source, Func const& f)
  {
    Accessor::invoke(&target_t::template apply_updates_to_hubs<Func>,
                     source, f);
  }

  template<typename VPR>
  void flush_updates_to_hubs(VPR const& vpr)
  {
    Accessor::invoke(&target_t::template flush_updates_to_hubs<VPR>,
                     vpr);
  }

  template<typename HubsIDCont>
  void hubs(HubsIDCont const& hubs_cont)
  {
    Accessor::invoke(&target_t::template hubs<HubsIDCont>, hubs_cont);
  }

}; //struct proxy



//////////////////////////////////////////////////////////////////////
/// @brief Algorithm to create a level of hierarchy based on hubs,
/// for use in @ref h_hubs_paradigm().
///
/// Creates a level of hierarchy based on the input @ref graph_view
/// and high-degree vertices (hubs) in the input graph, i.e., all hub
/// vertices will have hub-vertex representative supervertices on each
/// location.
/// Supervertices and superedges in the resulting output graph store
/// descriptors of their children in the input.
///
/// For use with the @ref h_hubs_paradigm(), this must be called before
/// calling @ref create_level_machine() on the input graph.
///
/// The algorithm first finds all vertices qualifying as hubs (i.e. have
/// out-degree greater than the given threshold (@p k). A new level of
/// hierarchy is created with one supervertex per location, and each such
/// supervertex contains representatives of all hub vertices. These
/// representatives can only be written to, and their values can be
/// flushed to the actual hub-vertices in the input graph.
///
/// This incurs O(h) storage per location, where h is the number of hubs,
/// as defined by the hub cutoff @p k.
///
/// @param gvw The input @ref graph_view over which the new level is to be
/// created.
/// @param k The cutoff degree for considering a vertex as a hub. All vertices
/// with out-degree greater than or equal to k are considered hub vertices.
/// @param delete_edges Indicates if the non-local edges for the input
/// graph will be deleted. This is true by default, as the @ref h_paradigm
/// requires the edges to be deleted.
/// @return A graph_view over the output supergraph.
/// @note Destructive, the input graph view will be mutated to delete some
/// or all edges, but the information of the graph is maintained between
/// the input and the hierarchy.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<typename GraphView>
graph_view<digraph<
  create_level_hubs_detail::super_vertex_hub_property<GraphView> > >
create_level_hubs(GraphView& gvw, size_t k, bool delete_edges = true)
{
  using namespace create_level_hubs_detail;
  typedef typename GraphView::vertex_descriptor         vertex_desc_t;
  typedef super_vertex_hub_property<GraphView>          sv_property_t;
  typedef digraph<sv_property_t>                        graph_cont_t;
  typedef graph_view<graph_cont_t>                      graph_view_t;

  typedef boost::unordered_set<vertex_desc_t> hubs_cont_t;

  hubs_cont_t hubs = map_reduce(find_hubs_wf<vertex_desc_t>(k),
                                reduce_hubs_wf<vertex_desc_t>(),
                                gvw);

  graph_cont_t* g = new graph_cont_t(gvw.get_num_locations(),
                                     sv_property_t(gvw));
  graph_view_t next_level_vw(g);

  map_func(init_sv_hubs<hubs_cont_t>(hubs), next_level_vw);

  typedef children_inform_supervertex_wf<graph_cont_t> wf_t;
  // propagate children info. (ids and edges) to supervertex in
  // next level graph.
  map_func(wf_t(hubs, g, delete_edges), gvw);

  gvw.sort_edges();

  return next_level_vw;
}

}  // namespace stapl

#endif
