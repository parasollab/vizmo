/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_CONTAINERS_GRAPH_BASE_CONTAINER_CSR_STORAGE_HPP
#define STAPL_CONTAINERS_GRAPH_BASE_CONTAINER_CSR_STORAGE_HPP

#include <stapl/containers/sequential/graph/graph_util.h>
#include <stapl/containers/sequential/graph/graph_iterator.h>
#include <stapl/containers/sequential/graph/adj_list_vertex_edge.h>
#include <stapl/containers/sequential/graph/vdg_intVD.h>
#include <stapl/runtime/serialization.hpp>

namespace stapl {

//to be used in friend declarations
template<class Traits> class csr_model;

//////////////////////////////////////////////////////////////////////
/// @brief Vertex for a Compressed Sparse-Row (CSR) graph.
/// @ingroup pgraphBaseCont
/// @tparam VD Vertex descriptor type for the graph.
/// @tparam Property Property type of the vertex.
/// @tparam AdjList Type of the edgelist for storing edges.
//////////////////////////////////////////////////////////////////////
template<class VD, class Property, class AdjList>
class vertex_csr_impl
{
  template<class Traits> friend class csr_model;
public:
  typedef VD                                     vertex_descriptor;
  typedef AdjList                                edgelist_type;
  typedef AdjList                                adj_edges_type;
  typedef typename AdjList::edge_descriptor      edge_descriptor;
  typedef typename AdjList::iterator             edgelist_it;
  typedef typename AdjList::const_iterator       const_edgelist_it;
  typedef edgelist_it                            adj_edge_iterator;
  typedef const_edgelist_it                      const_adj_edge_iterator;
  typedef Property                               property_type;
  typedef Property&                              property_reference;
  typedef Property const&                        const_property_reference;

protected:
  /// Descriptor of the vertex.
  VD             m_vd;
  /// Iterator to the beginning of this vertex's edgelist.
  edgelist_it    m_begin;
  /// Iterator to the end of this vertex's edgelist.
  edgelist_it    m_end;
  /// Property of this vertex.
  property_type  m_property;

public:
  vertex_csr_impl()
  { }

  vertex_csr_impl(vertex_csr_impl const& other)
    : m_vd(other.m_vd), m_begin(other.m_begin), m_end(other.m_end),
      m_property(other.m_property)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a vertex for the graph with the given descriptor
  /// and property.
  /// @param vd Provides the descriptor of this vertex.
  /// @param p Provides the property of this vertex.
  //////////////////////////////////////////////////////////////////////
  vertex_csr_impl(vertex_descriptor vd, Property const& p)
    : m_vd(vd), m_property(p)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the descriptor of the vertex.
  //////////////////////////////////////////////////////////////////////
  vertex_descriptor const& descriptor() const
  { return m_vd;  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns a copy of the outgoing edges.
  //////////////////////////////////////////////////////////////////////
  edgelist_type edges()
  { return edgelist_type(this->m_begin, this->m_end);  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Sets the beginning and the end of the edgelist for this vertex.
  /// @param begin An iterator pointing to the start of the edgelist.
  /// @param end An iterator pointing to the end of the edgelist.
  //////////////////////////////////////////////////////////////////////
  void set_edges(edgelist_it begin, edgelist_it end)
  {
    m_begin = begin;
    m_end = end;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the beginning of the edgelist of this vertex.
  //////////////////////////////////////////////////////////////////////
  edgelist_it begin()
  { return this->m_begin; }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the end of the edgelist of this vertex.
  //////////////////////////////////////////////////////////////////////
  edgelist_it end()
  { return this->m_end; }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the beginning of the edgelist of this vertex.
  //////////////////////////////////////////////////////////////////////
  const_edgelist_it begin() const
  { return this->m_begin; }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the end of the edgelist of this vertex.
  //////////////////////////////////////////////////////////////////////
  const_edgelist_it end() const
  { return this->m_end; }

  #if 0
  //////////////////////////////////////////////////////////////////////
  /// @brief Returns an iterator to the edge with the specified target.
  /// @param vd The descriptor of the target vertex.
  /// @todo This class does not have an m_edgelist, needs fixing
  //////////////////////////////////////////////////////////////////////
  //edgelist_it find_edge(vertex_descriptor const& vd)
  //{ return this->m_edgelist.find(vd); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns an iterator to the specified edge.
  /// @param ed The descriptor of the edge.
  /// @todo This class does not have an m_edgelist, needs fixing
  //////////////////////////////////////////////////////////////////////
  edgelist_it find_edge(edge_descriptor const& ed)
  { return this->m_edgelist.find(ed); }
  #endif

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the number of outgoing edges of this vertex.
  //////////////////////////////////////////////////////////////////////
  size_t size() const
  { return std::distance(m_begin, m_end); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the property of this vertex.
  //////////////////////////////////////////////////////////////////////
  property_reference property()
  { return m_property; }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the property of this vertex.
  //////////////////////////////////////////////////////////////////////
  const_property_reference property() const
  { return m_property; }

  void define_type(typer& t)
  {
    t.member(m_vd);
    t.member(m_begin);
    t.member(m_end);
    t.member(m_property);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization of @ref proxy for @ref vertex_csr_impl.
/// @ingroup pgraphdistObj
/// @tparam VD Vertex descriptor type for the graph.
/// @tparam Property Property type of the vertex.
/// @tparam AdjList Type of the edgelist for storing edges.
/// @tparam Accessor Type of the accessor for the proxy.
//////////////////////////////////////////////////////////////////////
template<class VD, class Property, class AdjList, typename Accessor>
class proxy<vertex_csr_impl<VD, Property, AdjList>, Accessor>
  : public Accessor
{
private:
  friend class proxy_core_access;
  typedef vertex_csr_impl<VD, Property, AdjList> target_t;
  typedef typename target_t::adj_edge_iterator        adj_edge_iter_t;
  typedef typename target_t::const_adj_edge_iterator  const_adj_edge_iter_t;

public:
  typedef typename target_t::property_type            property_type;
  typedef typename Accessor::property_reference       property_reference;
  typedef typename Accessor::const_property_reference const_property_reference;
  typedef typename target_t::vertex_descriptor        vertex_descriptor;
  typedef typename target_t::edge_descriptor          edge_descriptor;
  typedef typename target_t::adj_edges_type           adj_edges_type;

  typedef member_iterator<adj_edge_iter_t, Accessor>  adj_edge_iterator;
  typedef member_iterator<const_adj_edge_iter_t,
                          Accessor>                   const_adj_edge_iterator;

  explicit proxy(Accessor const& acc)
    : Accessor(acc)
  { }

  proxy const& operator=(proxy const& rhs)
  {
    Accessor::write(rhs);
    return *this;
  }

  proxy const& operator=(target_t const& rhs)
  {
    Accessor::write(rhs);
    return *this;
  }

  //////////////////////////////////////////////////////////////////////
  /// @todo Is this more efficient than accessor_t::read()?
  //////////////////////////////////////////////////////////////////////
  // operator target_t() const { return Accessor::ref(); }
  operator target_t() const
  { return Accessor::read(); }

  vertex_descriptor descriptor() const
  { return Accessor::index(); }

  property_reference property()
  { return Accessor::property(); }

  const_property_reference property() const
  { return Accessor::property(); }

  adj_edge_iterator begin()
  { return adj_edge_iterator(Accessor::invoke(&target_t::begin), *this); }

  adj_edge_iterator end()
  { return adj_edge_iterator(Accessor::invoke(&target_t::end), *this); }

  adj_edge_iterator find_edge(vertex_descriptor const& vd)
  { return adj_edge_iterator(Accessor::invoke(&target_t::find, vd), *this); }

  adj_edge_iterator find_edge(edge_descriptor const& ed)
  { return adj_edge_iterator(Accessor::invoke(&target_t::find, ed), *this); }

  const_adj_edge_iterator begin() const
  {
    return const_adj_edge_iterator(Accessor::const_invoke(&target_t::begin),
                                   *this);
  }

  const_adj_edge_iterator end() const
  {
    return const_adj_edge_iterator(Accessor::const_invoke(&target_t::end),
                                   *this);
  }

  const_adj_edge_iterator find_edge(vertex_descriptor const& vd) const
  {
    return const_adj_edge_iterator(Accessor::const_invoke(&target_t::find, vd),
                                   *this);
  }

  const_adj_edge_iterator find_edge(edge_descriptor const& ed) const
  {
    return const_adj_edge_iterator(Accessor::const_invoke(&target_t::find, ed),
                                   *this);
  }

  size_t size(void) const
  { return Accessor::ref().size();}

  adj_edges_type edges() const
  { return Accessor::ref().edges();}
}; //struct proxy


//////////////////////////////////////////////////////////////////////
/// @brief Specialization of @ref proxy for @ref vertex_csr_impl,
/// specialized for @ref local_accessor_graph, which lets us use a
/// faster iterator over the edges than the @ref member_iterator
/// needed for the @ref container_accessor.
/// @ingroup pgraphdistObj
/// @tparam VD Vertex descriptor type for the graph.
/// @tparam Property Property type of the vertex.
/// @tparam AdjList Type of the edgelist for storing edges.
/// @tparam Accessor Type of the accessor for the proxy.
//////////////////////////////////////////////////////////////////////
template<class VD, class Property, class AdjList, typename Container>
class proxy<vertex_csr_impl<VD, Property, AdjList>,
            local_accessor_graph<Container> >
  : public local_accessor_graph<Container>
{
private:
  typedef local_accessor_graph<Container> accessor_t;
  friend class proxy_core_access;
  typedef vertex_csr_impl<VD, Property, AdjList> target_t;
public:
  typedef typename target_t::property_type            property_type;
  typedef typename accessor_t::property_reference     property_reference;
  typedef typename target_t::vertex_descriptor        vertex_descriptor;
  typedef typename target_t::edge_descriptor          edge_descriptor;
  typedef typename target_t::adj_edge_iterator        adj_edge_iterator;
  typedef typename target_t::const_adj_edge_iterator  const_adj_edge_iterator;
  typedef typename target_t::adj_edges_type           adj_edges_type;

  explicit proxy(accessor_t const& acc)
    : accessor_t(acc)
  { }

  proxy const& operator=(proxy const& rhs)
  {
    accessor_t::write(rhs);
    return *this;
  }

  proxy const& operator=(target_t const& rhs)
  {
    accessor_t::write(rhs);
    return *this;
  }

  //////////////////////////////////////////////////////////////////////
  /// @todo Is this more efficient than accessor_t::read()?
  //////////////////////////////////////////////////////////////////////
  // operator target_t() const { return accessor_t::ref(); }
  operator target_t() const
  { return accessor_t::read(); }

  vertex_descriptor descriptor() const
  { return accessor_t::index(); }

  property_reference property()
  { return accessor_t::property(); }

  adj_edge_iterator begin()
  { return accessor_t::ref().begin(); }

  adj_edge_iterator end()
  { return accessor_t::ref().end(); }

  const_adj_edge_iterator begin() const
  { return accessor_t::ref().begin(); }

  const_adj_edge_iterator end() const
  { return accessor_t::ref().end(); }

  const_adj_edge_iterator find_edge(vertex_descriptor const& vd) const
  { return accessor_t::ref().find(vd); }

  const_adj_edge_iterator find_edge(edge_descriptor const& ed) const
  { return accessor_t::ref().find(ed); }

  size_t size(void) const
  { return accessor_t::ref().size();}

  adj_edges_type edges() const
  { return accessor_t::ref().edges();}
}; //struct proxy


//////////////////////////////////////////////////////////////////////
/// @brief The edgelist for a CSR graph, implemented as a vector of edges.
/// @ingroup pgraphBaseCont
/// @tparam Edge The type of edge to be stored.
//////////////////////////////////////////////////////////////////////
template <class Edge>
class csr_edgelist_impl
{
  /// The actual list of edges is stored in an std::vector.
  std::vector<Edge> m_data;

 public:
  //infer property
  typedef Edge     edge_type;
  typedef typename Edge::vertex_descriptor           vertex_descriptor;
  typedef typename Edge::edge_descriptor_type        edge_descriptor;
  typedef typename std::vector<Edge>::iterator       iterator;
  typedef typename std::vector<Edge>::const_iterator const_iterator;
  typedef typename std::iterator_traits<iterator>::reference  reference;
  typedef typename std::iterator_traits<iterator>::value_type value_type;

  //////////////////////////////////////////////////////////////////////
  /// Create a csr edgelist with specified number of edges.
  //////////////////////////////////////////////////////////////////////
  csr_edgelist_impl(size_t n=0)
    : m_data(n)
  { }

  csr_edgelist_impl(csr_edgelist_impl const& other)
  { m_data = other.m_data; }

  csr_edgelist_impl& operator=(csr_edgelist_impl const& other)
  {
    if (this != &other)
      m_data = other.m_data;
    return *this;
  }

  csr_edgelist_impl(iterator begin, iterator end)
  : m_data(begin, end)
  { }

  iterator begin()
  { return m_data.begin(); }

  const_iterator begin() const
  { return m_data.begin(); }

  iterator end()
  { return m_data.end(); }

  const_iterator end() const
  { return m_data.end(); }

  size_t size() const
  { return m_data.size(); }

  void define_type(typer& t)
  { t.member(m_data); }

  //////////////////////////////////////////////////////////////////////
  /// Adds an edge to the front of edgelist.
  //////////////////////////////////////////////////////////////////////
  void add_dummy()
  { m_data.push_front(Edge()); }

  /// Clears the edgelist.
  void clear()
  { m_data.clear(); }

  size_t memory_size(void) const
  { return memory_used<std::vector<Edge> >::size(m_data); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Adds the specified edge to the edgelist.
  /// @param ed The edge to be inserted into the edgelist.
  //////////////////////////////////////////////////////////////////////
  void add(Edge const& ed)
  { m_data.push_back(ed); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Erases the edge pointed to by the specified iterator.
  /// @param it An iterator pointing to the edge to be deleted.
  //////////////////////////////////////////////////////////////////////
  void erase(iterator it)
  { m_data.erase(it); }
};


//////////////////////////////////////////////////////////////////////
/// @brief Helper class to compare two edges based on their source
/// vertex-descriptors.
/// @ingroup pgraphBaseCont
//////////////////////////////////////////////////////////////////////
struct edge_source_comp
{
  template <class Edge>
  bool operator()(Edge const& e1, Edge const& e2)
  {
    return e1.source() < e2.source();
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief An CSR using an std::vector for storing vertices.
/// Used inside the @ref csr_model to store vertices.
/// @ingroup pgraphBaseCont
/// @tparam Traits The traits class for specifying the types of descriptors,
/// storages, edges and vertices.
//////////////////////////////////////////////////////////////////////
template <class Traits>
class csr_vector_storage
{
 public:
  typedef typename Traits::vertex_descriptor        vertex_descriptor;
  typedef typename Traits::vertex_property          vertex_property;
  typedef typename Traits::edge_type                edge_type;
  typedef csr_edgelist_impl<edge_type>              edgelist_type;
  typedef vertex_csr_impl<vertex_descriptor,
                          vertex_property,
                          edgelist_type>            vertex_impl_type;

  /// Type of the vertex descriptor generator.
  typedef sequential::vdg_base_int<vertex_descriptor> vdg_type;

  /// The vertices are stored in an std::vector.
  typedef std::vector<vertex_impl_type>             vertex_set_type;

 protected:
  // Data members.
  /// The vertex descriptor generator.
  vdg_type         m_vdg;

  /// The container for storing vertices.
  vertex_set_type  m_storage;

  /// The container for storing edges.
  edgelist_type    m_edgelist;

  /// The descriptor of the starting vertex in this storage.
  size_t           my_local_start_vd;

  //////////////////////////////////////////////////////////////////////
  /// @brief The flag indicates if the CSR is frozen. Edges may only be added
  /// to the CSR as long as it is unfrozen. Once it has been frozen by calling
  /// the commit() method, no further addition of edges is allowed.
  /// The CSR graph starts off in an unfrozen state, and may only be committed
  /// once, and may not be uncommitted thereafter.
  //////////////////////////////////////////////////////////////////////
  bool             m_committed;

 public:
  typedef typename vertex_set_type::iterator       iterator;
  typedef typename vertex_set_type::const_iterator const_iterator;
  typedef typename edgelist_type::iterator         edge_iterator;
  typedef typename edgelist_type::const_iterator   const_edge_iterator;

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructs a storage with the given number of vertices starting
  /// from the specified descriptor, with the given default value.
  /// Vertices have contiguous descriptors.
  /// @param start_vd The starting descriptor of the vertices.
  /// @param num_vds The number of vertices to be initially stored.
  /// @param default_value The default value for the vertices.
  //////////////////////////////////////////////////////////////////////
  csr_vector_storage(size_t const& start_vd, size_t const& num_vds,
                     vertex_impl_type const& default_value)
    : m_vdg(start_vd), my_local_start_vd(start_vd), m_committed(false)
  {
    //init the vertex descriptors and adj lists
    m_storage.reserve(num_vds);
    for (size_t i=0; i<num_vds; ++i) {
      vertex_descriptor vd = m_vdg.next();
      m_storage.push_back(vertex_impl_type(vd, default_value.property()));
    }
  }

  csr_vector_storage(csr_vector_storage const& other)
    : m_vdg(other.m_vdg),
      m_storage(other.m_storage), m_edgelist(other.m_edgelist),
      my_local_start_vd(get_location_id()*m_storage.size()),
      m_committed(other.m_committed)
  { }

  //////////////////////////////////////////////////////////////////////
  /// Resize the internal storage to accommodate specified number of vertices.
  //////////////////////////////////////////////////////////////////////
  void resize(size_t nv)
  {
    this->m_storage.clear();
    vertex_property p = vertex_property(); // default constructed property;
    for (size_t i=0; i<nv; ++i)
    {
      vertex_descriptor vd = m_vdg.next();
      m_storage.push_back(vertex_impl_type(vd, p));
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns an iterator to the start of the vertices.
  //////////////////////////////////////////////////////////////////////
  iterator begin()
  { return this->m_storage.begin(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns an iterator to the end of the vertices.
  //////////////////////////////////////////////////////////////////////
  iterator end()
  { return this->m_storage.end(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns an iterator to the start of the vertices.
  //////////////////////////////////////////////////////////////////////
  const_iterator begin() const
  { return this->m_storage.begin(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns an iterator to the end of the vertices.
  //////////////////////////////////////////////////////////////////////
  const_iterator end() const
  { return this->m_storage.end(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns an edge_iterator to the start of the edges.
  //////////////////////////////////////////////////////////////////////
  edge_iterator edges_begin()
  { return this->m_edgelist.begin(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns an edge_iterator to the end of the edges.
  //////////////////////////////////////////////////////////////////////
  edge_iterator edges_end()
  { return this->m_edgelist.end(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns an edge_iterator to the start of the edges.
  //////////////////////////////////////////////////////////////////////
  const_edge_iterator edges_begin() const
  { return this->m_edgelist.begin(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns an edge_iterator to the end of the edges.
  //////////////////////////////////////////////////////////////////////
  const_edge_iterator edges_end() const
  { return this->m_edgelist.end(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the number of vertices in the storage; for use
  /// in g.num_vertices().
  //////////////////////////////////////////////////////////////////////
  size_t size() const
  { return this->m_storage.size(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Adds an edge with given descriptor and default edge property.
  /// @param ed Descriptor of the desired edge.
  //////////////////////////////////////////////////////////////////////
  void add_edge(edge_type const& ed)
  { this->m_edgelist.add(ed); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Erases all vertices and edges in the graph.
  //////////////////////////////////////////////////////////////////////
  void clear(void)
  {
    this->m_storage.clear();
    this->m_edgelist.clear();
    this->m_vdg.reset();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Finds the vertex with the specified descriptor, and returns an
  /// iterator pointing to it. If not found, the end of the graph is
  /// returned.
  /// @param vd Descriptor of the vertex.
  /// @return An iterator to the specified vertex, if found, or an
  /// iterator to the end of the graph otherwise.
  //////////////////////////////////////////////////////////////////////
  iterator find_vertex(vertex_descriptor const& vd)
  {
    return m_storage.begin() + (vd - my_local_start_vd);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Not used by CSR graph, provided for compatibility.
  //////////////////////////////////////////////////////////////////////
  void update_descriptor(vertex_descriptor const&, iterator const&)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Updates the vertex descriptor generator with the next free
  ///        descriptor
  //////////////////////////////////////////////////////////////////////
  void update_next_descriptor(vertex_descriptor const& vd)
  {
    this->m_vdg.update(vd);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the current max descriptor.
  //////////////////////////////////////////////////////////////////////
  vertex_descriptor get_max_descriptor()
  { return m_vdg.curr_vd(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Freezes the CSR graph so that no further addition of edges is
  /// possible.
  /// @note This method must be called before using the CSR graph, and only
  /// after all edges have been added to it.
  /// Edges may only be added to the CSR as long as it is unfrozen. Once it
  /// has been frozen by calling commit(), no further addition of edges is
  /// allowed. The CSR may only be committed once, and may not be uncommitted
  /// thereafter.
  ///
  /// The CSR graph starts off in an unfrozen state. Freezing it internally
  /// sorts and copies the edges to the CSR format and updates the vertices
  /// accordingly to point to their adjacent sections in the edgelist.
  //////////////////////////////////////////////////////////////////////
  void commit()
  {
    // only do this if graph is not already committed.
    if (!m_committed) {
      // need to sort the edges first to make sure correct ordering for CSR.
      // edges should be sorted by source:
      std::sort(this->edges_begin(), this->edges_end(), edge_source_comp());

      edge_iterator ei = this->edges_begin();
      edge_iterator ei_tmp = ei;
      edge_iterator ei_end = this->edges_end();
      iterator vi = this->begin();
      iterator vi_end = this->end();

      for (; vi != vi_end; ++vi) {
        vi->set_edges(ei_end, ei_end);  // initialize to empty
        ei_tmp = ei;
        for (; ei != ei_end; ++ei) {
          if (! (ei->source() == vi->descriptor()) )
            break;
        }
        vi->set_edges(ei_tmp, ei);  // set to the correct edgelist
      }
      m_committed = true;  // can only commit once
    }
  }

  void define_type(typer& t)
  {
    t.member(m_storage);
    t.member(m_edgelist);
    t.member(m_vdg);
    t.member(m_committed);
  }
}; // csr_vector_storage.




//////////////////////////////////////////////////////////////////////
/// @brief This model describes operations on a graph that
/// is modeled as a Compressed Sparse-Row (CSR) graph. CSRs store a list
/// of vertices and a separate list of edges. Each vertex points to its adjacent
/// edges in the list of edges.
/// @ingroup pgraphBaseCont
/// @tparam Traits The traits class for specifying the types of descriptors,
/// storages, edges and vertices, etc.
///
/// Methods like find_vertex, find_edge are optimized for this particular
/// storage.
//////////////////////////////////////////////////////////////////////
template <class Traits>
class csr_model
{
 public:
  typedef typename Traits::vertex_descriptor  vertex_descriptor;
  typedef typename Traits::simple_vertex_descriptor  simple_vertex_descriptor;
  typedef typename Traits::edge_descriptor    edge_descriptor;
  typedef typename Traits::vertex_property    vertex_property;
  typedef typename Traits::edge_property      edge_property;
  /// The type of the edge.
  typedef typename Traits::edge_type          edge_type;
  /// The type of storage for vertices.
  typedef typename Traits::storage_type     vertex_set_type;
  /// The type of storage for edges on a vertex.
  typedef typename Traits::edgelist_type    edgelist_type;
  /// The type of the vertex.
  typedef typename Traits::vertex_impl_type vertex_impl_type;
  typedef vertex_impl_type                  value_type;

 protected:
  /// The container for storing the vertices.
  vertex_set_type m_vertices;

  /// The number of edges in this CSR.
  size_t m_ne;

  /// The number to keep track of unique edge-descriptors generated.
  size_t m_eid;

 public:

  typedef typename vertex_set_type::iterator           iterator;
  typedef typename vertex_set_type::const_iterator     const_iterator;
  typedef iterator                                     vertex_iterator;
  typedef const_iterator                               const_vertex_iterator;

  /// Type of the iterator over adjacent edges of a vertex.
  typedef typename edgelist_type::iterator             adj_edge_iterator;
  typedef typename edgelist_type::const_iterator       const_adj_edge_iterator;

  /// Type of the edge iterator over all edges stored in the adjacency-list.
  typedef sequential::edge_iterator_adaptor<vertex_iterator>    edge_iterator;
  typedef sequential::edge_iterator_adaptor<const_vertex_iterator>
                                                          const_edge_iterator;

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructs a CSR with the given number of vertices
  /// starting from the specified descriptor, with the given default value.
  /// Vertices have contiguous descriptors.
  /// @param start_vd The starting descriptor of the vertices.
  /// @param num_vds The number of vertices to be initially stored.
  /// @param default_value The default value for the vertices.
  //////////////////////////////////////////////////////////////////////
  csr_model(size_t const& start_vd, size_t const& num_vds,
            value_type const& default_value)
    : m_vertices(start_vd, num_vds, default_value), m_ne(0), m_eid(0)
  { }

  csr_model(csr_model const& other)
    : m_vertices(other.m_vertices), m_ne(other.m_ne), m_eid(other.m_eid)
  { }

  vertex_iterator begin()
  { return this->m_vertices.begin(); }

  vertex_iterator end()
  { return this->m_vertices.end(); }

  const_vertex_iterator begin() const
  { return this->m_vertices.begin(); }

  const_vertex_iterator end() const
  { return this->m_vertices.end(); }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the total number of vertices.
  //////////////////////////////////////////////////////////////////////
  size_t num_vertices() const
  {
    return this->m_vertices.size();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the total number of vertices.
  //////////////////////////////////////////////////////////////////////
  size_t size() const
  {
    return this->m_vertices.size();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the current max descriptor.
  //////////////////////////////////////////////////////////////////////
  size_t get_max_descriptor()
  {
    return this->m_vertices.get_max_descriptor();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the total number of edges.
  //////////////////////////////////////////////////////////////////////
  size_t num_edges() const
  {
    return m_ne;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Adds an edge with given descriptor and default edge property.
  /// @param ed Descriptor of the desired edge.
  /// @return edge_descriptor of the added edge.
  /// @ref edge_descriptor.id() is set to numeric_limits<size_t>::max() if edge
  /// was not added.
  //////////////////////////////////////////////////////////////////////
  edge_descriptor add_edge(edge_descriptor const& ed)
  {
    return add_edge(ed, edge_property());
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Adds an edge with given descriptor and edge property.
  /// @param ed Descriptor of the desired edge.
  /// @param ep Property of the edge.
  /// @return edge_descriptor of the added edge.
  /// @ref edge_descriptor.id() is set to numeric_limits<size_t>::max() if edge
  /// was not added.
  //////////////////////////////////////////////////////////////////////
  edge_descriptor add_edge(edge_descriptor const& ed, edge_property const& p)
  {
    // don't use reference for the two lines below.
    // this is done for versioning, to prevent adding edges
    // between invalid iterators from another graph.
    vertex_iterator vi = this->find_vertex(ed.source());
    if (vi != this->end()) {
      edge_descriptor ned;
      if (ed.id()!=INVALID_VALUE) {
        //we use the same edge id as the sibling edge
        this->m_vertices.add_edge(edge_type(ed, p));
        ned = ed;
      } else {
        //we allocate a new edge id by incrementing a local counter
        size_t id = m_eid++;
        ned = edge_descriptor(ed.source(), ed.target(), id);
        //edgelist is a method of the vertex
        this->m_vertices.add_edge(edge_type(ned, p));
      }
      ++m_ne;
      return ned;
    } else {
      return edge_descriptor(vertex_descriptor(INVALID_VALUE), ed.target(),
                             INVALID_VALUE);
    }
   }

  //////////////////////////////////////////////////////////////////////
  /// @brief Checks and adds an edge with given descriptor and property.
  /// The method checks if the edge exists, before adding it. If it doesn't,
  /// the edge is added, else it is discarded.
  /// @param ed Descriptor of the desired edge.
  /// @param ep Property of the edge.
  /// @return edge_descriptor of the added edge.
  /// @ref edge_descriptor.id() is set to numeric_limits<size_t>::max() if edge
  /// was not added.
  //////////////////////////////////////////////////////////////////////
  edge_descriptor check_add_edge(edge_descriptor const& ed,
                                 edge_property const& ep)
  {
    vertex_iterator vi;
    adj_edge_iterator ei;
    if (!find_edge(ed, vi, ei))
      return add_edge(ed, ep);
    else return edge_descriptor(INVALID_VALUE, INVALID_VALUE, INVALID_VALUE);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Checks and adds an edge with given descriptor and default property.
  /// The method checks if the edge exists, before adding it. If it doesn't,
  /// the edge is added, else it is discarded.
  /// @param ed Descriptor of the desired edge.
  /// @return edge_descriptor of the added edge.
  /// @ref edge_descriptor.id() is set to numeric_limits<size_t>::max() if edge
  /// was not added.
  //////////////////////////////////////////////////////////////////////
  edge_descriptor check_add_edge(edge_descriptor const& ed)
  {
    return check_add_edge(ed, edge_property());
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Erases all vertices and edges in the graph.
  //////////////////////////////////////////////////////////////////////
  void clear(void)
  {
    this->m_ne  = 0;
    this->m_eid = 0;
    this->m_vertices.clear();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Freezes the CSR graph so that no further addition of edges is
  /// possible.
  /// @note This method must be called before using the CSR graph, and only
  /// after all edges have been added to it.
  /// Edges may only be added to the CSR as long as it is unfrozen. Once it
  /// has been frozen by calling commit(), no further addition of edges is
  /// allowed. The CSR may only be committed once, and may not be uncommitted
  /// thereafter.
  ///
  /// The CSR graph starts off in an unfrozen state. Freezing it internally
  /// sorts and copies the edges to the CSR format and updates the vertices
  /// accordingly to point to their adjacent sections in the edgelist.
  //////////////////////////////////////////////////////////////////////
  void commit(void)
  {
    this->m_vertices.commit();
  }

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Finds the edge with the specified descriptor, and returns an
  /// iterator to its source vertex and an adj_edge_iterator pointing to the
  /// edge.
  /// @param ed Descriptor of the edge.
  /// @param vi vertex_iterator pointing to the source vertex of the edge,
  /// populated by the method.
  /// @param aei adj_edge_iterator pointing to the specified edge,
  /// populated by the method.
  /// @return Whether or not the edge was found.
  //////////////////////////////////////////////////////////////////////
  bool find_edge(edge_descriptor const& ed, vertex_iterator& vi,
                 adj_edge_iterator& ei)
  {
    //if the edge id is missing we look for target, source
    // and delete the first encountered
    if (ed.id() == (size_t)INVALID_VALUE)
      ei = graph_find(this->m_vertices.edges_begin(),
                      this->m_vertices.edges_end(),
                      eq_target<vertex_descriptor>(ed.target()));
    else  //if edge descriptor is complete, edgeid uniquely identifies an edge
      ei = graph_find(this->m_vertices.edges_begin(),
                      this->m_vertices.edges_end(),
                      eq_ed<edge_descriptor>(ed));

    if (ei != this->m_vertices.edges_end())
      return true;
    else return false;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Finds the edge with the specified descriptor, and returns an
  /// iterator to its source vertex and an adj_edge_iterator pointing to the
  /// edge.
  /// @param ed Descriptor of the edge.
  /// @param vi vertex_iterator pointing to the source vertex of the edge,
  /// populated by the method.
  /// @param aei adj_edge_iterator pointing to the specified edge,
  /// populated by the method.
  /// @return Whether or not the edge was found.
  //////////////////////////////////////////////////////////////////////
  bool find_edge(edge_descriptor const& ed, const_vertex_iterator& vi,
                 const_adj_edge_iterator& ei) const
  {
    vertex_iterator   i_vi;
    adj_edge_iterator i_ei;
    bool res = const_cast<csr_model*>(this)->find_edge(ed, i_vi, i_ei);
    vi = const_vertex_iterator(i_vi);
    ei = const_adj_edge_iterator(i_ei.base());
    return res;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Finds the vertex with the specified descriptor, and returns a
  /// vertex_iterator pointing to it. If not found, the end of the graph is
  /// returned.
  /// @param vd Descriptor of the vertex.
  /// @return A vertex_iterator to the specified vertex, if found, or a
  /// vertex_iterator to the end of the graph otherwise.
  //////////////////////////////////////////////////////////////////////
  const_vertex_iterator find_vertex(vertex_descriptor const& vd) const
  {
    return const_cast<csr_model*>(this)->find_vertex(vd);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Finds the vertex with the specified descriptor, and returns a
  /// vertex_iterator pointing to it. If not found, the end of the graph is
  /// returned.
  /// @param vd Descriptor of the vertex.
  /// @return A vertex_iterator to the specified vertex, if found, or a
  /// vertex_iterator to the end of the graph otherwise.
  //////////////////////////////////////////////////////////////////////
  vertex_iterator find_vertex(vertex_descriptor const& vd)
  {
    return this->m_vertices.find_vertex(vd);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Same as @ref find_vertex, provided for compatibility.
  //////////////////////////////////////////////////////////////////////
  vertex_iterator find(vertex_descriptor const& vd)
  {
    return this->m_vertices.find_vertex(vd);
  }

  void define_type(typer& t)
  {
    t.member(m_vertices);
    t.member(m_ne);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Updates the vertex descriptor generator with the next free
  ///        descriptor
  //////////////////////////////////////////////////////////////////////
  void update_next_descriptor(vertex_descriptor const& vd)
  {
    this->m_vertices.update_next_descriptor(vd);
  }

 private:

  //////////////////////////////////////////////////////////////////////
  /// @brief Adds two edges: one from source to target of the specified
  /// descriptor and the other from the target to source. Adds first to
  /// the vertex with the smaller id of source() and target().
  /// This will generate the id of the edge and this id will be the same
  /// in the second (sibling) edge added. This is important to match the
  /// edges when there are duplicates.
  /// Uses @ref add_internal_edge to add the sibling edge.
  /// @param g The graph to which the edge will be added.
  /// @param ed The descriptor of the edge to be added. Id of the descriptor
  /// must have been externally assigned.
  /// @param p The property of the edge being added.
  //////////////////////////////////////////////////////////////////////
  friend edge_descriptor add_edge_pair(csr_model& g, edge_descriptor const& ed,
                                       edge_property const& p)
  {
    edge_descriptor n_ed;
    bool reversed = false;
    if (ed.source() > ed.target()) {
      n_ed = reverse(ed);
      reversed = true;
    }
    else n_ed = ed;
    // this is done for versioning, to prevent adding edges
    // between invalid iterators from another graph.
    simple_vertex_descriptor n_ed_source = n_ed.source();
    simple_vertex_descriptor n_ed_target = n_ed.target();

    vertex_iterator vi1 = g.find_vertex(n_ed_source);
    vertex_iterator vi2 = g.find_vertex(n_ed_target);

    vertex_descriptor v1, v2;
    if (vi1 == g.end()) v1 = vertex_descriptor(INVALID_VALUE);
    else v1 = ed.source();

    if (vi2 == g.end()) v2 = vertex_descriptor(INVALID_VALUE);
    else v2 = ed.target();

    if (vi1 != g.end() && vi2 != g.end()) {
      size_t eid = g.m_eid++;
      edge_descriptor ied(n_ed_source, n_ed_target, eid);
      edge_type edge(ied, p);
      g.add_edge(edge);          // it has to be like this
      g.add_edge(reverse(edge) ); //because sibling edges share same property
      g.m_ne += 2;
      return ied;
    }
    //here error encountered
    if (!reversed)
      return edge_descriptor(v1, v2, INVALID_VALUE);
    else
      return edge_descriptor(v2, v1, INVALID_VALUE);
  }

}; // class csr_model

} // namespace stapl
#endif
