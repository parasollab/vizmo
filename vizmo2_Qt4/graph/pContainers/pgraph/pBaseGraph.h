/*!
	\file pBaseGraph.h
	\author Gabriel Tanase	
	\date Jan. 9, 03
	\ingroup stapl
	\brief Base for the parallel pGraph container;

 	Internal to stapl developer. 
*/
#ifndef pBaseGraph_h
#define pBaseGraph_h

///////////////////////////////////////////////////////////////
//                       include standard headers
///////////////////////////////////////////////////////////////
#include "Defines.h"
#include "runtime.h"

#include "BaseGraph.h"
#include "Graph.h"
#include "BasePContainer.h"
#include "GraphPart.h"
#include "GraphAlgo.h"
#include <deque>

#include "../base/splBaseElement.h"
#include "../distribution/DistributionDefines.h"
#include "../distribution/BaseDistribution.h"
//#include "../distribution/BaseDistributionRanges.h"
#include "../distribution/GraphDistribution.h"
#include "../base/pContainerRandomIterator.h"


namespace stapl{


#ifndef STAPL_FWD
#define STAPL_FWD 0
#endif

#ifndef STAPL_BACK
#define STAPL_BACK 1
#endif

//colors used for DFS/BFS traversals
#ifndef STAPL_WHITE
#define STAPL_WHITE 0
#endif

#ifndef STAPL_GRAY
#define STAPL_GRAY 1
#endif
#ifndef STAPL_BLACK
#define STAPL_BLACK 2
#endif


/**
 * @ingroup pgraph
 * @{
*/

/**pBaseGraph class
 *Implement the concept of the distributed graph. When an object of type pGraph
 *will be instantiated there will be one representative of it on all available  
 *threads of execution. The representatives work together in keeping the 
 *pGraph balanced among threads and to facilitate the access from every thread 
 *to every vertex/edge of the pGraph. The pGraph inherits from BasePContainer 
 *where general methods related with STAPL framework are implemented. 
*/

template <class VERTEX,class WEIGHT=int>
class pBaseGraph : public BasePContainer<
  GraphPart<VERTEX,WEIGHT >,
  GraphDistribution<vector<int> >,
  no_trace,
  bidirectional_iterator_tag,
  stapl_base_element_tag                                     
	>
{

  public:
  /**@brief  
   *Vertex data structure. It is internal to pBaseGraph and encapsulates the user
   *data structure for vertex;
   */
  typedef WtVertexType<VERTEX,WEIGHT> Vertex;
  
  /**@brief 
   *Edge data structure. Internal to pBaseGraph.
   */
  typedef WtEdgeType<VERTEX,WEIGHT>    WtEdge;

  /**@brief 
   *Base graph corresponding to the GraphPart.
   */
  typedef BaseGraph<VERTEX,WEIGHT>     BASEGRAPH;

  /**@brief 
   *The current pBaseGraph.
   */
  typedef pBaseGraph<VERTEX,WEIGHT>  pBaseGraph_type;
  
  /**@brief 
   *pBaseGraph distribution
   */
  typedef GraphDistribution<vector<int> >    GRAPH_DIST;
  

  /**@brief 
   *The user data associated with a vertex.
   */
  typedef  VERTEX value_type;

  /**@brief 
   *The user data for an edge weight
   */
  typedef  WEIGHT weight_type;
      
  /*
   *The pBaseGraph boundary; Is defined as a set of vertices;
   */
  typedef vector<GID> ELEMENT_SET;
  
  
  typedef BasePContainer<GraphPart<VERTEX,WEIGHT>, 
    GraphDistribution<vector<int> >,no_trace, bidirectional_iterator_tag, stapl_base_element_tag
    > BasePContainer_type;

  /**@brief 
   *pContainer part; The wrapper arround the Graph class to implement 
   *support for the pBaseGraph;
   */
  typedef  typename BasePContainer_type::pContainerPart_type pContainerPart_type;
  typedef typename BasePContainer_type::Element_Set_type Element_Set_type;

  /**@brief 
   * Vertices iterator.
   */
  typedef typename BasePContainer_type::iterator iterator;

  /**@brief 
   * Vertices const iterator.
   */
  typedef typename BasePContainer_type::const_iterator const_iterator;


  ///Vertices iterator for parts;  This is the  sequential graph teartor
  typedef typename BasePContainer_type::pContainerPart_type::iterator       part_iterator;

  ///Vertices const_terator for parts;  This is the  sequential graph teartor
  typedef typename BasePContainer_type::pContainerPart_type::const_iterator const_part_iterator;


  typedef typename BasePContainer_type::parts_internal_iterator parts_internal_iterator;
  typedef typename BasePContainer_type::const_parts_internal_iterator const_parts_internal_iterator;

  /**
   * Graph Specific Names for iterators
   */

  ///Vertices iterator.
  typedef iterator       VI;  

  ///Constant vertices iterator.
  typedef const_iterator CVI;
  
  ///Reverse vertices iterator
  typedef iterator       RVI; 
  
  ///Const reverse vertices iterator.
  typedef const_iterator CRVI;

  /**@brief 
   *Edge Iterators
   */
  
  typedef typename pContainerPart_type::WtEdge_VECTOR WtEdge_VECTOR;

  ///Edge Iterator
  typedef typename WtEdge_VECTOR::iterator               EI;   

  ///Constant Edge Iterator
  typedef typename WtEdge_VECTOR::const_iterator         CEI;  

  ///Reverse Edge Iterator
  typedef typename WtEdge_VECTOR::reverse_iterator       REI;  

  ///Constant Reverse Edge Iterator
  typedef typename WtEdge_VECTOR::const_reverse_iterator CREI; 

public:

  //=======================================
  //constructors and destructor
  //=======================================
  /**@name Constructors and Destructor */
  //@{
  /**@brief 
   *Contructor; Used to initialize an empty pBaseGraph
   */
  pBaseGraph();

  /**@brief 
   *Contructor; Used to initialize an empty pBaseGraph; 
   *It is used by hierarchical prange
   */
  pBaseGraph(int size);

  /**@brief 
   *Copy Constructor
   */
  pBaseGraph(const pBaseGraph_type&);

  /*Builds a distributed pgraph based on user inputs;
     See UserInputStruct.h
   */
  //pBaseGraph(PartitionInputs* user_inputs);
  
  /**@brief Destructor; The pGraph will be erased.
   */
  ~pBaseGraph();
  //@}


  public:
  
  /**@brief 
   * @return the size(global) of the pcontainer
   */
  //size_t size() const ;//see if it can be const; possible conflicts with size from AbstractBasePContainer

  /**@brief 
   * @return the size of the local data  of the pcontainer
   */
  //size_t local_size() const ;

  //size_t _size() const;

  /**@brief Local edges.
   * @return the number of edges on the local storage
   */
  size_t GetLocalEdgeCount() /*const*/;

  /**@brief 
   *Return the total number of edges;
   */
  size_t GetEdgeCount() /*const*/;

  /**@brief 
   *Return the total number of vertices;
   */
  size_t GetVertexCount() /*const*/;

  /**@brief Test if the pBaseGraph is empty.
   * @return true if the pcontainer is empty.
   */
  bool empty() ;
  
  /**@brief Test if the local storage is empty.
   *@return true if the local data of the pcontainer is empty.
   */
  bool local_empty() const;
  

  /**@brief Return the total storage for the pBaseGraph.
   *@return the size of the pcontainer as a number of bytes
   */
  size_t ByteSize() ;

  /**@brief The total local storage in bytes.
   * @return the size of the local data  of the pcontainer as a number of bytes
   */
  size_t LocalByteSize() ;


  //=======================================
  //Accessors
  //=======================================
  /**@name Accessors */
  //@{
  /**@brief Begin.
   *@returns VI local begin
   */
  VI begin() {
    return BasePContainer_type::local_begin();
  }

  /**@brief End.
   *@returns VI local end
   */
  VI end()  {
    return BasePContainer_type::local_end();
  }

  /*
  CVI begin()  const {
    return (CVI)BasePContainer_type::local_begin();
  }
  
  CVI end() const  {
    return (CVI)BasePContainer_type::local_end();
  }
  */


  /**@brief Local begin.
   *@return VI local begin
   */
  VI local_begin() {
    return (VI)BasePContainer_type::local_begin();
  }

  /**@brief Local end.
   *@return VI local end
   */
  VI local_end()  {
    return (VI)BasePContainer_type::local_end();
  }

  /**@brief Const local begin.
   *Returns local begin as a const iterator
   */  
  CVI local_begin() const {
    return (CVI)BasePContainer_type::local_begin();
  }

  /**@brief Const local end.
   *return CVI local end as a const iterator
   */
  CVI local_end() const {
    return (CVI)BasePContainer_type::local_end();
  }

  //==============================================================================
  //      Methods to get references to vertices/edges datastructures
  //==============================================================================
  public:
  /**@brief 
   *Check if the corresponding vertex exist in the distributed pGraph
   *The check is performed globaly;
   *@param GID the vertex identifier.
   */
  bool IsVertex(VID _gid);

  /**@brief 
   *Check if the vertex is local; If the vertex is local an iterator to it is initialized
   *and returned;
   *@param GID the vertex identifier.
   *@param VI* pointer to an iterator. If the vertex is found this iterator 
   *will be initialized with the right position.
   *@return true or false depending if the vertex exist local or not.
   *Note: This is not a global check
   */
  bool IsVertex(GID _gid,  VI* _pit);

  /**@brief 
   *Check if the vertex is local; In the vertex is local a const iterator to it is initialized
   *and returned;
   *@param GID the vertex identifier.
   *@param CVI* pointer to an iterator. If the vertex is found this iterator 
   *will be initialized with the right position.
   *@return true or false depending if the vertex exist local or not.
   *Note: This is not a global check
   */
  bool IsVertex(GID _gid, CVI* _cpit) const;

  //the next two methods need to be implemented

    /**@brief 
   *Check if the corresponding vertex exist in the distributed pGraph
   *The check is performed globaly;
   *@param VERTEX& reference to the data that identifies the vertex.
   */
  bool IsVertex(VERTEX&);

  /**@brief 
   *Check if the vertex is local; If the vertex is local an iterator to it is initialized
   *and returned;
   *@param VERTEX the vertex identifier.
   *@param VI* pointer to an iterator. If the vertex is found this iterator 
   *will be initialized with the right position.
   *@return true or false depending if the vertex exist local or not.
   *Note: This is not a global check
   */
  bool IsVertex(VERTEX&, VI*);

  /**@brief 
   *Check if the vertex is local; If the vertex is local an iterator to it is initialized
   *and returned;
   *@param const VERTEX the vertex identifier.
   *@param CVI* pointer to a const_iterator. If the vertex is found this iterator 
   *will be initialized with the right position.
   *@return true or false depending if the vertex exist local or not.
   *Note: This is not a global check
   */
  bool IsVertex(VERTEX&, CVI*) const;

  GID _GetVID(VERTEX&);

  /**@brief
   *Get the GID associated with a certain user data stored in the vertex.
   *@param VERTEX& reference to the vertex identifier.
   *@return GID the vertex identifier.
   */
  GID GetVID(VERTEX&);

  /**@brief 
   *Check if the corresponding edge exist;
   *@param VID _v1id source vertex id.
   *@param VID _v2id destination vertex id.
   *@return true if the edge exists and false otherwise.
   */
  bool IsEdge(VID _v1id, VID _v2id);

  /**@brief 
   *Check if the corresponding edge exist locally;
   *@param VID _v1id source vertex id.
   *@param VID _v2id destination vertex id.
   *@param VI* pointer to a vertex iterator. If the edge is found this iterator 
   *will be initialized with the location of source vertex.
   *@param EI* pointer to an edge iterator. If the edge is found this iterator 
   *will be initialized with the location of the edge.
   *@return true if the edge exist locally and false otherwise;Also Iterators
   *to vertex and edge are initialized if the edge is local;
   */
  bool IsEdge(VID, VID, VI*, EI*);

  /**@brief 
   *Check if the corresponding edge exist locally;
   *@param VID _v1id source vertex id.
   *@param VID _v2id destination vertex id.
   *@param WEIGHT _weight the weight for the edge we are looking for.
   *@param VI* pointer to a vertex iterator. If the edge is found this iterator 
   *will be initialized with the location of source vertex.
   *@param EI* pointer to an edge iterator. If the edge is found this iterator 
   *will be initialized with the location of the edge.
   *@return true if the edge exist locally and false otherwise;Also Iterators
   *to vertex and edge are initialized if the edge is local;
   */
  bool IsEdge(VID _v1id, VID _v2id, WEIGHT& _weight, VI* _vit, EI* _ei);

  /**Check if there is any edge connected from vid1 to vid2 of specified weight.
   *@param VID _v1id source vertex id.
   *@param VID _v2id destination vertex id.
   *@return false if vid1 or vid2 are not in graph, or
   *there is no edge from vid1 to vid2.
   */
  bool IsEdge(VID _v1id, VID _v2id, WEIGHT&);


  /**@brief 
   *Check if the corresponding edge exist locally; The edge is 
   *identified by its edge identifier.
   *@param VID  source vertex id.
   *@param int edge identifier.
   *@param VI* pointer to a vertex iterator. If the edge is found this iterator 
   *will be initialized with the location of source vertex.
   *@param EI* pointer to an edge iterator. If the edge is found this iterator 
   *will be initialized with the location of the edge.
   *@return true if the edge is local;Also Iterators
   *to vertex and edge are initialized if the edge is local;
   */
  bool IsEdgeId(VID,int,VI* ,EI*);
  //???? add the const versions of the above functions


 /**@brief 
   *Check if the corresponding edge exist;
   *@param VERTEX& _v1 data of the source vertex.
   *@param VERTEX& _v2 data of the destination vertex.
   *@return true if the edge exists and false otherwise.
   */
  bool IsEdge(VERTEX& _v1, VERTEX& _v2);

  /**@brief 
   *Check if the corresponding edge exist locally;
   *@param VERTEX& _v1 data of the source vertex.
   *@param VERTEX& _v2 data of the destination vertex.
   *@param VI* pointer to a vertex iterator. If the edge is found this iterator 
   *will be initialized with the location of source vertex.
   *@param EI* pointer to an edge iterator. If the edge is found this iterator 
   *will be initialized with the location of the edge.
   *@return true if the edge exist locally and false otherwise;Also Iterators
   *to vertex and edge are initialized if the edge is local;
   */
  bool IsEdge(VERTEX&, VERTEX&, VI*, EI*);

  /**@brief 
   *@return the data corresponding to the specified vertex;
   */
  VERTEX GetData(VID);
   
  WEIGHT GetEdgeWeight(VID _vid1, VID _vid2);

  /**@brief 
   *Access user data;
   *@return the user data stored in the vertex pointed by VI 
   */
  virtual VERTEX& GetUserData(VI);


  /**@brief 
   *@return the vertex identifier corresponding to the specified data;
   */
  int getVID(const VERTEX&);

  protected:
  bool _IsEdge(VID , VID , VI* , EI* ,PARTID );
  bool _IsEdge(_StaplTriple<VID,VID,WEIGHT>& _args);
  bool _IsEdge(VID , VID , WEIGHT&, VI* , EI* ,PARTID );
  VERTEX _GetData(VID);
  WEIGHT _GetEdgeWeight(VID _vid1, VID _vid2);
  bool _IsEdgeId(VID , int , VI* , EI* ,PARTID );

  //==============================================================================
  //      Methods to access graph datastructure(Adjacency list)
  //==============================================================================

  public:
  /**@brief Get vertices which are adjacent to this specified vertex.
   *@param VID the vertex for which we want adjacency information
   *@param vector<VID>& vector of VIDs which are the VIDs of those who are next to the specified vertex.
   *@return int the number of elements pushed in vector<VID>.
   *If this specified VID is not found, an empty list will be returned and ERROR status will be returned.
   *@note The method doesn't clear the vector inside. It only resizes if the input 
   *vector is smaller than the actual number of adjacent vertices. If the
   *size of adjacent vertices is less than the size of the input vector _succ
   *the user will have to iterate from begin() to begin() + returned size;
   */
  int GetAdjacentVertices(VID _v1id, vector<VID>& _succ) /*const*/;

  /**@brief Get vertices which are adjacent to this specified vertex.
   *@param VID the vertex for which we want adjacency information
   *@param vector<VERTEX>& vector of user data which for the VIDs who are next to the specified vertex.
   *@return int the number of elements pushed in vector<VERTEX>.
   *If this specified VID is not found, an empty list will be returned and ERROR status will be returned.
   *@note The method doesn't clear the vector inside. It only resizes if the input 
   *vector is smaller than the actual number of adjacent vertices. If the
   *size of adjacent vertices is less than the size of the input vector _succ
   *the user will have to iterate from begin() to begin() + returned size;
   */
  int GetAdjacentVerticesDATA(VID _v1id, vector<VERTEX>& _succ) /*const*/;

  /**@brief Get vertices which are adjacent to this specified vertex.
   *@param VI the pointer to the vertex for which we want adjacency information.
   *@param vector<VID>& vector of VIDs which are the VIDs of those who are next to the specified vertex.
   *@return int the number of elements pushed in vector<VID>.
   *If this specified VID is not found, an empty list will be returned and ERROR status will be returned.
   *@note The method doesn't clear the vector inside. It only resizes if the input 
   *vector is smaller than the actual number of adjacent vertices. If the
   *size of adjacent vertices is less than the size of the input vector _succ
   *the user will have to iterate from begin() to begin() + returned size;
   */
  int GetAdjacentVertices(VI _vi, vector<VID>& _succ) const;
  

  /**@brief Get vertices which are adjacent to this specified vertex.
   *@param VI the pointer to the vertex for which we want adjacency information
   *@param vector<VERTEX>& vector of user data which for the VIDs who are next to the specified vertex.
   *@return int the number of elements pushed in vector<VERTEX>.
   *If this specified VID is not found, an empty list will be returned and ERROR status will be returned.
   *@note The method doesn't clear the vector inside. It only resizes if the input 
   *vector is smaller than the actual number of adjacent vertices. If the
   *size of adjacent vertices is less than the size of the input vector _succ
   *the user will have to iterate from begin() to begin() + returned size;
   */
  int GetAdjacentVerticesDATA(VI _vi, vector<VERTEX>& _succ) /*const*/;

  /**@brief Get vertices which are predecessors to this specified vertex.
   *@param VID the vertex for which we want adjacency information
   *@param vector<VID>& vector of VIDs which are the VIDs of those who are predecessors to the specified vertex.
   *@return int the number of elements pushed in vector<VID>.
   *If this specified VID is not found, zero will be
   *returned. If pSetPredecessors() method wasn't called the return size is zero;
   *@note The method doesn't clear the vector inside. It only resizes if the input 
   *vector is smaller than the actual number of predecessors. If the
   *size of predecessors is less than the size of the input vector _succ
   *the user will have to iterate from begin() to begin() + returned size;
   */
  int GetPredVertices(VID _v1id, vector<VID>& _succ) /*const*/;


  /**@brief Get vertices which are predecessors to this specified vertex.
   *@param VID the vertex for which we want adjacency information
   *@param vector<VERTEX>& vector of user data which for the VIDs who are next to the specified vertex.
   *@return int the number of elements pushed in vector<VERTEX>.
   *If this specified VID is not found, zero will be
   *returned. If pSetPredecessors() method wasn't called the return size is zero;
   *@note The method doesn't clear the vector inside. It only resizes if the input 
   *vector is smaller than the actual number of predecessors. If the
   *size of predecessors is less than the size of the input vector _succ
   *the user will have to iterate from begin() to begin() + returned size;
   */
  int GetPredVerticesDATA(VID _v1id, vector<VERTEX>& _succ) /*const*/;

  /**@brief Get vertices which are predecessors to this specified vertex.
   *@param VI the pointer to the vertex for which we want adjacency information
   *@param vector<VID>& vector of VIDs which are the VIDs of those who are predecessors to the specified vertex.
   *@return int the number of elements pushed in vector<VID>.
   *If this specified VID is not found, an empty list will be returned and ERROR status will be 
   *returned.If pSetPredecessors() method wasn't called the return size is zero;
   *@note The method doesn't clear the vector inside. It only resizes if the input 
   *vector is smaller than the actual number of predecessors. If the
   *size of predecessors is less than the size of the input vector _succ
   *the user will have to iterate from begin() to begin() + returned size;
   */
  int GetPredVertices(VI _vi, vector<VID>& _preds) const;
  

  /**@brief Get vertices which are predecessors to this specified vertex.
   *@param VI the pointer to the vertex for which we want adjacency information
   *@param vector<VERTEX>& vector of user data which for the VIDs who are next to the specified vertex.
   *@return int the number of elements pushed in vector<VERTEX>.
   *If this specified VID is not found, an empty list will be returned and ERROR status will be 
   *returned.If pSetPredecessors() method wasn't called the return size is zero;
   *@note The method doesn't clear the vector inside. It only resizes if the input 
   *vector is smaller than the actual number of predecessors. If the
   *size of predecessors is less than the size of the input vector _succ
   *the user will have to iterate from begin() to begin() + returned size;
   */
  int GetPredVerticesDATA(VI _vi, vector<VERTEX>& _preds) /*const*/;

  /**@brief Get the vertices corresponding to the local storage
   *@param vector<VID>& vector of VIDs of the local vertices.
   *@return int the number of elements pushed in vector<VID>.
   */
  int GetVertices(vector<VID>& _verts) const;

  /**@brief Get the vertices corresponding to the local storage
   *@param vector<VI>& vector of iterators pointing to the local vertices.
   *@return int the number of elements pushed in vector<VI> .
   */
  int GetVertices(vector<VI>& _verts) /*const*/;

  /**@brief Get the vertices corresponding to the local storage
   *@param vector<VERTEX>& vector of user data stored inside the local vertices
   *@return int the number of elements pushed in vector<VERTEX>. 
   */
  int GetVertices(vector<VERTEX>& _verts) const;

  /**@brief 
   *Get the edges corresponding to the local storage.
   *@param vector<pair<VID,VID> >& vector of edges(pair source, destination)
   *@return int the number of elements pushed in vector<pair<VID,VID> >.
   *@note Test this for undirected.
   */
  int GetEdges(vector<pair<VID,VID> >& _edges) const;

  /**@brief 
   *Get the edges corresponding to the local storage.
   *@param  vector<pair<pair<VID,VID>,WEIGHT> >& vector of edges(triplet source, destination, weight).
   *@return int the number of elements pushed in edges.
   */
  int GetEdges(vector<pair<pair<VID,VID>,WEIGHT> >& _edges) const;

  /**
   *Get out degree of a given vertex.
   *out degree of a vertex is number of edges
   *that are from this vertex to any other vertex in graph.
   *@return the out degree for the specifed vertex or -1 if the vertex doesn't exist.
   */
  int GetVertexOutDegree(VID _v1) ;

  //------ edges

  /**Get Edges which are incident to this specified vertex.
   *Outgoing edges are edges around this specified vertex.
   *@return a list of edges which is defined by 2 VIDs of its endpoints.
   *Empty list will be returned if this specified VID 
   *is not in graph and error message will be in standard output.
   */
  int GetOutgoingEdges(VID _v1id,vector< pair<VID,VID> >& iedges) /*const*/;

  /**Get Edges which are incident to this specified vertex.
   *Outgoing edges are edges around this specified vertex.
   *@return a list of edges which is defined by two VIDs of its endpoints 
   *and the edge weight.
   * Empty list will be returned if this specified VID 
   *is not in graph and error message will be in standard output.
   */
  int GetOutgoingEdges(VID _v1id,vector< pair<pair<VID,VID>,WEIGHT> >& iedges) /*const*/ ;

  /**Get Edges which are incident to this specified vertex and user data associated with
   *these edges.
   *Outgoing edges are edges around this specified vertex.
   *@return a list of edges which is defined by 2 user data of its endpoints.
   *Empty list will be returned if this specified VID 
   *is not in graph and error message will be in standard output.
   */
  int GetOutgoingEdgesVData(VID _v1id, vector< pair<VERTEX,VERTEX> >& iedges) ;

 /**Get Edges which are incident to this specified vertex and user data associated with
   *these edges.
   *Outgoing edges are edges around this specified vertex.
   *@return a list of edges which is defined by 2 user data of its endpoints and
   *the weight of edge. Empty list will be returned if this specified VID 
   *is not in graph and error message will be in standard output.
   */
  int GetOutgoingEdgesVData(VID _v1id,vector< pair<pair<VERTEX,VERTEX>,WEIGHT> >& iedges) /*const*/;


  protected:
  vector<VID> _GetAdjacentVertices(VID _v1id) const ;
  vector<VID> _GetPredVertices(VID _v1id) const;
  vector<VERTEX> _GetAdjacentVerticesDATA(VID _v1id) /*const*/;
  vector<VERTEX> _GetPredVerticesDATA(VID _v1id) /*const*/;
  vector< pair<VID,VID> > _GetOutgoingEdges(VID _v1id) /*const*/;
  vector< pair<pair<VID,VID>,WEIGHT> > _GetOutgoingEdgesWeight(VID _v1id) /*const*/;
  vector< pair<VERTEX,VERTEX> > _GetOutgoingEdgesVData(VID _v1id) /*const*/;
  vector< pair<pair<VERTEX,VERTEX>,WEIGHT> > _GetOutgoingEdgesWeightVData(VID _v1id) /*const*/;
  //int _GetVertexOutDegree(VID _v1) ;
  //@}

  //==============================================================================
  //      Methods to get graph properties
  //==============================================================================
  public:
  /**@name Add/Delete vertices/edges */
  //@{

  //==============================================================================
  //      Methods to add, delete vertices
  //==============================================================================
  
  /**@brief 
   *The basic method for adding a vertex to the graph. 
   *@param VERTEX _v is the data that should be put in the vertex
   *@param VID _vid is the ID of the vetex (VID)
   *@param int where gives the thread identifier where the vertex should be added.
   *@return int the status of the operation.
   * Warning :: It is the responsibility of the USER to make sure that the VIDs are unique 
   */
  public:
  int AddVertex(VERTEX& _v, VID _vid,int where);

  /**@brief 
   *Add a vertex to the pGraph. 
   *@param VERTEX _v is the data that should be put in the vertex
   *@param VID _vid is the ID of the vetex (VID)
   *@return int the status of the operation.
   *The vertex will be added on the caller thread
   * Warning :: It is the responsibility of the USER to make sure that the VIDs are unique
   */
  int AddVertex(VERTEX& _v, VID _vid);

  /**@brief 
   *Add a new vertex; The vertex id will be generated automatically;
   *@return VID associated with the vertex.
   */
  int AddVertex(VERTEX& _v);

  /**@brief 
   *Delete the vertex corresponding to the specified identifier;
   *@return int the status of the operation.
   */
  int DeleteVertex(VID _vid);

  /**@brief Delete a set of vertices. Some optimizations can be employed 
   *when we delete vertices in bulk.
   *@param vector<VID>& vertices to be deleted.
   *@return the status of the operation.
   */
  int pDeleteVertices(vector<VID>& _vv);

  pair<double,double> pSimpleDeleteVertices(vector<VID>& _vv);


  protected:
   void _deleteGhosts(vector<pair<VID,VID> > _vv);
   void _simpleDeleteGhosts(vector<pair<VID,VID> > _vv,vector<pair<VID,VID> > _vv_succ );
  /*
   *This function is used internally by AddVertex. Dont use it anywhere else
   */
  void  _AddVertex(VERTEX& _v, VID _vid);

  /*
   *This function is used internally by DeleteVertex. Dont use it anywhere else
   */
  void _iDeleteVertex(VID _vid, PID _caller);
  
  /*
   *This function is used internally by DeleteVertex. Deletes all edges to _vid
   *Dont use it anywhere else
  */
  void _iDeleteAllEdgesToV(VID _vid);

  public:
  /**@brief 
   *Adds an edge to the pgraph.
   *@param VID the first vertex
   *@param VID the second vertex
   *@param _weight the weight of the edge.
   *@param _multi check for edge multiplicity. If the edge is already present don't added;
   *@return the edge id 
   */
  int AddEdge(VID _v1id, VID _v2id, WEIGHT& _weight,bool _multi);
  void AddEdgeAsync(VID _v1id, VID _v2id, WEIGHT& _weight,bool _multi);

  /**@brief 
   *Adds an edge to the pgraph.
   *@param VID the first vertex
   *@param VID the second vertex
   *@param _weight the weight of the edge.
   *@return 
   */
  int AddEdge(VID _v1id, VID _v2id, pair<WEIGHT,WEIGHT>& _weight, bool _multi);
  void AddEdgeAsync(VID _v1id, VID _v2id, pair<WEIGHT,WEIGHT>& _weight, bool _multi);

  /**@brief 
   *Delete all the edges between the specified vertices; 
   *@param VID the first vertex
   *@param VID the second vertex
   *@return the status of the operation.
   */
  int DeleteEdge(VID _v1id, VID _v2id);

  int DeleteEdgeAndPredecessor(VID _v1id, VID _v2id);
  int DeleteEdgeAndPredecessor_succ(VID _v1id, VID _v2id);
  int DeleteEdgeAndPredecessor_preds(VID _v1id, VID _v2id);

  /**@brief 
   *Delete all the edges between the specified vertices that have the specified weight; 
   *@param VID the first vertex
   *@param VID the second vertex
   *@param WEIGHT& reference to a weight class used to identify 
   *the edges to be deleted 
   *@return the status of the operation.
   */
  int DeleteWtEdge(VID _v1id, VID _v2id,WEIGHT& _w);

  /**@brief 
   *Delete the specified edge; 
   *@param VID the vertex that owns the edge.
   *@param int edge identifier.
   */
  int DeleteEdgeId(VID _v1id, int _edgeid);

  protected:
  /*
   *Internal function used by AddEdge
  */
  int _iAddEdge(_StaplQuad<VID,VID,WEIGHT,bool> _vvw);
  void _iAddEdgeAsync(_StaplQuad<VID,VID,WEIGHT,bool> _vvw);

  /*
   *This is the function where most of the computation of AddEdge is done.
   *This is used internally only
  */
  int _iAddEdge(VID _v1id, VID _v2id, WEIGHT _weight,PARTID _pid,bool  _multi);
  void _iAddEdgeAsync(VID _v1id, VID _v2id, WEIGHT _weight,PARTID _pid,bool  _multi);
  
  void _DeleteWtEdge(_StaplTriple<VID,VID,WEIGHT>);
  
  public:  
  /**@brief  
   *Remove all vertices and all edges from the current pGraph.
   */
  void ErasePGraph();

  //@}
  
  //==============================================================================
  //      Set/Get Predecessors Methods
  //==============================================================================

  void _AddPredecessor(VID _v2, WtEdge _e);

  /**@brief Initialize predecessors in the data field of Vertex.
   *After predecessors  are set we know the sources of all 
   *incoming edges for all vertices.
   */
  void pSetPredecessors();

  /**@brief 
   *Clear the predecessors for all the vertices in the graph
  */
  void ClearPredecessors();

  /**@brief 
   *Set the predecessors for all the vertices in the graph
   *in an asynchronous fashion.
   *It assumes that the predecessors are cleared;
  */
  void pAsyncSetPredecessors();

  /**@brief Get the edges that cross processor boundaries.
   *@param set<pair<VID,VID> >& set of remote edges.
   */
  void pGetCutEdges(set<pair<VID,VID> >& cut);
  
  //////////////////////////////////////////////////////////////////////////
  //       Basic Traversal DFS like on the current pgraph
  //////////////////////////////////////////////////////////////////////////
  template <class VISITOR>
  int DFS_traversal_wrapper(_StaplQuad<VID,VISITOR,int,bool> _arg);

  template <class VISITOR>
  int DFS_traversal(VID _start, VISITOR& _vis, int _traversal_id,bool _preds);


  //////////////////////////////////////////////////////////////////////////
  //       Basic Traversal BFS like on the current pgraph
  //////////////////////////////////////////////////////////////////////////
  public:
  void StartCounting(){
    this->dist.StartCounting();
  }

  void StopCounting(){
    this->dist.StopCounting();
  }

  void PrintCounter(){
    this->dist.PrintCounter();
  }

  template <class VISITOR>
  void BFS_traversal_wrapper(_StaplQuad<VID,VISITOR,int,bool> _arg);

  template <class VISITOR>
  int BFS_traversal(VID _start, VISITOR& _vis, int _traversal_id,bool _preds);

  template <class VISITOR>
  void BFS_traversal_wrapper_EQ(_StaplQuad<VID,VISITOR,int,bool> _arg);

  template <class VISITOR>
  int BFS_traversal_EQ(VID _start, VISITOR& _vis, int _traversal_id,bool _preds);

  template <class VISITOR>
  void TOPO_traversal_wrapper(_StaplPenta<VID,VISITOR,int,bool,VID> _arg);

  template <class VISITOR>
  int TOPO_traversal(VID _start, VISITOR& _vis, int _traversal_id,bool _preds);

  void SetTraversalNumber(int _tr_nr);
  void ResetTraversals();
  int ResetTraversal(int _tr_id);
  void SetDependencies();

  void SetDependencies(int _tr_id);
  //using in/out vectors
  void GetZeroDependencies(int _tr_id,vector<VID>&,vector<VID>&);

  //using edgelist/predecessors size
  void GetZeroDependencies(vector<VID>&,vector<VID>&);


  //==============================================================================
  //      Methods for getting edge weight field and vertex field
  //==============================================================================
  /**@name Set/Get methods*/
  //@{
public:

  /**@brief 
   *Return a specific field of the weight; 
   * The template T is the type that will be returned.
   * @param VID the source vertex of the edge.
   * @param VID the destination vertex of the edge.
   * @param T (WEIGHT::* method)() is the method that will be called on the WEIGHT class.
   *@return T the result of the method on the edge data.
   */
  template <class T>
  T pGetWeightField(VID _v1id,VID _v2id,T (WEIGHT::* method)());

  /**@brief 
   *Return a specific field of the weight;
   * The template T is the type that will be returned.
   * @param VID the source vertex of the edge.
   * @param VID the destination vertex of the edge.
   * @param T (WEIGHT::* method)() CONST that will be called on the WEIGHT clas
   * @return T the result of the method on the edge data.
   */
  template <class T>
  T pGetWeightField(VID _v1id,VID _v2id,T (WEIGHT::* method)() const);

  /**@brief 
   *Return a specific field of the weight; 
   * The template T is the type that will be returned.
   * @param VID the source vertex of the edge.
   * @param VID the destination vertex of the edge.
   * @param T (WEIGHT::* method)(T1) is the method that will be called on the WEIGHT class.
   * @param T1 the first argument of the method.
   * @return T the result of the method on the edge data.
   */
  template <class T, class T1>
  T pGetWeightField(VID _v1id,VID _v2id,T (WEIGHT::* method)(T1&),T1&);

  /**@brief 
   *Return a specific field of the weight; 
   * The template T is the type that will be returned.
   * @param VID the source vertex of the edge.
   * @param VID the destination vertex of the edge.
   * @param T (WEIGHT::* method)(T1) const is the method that will be called on the WEIGHT class.
   * @param T1 the first argument of the method.
   * @return T the result of the method on the edge data.
   */
  template <class T, class T1>
  T pGetWeightField(VID _v1id,VID _v2id,T (WEIGHT::* method)(T1&) const,T1&);

  /**@brief 
   *Return a specific field of the weight; 
   * The template T is the type that will be returned.
   * @param VID the source vertex of the edge.
   * @param VID the destination vertex of the edge.
   * @param T (WEIGHT::* method)(T1,T2) is the method that will be called on the WEIGHT class.
   * @param T1 the first argument of the method.
   * @param T2 the second argument of the method.
   * @return T the result of the method on the edge data.
   */
  template <class T, class T1,class T2>
  T pGetWeightField(VID _v1id,VID _v2id,T (WEIGHT::* method)(T1&,T2&),T1&,T2&);

  /**@brief 
   *Return a specific field of the weight; 
   * The template T is the type that will be returned.
   * @param VID the source vertex of the edge.
   * @param VID the destination vertex of the edge.
   * @param T (WEIGHT::* method)(T1,T2) CONST method that will be called on the WEIGHT clas
   * @param T1 the first argument of the method.
   * @param T2 the second argument of the method.
   * @return T the result of the method on the edge data.
   */
  template <class T, class T1,class T2>
  T pGetWeightField(VID _v1id,VID _v2id,T (WEIGHT::* method)(T1&,T2&) const,T1&,T2&);

protected:
  template<class T> 
  T __GetWeightField(_StaplTriple<VID,VID,T (WEIGHT::*)()> args);
  template<class T> 
  T __GetWeightField(_StaplTriple<VID,VID,T (WEIGHT::*)() const> args);
  
  template<class T,class ARG> 
  T __GetWeightField(_StaplQuad<VID,VID,T (WEIGHT::*)(ARG&),ARG> args);
  
  template<class T,class ARG> 
  T __GetWeightField(_StaplQuad<VID,VID,T (WEIGHT::*)(ARG&) const,ARG> args);

  template<class T,class T1,class T2> 
  T __GetWeightField(_StaplPenta<VID,VID,T (WEIGHT::*)(T1&,T2&),T1,T2> args);
  template<class T,class T1,class T2> 
  T __GetWeightField(_StaplPenta<VID,VID,T (WEIGHT::*)(T1&,T2&) const ,T1,T2> args);

 
  /*
   * This method will be used when the user will write on the weight 
   * corresponding to the edge; Some edges are between vertices that lives on 
   * two different processors; In that case I have to update the weight on the edge 
   * corresponding to the ghost node on the remote processor
   * @param VID v1 iterator to the vertex one of the edge
   * @param VID v2 iterator to the edge(second vertex and weight)
   * @param method is the method that will be called on the WEIGHT class
   * @param arg is the argument that will be passed to the method
   */
  public:
  /**@brief 
   *Set a specific field of the weight of the edge; 
   * @param VID the source vertex of the edge.
   * @param VID the destination vertex of the edge.
   * @param int (WEIGHT::* method)(T1) is the method that will be called on the WEIGHT class.
   * @param T the argument of the method.
   * @return int the status of the operation.
   */
  template <class T>
  int pSetWeightField(VID v1, VID v2,int (WEIGHT::* method)(T&),T&);
  /**@brief 
   *Set a specific field of the weight of the edge; 
   * @param VID the source vertex of the edge.
   * @param VID the destination vertex of the edge.
   * @param int (WEIGHT::* method)(T1,T2) is the method that will be called on the WEIGHT class.
   * @param T1 the first argument of the method.
   * @param T2 the second argument of the method.
   * @return int the status of the operation.
   */
  template <class T1,class T2>
  int pSetWeightField(VID v1, VID v2,int (WEIGHT::* method)(T1&,T2&),T1&,T2&);

  /**@brief 
   *Set a specific field of the weight of the edge; 
   * @param VID the source vertex of the edge.
   * @param VID the destination vertex of the edge.
   * @param int (WEIGHT::* method)(T1,T2,T3) is the method that will be called on the WEIGHT class.
   * @param T1 the first argument of the method.
   * @param T2 the second argument of the method.
   * @param T3 the third argument of the method.
   * @return int the status of the operation.
   */
  template <class T1,class T2,class T3>
  int pSetWeightField(VID v1, VID v2,int (WEIGHT::* method)(T1&,T2&,T3&),T1&,T2&,T3&);

  protected:
  template<class T> 
  void __SetWeightField(_StaplQuad<VID,VID,int (WEIGHT::*)(T&),T>);

  template<class T1,class T2> 
  void __SetWeightField(_StaplPenta<VID,VID,int (WEIGHT::*)(T1&,T2&),T1,T2>);

  template<class T1,class T2,class T3> 
  void __SetWeightField(_StaplSix<VID,VID,int (WEIGHT::*)(T1&,T2&,T3&),T1,T2,T3>);


  //=============================================================================
  //      SetWeight for where the edge is identified by source vertex and edge id
  //=============================================================================
  public:
  /**@brief 
   *Return a specific field of the weight; 
   * The template T is the type that will be returned.
   * @param VID the source vertex of the edge.
   * @param int the edge identifier.
   * @param T (WEIGHT::* method)() is the method that will be called on the WEIGHT class.
   *@return T the result of the method on the edge data.
   */
  template <class T>
  T pGetWeightFieldEdgeId(VID,int,T (WEIGHT::* method)());

  /**@brief 
   *Return a specific field of the weight; 
   * The template T is the type that will be returned.
   * @param VID the source vertex of the edge.
   * @param int the edge identifier.
   * @param T (WEIGHT::* method)() const is the method that will be called on the WEIGHT class.
   *@return T the result of the method on the edge data.
   */
  template <class T>
  T pGetWeightFieldEdgeId(VID,int,T (WEIGHT::* method)() const);

  /**@brief 
   *Return a specific field of the weight; 
   * The template T is the type that will be returned.
   * @param VID the source vertex of the edge.
   * @param int the edge identifier.
   * @param T (WEIGHT::* method)(T1,T2) is the method that will be called on the WEIGHT class.
   * @param T1 the first argument of the method.
   * @return T the result of the method on the edge data.
   */
  template <class T, class T1>
  T pGetWeightFieldEdgeId(VID,int,T (WEIGHT::* method)(T1&),T1&);

  /**@brief 
   *Return a specific field of the weight; 
   * The template T is the type that will be returned.
   * @param VID the source vertex of the edge.
   * @param int the edge identifier.
   * @param T (WEIGHT::* method)(T1,T2) const is the method that will be called on the WEIGHT class.
   * @param T1 the first argument of the method.
   * @return T the result of the method on the edge data.
   */
  template <class T, class T1>
  T pGetWeightFieldEdgeId(VID,int,T (WEIGHT::* method)(T1&) const,T1&);

  /**@brief 
   *Return a specific field of the weight; 
   * The template T is the type that will be returned.
   * @param VID the source vertex of the edge.
   * @param int the edge identifier.
   * @param T (WEIGHT::* method)(T1,T2) is the method that will be called on the WEIGHT class.
   * @param T1 the first argument of the method.
   * @param T2 the second argument of the method.
   * @return T the result of the method on the edge data.
   */
  template <class T, class T1,class T2>
  T pGetWeightFieldEdgeId(VID,int,T (WEIGHT::* method)(T1&,T2&),T1&,T2&);

  /**@brief 
   *Return a specific field of the weight; 
   * The template T is the type that will be returned.
   * @param VID the source vertex of the edge.
   * @param int the edge identifier.
   * @param T (WEIGHT::* method)(T1,T2) const is the method that will be called on the WEIGHT class.
   * @param T1 the first argument of the method.
   * @param T2 the second argument of the method.
   * @return T the result of the method on the edge data.
   */
  template <class T, class T1,class T2>
  T pGetWeightFieldEdgeId(VID,int,T (WEIGHT::* method)(T1&,T2&) const,T1&,T2&);

  protected:
  template<class T> 
  T __GetWeightFieldEdgeId(_StaplTriple<VID,VID,T (WEIGHT::*)()> args);

  template<class T> 
  T __GetWeightFieldEdgeId(_StaplTriple<VID,VID,T (WEIGHT::*)() const> args);
  
  template<class T,class ARG> 
  T __GetWeightFieldEdgeId(_StaplQuad<VID,VID,T (WEIGHT::*)(ARG&),ARG> args);

  template<class T,class ARG> 
  T __GetWeightFieldEdgeId(_StaplQuad<VID,VID,T (WEIGHT::*)(ARG&) const,ARG> args);

  template <class T, class T1,class T2>
  T __GetWeightFieldEdgeId(_StaplPenta<VID,VID,T (WEIGHT::*)(T1&,T2&),T1,T2> args);

  template <class T, class T1,class T2>
  T __GetWeightFieldEdgeId(_StaplPenta<VID,VID,T (WEIGHT::*)(T1&,T2&) const,T1,T2> args);

  public:
  /**@brief 
   *Set a specific field of the weight; 
   * @param VID the source vertex of the edge.
   * @param int the edge identifier.
   * @param int (WEIGHT::* method)(T) is the method that will be called on the WEIGHT class.
   * @param T the argument of the method.
   * @return int the status of the operation.
   */
  template <class T>
  int pSetWeightFieldEdgeId(VID, int ,int (WEIGHT::* method)(T&),T&);

  /**@brief 
   *Set a specific field of the weight; 
   * @param VID the source vertex of the edge.
   * @param int the edge identifier.
   * @param int (WEIGHT::* method)(T1,T2) is the method that will be called on the WEIGHT class.
   * @param T1 the first argument of the method.
   * @param T2 the second argument of the method.
   * @return int the status of the operation.
   */
  template <class T1,class T2>
  int pSetWeightFieldEdgeId(VID, int, int (WEIGHT::* method)(T1&,T2&),T1&,T2&);

  /**@brief 
   *Set a specific field of the weight; 
   * @param VID the source vertex of the edge.
   * @param int the edge identifier.
   * @param int (WEIGHT::* method)(T1,T2) is the method that will be called on the WEIGHT class.
   * @param T1 the first argument of the method.
   * @param T2 the second argument of the method.
   * @param T3 the third argument of the method.
   * @return int the status of the operation.
   */
  template <class T1,class T2,class T3>
  int pSetWeightFieldEdgeId(VID, int ,int (WEIGHT::* method)(T1&,T2&,T3&),T1&,T2&,T3&);

  protected:
  template<class T> 
  void __SetWeightFieldEdgeId(_StaplQuad<VID,int,int (WEIGHT::*)(T&),T>);

  template<class T1,class T2> 
  void __SetWeightFieldEdgeId(_StaplPenta<VID,int,int (WEIGHT::*)(T1&,T2&),T1,T2>);

  template<class T1,class T2,class T3> 
  void __SetWeightFieldEdgeId(_StaplSix<VID,int,int (WEIGHT::*)(T1&,T2&,T3&),T1,T2,T3>);

  //==============================================================================
  //    Get/Set Vertex field
  //      Methods for setting/getting vertex fields
  //      run a user specified method on the weight or vertex class
  //      this is used to hide the local/remote issue
  //==============================================================================
  public:
  /**@brief 
   *Return a specific field of the vertex; 
   * The template T is the type that will be returned.
   * @param VID the vertex identifier.
   * @param T (WEIGHT::* method)() is the method that will be called on the VERTEX class.
   *@return T the result of the method on the edge data.
   */
  template <class T>
  T pGetVertexField(VID vid,T (VERTEX::* method)());

  /**@brief 
   *Return a specific field of the vertex; 
   * The template T is the type that will be returned.
   * @param VID the vertex identifier.
   * @param T (WEIGHT::* method)() const is the method that will be called on the VERTEX class.
   *@return T the result of the method on the edge data.
   */
  template <class T>
  T pGetVertexField(VID vid,T (VERTEX::* method)() const);

  /**@brief 
   *Return a specific field of the vertex; 
   * The template T is the type that will be returned.
   * @param VID the vertex identifier.
   * @param T (WEIGHT::* method)(T1) is the method that will be called on the VERTEX class.
   * @param T1 the argument of the method.
   *@return T the result of the method on the edge data.
   */
  template <class T,class T1>
  T pGetVertexField(VID vid,T (VERTEX::* method)(T1&),T1&);

  /**@brief 
   *Return a specific field of the vertex; 
   * The template T is the type that will be returned.
   * @param VID the vertex identifier.
   * @param T (WEIGHT::* method)(T1) const is the method that will be called on the VERTEX class.
   * @param T1 the argument of the method.
   *@return T the result of the method on the edge data.
   */
  template <class T,class T1>
  T pGetVertexField(VID vid,T (VERTEX::* method)(T1&) const,T1&);

  protected:
  template <class T>
  T __GetVertexField(_StaplPair<VID,T (VERTEX::*)()>);
  template <class T>
  T __GetVertexField(_StaplPair<VID,T (VERTEX::*)() const>);

  template <class T,class ARG>
  T __GetVertexField(_StaplTriple<VID,T (VERTEX::*)(ARG&),ARG>);
  template <class T,class ARG>
  T __GetVertexField(_StaplTriple<VID,T (VERTEX::*)(ARG&) const,ARG>);

 
  /*
   * This method will be used when the user will write on the data 
   * corresponding to the vertex; 
   * If the vertex is remote an async_rmi will be sent to the remote processors;
   * @param VID _vid iterator to the vertex one of the edge
   * @param VERTEX::* method method to be executed on the VERTEX datastructure
   * @param arg is the argument that will be passed to the method
   */
  public:
  /**@brief 
   *Set a specific field of the vertex; 
   * @param VID the vertex identifier.
   * @param int (WEIGHT::* method)(T) is the method that will be called on the VERTEX class.
   * @param T the argument of the method.
   * @return int the status of the operation.
   */
  template<class T> 
  int pSetVertexField(VID _vid,int (VERTEX::* method)(T&),T& _arg);

  /**@brief 
   *Set a specific field of the vertex; 
   * @param VID the vertex identifier.
   * @param int (WEIGHT::* method)(T) is the method that will be called on the VERTEX class.
   * @param T1 the first argument of the method.
   * @param T2 the second argument of the method.
   * @return int the status of the operation.
   */
  template<class T1,class T2> 
  int pSetVertexField(VID _vid,int (VERTEX::* method)(T1&,T2&),T1&,T2&);

  /**@brief 
   *Set a specific field of the vertex; 
   * @param VID the vertex identifier.
   * @param int (WEIGHT::* method)(T) is the method that will be called on the VERTEX class.
   * @param T1 the first argument of the method.
   * @param T2 the second argument of the method.
   * @param T3 the third argument of the method.
   * @return int the status of the operation.
   */
  template<class T1,class T2,class T3> 
  int pSetVertexField(VID _vid,int (VERTEX::* method)(T1&,T2&,T3&),T1&,T2&,T3&);

  protected:
  template<class T> 
  void __SetVertexField(_StaplTriple<VID,int (VERTEX::*)(T&),T> args);
  template<class T1,class T2> 
  void __SetVertexField(_StaplQuad<VID,int (VERTEX::*)(T1&,T2&),T1,T2> args);
  template<class T1,class T2,class T3> 
  void __SetVertexField(_StaplPenta<VID,int (VERTEX::*)(T1&,T2&,T3&),T1,T2,T3> args);

  //@}


  //====================================================
  //   Distribution related methods.
  //====================================================
  public:
  /*
    Update the maps corresponding to the local graphs(parts); it is
    usefull when a lot of delete vertices are called
   */
  void UpdateMap(){
    pContainerPart_type* p;
    for(int i=0;i<this->GetPartsCount();i++){
      p = this->GetPart(i);
      p->UpdateMap();
    }    
  }
  /**@name Distribution related methods */
  //@{

  /**@brief
   * Merge all parts that are logically contiguous.
   */
  void MergeSubContainer(){
  }

  //distribution related methods
  int SetSubContainer(const vector<pContainerPart_type>  _ct){
    printf("\n This is -SetSubContainer- method from pBaseGraph ...... NOT YET IMPLEMENTED");
    return OK;
  }

  void BuildSubContainer( const vector<Element_Set_type>&, vector<pContainerPart_type>&){
    printf("\n This is -BuildSubContainer- method from pBaseGraph ...... NOT YET IMPLEMENTED");
  }

  /**@brief 
   *Distribute the pGraph according to the specified datamap.
   *@param map<PID,vector<VID> >& reference to a map that contains redistribution information.
   */
  void CentralDistribute(const map<PID,vector<VID> >& datamap);

  /**@brief 
   *Distribute the pGraph according to the specified datamap.
   *This is a global method and all the threads should call it.
   *@param map<PID,vector<VID> >& reference to a map that contains redistribution information.
   */
  void pGraphDistribute(const map<PID,vector<VID> >& data_map);

  void balance();

  protected:
  void PackVertices(vector<VID> _group, vector<Vertex>& _pg);
  void DeleteVertices(vector<VID> _group);
  void __SetSubContainer(vector<Vertex>& _pg);
  int _local_sz();
  void initializeDistribution();


  public:
  /**@brief 
   *Initialize the cache corresponding to the pgraph's distribution with information about
   *corresponding remote edges;
   */
  void WarmCache();

  protected:
  void _FlushCache();
  //@}
  //==============================================================================
  //      I/O     Methods
  //==============================================================================
  /**@name Input/Output methods */
  //@{
  public:  
  int __pDisplayGraph();

  /**@brief 
   *Display the content of the pGraph.
   */
  void  pDisplayGraph();
  
  /**@brief Display the distribution maps.
   */
  void DisplayMaps(){
    this->dist.DisplayElementLocationMap();
  }
  //@}
};//end pBaseGraph class


//==============================================================================
//                 pBaseGraph methods; Implementation
//==============================================================================

//==============================================================================
//                 constructors and destructor
//==============================================================================
template <class VERTEX, class WEIGHT>
pBaseGraph<VERTEX,WEIGHT>::
pBaseGraph() {
  //printf("Default constructor\n");
  this->register_this(this);
  stapl::rmi_fence();
}

template <class VERTEX, class WEIGHT>
pBaseGraph<VERTEX,WEIGHT>::
pBaseGraph(int size) {
  this->register_this(this);
  stapl::rmi_fence();  
  VERTEX data;
  if(this->myid != (this->nprocs-1)){
    for(int i=0;i<size/this->nprocs;i++){
      this->AddVertex(data,this->GetNextGid(),this->myid);
    }
  }
  else{
    //the last proc will add the remaining vertices
    for(int i=0;i<size - (this->nprocs-1)*size/this->nprocs;i++){
      this->AddVertex(data,this->GetNextGid(),this->myid);
    }
  }
}

template <class VERTEX, class WEIGHT>
pBaseGraph<VERTEX,WEIGHT>::
pBaseGraph(const typename pBaseGraph<VERTEX,WEIGHT>::pBaseGraph_type& _g):BasePContainer_type(_g) {

  //cout<<"Copy constructor"<<endl;
  this->register_this(this);
  stapl::rmi_fence();
  //copy the content of graph _g to "this"  
}

/*
template <class VERTEX, class WEIGHT>
pBaseGraph<VERTEX,WEIGHT>::
pBaseGraph(PartitionInputs* user_inputs){
  //.....................
  this->register_this(this);
  stapl::rmi_fence();
}
*/


template <class VERTEX, class WEIGHT>
pBaseGraph<VERTEX,WEIGHT>::
~pBaseGraph() {
  ///the pBaseGraph is destroyed  by calling the BasePContainer destructor
  this->local_clear();
  rmiHandle handle = this->getHandle();
  if (handle > -1)
    stapl::unregister_rmi_object(handle);
}



//==============================================================================
//      Methods to add, delete vertices
//==============================================================================
template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
AddVertex(VERTEX& _v, VID _vid) {	
  this->AddVertex(_v,_vid,this->myid);
  return OK;
}

template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
AddVertex(VERTEX& _v, VID _vid,int where) {
  if(where != this->myid) {   
    stapl::async_rmi(where,
		     this->getHandle(),
		     &pBaseGraph_type::_AddVertex, _v, _vid);
  }
  else{
    //add it locally
    this->AddElement(_v,_vid);	
  }
  return OK;  
}


template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
AddVertex(VERTEX& _v) {	
  GID _gid = this->GetNextGid();
  if( AddVertex(_v,_gid,this->myid) == OK) return _gid;
  else return -1;
}

template <class VERTEX, class WEIGHT>
void 
pBaseGraph<VERTEX,WEIGHT>::
_AddVertex(VERTEX& _v, VID _vid) {
  this->AddElement(_v,_vid);	
}


template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
AddEdge(VID _v1id, VID _v2id, WEIGHT& _weight, bool  _multi){
  PARTID partid;
  if(this->IsLocal(_v1id,partid)){
    return _iAddEdge(_v1id,_v2id,_weight,partid, _multi);
  }
  else{
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the second vertex of AddEdge");    
    _StaplQuad<VID,VID,WEIGHT,bool> _quad(_v1id, _v2id, _weight,_multi);
    return stapl::sync_rmi<pBaseGraph_type,
	int,
	_StaplQuad<VID,VID,WEIGHT,bool> >(_n.locpid(),
		this->getHandle(),
		(int (pBaseGraph_type::*)(_StaplQuad<VID,VID,WEIGHT,bool>))&pBaseGraph_type::_iAddEdge,
		_quad);
  }
}


template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
AddEdgeAsync(VID _v1id, VID _v2id, WEIGHT& _weight, bool  _multi){
  PARTID partid;
  if(this->IsLocal(_v1id,partid)){
    _iAddEdgeAsync(_v1id,_v2id,_weight,partid, _multi);
  }
  else{
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the second vertex of AddEdge");    
    _StaplQuad<VID,VID,WEIGHT,bool> _quad(_v1id, _v2id, _weight,_multi);
    stapl::async_rmi(_n.locpid(),
		this->getHandle(),
		(void (pBaseGraph_type::*)(_StaplQuad<VID,VID,WEIGHT,bool>))&pBaseGraph_type::_iAddEdgeAsync,
		_quad);
  }
}

template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
AddEdge(VID _v1id, VID _v2id, pair<WEIGHT,WEIGHT>& _weight, bool _multi){
    AddEdge(_v1id,_v2id,_weight.first,_multi);
    AddEdge(_v2id,_v1id,_weight.second,_multi);
    return 1;
}


template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
AddEdgeAsync(VID _v1id, VID _v2id, pair<WEIGHT,WEIGHT>& _weight, bool _multi){
    AddEdgeAsync(_v1id,_v2id,_weight.first,_multi);
    AddEdgeAsync(_v2id,_v1id,_weight.second,_multi);
}


template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
_iAddEdge(_StaplQuad<VID,VID,WEIGHT,bool> _vvw){
  //find the part
  //a partid is comming also as a parameter; try too see how it works

  PARTID partid;
  if(this->IsLocal(_vvw.first , partid)){
    return _iAddEdge(_vvw.first,_vvw.second,_vvw.third, partid,_vvw.fourth);
  }
  else{
    //throw an exception
    cout<<"ERROR:: _iAddEdge() You have reached a remote place but something is wrong"<<endl;
    cout<<"You should add the vertex prior to adding an edge with it as a source"<<endl;
    return ERROR;
  }
}


template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
_iAddEdgeAsync(_StaplQuad<VID,VID,WEIGHT,bool> _vvw){
  
  PARTID partid;
  if(this->IsLocal(_vvw.first , partid)){
    _iAddEdgeAsync(_vvw.first,_vvw.second,_vvw.third, partid,_vvw.fourth);
  }
  else{
    //throw an exception
    cout<<"ERROR:: _iAddEdge() You have reached a remote place but something is wrong"<<endl;
    cout<<"You should add the vertex prior to adding an edge with it as a source"<<endl;
    return ;
  }
}



template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
_iAddEdgeAsync(VID _v1id, VID _v2id, WEIGHT _weight,PARTID _pid,bool  _multi){  
  /*
    The processor in charge of adding the edge should have the following characteristics.
    1. It should have a NON GHOST _v1id local   
    If v2id is local then the normal Edge Addition is done
    else add edge also local but update the cache with the location of the remote 
    _v2id
  */

  //first find the part and the reference inside the part
  pContainerPart_type* p = this->GetPart(_pid);
  
  //next line needs to be changed when the global iterator will be added
  //Graph Part needs to return a global iterator
  part_iterator _vi;

  if(!p->ContainElement(_v1id , &_vi)){
    cout<<"Error:: _iAddEdgeSync vertex one inexistent while trying to add an edge"<<endl;
    return;
  }
  //here vi is initialized with the correct vertex iterator;
  //for correctness I should check if vertex 2 exist
  if(_multi)
    _vi->AddEdge(_v2id,_weight);
  else if(!_vi->IsEdge(_v2id))
    _vi->AddEdge(_v2id,_weight);
}

template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
_iAddEdge(VID _v1id, VID _v2id, WEIGHT _weight,PARTID _pid,bool  _multi){  
  /*
    The processor in charge of adding the edge should have the following characteristics.
    1. It should have a NON GHOST _v1id local   
    If v2id is local then the normal Edge Addition is done
    else add edge also local but update the cache with the location of the remote 
    _v2id
  */

  //first find the part and the reference inside the part
  pContainerPart_type* p = this->GetPart(_pid);
  
  //next line needs to be changed when the global iterator will be added
  //Graph Part needs to return a global iterator
  part_iterator _vi;

  if(!p->ContainElement(_v1id , &_vi)){
    cout<<"Error:: _iAddEdge vertex one inexistent while trying to add an edge"<<endl;
    return -1;
  }
  //here vi is initialized with the correct vertex iterator;
  //for correctness I should check if vertex 2 exist
  if(_multi)
    return _vi->AddEdge(_v2id,_weight);
  else if(!_vi->IsEdge(_v2id))
    return _vi->AddEdge(_v2id,_weight);
  else return ERROR;
}


template <class VERTEX, class WEIGHT>
void 
pBaseGraph<VERTEX,WEIGHT>::
_iDeleteAllEdgesToV(VID _vid){
  pContainerPart_type* p;
  for(int i=0;i<this->GetPartsCount();i++){
    p = this->GetPart(i);
    p->SimpleDeleteAllEdgesToV(_vid);
  }
  // remove from cache
  this->dist.DeleteFromCache(_vid);
}

template <class VERTEX, class WEIGHT>
void 
pBaseGraph<VERTEX,WEIGHT>::
_iDeleteVertex(VID _vid, PID _caller){
  //delete the vertex locally and 
  this->DeleteElement(_vid);

  //the remote edges pointing at it
  //this step is time consuming; look for optimizations
  for(int i=0;i<this->nprocs;i++){
    if(i != _caller){
      stapl::async_rmi(i,this->getHandle(),&pBaseGraph_type::_iDeleteAllEdgesToV,_vid);
    }
  }
}

template <class VERTEX, class WEIGHT>
int 
pBaseGraph<VERTEX,WEIGHT>::
DeleteVertex(VID _vid){
  //first check if _vid is local
  if(this->IsLocal(_vid)){
    _iDeleteVertex(_vid,this->myid);
  }
  else{
    //if remote ask the owner to remove the vertex
    //1) find were it lives
    Location _n=this->Lookup(_vid);

    //next line can be replaced with return ERROR
    if (_n.ValidLocation()) {

      //2) remove local edges pointing at _vid from all the parts
      _iDeleteAllEdgesToV(_vid);

      //3) remove remote
      stapl::async_rmi(_n.locpid(),this->getHandle(),&pBaseGraph_type::_iDeleteVertex,_vid,this->myid);  
    } else {
      return ERROR;
    }
  }
  return OK;
}

template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
DeleteEdge(VID _v1id, VID _v2id){
  PARTID partid;
  pContainerPart_type* p;  
  if(this->IsLocal(_v1id,partid)){
    p = this->GetPart(partid);
    p->DeleteEdge(_v1id,_v2id);
    //!!!!! as an optimization we can remove _v2id from cache if no references to it;
    //this optimization require reference tracking and extra cycles; 
    //maybe latter one
    return OK;
  }
  else{
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in DeleteEdge");
    stapl::async_rmi(_n.locpid(),this->getHandle(),&pBaseGraph_type::DeleteEdge,_v1id,_v2id);
    return OK;
  }
}

template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
DeleteEdgeAndPredecessor_succ(VID _v1id, VID _v2id){
  PARTID partid;
  pContainerPart_type* p;
  typename pContainerPart_type::iterator pit;
  if(this->IsLocal(_v1id,partid)){
    p = this->GetPart(partid);
    //p->DeleteEdge(_v1id,_v2id);
    if(p->IsVertex(_v1id,&pit)){
      pit->DeleteXEdges(_v2id,1);
      pit->SetDependencies(0);
    }
    else{
      stapl_assert(true,"Invalid v1id for the vertex in DeleteEdge");
      return ERROR;
    }
    //!!!!! as an optimization we can remove _v2id from cache if no references to it;
    //this optimization require reference tracking and extra cycles; 
    //maybe latter one
    return OK;
  }
  else{
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in DeleteEdge");
    stapl::async_rmi(_n.locpid(),this->getHandle(),&pBaseGraph_type::DeleteEdgeAndPredecessor_succ,_v1id,_v2id);
    return OK;
  }
}

template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
DeleteEdgeAndPredecessor_preds(VID _v1id, VID _v2id){
  PARTID partid;
  pContainerPart_type* p;
  typename pContainerPart_type::iterator pit;
  if(this->IsLocal(_v2id,partid)){
    p = this->GetPart(partid);
    //p->DeleteEdge(_v1id,_v2id);
    if(p->IsVertex(_v2id,&pit)){
      pit->DeleteXPredEdges(_v1id,1);
      pit->SetDependencies(0);
    }
    else{
      stapl_assert(true,"Invalid v1id for the vertex in DeleteEdgeAndPredecessor_preds");
      return ERROR;
    }
    //!!!!! as an optimization we can remove _v2id from cache if no references to it;
    //this optimization require reference tracking and extra cycles; 
    //maybe latter one
    return OK;
  }
  else{
    Location _n=this->Lookup(_v2id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in DeleteEdge");
    stapl::async_rmi(_n.locpid(),this->getHandle(),&pBaseGraph_type::DeleteEdgeAndPredecessor_preds,_v1id,_v2id);
    return OK;
  }
}

template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
DeleteEdgeAndPredecessor(VID _v1id, VID _v2id){
    if(DeleteEdgeAndPredecessor_succ( _v1id, _v2id) != OK) return ERROR;
    if(DeleteEdgeAndPredecessor_preds( _v1id, _v2id) != OK) return ERROR;
    return OK;
}


template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
_DeleteWtEdge(_StaplTriple<VID,VID,WEIGHT> _args){
  PARTID partid;
  pContainerPart_type* p;  
  if(this->IsLocal(_args.first,partid)){
    p = this->GetPart(partid);
    p->DeleteEdge(_args.first,_args.second,_w);
  }
  else{
    cout<<"received message to delete an edge for an inexistent vertex"<<endl;
  }
}

template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
DeleteWtEdge(VID _v1id, VID _v2id,WEIGHT& _w){
  PARTID partid;
  pContainerPart_type* p;  
  if(this->IsLocal(_v1id,partid)){
    p = this->GetPart(partid);
    return p->DeleteEdge(_v1id,_v2id,_w);
  }
  else{
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in DeleteEdge");
    _StaplTriple<VID,VID,WEIGHT> args(_v1id,_v2id,_w);
    stapl::async_rmi(_n.locpid(),
		     this->getHandle(),
		     &pBaseGraph_type::_DeleteWtEdge,args);
    return OK;
  }
}


template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
DeleteEdgeId(VID _v1id, int _edgeid){
  PARTID partid;
  pContainerPart_type* p;  
  if(this->IsLocal(_v1id,partid)){
    p = this->GetPart(partid);
    p->DeleteEdgeId(_v1id,_edgeid);
    //!!!!! as an optimization we can remove _v2id from cache if no references to it;
    //this optimization require reference tracking and extra cycles; 
    //maybe latter one
    return OK;
  }
  else{
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in DeleteEdge");
    stapl::async_rmi(_n.locpid(),this->getHandle(),&pBaseGraph_type::DeleteEdgeId,_v1id,_edgeid);
    return OK;
  }
}

template <class VERTEX, class WEIGHT>
void 
pBaseGraph<VERTEX,WEIGHT>::
ErasePGraph(){
  for(int i =0;i<this->GetPartsCount();i++){
    this->pcontainer_parts[i]->EraseGraph();
  }
  this->dist.FlushCache();
  this->dist.FlushLocationMap();
  this->dist.FlushPartsMap();
}


//==============================================================================
//      Methods to get references to vertices/edges datastructures
//==============================================================================
template <class VERTEX, class WEIGHT>
bool
pBaseGraph<VERTEX,WEIGHT>::
IsVertex(GID _gid){
  Location _n=this->Lookup(_gid);
  return _n.ValidLocation();
}

template <class VERTEX, class WEIGHT>
bool
pBaseGraph<VERTEX,WEIGHT>::
IsVertex(GID _gid, VI* _pit){
  //first get part where _gid lives
#ifndef _STAPL_PGRAPH_ONEPART
  PARTID _partid;
  if(this->dist.IsLocal(_gid,_partid)){
    //_pid is already initialized with the part where _gid lives
    stapl_assert(_partid >= 0,"invalid part id");
    part_iterator _vi;
    if(this->pcontainer_parts[_partid]->ContainElement(_gid,&_vi)){
      *_pit = VI(this, this->pcontainer_parts.begin() + _partid, _vi);      
      return true;
    }
    else return false;
  }
  return false;
#else
  //there is only one part
  part_iterator _vi;
  if(this->pcontainer_parts[0]->IsVertex(_gid,&_vi)){
    *_pit = VI(this, this->pcontainer_parts.begin(), _vi);      
    return true;
  }
  else return false;
#endif
}


template <class VERTEX, class WEIGHT>
bool
pBaseGraph<VERTEX,WEIGHT>::
IsVertex(GID _gid, CVI* _cpit) const {
  //first get part where _gid lives
#ifndef _STAPL_PGRAPH_ONEPART
  PARTID _partid;
  VI* _pit = const_cast<VI*>(_cpit);
  if(this->IsLocal(_gid,_partid)){
    //_pid is already initialized with the part where _gid lives
    stapl_assert(_partid >= 0,"invalid part id");
    part_iterator _vi;
    if(this->pcontainer_parts[_partid]->ContainElement(_gid,&_vi)){
      //here I have to setup  the parallel iterator to point at _vi
      pBaseGraph_type* temp = const_cast<pBaseGraph_type*>(this);
      *_pit = VI(temp,temp->pcontainer_parts.begin()+_partid , _vi);
      return true;
    }
    else return false;
  }
  return false;
#else
  part_iterator _vi;
  VI* _pit = const_cast<VI*>(_cpit);
  if(this->pcontainer_parts[0]->IsVertex(_gid,&_vi)){
    //here I have to setup  the parallel iterator to point at _vi
    pBaseGraph_type* temp = const_cast<pBaseGraph_type*>(this);
    *_pit = VI(temp,temp->pcontainer_parts.begin(), _vi);
    return true;
  }
  else return false;
#endif
}

template <class VERTEX, class WEIGHT>
GID
pBaseGraph<VERTEX,WEIGHT>::
_GetVID(VERTEX& _v){
  part_iterator _vi;
  parts_internal_iterator pii = this->pcontainer_parts.begin();
  parts_internal_iterator pii_end = this->pcontainer_parts.end();
  while(pii != pii_end){
    if((*pii)->ContainElement(_v, &_vi)){
      return _vi->vid;
    }
    ++pii;
  }
  return -1;
}

template <class VERTEX, class WEIGHT>
GID
pBaseGraph<VERTEX,WEIGHT>::
GetVID(VERTEX& _v){
  VID _gid = _GetVID(_v);
  if(_gid != -1) return _gid;
  //if we are here it means that VERTEX is not local; 
  //we have to go remote and do a complete search
  for(int i=0;i<this->nprocs; ++i){
    if(i != this->myid)
      _gid = stapl::sync_rmi(i,
			     this->getHandle(),
			     &pBaseGraph_type::_GetVID,
			     _v);
    if(_gid != -1) return _gid;
  }
  return -1;
}

template <class VERTEX, class WEIGHT>
bool
pBaseGraph<VERTEX,WEIGHT>::
IsVertex(VERTEX& _v, VI* _pit){
   //first get part where _gid lives
#ifndef _STAPL_PGRAPH_ONEPART
  part_iterator _vi;
  parts_internal_iterator pii = this->pcontainer_parts.begin();
  parts_internal_iterator pii_end = this->pcontainer_parts.end();
  while(pii != pii_end){
    if((*pii)->ContainElement(_v, &_vi)){
      *_pit = VI(this, pii, _vi);
      return true;
    }
    ++pii;
  }
#else
  //there is only one part
  part_iterator _vi;
  if(this->pcontainer_parts[0]->ContainElement(_v,&_vi)){
    *_pit = VI(this, this->pcontainer_parts.begin(), _vi);      
    return true;
  }
  else return false;
#endif
}


template <class VERTEX, class WEIGHT>
bool
pBaseGraph<VERTEX,WEIGHT>::
IsVertex(VERTEX& _v, CVI* _cpit) const {

  VI* _pit = const_cast<VI*>(_cpit);

#ifndef _STAPL_PGRAPH_ONEPART
  part_iterator _vi;
  for(int _partid = 0; _partid < this->pcontainer_parts.size();_partid++){
    if(this->pcontainer_parts[_partid]->ContainElement(_v, &_vi)){
      pBaseGraph_type* temp = const_cast<pBaseGraph_type*>(this);
      *_pit = VI(temp,temp->pcontainer_parts.begin()+_partid , _vi);
      return true;
    }
  }
#else
  //there is only one part
  part_iterator _vi;
  if(this->pcontainer_parts[0]->ContainElement(_v,&_vi)){
    pBaseGraph_type* temp = const_cast<pBaseGraph_type*>(this);
    *_pit = VI(temp,temp->pcontainer_parts.begin()+_partid , _vi);
    return true;
  }
  else return false;
#endif
}

template <class VERTEX, class WEIGHT>
bool
pBaseGraph<VERTEX,WEIGHT>::
IsVertex(VERTEX& _v){
  if(this->GetVID(_v) != -1) return true;
  else return false;
}

template <class VERTEX, class WEIGHT>
bool 
pBaseGraph<VERTEX,WEIGHT>::
_IsEdge(VID _v1id, VID _v2id, VI* _vit, EI* _ei,PARTID _pid){
  stapl_assert(_pid >= 0,"invalid part id");
  pContainerPart_type* p = this->GetPart(_pid);
  /*   typename pContainerPart_type::iterator _vi; To compile with aCC*/
  part_iterator _vi;
  bool temp = p->IsEdge(_v1id,_v2id,&_vi,_ei);
  if (temp == true){
     *_vit = VI(this, this->pcontainer_parts.begin() + _pid, _vi);
  }
  return temp;
}

template <class VERTEX, class WEIGHT>
bool 
pBaseGraph<VERTEX,WEIGHT>::
IsEdge(VID _v1id, VID _v2id){
  PARTID _pid;
  if(this->IsLocal(_v1id,_pid)){
    VI _vi;
    EI _ei;
    return _IsEdge(_v1id,_v2id,&_vi,&_ei,_pid);
  }
  else{
    //the edge is remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pBaseGraph::IsEdge");
    return stapl::sync_rmi<pBaseGraph_type,bool,VID,VID>(_n.locpid(),
			   this->getHandle(),
			   (bool (pBaseGraph_type::*)(VID,VID))&pBaseGraph_type::IsEdge,
			   _v1id,_v2id);
  }
}

template <class VERTEX, class WEIGHT>
bool 
pBaseGraph<VERTEX,WEIGHT>::
_IsEdge(VID _v1id, VID _v2id, WEIGHT& _weight, VI* _vit, EI* _ei,PARTID _pid){
  stapl_assert(_pid >= 0,"invalid part id");
  pContainerPart_type* p = this->GetPart(_pid);
  /*   typename pContainerPart_type::iterator _vi; To compile with aCC*/
  part_iterator _vi;
  bool temp = p->IsEdge(_v1id,_v2id,_weight, &_vi,_ei);
  if (temp == true){
     *_vit = VI(this, this->pcontainer_parts.begin() + _pid, _vi);
  }
  return temp;
}

template <class VERTEX, class WEIGHT>
bool 
pBaseGraph<VERTEX,WEIGHT>::
IsEdge(VID _v1id, VID _v2id, WEIGHT& _weight, VI* _vit, EI* _ei){
  PARTID _pid;
  if(this->IsLocal(_v1id,_pid)){
    return _IsEdge(_v1id,_v2id,_weight, _vit,_ei,_pid);
  }
}


template <class VERTEX, class WEIGHT>
bool 
pBaseGraph<VERTEX,WEIGHT>::
_IsEdge(_StaplTriple<VID,VID,WEIGHT>& _args){
  return IsEdge(_args.first, _args.second, _args.third);
}

template <class VERTEX, class WEIGHT>
bool 
pBaseGraph<VERTEX,WEIGHT>::
IsEdge(VID _v1id, VID _v2id, WEIGHT& _weight){
  PARTID _pid;
  if(this->IsLocal(_v1id,_pid)){
    VI _vi;
    EI _ei;
    return _IsEdge(_v1id,_v2id,_weight, &_vi,&_ei,_pid);
  }
  else{
    //the edge is remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pBaseGraph::IsEdge");
    _StaplTriple<VID,VID,WEIGHT> args(_v1id,_v2id,_weight);
    return stapl::sync_rmi(_n.locpid(),
			   this->getHandle(),
			   (bool (pBaseGraph_type::*)(_StaplTriple<VID,VID,WEIGHT>&))&pBaseGraph_type::_IsEdge,
			   args);
  }
}


template <class VERTEX, class WEIGHT>
bool 
pBaseGraph<VERTEX,WEIGHT>::
IsEdge(VID _v1id, VID _v2id, VI* _vit, EI* _ei){
  PARTID _pid;
  if(this->IsLocal(_v1id,_pid)){
    return _IsEdge(_v1id,_v2id,_vit,_ei,_pid);
  }
  return false;
}


template <class VERTEX, class WEIGHT>
bool 
pBaseGraph<VERTEX,WEIGHT>::
_IsEdgeId(VID _v1id, int _edgeid, VI* _vit, EI* _ei,PARTID _pid){
  stapl_assert(_pid >= 0,"invalid part id");
  pContainerPart_type* p = this->GetPart(_pid);
/*   typename pContainerPart_type::iterator _vi; To compile with aCC*/
  part_iterator _vi;
  bool temp = p->IsEdgeId(_v1id,_edgeid,&_vi,_ei);
  if (temp == true){
    *_vit = VI(this, this->pcontainer_parts.begin() + _pid, _vi);      
  }
  return temp;
}

template <class VERTEX, class WEIGHT>
bool 
pBaseGraph<VERTEX,WEIGHT>::
IsEdgeId(VID _v1id, int _edgeid, VI* _vit, EI* _ei){
  PARTID _pid;
  if(this->IsLocal(_v1id,_pid)){
    return _IsEdgeId(_v1id,_edgeid,_vit,_ei,_pid);
  }
  return false;
}


template <class VERTEX, class WEIGHT>
bool 
pBaseGraph<VERTEX,WEIGHT>::
IsEdge(VERTEX& _v1, VERTEX& _v2){
  VID gid1 = GetVID(_v1);
  VID gid2 = GetVID(_v2);
  return IsEdge(gid1,gid2);
}

template <class VERTEX, class WEIGHT>
bool 
pBaseGraph<VERTEX,WEIGHT>::
IsEdge(VERTEX& _v1, VERTEX& _v2, VI* _vit, EI* _ei){
  VID gid1 = GetVID(_v1);
  VID gid2 = GetVID(_v2);
  return IsEdge(gid1,gid2,_vit,_ei);
}

template <class VERTEX, class WEIGHT>
VERTEX
pBaseGraph<VERTEX,WEIGHT>::
_GetData(VID _vid){
  VI vi;
  if(IsVertex(_vid,&vi)){
    //if local
    return vi->data;
  }
  else{
    //ERROR
    stapl_assert(1,"Invalid id for the vertex in pBaseGraph::_GetData");
    //The return statement is just to satisfy compilers that would complain about the method returning nothing.  It should not be executed.
    return vi->data;
  }
}

template <class VERTEX, class WEIGHT>
VERTEX
pBaseGraph<VERTEX,WEIGHT>::
GetData(VID _vid){
  VI vi;
  if(IsVertex(_vid,&vi)){
    //if local
    return vi->data;
  }
  else{
    //remote
    Location _n=this->Lookup(_vid);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pBaseGraph::GetData");
    return stapl::sync_rmi(_n.locpid(),
			   this->getHandle(),
			   &pBaseGraph_type::_GetData,
			   _vid);
  }
}

template <class VERTEX, class WEIGHT>
WEIGHT
pBaseGraph<VERTEX,WEIGHT>::
_GetEdgeWeight(VID _vid1, VID _vid2){
  VI vi;
  EI ei;
  if(IsEdge(_vid1,_vid2,&vi,&ei)){
    return ei->weight;
  }
  else{
    return WEIGHT();
  }
}

template <class VERTEX, class WEIGHT>
WEIGHT
pBaseGraph<VERTEX,WEIGHT>::
GetEdgeWeight(VID _vid1, VID _vid2){
  VI vi;
  EI ei;
  if(IsEdge(_vid1,_vid2,&vi,&ei)){
    return ei->weight;
  }
  else{
    //remote
    Location _n=this->Lookup(_vid1);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pBaseGraph::GetData");
    return stapl::sync_rmi(_n.locpid(),
			   this->getHandle(),
			   &pBaseGraph_type::_GetEdgeWeight,
			   _vid1,_vid2);
  }
}

template <class VERTEX, class WEIGHT>
VERTEX&
pBaseGraph<VERTEX,WEIGHT>::
GetUserData(VI _vi) {
  return *_vi;
}

template <class VERTEX, class WEIGHT>
VID
pBaseGraph<VERTEX,WEIGHT>::
getVID(const VERTEX&){
  cout<<"The methods with user data as arguments are not implemented yet"<<endl;
  return -1;
}


//==============================================================================
//      Methods to access graph datastructure(Adjacency list)
//==============================================================================
template <class VERTEX, class WEIGHT>
vector<VID>
pBaseGraph<VERTEX,WEIGHT>::
_GetAdjacentVertices(VID _v1id) const {
  CVI v1;
  int sz=0; 
  vector<VID> _succ;
  if ( IsVertex(_v1id,&v1) ) {
    _succ.resize(v1->edgelist.size());
    for (CEI ei = v1->edgelist.begin(); ei != v1->edgelist.end(); ++ei) {
      _succ[sz++] = ei->vertex2id;
    }
  }
  return _succ;
}

template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
GetAdjacentVertices(VID _v1id, vector<VID>& _succ) /*const*/ {
  VI v1;    
  if ( IsVertex(_v1id,&v1) ) {
    return GetAdjacentVertices(v1,_succ);
  } else {
    Location _n=this->Lookup(_v1id);
    if(_n.ValidLocation()){
      _succ = stapl::sync_rmi(_n.locpid(),
			      this->getHandle(),
			      &pBaseGraph_type::_GetAdjacentVertices,_v1id);
      return _succ.size();
    }
    else return 0;
  }
}
 
template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
GetAdjacentVertices(VI _vi, vector<VID>& _succ) const {
  int sz = _vi->edgelist.size();
  if(sz > _succ.size()) _succ.resize(sz);
  sz = 0;
  for (CEI ei = _vi->edgelist.begin(); ei != _vi->edgelist.end(); ++ei) {
    _succ[sz++] = ei->vertex2id;
  }
  return sz;
}

//--------------------------------------------
template <class VERTEX, class WEIGHT>
vector<VERTEX>
pBaseGraph<VERTEX,WEIGHT>::
_GetAdjacentVerticesDATA(VID _v1id) /*const*/ {
  VI v1;
  int sz=0; 
  vector<VERTEX> _succ;
  if ( IsVertex(_v1id,&v1) ) {
    _succ.resize(v1->edgelist.size());
    for (CEI ei = v1->edgelist.begin(); ei != v1->edgelist.end(); ++ei) {
      _succ[sz++] = GetData(ei->vertex2id);
    }
  }
  return _succ;
}

template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
GetAdjacentVerticesDATA(VID _v1id, vector<VERTEX>& _succ) /*const*/ {
  VI v1;    
  if ( IsVertex(_v1id,&v1) ) {
    return GetAdjacentVerticesDATA(v1,_succ);
  } else {
    Location _n=this->Lookup(_v1id);
    if(_n.ValidLocation()){
      _succ = stapl::sync_rmi(_n.locpid(),
			      this->getHandle(),
			      &pBaseGraph_type::_GetAdjacentVerticesDATA,_v1id);
      return _succ.size();
    }
    else return 0;
  }
}
 
template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
GetAdjacentVerticesDATA(VI _vi, vector<VERTEX>& _succ) /*const*/ {
  int sz = _vi->edgelist.size();
  if(sz > _succ.size()) _succ.resize(sz);
  sz = 0;
  for (CEI ei = _vi->edgelist.begin(); ei != _vi->edgelist.end(); ++ei) {
    _succ[sz++] = this->GetData(ei->vertex2id);
  }
  return sz;
}

//----------------------------------------------------
template <class VERTEX, class WEIGHT>
vector<VID>
pBaseGraph<VERTEX,WEIGHT>::
_GetPredVertices(VID _v1id) const {
  VI v1;
  int sz=0; 
  vector<VID> _preds;
  if ( IsVertex(_v1id,&v1) ) {
    _preds.resize(v1->predecessors.size());
    for (CEI ei = v1->predecessors.begin(); ei != v1->predecessors.end(); ++ei) {
      _preds[sz++] = ei->vertex2id;
    }
  }
  return _preds;
}

template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
GetPredVertices(VID _v1id, vector<VID>& _preds) /*const*/ {
  VI v1;    
  int sz=0; 
  if ( IsVertex(_v1id,&v1) ) {
    return GetPredVertices(v1,_preds);
  } else {
    Location _n=this->Lookup(_v1id);
    if(_n.ValidLocation()){
      _preds = stapl::sync_rmi(_n.locpid(),
			       this->getHandle(),
			       &pBaseGraph_type::_GetPredVertices,_v1id);
      return _preds.size();
    }
    else return 0;
  }
}
 

template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
GetPredVertices(VI _vi, vector<VID>& _preds) const {
  int sz = _vi->predecessors.size();
  if(sz > _preds.size()) _preds.resize(sz);
  sz = 0;
  for (CEI ei = _vi->predecessors.begin(); ei != _vi->predecessors.end(); ei++) {
    _preds[sz++] = ei->vertex2id;
  }
  return sz;
}
//------------------
template <class VERTEX, class WEIGHT>
vector<VERTEX>
pBaseGraph<VERTEX,WEIGHT>::
_GetPredVerticesDATA(VID _v1id) /*const*/ {
  VI v1;
  int sz=0; 
  vector<VERTEX> _preds;
  if ( IsVertex(_v1id,&v1) ) {
    _preds.resize(v1->predecessors.size());
    for (CEI ei = v1->predecessors.begin(); ei != v1->predecessors.end(); ++ei) {
      _preds[sz++] = this->GetData(ei->vertex2id);
    }
  }
  return _preds;
}

template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
GetPredVerticesDATA(VID _v1id, vector<VERTEX>& _preds) /*const*/ {
  VI v1;    
  int sz=0; 
  if ( IsVertex(_v1id,&v1) ) {
    return GetPredVerticesDATA(v1,_preds);
  } else {
    Location _n=this->Lookup(_v1id);
    if(_n.ValidLocation()){
      _preds = stapl::sync_rmi(_n.locpid(),
			       this->getHandle(),
			       &pBaseGraph_type::_GetPredVerticesDATA,_v1id);
      return _preds.size();
    }
    else return 0;
  }
}
 

template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
GetPredVerticesDATA(VI _vi, vector<VERTEX>& _preds) /*const*/ {
  int sz = _vi->predecessors.size();
  if(sz > _preds.size()) _preds.resize(sz);
  sz = 0;
  for (CEI ei = _vi->predecessors.begin(); ei != _vi->predecessors.end(); ei++) {
    _preds[sz++] = this->GetData(ei->vertex2id);
  }
  return sz;
}

//----------------------------------------------------

template <class VERTEX, class WEIGHT>
int 
pBaseGraph<VERTEX,WEIGHT>::
GetVertices(vector<VID>& _verts) const{
  _verts.clear();
  _verts.resize(this->local_size());
  int counter=0;
  for(int p=0;p<this->GetPartsCount();p++){
    pContainerPart_type* pp = this->pcontainer_parts[p];
    for(typename pContainerPart_type::iterator vi=pp->begin();vi != pp->end();vi++){
      _verts[counter++] = vi->vid;
    }
  }
  return _verts.size();
}

template <class VERTEX, class WEIGHT>
int 
pBaseGraph<VERTEX,WEIGHT>::
GetVertices(vector<VI>& _verts) {
  _verts.clear();
  _verts.resize(local_size());
  int counter=0;
  typename  BasePContainer_type::const_parts_internal_iterator pp;
  for(pp=this->parts_begin();pp!=this->parts_end();pp++){
    for(typename pContainerPart_type::iterator vi=(*pp)->begin();vi != (*pp)->end();vi++){
      _verts[counter]=VI(this, pp, vi);
      counter++;
    }
  }
  return _verts.size();
}

template <class VERTEX, class WEIGHT>
int 
pBaseGraph<VERTEX,WEIGHT>::
GetVertices(vector<VERTEX>& _verts) const{
  _verts.clear();
  _verts.resize(local_size());
  int counter=0;

  for(int p=0;p<this->GetPartsCount();p++){
    pContainerPart_type* pp = this->pcontainer_parts[p];
    for(typename pContainerPart_type::iterator vi=pp->begin();vi != pp->end();vi++){
      _verts[counter] =  vi->data;
      counter++;
    }
  }
  return _verts.size();
}

template <class VERTEX, class WEIGHT>
int 
pBaseGraph<VERTEX,WEIGHT>::
GetEdges(vector<pair<VID,VID> >& _edges) const{
 
  cout<<"GetEdges need to be optimized"<<endl;
  _edges.clear();
 
  //                   !!!!!!!!!!!!     TO BE DONE
  //this implementation is not the optimal one; don't use push_back
  //better compute first the size of edges and after that add edges

  //_edges.resize(local_size());
  //int counter=0;
  for(int p=0;p<this->GetPartsCount();p++){
    pContainerPart_type* pp = this->pcontainer_parts[p];
    for(typename pContainerPart_type::iterator vi=pp->begin();vi != pp->end();vi++){
      for(EI ei=vi->edgelist.begin();ei!=vi->edgelist.end();ei++){
	_edges.push_back(pair<VID,VID>(vi->vid,ei->vertex2id));
      }
    }
  }
  return _edges.size();
}

template <class VERTEX, class WEIGHT>
int 
pBaseGraph<VERTEX,WEIGHT>::
GetEdges(vector<pair<pair<VID,VID>,WEIGHT> >& _edges) const{

  cout<<"GetEdges need to be optimized"<<endl;
  _edges.clear();
  //                   !!!!!!!!!!!!     TO BE DONE
  //this implementation is not the optimal one; don't use push_back
  //better compute first the size of edges and after that add edges

  //_edges.resize(local_size());
  //int counter=0;
  for(int p=0;p<this->GetPartsCount();p++){
    pContainerPart_type* pp = this->pcontainer_parts[p];
    for(typename pContainerPart_type::iterator vi=pp->begin();vi != pp->end();vi++){
      for(EI ei=vi->edgelist.begin();ei!=vi->edgelist.end();ei++){
	_edges.push_back(
			 pair<pair<VID,VID>,WEIGHT> (
			       pair<VID,VID>(vi->vid,ei->vertex2id),ei->weight)
			 );
      }
    }
  }
  return _edges.size();
}

template <class VERTEX, class WEIGHT>
int 
pBaseGraph<VERTEX,WEIGHT>::
GetVertexOutDegree(VID _v1id){
  VI v1;
  if ( IsVertex(_v1id,&v1) ) {
    return v1->edgelist.size();
  } else {
    Location _n=this->Lookup(_v1id);
    if(_n.ValidLocation()){
      return  stapl::sync_rmi(_n.locpid(),
			      this->getHandle(),
			      &pBaseGraph_type::GetVertexOutDegree,_v1id);
    }
    else return -1;
  }  
}
 

template <class VERTEX, class WEIGHT>
vector< pair<VID,VID> >
pBaseGraph<VERTEX,WEIGHT>::
_GetOutgoingEdges(VID _v1id){
  VI v1;
  vector< pair<VID,VID> > iedges;
  if ( IsVertex(_v1id,&v1) ) {
    iedges.reserve( v1->edgelist.size() );
      for (CEI ei = v1->edgelist.begin(); ei != v1->edgelist.end(); ++ei) {
	pair<VID,VID> nextedge(_v1id,ei->vertex2id);
	//pair<pair<VID,VID>,WEIGHT> nextedgewt(nextedge,ei->weight);
	iedges.push_back( nextedge );
      }
  } else {
    cout << "\nGetOutgoingEdges: vertex "<< _v1id << " not in graph";
  }
  return iedges;
}

template <class VERTEX, class WEIGHT>
int 
pBaseGraph<VERTEX,WEIGHT>::
GetOutgoingEdges(VID _v1id,vector< pair<VID,VID> >& iedges){
  VI v1;    
  iedges.clear();
  if ( IsVertex(_v1id,&v1) ) {
    iedges.reserve( v1->edgelist.size() );
    for (CEI ei = v1->edgelist.begin(); ei != v1->edgelist.end(); ++ei) {
      pair<VID,VID> nextedge(_v1id,ei->vertex2id);
      //pair<pair<VID,VID>,WEIGHT> nextedgewt(nextedge,ei->weight);
      iedges.push_back( nextedge );
    }
    return iedges.size();
  } else {
    Location _n=this->Lookup(_v1id);
    if(_n.ValidLocation()){
      iedges = stapl::sync_rmi(_n.locpid(),
			       this->getHandle(),
			       &pBaseGraph_type::_GetOutgoingEdges,_v1id);
      return iedges.size();
    }
    else return 0;
  }
}


template <class VERTEX, class WEIGHT>
vector< pair<pair<VID,VID>,WEIGHT> >
pBaseGraph<VERTEX,WEIGHT>::
_GetOutgoingEdgesWeight(VID _v1id){
  VI v1;
  vector< pair<pair<VID,VID>,WEIGHT> > iedges;
  if ( IsVertex(_v1id,&v1) ) {
    iedges.reserve( v1->edgelist.size() );
      for (CEI ei = v1->edgelist.begin(); ei != v1->edgelist.end(); ++ei) {
	pair<VID,VID> nextedge(_v1id,ei->vertex2id);
	pair<pair<VID,VID>,WEIGHT> nextedgewt(nextedge,ei->weight);
	iedges.push_back( nextedgewt );
      }
    } else {
      cout << "\nGetOutgoingEdges: vertex "<< _v1id << " not in graph";
    }
  return iedges;
}

template <class VERTEX, class WEIGHT>
int 
pBaseGraph<VERTEX,WEIGHT>::
GetOutgoingEdges(VID _v1id,vector< pair<pair<VID,VID>,WEIGHT> >& iedges) /*const*/ {
  VI v1;    
  iedges.clear();
  if ( IsVertex(_v1id,&v1) ) {
    iedges.reserve( v1->edgelist.size() );
    for (CEI ei = v1->edgelist.begin(); ei != v1->edgelist.end(); ++ei) {
      pair<VID,VID> nextedge(_v1id,ei->vertex2id);
      pair<pair<VID,VID>,WEIGHT> nextedgewt(nextedge,ei->weight);
      iedges.push_back( nextedgewt );
    }
    return iedges.size();
  } else {
    Location _n=this->Lookup(_v1id);
    if(_n.ValidLocation()){
      iedges = stapl::sync_rmi(_n.locpid(),
			       this->getHandle(),
			       &pBaseGraph_type::_GetOutgoingEdgesWeight,_v1id);
      return iedges.size();
    }
    else return 0;
  }
}


template <class VERTEX, class WEIGHT>
vector< pair<VERTEX,VERTEX> >
pBaseGraph<VERTEX,WEIGHT>::
_GetOutgoingEdgesVData(VID _v1id){
  VI v1;
  vector< pair<VERTEX,VERTEX> > iedges;
  if ( IsVertex(_v1id,&v1) ) {
    iedges.reserve( v1->edgelist.size() );
      for (CEI ei = v1->edgelist.begin(); ei != v1->edgelist.end(); ++ei) {
	pair<VERTEX,VERTEX> nextedge(v1->data,this->GetData(ei->vertex2id));
	//pair<pair<VID,VID>,WEIGHT> nextedgewt(nextedge,ei->weight);
	iedges.push_back( nextedge );
      }
    } else {
      cout << "\nGetOutgoingEdges: vertex "<< _v1id << " not in graph";
    }
  return iedges;
}

template <class VERTEX, class WEIGHT>
int 
pBaseGraph<VERTEX,WEIGHT>::
GetOutgoingEdgesVData(VID _v1id, vector< pair<VERTEX,VERTEX> >& iedges){
  VI v1;    
  iedges.clear();
  if ( IsVertex(_v1id,&v1) ) {
    iedges.reserve( v1->edgelist.size() );
    for (CEI ei = v1->edgelist.begin(); ei != v1->edgelist.end(); ++ei) {
      pair<VERTEX,VERTEX> nextedge(v1->data,this->GetData(ei->vertex2id));
      //pair<pair<VID,VID>,WEIGHT> nextedgewt(nextedge,ei->weight);
      iedges.push_back( nextedge );
    }
    return iedges.size();
  } else {
    Location _n=this->Lookup(_v1id);
    if(_n.ValidLocation()){
      iedges = stapl::sync_rmi(_n.locpid(),
			       this->getHandle(),
			       &pBaseGraph_type::_GetOutgoingEdgesVData,_v1id);
      return iedges.size();
    }
    else return 0;
  }
}


template <class VERTEX, class WEIGHT>
vector< pair<pair<VERTEX,VERTEX>,WEIGHT> >
pBaseGraph<VERTEX,WEIGHT>::
_GetOutgoingEdgesWeightVData(VID _v1id){
  VI v1;
  vector< pair<pair<VERTEX,VERTEX>,WEIGHT> > iedges;
  if ( IsVertex(_v1id,&v1) ) {
    iedges.reserve( v1->edgelist.size() );
      for (CEI ei = v1->edgelist.begin(); ei != v1->edgelist.end(); ++ei) {
	pair<VERTEX,VERTEX> nextedge(v1->data,this->GetData(ei->vertex2id));
	pair<pair<VERTEX,VERTEX>,WEIGHT> nextedgewt(nextedge,ei->weight);
	iedges.push_back( nextedgewt );
      }
    } else {
      cout << "\nGetOutgoingEdges: vertex "<< _v1id << " not in graph";
    }
  return iedges;
}


template <class VERTEX, class WEIGHT>
int 
pBaseGraph<VERTEX,WEIGHT>::
GetOutgoingEdgesVData(VID _v1id,vector< pair<pair<VERTEX,VERTEX>,WEIGHT> >& iedges) /*const*/ {
  VI v1;    
  iedges.clear();
  if ( IsVertex(_v1id,&v1) ) {
    iedges.reserve( v1->edgelist.size() );
    for (CEI ei = v1->edgelist.begin(); ei != v1->edgelist.end(); ++ei) {
      pair<VERTEX,VERTEX> nextedge(v1->data,this->GetData(ei->vertex2id));
      pair<pair<VERTEX,VERTEX>,WEIGHT> nextedgewt(nextedge,ei->weight);
      iedges.push_back( nextedgewt );
    }
    return iedges.size();
  } else {
    Location _n=this->Lookup(_v1id);
    if(_n.ValidLocation()){
      iedges = stapl::sync_rmi(_n.locpid(),
			       this->getHandle(),
			       &pBaseGraph_type::_GetOutgoingEdgesWeightVData,_v1id);
      return iedges.size();
    }
    else return 0;
  }
}

/*
template <class VERTEX, class WEIGHT>
size_t 
pBaseGraph<VERTEX,WEIGHT>::
size() const {
  //this can be done with an rmi_reduce if all the threads are calling
  //symultaneous; If not I'll call explicit local_size on all the other threads;  
  size_t temp = 0;
  temp += local_size();
  for(int where=0;where < this->nprocs;where++){
    if(where != this->myid)    
      temp += stapl::sync_rmi(where,
			      this->getHandle(),
			      (void (pBaseGraph_type::*)() const)&pBaseGraph_type::_size);
      temp+=0;
  }
  return temp;
}

template <class VERTEX, class WEIGHT>  
size_t 
pBaseGraph<VERTEX,WEIGHT>::
_size() const {
  return this->local_size();
}

template <class VERTEX, class WEIGHT>  
size_t 
pBaseGraph<VERTEX,WEIGHT>::
local_size() const  {
  size_t temp=0;
  for(int i=0;i<this->GetPartsCount();i++){
    temp+=this->pcontainer_parts[i]->size();
  }
  return temp;
}
*/

template <class VERTEX, class WEIGHT>  
size_t 
pBaseGraph<VERTEX,WEIGHT>::
GetLocalEdgeCount() /*const*/ {
  size_t count = 0;
  // ??? check next lines
  //for(int i=0;i<this->GetPartsCount();++i){
  //count += this->pcontainer_parts[i]->GetEdgeCount();
  //}
  for(VI vi = local_begin(); vi!=local_end();++vi)
    count += vi->edgelist.size();
  return count;
}

template <class VERTEX, class WEIGHT>  
size_t 
pBaseGraph<VERTEX,WEIGHT>::
GetEdgeCount() /*const*/{
  size_t _s=0;
  for(int i=0; i<nprocs; ++i){
    if(myid == i) _s += GetLocalEdgeCount();
    else
      _s+=stapl::sync_rmi(i,getHandle(),
			  (size_t (pBaseGraph<VERTEX,WEIGHT>::*)())&pBaseGraph<VERTEX,WEIGHT>::GetLocalEdgeCount);
  }
  return _s;
}

template <class VERTEX, class WEIGHT>  
size_t 
pBaseGraph<VERTEX,WEIGHT>::
GetVertexCount() /*const*/{
  return size();
}


template <class VERTEX, class WEIGHT> 
bool 
pBaseGraph<VERTEX,WEIGHT>::
empty() {
  if( size() == 0) return true;
  else return false;
}

template <class VERTEX, class WEIGHT>
bool
pBaseGraph<VERTEX,WEIGHT>::
local_empty() const {
  if(local_size() == 0) return true;
  else return false;
}


template <class VERTEX, class WEIGHT>
size_t 
pBaseGraph<VERTEX,WEIGHT>::
ByteSize()  {
  //this can be done with an rmi_reduce if all the threads are calling
  //symultaneous; If not I'll call explicit local_size on all the other threads;  
  size_t temp = 0;
  temp += LocalByteSize();
  for(int where=0;where < this->nprocs;where++){
    if(where != this->myid)    
      temp += stapl::sync_rmi(where,
			      this->getHandle(),
			      &pBaseGraph_type::LocalByteSize);
  }
  return temp;
}

template <class VERTEX, class WEIGHT>  
size_t 
pBaseGraph<VERTEX,WEIGHT>::
LocalByteSize()  {
  size_t temp=0;

  VI pit = this->local_begin();
  for(;pit!=this->local_end();pit++){
    temp += pit->ByteSize();
  }

  return temp;
}



//==============================================================================
//      I/O     Methods
//==============================================================================


template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
__pDisplayGraph(){
  cout<<"============== pDisplayGraph ========["<<this->myid<<"]=========="<<endl;
  parts_internal_iterator pi = this->parts_begin();
  for(;pi!=this->parts_end();pi++){
    (*pi)->DisplayGraph();
  }
  return OK;
}

template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
pDisplayGraph(){
  if(this->myid==0){
    for(int p=0;p<this->nprocs;p++){
      int t=stapl::sync_rmi(p,
			    this->getHandle(), 
			    &pBaseGraph_type::__pDisplayGraph); 
    }
  }
  stapl::rmi_fence();
}

//==============================================================================
//      Methods for sub_graph building, distribution, and restoring using RMI
//==============================================================================

template <class VERTEX, class WEIGHT>
void 
pBaseGraph<VERTEX,WEIGHT>::
_FlushCache(){
  this->dist.FlushCache();
}

template <class VERTEX, class WEIGHT>
void 
pBaseGraph<VERTEX,WEIGHT>::
WarmCache(){
  EI ei;
  VI temp;
  VI pit = this->local_begin();
  for(;pit!=this->local_end();pit++){
    for(ei=pit->edgelist.begin();ei != pit->edgelist.end();ei++){
      if(!IsVertex(ei->vertex2id,&temp)){
	Location _n=this->Lookup(ei->vertex2id);
	//cout<<"While warming up the cache "<<_n.locpid()<<" "<<_n.partid()<<endl;
	stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in WarmCache");
      }
    }
    for(ei=pit->predecessors.begin();ei != pit->predecessors.end();ei++){
      if(!IsVertex(ei->vertex2id,&temp)){
	Location _n=this->Lookup(ei->vertex2id);
	//cout<<"While warming up the cache "<<_n.locpid()<<" "<<_n.partid()<<endl;
	stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in WarmCache");
      }
    }
  }
}


template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
CentralDistribute(const map<PID,vector<VID> >& data_map) {
  vector<Vertex> sub_data;
  map<PID,vector<VID> >::const_iterator its;

  for(its=data_map.begin(); its!=data_map.end(); ++its) {
    if(its->first != this->myid) {
      PackVertices(its->second, sub_data);
      cout<<"Vertices packed and sent to : "<<its->first<<" size "<<sub_data.size()<<endl;
      stapl::async_rmi(its->first,
      		       this->getHandle(), 
		       &pBaseGraph_type::__SetSubContainer,sub_data); 
      //here we have to delete the vertices just shiped
      DeleteVertices(its->second);

      //update the maps for all moved vertices
      Location _l(its->first,0);//the zero is the partid which is zero by default

      for(vector<VID>::const_iterator it = its->second.begin();it != its->second.end();it++){
	_StaplPair<GID,Location> temp (*it,_l);
	this->dist.UpdateLocationMap(temp);
      }
    }
  }

  //Invalidate the cache and update the maps
  for(int p=0;p<this->nprocs;p++){
    stapl::async_rmi(p,
		     this->getHandle(), 
		     &pBaseGraph_type::_FlushCache); 
    //stapl::async_rmi(p,
    //	     this->getHandle(), 
    //	     &pBaseGraph_type::WarmCache); 
  }

}

/////////////////////////////////////////////
template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
_local_sz(){
    return this->local_size();
}

template <class VERTEX, class WEIGHT>
void 
pBaseGraph<VERTEX,WEIGHT>::
balance(){
    int i,j,total=0;
    vector<int> cd(this->nprocs);
    vector<int> extra(this->nprocs);
    vector<vector<pair<int,int> > > newd(this->nprocs);
    map<PID,vector<VID> > dist_map;
    vector<Vertex> sub_data;    
    //redistribute the pgraph trying to balance the nodes 
    //across compute nodes
    stapl::rmi_fence();
    for(i=0;i<this->nprocs;i++){
      if(i != this->myid){
	cd[i] = stapl::sync_rmi(i,
		     this->getHandle(),
			  &pBaseGraph_type::_local_sz);
	total += cd[i];
      }
      else{
	cd[i] = this->local_size();
	total += cd[i];
      }
    }
    stapl::rmi_fence();
    if(myid == 0){
      cout<<this->myid<<":"<<total<<endl;
      for(i=0;i<this->nprocs;i++){
	cout<<this->myid<<":"<<cd[i]<<endl;
      }
    }

    //now compute the redistribution map
    int elpp = total / this->nprocs;
    for(i=0;i<this->nprocs;i++){
      extra[i] = cd[i] - elpp;
    }
    i=0;
    while(i<this->nprocs){
      //for(i=0;i<this->nprocs;i++){
      if(extra[i] < 0){
	for(j=0;j<this->nprocs;j++){
	  if(extra[j] > 0 && extra[i]<0){//if there are extra elements
	    //if enough to balance i
	    if(extra[j] >= -1 * extra[i]){
	      //take only a part of extras on j
	      newd[j].push_back(pair<int,int>(i,-1*extra[i]));
	      extra[j] += extra[i];
	      extra[i] = 0;
	      i++;
	      break;
	    }
	    //else balance what you can
	    else{
	      newd[j].push_back(pair<int,int>(i,extra[j]));
	      extra[i] += extra[j];
	      extra[j] = 0; 
	    }
	  }
	}
      }
      else i++;
    }
    stapl::rmi_fence();

    // map<PID,vector<VID> > dist_map;
    if(newd[myid].size() > 0){
      VI it = this->local_begin();
      for(i=0;i<elpp + extra[myid];i++) ++it;

      for(int j=0;j<newd[myid].size();j++){
	cout<<myid<<"  send:"<<newd[myid][j].first<<":"<<newd[myid][j].second<<endl;
	//pack
	sub_data.clear();
	for(i=0; i<newd[myid][j].second;i++){
	  sub_data.push_back(*(it.get_iterator()));
	  ++it;
	}
	//sent 
	stapl::async_rmi(newd[myid][j].first,
			 this->getHandle(), 
			 &pBaseGraph_type::__SetSubContainer,sub_data); 
      }
      //delete
      //just resize part 0
      this->pcontainer_parts[0]->resize(elpp + extra[myid]);
      this->pcontainer_parts[0]->UpdateMap();
      //update the gid to position map
      
    }//end if there is data to send
    stapl::rmi_fence();

    this->initializeDistribution();
    stapl::rmi_fence() ;

    if(myid == 0){
      for(i=0;i<this->nprocs;i++){
	cout<<this->myid<<"::::::"<<extra[i]<<endl;
	for(int j=0;j<newd[i].size();j++){
	  cout<<i<<"  send:"<<newd[i][j].first<<":"<<newd[i][j].second<<endl;
	}
      }
    }
}
template <class VERTEX, class WEIGHT>
void 
pBaseGraph<VERTEX,WEIGHT>::
initializeDistribution(){
  this->dist.FlushCache();
  this->dist.FlushPartsMap();
  this->dist.FlushLocationMap();

  stapl::rmi_fence();

  for(VI vi = local_begin(); vi!=local_end();++vi){
    dist.Add2PartIDMap(vi->vid,0);	
    Location location(myid,0);
    pair<GID,Location> tp(vi->vid,location);
    dist.Add2LocationMap(tp);
  }
 }


template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
PackVertices(vector<VID> _group, vector<Vertex>& _pg) {
  _pg.clear();
  //  _pg.resize(_group.size());//if resize is used we should use [] to access elems
  VI pvi;

  for(vector<VID>::iterator it = _group.begin();it != _group.end();it++){
    if(IsVertex(*it,&pvi)){
      //I am making a copy;; See if it can be fixed
      _pg.push_back(*(pvi.get_iterator()));
    }
    else{
      cout<<"Pack Vertices:: Inexistent vertex"<<endl;
    }
  }
}

template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
DeleteVertices(vector<VID> _group) {
  vector<VID>::iterator it;
  PARTID _pid;
  for(it = _group.begin();it != _group.end();it++){
      if(this->IsLocal(*it,_pid)){
	this->pcontainer_parts[_pid]->SimpleDeleteVertex(*it);
	//also delete from element to partid map
	this->dist.DeleteFromPartIDMap(*it);
      }
  }
}

template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
__SetSubContainer(vector<Vertex>& _pg) {
  //here I have to add the pWtVertices to the local storage
  //by default I'll add them to part0
  typename vector<Vertex>::iterator it;
  for(it = _pg.begin();it != _pg.end();it++){
    AddVertex(it->data, it->vid, this->myid);
    //add all edges now;
    for(EI ei = it->edgelist.begin();ei!=it->edgelist.end();ei++){
      AddEdge(it->vid, ei->vertex2id,ei->weight,true);
    }
  }
}

template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
pGraphDistribute(const map<PID,vector<VID> >& data_map) {

  vector<Vertex> sub_data;
  map<PID,vector<VID> >::const_iterator its;

  for(its=data_map.begin(); its!=data_map.end(); ++its) {
    if(its->first != this->myid) {
      PackVertices(its->second, sub_data);
      cout<<"Vertices packed and sent to :"<<its->first<<"size "<<sub_data.size()<<endl;
      stapl::async_rmi(its->first,
      		       this->getHandle(), 
		       &pBaseGraph_type::__SetSubContainer,sub_data); 
      //here we have to delete the vertices just shiped
      cout<<"Delete vertices"<<endl;
      DeleteVertices(its->second);

      //update the maps for all moved vertices
      Location _l(its->first,0);//the zero is the partid which is zero by default
      cout<<"update location map"<<endl;
      for(vector<VID>::const_iterator it = its->second.begin();it != its->second.end();it++){
	_StaplPair<GID,Location> temp (*it,_l);
	this->dist.UpdateLocationMap(temp);
      }
    }
  }

  stapl::rmi_fence();

  //Invalidate the cache and update the maps
  for(int p=0;p<this->nprocs;p++){
    stapl::async_rmi(p,
		     this->getHandle(), 
		     &pBaseGraph_type::_FlushCache); 
    //stapl::async_rmi(p,
    //	     this->getHandle(), 
    //	     &pBaseGraph_type::WarmCache); 
  }
  stapl::rmi_fence();
}

//////////////////////////////////////////////////////////////////////////
//                        Set Predecessors
//////////////////////////////////////////////////////////////////////////
template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
_AddPredecessor(VID _v2, WtEdge _e){
  VI v2;
  if ( this->IsVertex(_v2, &v2) ) {
    v2->predecessors.push_back(_e);
  }
  else{
    cout<<"ERROR::pBG::_AddPredecessor while trying to set predecessors"<<endl;
  }
}

template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
ClearPredecessors(){
  VI   v1;
  for(v1 = this->local_begin(); v1 != this->local_end(); v1++) {
    v1->predecessors.clear();
  }
}


template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
pSetPredecessors() {
  VI   v1, v2;
  VID  _v2id;

  for(v1 = this->local_begin(); v1 != this->local_end(); v1++) {
    v1->predecessors.clear();
  }
  
  stapl::rmi_fence();

  for(v1 = this->local_begin(); v1 != this->local_end(); v1++) {
    for (EI ei = v1->edgelist.begin(); ei != v1->edgelist.end(); ei++) {
      _v2id = ei->vertex2id;
      //if(this->IsLocal(_v2id)){
      if ( this->IsVertex(_v2id, &v2) ) {
	WtEdge newEdge( v1->vid, ei->weight );
	v2->predecessors.push_back(newEdge);
      }
      //  else{
      //    cout<<"ERROR:: while trying to set predecessors for:"<<_v2id<<"from:"<<v1->vid<<endl;
      //  }
      //}
      else{
	//remote
	Location _n=this->Lookup(_v2id);
	stapl_assert(_n.ValidLocation(),"Invalid id for the second vertex while SETPredecessors");    
	WtEdge newEdge( v1->vid, ei->weight );
	//cout<<"destination"<<_n.locpid()<<endl;  
	stapl::async_rmi(_n.locpid(),
			 this->getHandle(),
			 &pBaseGraph<VERTEX,WEIGHT>::_AddPredecessor,
			 _v2id,
			 newEdge);  
      }
    }//for
  }//for 
  stapl::rmi_fence();
}//end SetPredecessors

template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
pAsyncSetPredecessors() {
  VI   v1, v2;
  VID  _v2id;
  for(v1 = this->local_begin(); v1 != this->local_end(); v1++) {
    for (EI ei = v1->edgelist.begin(); ei != v1->edgelist.end(); ei++) {
      _v2id = ei->vertex2id;
      if ( this->IsVertex(_v2id, &v2) ) {
	WtEdge newEdge( v1->vid, ei->weight );
	v2->predecessors.push_back(newEdge);
      }
      else{
	//remote
	Location _n=this->Lookup(_v2id);
	stapl_assert(_n.ValidLocation(),"Invalid id for the second vertex while SETPredecessors");    
	WtEdge newEdge( v1->vid, ei->weight );
	//cout<<"destination"<<_n.locpid()<<endl;  
	stapl::async_rmi(_n.locpid(),
			 this->getHandle(),
			 &pBaseGraph<VERTEX,WEIGHT>::_AddPredecessor,
			 _v2id,
			 newEdge);  
      }
    }//for
  }//for
}//end pAsyncSetPredecessors
  
template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
pGetCutEdges(set<pair<VID,VID> >& cut) {
  VI   v1, v2;
  EI ei;
  VID  _v2id;
  pair<VID,VID> r_edge;
  
  cut.clear();    
  pSetPredecessors();
  //the above call has a fence inside
  
  for(v1 = this->local_begin(); v1 != this->local_end(); v1++) {
    //iterate trough succesors for remote out going edges
    for (ei = v1->edgelist.begin(); ei != v1->edgelist.end(); ei++) {
      _v2id = ei->vertex2id;
      if(!this->IsLocal(_v2id)){
	r_edge.first  = v1->vid;
	r_edge.second =_v2id;
	cut.insert(r_edge);
      }
    }//for
    //iterate trough predecessors for remote incoming edges
    for (ei = v1->predecessors.begin(); ei != v1->predecessors.end(); ei++) {
      _v2id = ei->vertex2id;
      if(!this->IsLocal(_v2id)){
	r_edge.first  = v1->vid;
	r_edge.second =_v2id;
	cut.insert(r_edge);
      }
    }//for
  }//for
}//end pGetCutEdges

//////////////////////////////////////////////////////////////////////////
//                 Delete a bulk of vertices
//////////////////////////////////////////////////////////////////////////

template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
_simpleDeleteGhosts(vector<pair<VID,VID> > _vv,vector<pair<VID,VID> > _vv_succ ){
  VI v1;
  vector<pair<VID,VID> >::iterator vit;
  for(vit = _vv.begin(); vit!=_vv.end();vit++){
    //this->DeleteEdge(vit->first,vit->second);
    if(this->IsVertex(vit->first,&v1)){
      v1->DeleteXEdges(vit->second,1);
      //v1->SetDependencies(0);
    }
  }
  //delete succesor's predecessors
  for(vit = _vv_succ.begin(); vit!=_vv_succ.end();vit++){
    //if(vit->first == 8027){
    //cout<<"message received to delete pred "<<vit->second<<endl;
    //}
    if(this->IsVertex(vit->first,&v1)){
      v1->DeleteXPredEdges(vit->second,1);
      //v1->SetDependencies(0);
    }
  }
  //here something related with the maps;
  //this->dist.DeleteFromCache(vit->second);
}

template <class VERTEX, class WEIGHT>
  pair<double,double>
pBaseGraph<VERTEX,WEIGHT>::
pSimpleDeleteVertices(vector<VID>& _vv) {
  VI v1,v2;
  EI ei;
  vector<VID>::iterator vit;
  vector<VID> verts;
  int i,j,_np;
  
  //the next map accumulate the processors(PID) were nodes that are removed locally
  //are ghost; After the next loop I have to inform the procs
  //were local nodes are ghost to remove them
  vector<vector<pair<VID,VID> > > to_delete(this->nprocs);
  vector<vector<pair<VID,VID> > > to_delete_succ(this->nprocs);
  vector<vector<pair<VID,VID> > >::iterator mit,msit;
  vector<VID> neighbours;
  stapl::timer t_l;

  pair<VID,VID> p;
  for(vit=_vv.begin();vit != _vv.end();vit++){
    if(this->IsVertex(*vit, &v1)){
      v1->edgelist.clear();
      v1->predecessors.clear();
    }
  }
  //stapl::rmi_fence();
  t_l = stapl::start_timer();
  pair<double,double> res;
 
  //now I have to compact;
  //this version works for only one part per thread;
  assert(this->GetPartsCount() <= 1);
  
  pContainerPart_type* _part0= this->GetPart(0);
  int dest=-1;
  int pos=0;
  int deleted = 0;
  typename pContainerPart_type::iterator vi,viend;
  vi=_part0->begin();
  viend = _part0->end();
  if(vi != viend) --viend;
  while(vi != viend){

    while ((vi != viend) 
	   && 
	   (vi->edgelist.size() != 0 || vi->predecessors.size() != 0)) {
      ++pos;
      ++vi; 
    }

    while ((vi != viend) 
	   && 
	   (viend->edgelist.size() == 0 && viend->predecessors.size() == 0)) {
      --viend; 
      deleted++;
    }
    if(vi != viend) {
      dest = vi->vid;
      *vi = *viend;
      _part0->gid_localid_map[vi->vid] = pos;
      _part0->gid_localid_map.erase(dest);
      viend->edgelist.clear();
      viend->predecessors.clear();
    }
  }

  if(viend !=_part0->end() && vi == viend && vi->edgelist.size() == 0 && vi->predecessors.size() == 0) deleted++; 
  if(deleted > 0)
    _part0->v.resize(_part0->v.size() - deleted);
  //stapl::rmi_fence();
  res.first = stapl::stop_timer(t_l);
  t_l = stapl::start_timer();
  //and here remove the ghosts of the nodes removed;

  /*
  i=0;
  msit = to_delete_succ.begin();
  for(mit = to_delete.begin();mit != to_delete.end();++mit,++i,++msit){
    if(mit->size() > 0 || msit->size() > 0){
      stapl::async_rmi(i,
		       this->getHandle(),
		       &pBaseGraph<VERTEX,WEIGHT>::_deleteGhosts,to_delete[i],to_delete_succ[i]);
      
    }
  } 
  //stapl::rmi_fence();
  */

  res.second = stapl::stop_timer(t_l);
  return res;
}

template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
_deleteGhosts(vector<pair<VID,VID> > _vv){
  VI v1;
  vector<pair<VID,VID> >::iterator vit;
  for(vit = _vv.begin(); vit!=_vv.end();vit++){
    this->DeleteEdge(vit->first,vit->second);
  }
  //here something related with the maps;
  //this->dist.DeleteFromCache(vit->second);
}


template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
pDeleteVertices(vector<VID>& _vv) {
  VI v1,v2;
  EI ei;
  vector<VID>::iterator vit;
  vector<VID> verts;
  int i;
  
  //the next map accumulate the processors(PID) were nodes that are removed locally
  //are ghost; After the next loop I have to inform the procs
  //were local nodes are ghost to remove them
  vector<vector<pair<VID,VID> > > to_delete(this->nprocs);
  vector<vector<pair<VID,VID> > >::iterator mit;
  
  pair<VID,VID> p;
  
  for(vit=_vv.begin();vit != _vv.end();vit++){
    if(this->IsVertex(*vit, &v1)){
      //check if predecessors remote
      for (ei = v1->predecessors.begin(); ei != v1->predecessors.end(); ei++) {
	if (! this->IsLocal(ei->vertex2id)){
	  Location _n=this->Lookup(ei->vertex2id);
	  stapl_assert(_n.ValidLocation(),"Invalid id for the second vertex while SETPredecessors");   
	  //the location is added to the map
	  p.first = ei->vertex2id;
	  p.second = *vit;
	  to_delete[_n.locpid()].push_back(p);
	}
	else{
	  //delete the edge locally
	  if(IsVertex(ei->vertex2id,&v2)){
	    v2->DeleteXEdges(v1->vid,-1);
	  }
	  else{
	    cout<<"WARNING: While SetPredecessors: vertex2id is not local nor remote ??"<<endl;
	  }
	}
      }
      //cout<<"DEleted:"<<v1->vid<<endl;
      v1->edgelist.clear();
      v1->predecessors.clear();
    }
  }

  //now I have to compact;
  //this version works for only one part per thread;
  assert(this->GetPartsCount() <= 1);
  
  pContainerPart_type* _part0= this->GetPart(0);
  int dest=-1;
  int pos=0;
  int deleted = 0;
  typename pContainerPart_type::iterator vi;
  for(vi=_part0->begin();vi!=_part0->end();vi++){
    if(vi->edgelist.size() == 0 && vi->predecessors.size()==0) {
      
      //cout<<"DEleted:"<<vi->vid<<endl;
      
      //this->dist.DeleteFromLocationMap(vi->vid);
      
      if(dest== -1) dest=pos;
      pos++;
      deleted ++;
      continue;
      }
    if(dest != -1){
      //here we are if there is a spot were I can move;
      _part0->v[dest] = _part0->v[pos];
      dest++;
    }
    pos++;
  } 
  _part0->v.resize(_part0->v.size() - deleted);
  
  //stapl::rmi_fence();
  
  //and here remove the ghosts of the nodes removed;
  i=0;
  for(mit = to_delete.begin();mit != to_delete.end();mit++,i++){
    if(mit->size() > 0){
      stapl::async_rmi(i,
		       this->getHandle(),
		       &pBaseGraph<VERTEX,WEIGHT>::_deleteGhosts,to_delete[i]);
      
    }
  } 
  //stapl::rmi_fence();
  return OK;
}

//////////////////////////////////////////////////////////////////////////
//       Basic DFS Traversal on the current pgraph
//////////////////////////////////////////////////////////////////////////

template <class VERTEX, class WEIGHT>
template <class VISITOR>
int
pBaseGraph<VERTEX,WEIGHT>::
DFS_traversal_wrapper(_StaplQuad<VID,VISITOR,int,bool> _arg){
  VID v1 = _arg.first;
  VISITOR v2=_arg.second;
  int v3 =_arg.third;
  bool v4 =_arg.fourth;
  return DFS_traversal(v1,v2,v3,v4);
}

template <class VERTEX, class WEIGHT>
template <class VISITOR>
int 
pBaseGraph<VERTEX,WEIGHT>::
DFS_traversal(VID _start, VISITOR& _vis, int _traversal_id,bool _preds){
  VID _v2id;
  VI vi_start,v1,v2;
  EI ei;
  int temp;
  STAPL_GRAPH_COLOR color;    
  vector<VID> succs;
  PARTID partid;

  if(this->IsVertex(_start,&vi_start)){
    if(!_preds){
      color = vi_start->GetColorIn(_traversal_id) ;
      if(color == STAPL_WHITE){//if color is STAPL_WHITE
	vi_start->SetColorIn(_traversal_id,STAPL_GRAY);
      }
      else return OK;
    }
    else{//going back trough preds
      color = vi_start->GetColorOut(_traversal_id) ;
      if(color == STAPL_WHITE){//if color is STAPL_WHITE
	vi_start->SetColorOut(_traversal_id,STAPL_GRAY);
      }
      else return OK;
    }
  }
  else{
    printf("ERROR::DFS_traversal: inexistent vertex:%d on thread %d\n",_start,myid);
    return ERROR;
  }

  temp = _vis.vertex(vi_start);
  if(temp == EARLY_QUIT) return EARLY_QUIT;

  int t = this->GetAdjacentVertices(vi_start,succs);
  for(int _v2id=0;_v2id < t;_v2id++){
    if(this->IsLocal(succs[_v2id],partid)){  
      temp =  DFS_traversal(succs[_v2id], _vis, _traversal_id, _preds);
      //tt = temp;
    }
    else{
      //find where _v2id lives and call async there
      Location _n=this->Lookup(succs[_v2id]);
      if(!_n.ValidLocation()){
	cout<<"ERROR::: "<<succs[_v2id]<<" "<<stapl::get_thread_id()<<endl;
      }
      stapl_assert(_n.ValidLocation(),"Invalid id for the second vertex of an edge in DFS");
      //cout<<"DEST:"<<_n.locpid()<<"::"<< _v2id<<"::"<<this->getHandle()<<endl;
      _StaplQuad<VID,VISITOR,int,bool> args(succs[_v2id] , _vis, _traversal_id, _preds);
      temp = stapl::sync_rmi<pBaseGraph_type,int,_StaplQuad<VID,VISITOR,int,bool> >( _n.locpid(), 
			      this->getHandle(),
			      (int (pBaseGraph_type::*)(_StaplQuad<VID,VISITOR,int,bool>))&pBaseGraph_type::DFS_traversal_wrapper,args);
      //tt = temp;
    }
    if(temp == ERROR || temp == EARLY_QUIT) return temp;
    //ei++;
  }
  //vi_start->SetColorIn(_traversal_id,STAPL_BLACK);
  //_vis->finish_vertex(vi_start,++tt);  
  return OK;
}
 
 
//////////////////////////////////////////////////////////////////////////
//       Basic Traversal BFS like on the current pgraph
//////////////////////////////////////////////////////////////////////////
template <class VERTEX, class WEIGHT>
template <class VISITOR>
void 
pBaseGraph<VERTEX,WEIGHT>::
BFS_traversal_wrapper(_StaplQuad<VID,VISITOR,int,bool> _arg){
  VID v1 = _arg.first;
  VISITOR v2=_arg.second;
  int v3 =_arg.third;
  bool v4 =_arg.fourth;
  BFS_traversal(v1,v2,v3,v4);
}

template <class VERTEX, class WEIGHT>
template <class VISITOR>
int 
pBaseGraph<VERTEX,WEIGHT>::
BFS_traversal(VID _start, VISITOR& _vis, int _traversal_id,bool _preds){
  //sweep to the graph associated with this trimer
  typename pBaseGraph_type::VI vi,vi_start;
  
  list<_StaplTriple<typename pBaseGraph_type::VI,int,int> > QT;//queue used for trimer traversals

  //vi , index, direction
  _StaplTriple<typename pBaseGraph_type::VI,int,int> v;
  
  //map<VID, int> remote_color;
  hash_map<VID, int> remote_color;

  STAPL_GRAPH_COLOR color;    
  vector<VID> succs;
  
  //cout<<"START BFS traversal"<<endl;

  if(this->IsVertex(_start,&vi_start)){
    //cout<<"Vertex pushed"<<endl;
    if(!_preds){
      color = vi_start->GetColorIn(_traversal_id) ;
      if(color == STAPL_WHITE){//if color is STAPL_WHITE
	vi_start->SetColorIn(_traversal_id,STAPL_GRAY);
	QT.push_back(_StaplTriple<typename pBaseGraph_type::VI,int,int>(vi_start,_traversal_id,STAPL_FWD));
      }
    }
    else{//going back trough preds
      color = vi_start->GetColorOut(_traversal_id) ;
      if(color == STAPL_WHITE){//if color is STAPL_WHITE
	vi_start->SetColorOut(_traversal_id,STAPL_GRAY);
	QT.push_back(_StaplTriple<typename pBaseGraph_type::VI,int,int>(vi_start,_traversal_id,STAPL_BACK));
      }
    }
  }
  
  while(QT.size() > 0){//while there are more vertices
    //pop a vertex from Q
    v = QT.front();
    QT.pop_front();
    
    _vis.vertex(v.first);
    
    //for every child u of v
    if(v.third == STAPL_FWD){
      int t = this->GetAdjacentVertices(v.first,succs);
      //cout<<"Adjacency for "<<v.first->vid<<" "<< t <<" "<<succs.size()<<endl;
      for(int v2id=0;v2id < t;v2id++){
	if(!this->IsVertex(succs[v2id],&vi)){
	  //the vertex is not local
	  //here add trough an rmi the vertex in the other 
	  //processor queue of ready vertices
	  
	  Location _n=this->Lookup(succs[v2id]);
	  stapl_assert(_n.ValidLocation(),
		       "Invalid id for the second vertex of an edge in BFS");
	  //cout<<"remote vertex "<<succs[v2id]<<" "<<_n.locpid()<<endl;
	  _StaplQuad<VID,VISITOR,int,bool> args(succs[v2id],_vis,_traversal_id,_preds);
	  stapl::async_rmi( _n.locpid(),
			    this->getHandle(),
			    (void (pBaseGraph_type::*)(_StaplQuad<VID,VISITOR,int,bool>))&pBaseGraph_type::BFS_traversal_wrapper<VISITOR>,args);
			    //&pBaseGraph_type::BFS_traversal_wrapper<VISITOR>,args);
	}
	else{
	  //here we are if the vertex is local
	  //first check the color of the node
	  color = vi->GetColorIn(v.second) ;
	  if(color == STAPL_WHITE){//if color is STAPL_WHITE
	    vi->SetColorIn(v.second,STAPL_GRAY);
	    QT.push_back(_StaplTriple<typename pBaseGraph_type::VI,int,int>(vi,v.second,STAPL_FWD));
	  }
	}
      }//for all succs
    }//if fwd
    else{
      //here if we have backward also
      int t = this->GetPredVertices(v.first,succs);
      //cout<<"Preds for "<<v.first->vid<<" "<< t <<" "<<succs.size()<<endl;
      for(int v2id=0;v2id < t;v2id++){
	if(!this->IsVertex(succs[v2id],&vi)){
	  //the vertex is not local
	  //here add trough an rmi the vertex in the other 
	  //processor queue of ready vertices
	  
	  Location _n=this->Lookup(succs[v2id]);
	  stapl_assert(_n.ValidLocation(),
		       "Invalid id for the second vertex of an edge in BFS");
	  //cout<<"remote vertex "<<succs[v2id]<<" "<<_n.locpid()<<endl;
	  _StaplQuad<VID,VISITOR,int,bool> args(succs[v2id],_vis,_traversal_id,_preds);
	  stapl::async_rmi( _n.locpid(),
			    this->getHandle(),
			    (void (pBaseGraph_type::*)(_StaplQuad<VID,VISITOR,int,bool>))&pBaseGraph_type::BFS_traversal_wrapper,args);
	  //&pBaseGraph_type::BFS_traversal_wrapper<VISITOR>,args);
	}
	else{
	  //here we are if the vertex is local
	  //first check the color of the node
	  color = vi->GetColorOut(v.second) ;
	  if(color == STAPL_WHITE){//if color is STAPL_WHITE
	    vi->SetColorOut(v.second,STAPL_GRAY);
	    QT.push_back(_StaplTriple<typename pBaseGraph_type::VI,int,int>(vi,v.second,STAPL_BACK));
	  }
	}
      }//for all succs
    }//else  --> STAPL_BACK
  }//while
  return OK;
}//end method BFS_traversal 
 


template <class VERTEX, class WEIGHT>
template <class VISITOR>
void 
pBaseGraph<VERTEX,WEIGHT>::
BFS_traversal_wrapper_EQ(_StaplQuad<VID,VISITOR,int,bool> _arg){
  VID v1 = _arg.first;
  VISITOR v2=_arg.second;
  int v3 =_arg.third;
  bool v4 =_arg.fourth;
  BFS_traversal_EQ(v1,v2,v3,v4);
}

template <class VERTEX, class WEIGHT>
template <class VISITOR>
int 
pBaseGraph<VERTEX,WEIGHT>::
BFS_traversal_EQ(VID _start, VISITOR& _vis, int _traversal_id,bool _preds){
  //sweep to the graph associated with this trimer
  typename pBaseGraph_type::VI vi,vi_start;
  typename pBaseGraph_type::EI ei;
  int v2id;

  deque<_StaplTriple<typename pBaseGraph_type::VI,int,int> > QT;//queue used for trimer traversals

  //vi , index, direction
  _StaplTriple<typename pBaseGraph_type::VI,int,int> v;
  
  //map<VID, int> remote_color;
  //hash_map<VID, int> remote_color;

  STAPL_GRAPH_COLOR color;    
  //vector<VID> succs;
  
  if(this->IsVertex(_start,&vi_start)){
    //cout<<"Vertex pushed"<<endl;
    if(!_preds){
      color = vi_start->GetColorIn(_traversal_id) ;
      if(color == STAPL_WHITE){//if color is STAPL_WHITE
	vi_start->SetColorIn(_traversal_id,STAPL_GRAY);
	QT.push_back(_StaplTriple<typename pBaseGraph_type::VI,int,int>(vi_start,_traversal_id,STAPL_FWD));
      }
    }
    else{//going back trough preds
      color = vi_start->GetColorOut(_traversal_id) ;
      if(color == STAPL_WHITE){//if color is STAPL_WHITE
	vi_start->SetColorOut(_traversal_id,STAPL_GRAY);
	QT.push_back(_StaplTriple<typename pBaseGraph_type::VI,int,int>(vi_start,_traversal_id,STAPL_BACK));
      }
    }
  }


  while(QT.size() > 0){//while there are more vertices
    //pop a vertex from Q
    v = QT.back();
    QT.pop_back();
    
    if(_vis.vertex(v.first) < 0) continue;
    
    //for every child u of v
    if(v.third == STAPL_FWD){
      //int t = this->GetAdjacentVertices(v.first,succs);
      //cout<<"Adjacency for "<<v.first->vid<<" "<< t <<" "<<succs.size()<<endl;
      //for(int v2id=0;v2id < t;v2id++){
      for(ei = v.first->edgelist.begin();ei != v.first->edgelist.end();++ei){

	//_vis.tree_edge(v.first,ei);

	v2id = ei->vertex2id;
	if(!this->IsVertex(v2id,&vi)){
	  //the vertex is not local
	  //here add trough an rmi the vertex in the other 
	  //processor queue of ready vertices
	  Location _n=this->Lookup(v2id);
	  stapl_assert(_n.ValidLocation(),
		       "Invalid id for the second vertex of an edge in BFS");
	  //cout<<"remote vertex "<<succs[v2id]<<" "<<_n.locpid()<<endl;
	  _StaplQuad<VID,VISITOR,int,bool> args(v2id,_vis,_traversal_id,_preds);
	  stapl::async_rmi<pBaseGraph_type, void, _StaplQuad<VID,VISITOR,int,bool> >( _n.locpid(),
			    this->getHandle(),
			    (void (pBaseGraph_type::* const)(_StaplQuad<VID,VISITOR,int,bool>))&pBaseGraph_type::BFS_traversal_wrapper_EQ,args);
	}
	else{
	  //here we are if the vertex is local
	  //first check the color of the node
	  color = vi->GetColorIn(v.second) ;
	  if(color == STAPL_WHITE){//if color is STAPL_WHITE
	    vi->SetColorIn(v.second,STAPL_GRAY);
	    QT.push_back(_StaplTriple<typename pBaseGraph_type::VI,int,int>(vi,v.second,STAPL_FWD));
	  }
	}
      }//for all succs
    }//if fwd
    else{
      //here if we have backward also
      //int t = this->GetPredVertices(v.first,succs);
      //cout<<"Preds for "<<v.first->vid<<" "<< t <<" "<<succs.size()<<endl;
      //for(int v2id=0;v2id < t;v2id++){
      for(ei = v.first->predecessors.begin();ei != v.first->predecessors.end();++ei){

	_vis.tree_edge(v.first,ei);

	v2id = ei->vertex2id;
	if(!this->IsVertex(v2id,&vi)){
	  //the vertex is not local
	  //here add trough an rmi the vertex in the other 
	  //processor queue of ready vertices
	  Location _n=this->Lookup(v2id);
	  stapl_assert(_n.ValidLocation(),
		       "Invalid id for the second vertex of an edge in BFS");
	  //cout<<"remote vertex preds"<<succs[v2id]<<" "<<_n.locpid()<<endl;
	  _StaplQuad<VID,VISITOR,int,bool> args(v2id,_vis,_traversal_id,_preds);
	  stapl::async_rmi<pBaseGraph_type, void, _StaplQuad<VID,VISITOR,int,bool> >( _n.locpid(),
			    this->getHandle(),
			    (void (pBaseGraph_type::* const)(_StaplQuad<VID,VISITOR,int,bool>))&pBaseGraph_type::BFS_traversal_wrapper_EQ,args);
	}
	else{
	  //here we are if the vertex is local
	  //first check the color of the node
	  color = vi->GetColorOut(v.second) ;
	  if(color == STAPL_WHITE){//if color is STAPL_WHITE
	    vi->SetColorOut(v.second,STAPL_GRAY);
	    QT.push_back(_StaplTriple<typename pBaseGraph_type::VI,int,int>(vi,v.second,STAPL_BACK));
	  }
	}
      }//for all succs
    }//else  --> STAPL_BACK
  }//while
  return OK;
}//end method BFS_traversal 

//////////////////////////////////////////////////////////////////////////
//       BAsic Traversal TOPOLOGICAL-SORT like on the current pgraph
//       Work more on the description;
//////////////////////////////////////////////////////////////////////////
template <class VERTEX, class WEIGHT>
template <class VISITOR>
void 
pBaseGraph<VERTEX,WEIGHT>::
  TOPO_traversal_wrapper(_StaplPenta<VID,VISITOR,int,bool,VID> _arg){
  VID v1 = _arg.first;      //start vertex
  VISITOR v2=_arg.second;   //visitor to be applied
  int v3 =_arg.third;       //traversal_id
  bool preds =_arg.fourth;     //going normal trough succesors(false) or trough preds(true)
  typename pBaseGraph_type::VI vi_start,tempvi;
  STAPL_GRAPH_COLOR _color;

  assert(v3==0);

  if(this->IsVertex(v1,&vi_start)){
    if(!preds){
      _color = vi_start->GetColorIn(v3) ;
      assert(_color >= 0);
      if (_color == STAPL_WHITE) {
	cout<<"Check this if in TOPO_traversal_wrapper"<<endl;
	return;
      }
      _color = _color - 1;
      
      vi_start->SetColorIn(v3,_color);
      vi_start->DeleteXPredEdges(_arg.fifth,1);

      if(_color == STAPL_WHITE){//if color is STAPL_WHITE
	TOPO_traversal(v1,v2,v3,preds);
      }
    }//if not preds()
    else{
      _color = vi_start->GetColorOut(v3) ;
      assert(_color >= 0);
      if (_color == STAPL_WHITE) {
	//int k;
	//k=0;
	//for(tempvi = this->local_begin(); tempvi != this->local_end();tempvi++){
	  //cout<<"["<<tempvi->GetColorOut(v3)<<":"<<tempvi->GetColorIn(v3)<<"]"<<endl;
	  //if(tempvi->GetColorOut(v3)!=0 || tempvi->GetColorIn(v3)!=0) k++;
	  //if(k==50) break;
	//}
	cout<<"Check this if in TOPO_traversal_wrapper preds"<<endl;
	return;
      }
      _color = _color - 1;
      vi_start->DeleteXEdges(_arg.fifth,1);
      vi_start->SetColorOut(v3,_color);

      if(_color == STAPL_WHITE){//if color is STAPL_WHITE
	TOPO_traversal(v1,v2,v3,preds);
      }
    }
  }//if IsVertex()
  else{
    //cout<<"Warning:: Destination vertex for a remote edge in TOPO_traversal doesn't exist anymore; please check TOPO_traversal_wrapper"<<endl;
  }
}

template <class VERTEX, class WEIGHT>
template <class VISITOR>
int 
pBaseGraph<VERTEX,WEIGHT>::
TOPO_traversal(VID _start, VISITOR& _vis, int _traversal_id,bool _preds){
  //sweep to the graph associated with this trimer

  typename pBaseGraph_type::VI vi,vi_start;
  //typedef typename pBaseGraph_type PBG;//fix for HP

  list<_StaplTriple<typename pBaseGraph_type::VI,int,int> > QT;//queue used for trimer traversals

  //vi , index, direction
  _StaplTriple<typename pBaseGraph_type::VI,int,int> v;
  
  //map<VID, int> remote_color;
  hash_map<VID, int> remote_color;

  STAPL_GRAPH_COLOR _color;    
  vector<VID> succs;
  
  //cout<<"START TOPO traversal"<<endl;

  if(this->IsVertex(_start,&vi_start)){
    //cout<<"Vertex pushed"<<endl;
    if(!_preds){
      _color = vi_start->GetColorIn(_traversal_id) ;
      if(_color == STAPL_WHITE){//if _color is STAPL_WHITE
	vi_start->SetColorIn(_traversal_id,STAPL_WHITE);
	QT.push_back(_StaplTriple<typename pBaseGraph_type::VI,int,int>(vi_start,_traversal_id,STAPL_FWD));
      }
    }
    else{//going back trough preds
      _color = vi_start->GetColorOut(_traversal_id) ;
      if(_color == STAPL_WHITE){//if _color is STAPL_WHITE
	vi_start->SetColorOut(_traversal_id,STAPL_WHITE);
	QT.push_back(_StaplTriple<typename pBaseGraph_type::VI,int,int>(vi_start,_traversal_id,STAPL_BACK));
      }
    }
  }
  
  while(QT.size() > 0){//while there are more vertices
    //pop a vertex from Q
    v = QT.front();
    QT.pop_front();
    
    _vis.vertex(v.first);
    
    //for every child u of v
    if(v.third == STAPL_FWD){
      int t = this->GetAdjacentVertices(v.first,succs);
      v.first->edgelist.clear();
      //cout<<"Adjacency for "<<v.first->vid<<" "<< t <<" "<<succs.size()<<endl;
      for(int v2id=0;v2id < t;v2id++){
	if(!this->IsVertex(succs[v2id],&vi)){
	  //the vertex is not local
	  //here add trough an rmi the vertex in the other 
	  //processor queue of ready vertices
	  
	  Location _n=this->Lookup(succs[v2id]);
	  stapl_assert(_n.ValidLocation(),
		       "Invalid id for the second vertex of an edge in TOPO_traversal");
	  //cout<<"remote vertex "<<succs[v2id]<<" "<<_n.locpid()<<endl;
	  //_StaplQuad<VID,VISITOR,int,bool> args(succs[v2id],_vis,_traversal_id,_preds);
	  _StaplPenta<VID,VISITOR,int,bool,VID> args(succs[v2id],_vis,_traversal_id,_preds,v.first->vid);
	  stapl::async_rmi<pBaseGraph_type, void, _StaplPenta<VID,VISITOR,int,bool,VID> >( _n.locpid(),
			    this->getHandle(),
			    (void (pBaseGraph_type::* const)(_StaplPenta<VID,VISITOR,int,bool,VID>))&pBaseGraph_type::TOPO_traversal_wrapper,args);
	  
	}
	else{
	  //here we are if the vertex is local
	  //first check the _color of the node
	  _color = vi->GetColorIn(v.second) ;
	  if (_color == STAPL_WHITE) {
	    cout<<"Check this if in TOPO_traversal"<<endl;
	    continue;
	  }
	  _color = _color - 1;

	  vi->DeleteXPredEdges(v.first->vid,1);

	  vi->SetColorIn(v.second,_color);
	  if(_color == STAPL_WHITE){//if _color is STAPL_WHITE; zero dependencies
	    QT.push_back(_StaplTriple<typename pBaseGraph_type::VI,int,int>(vi,v.second,STAPL_FWD));
	  }
	}
      }//for all succs
    }//if fwd
    else{
      //here if we have backward also
      int t = this->GetPredVertices(v.first,succs);
      v.first->predecessors.clear();
      //cout<<"Preds for "<<v.first->vid<<" "<< t <<" "<<succs.size()<<endl;
      for(int v2id=0;v2id < t;v2id++){
	if(!this->IsVertex(succs[v2id],&vi)){
	  //the vertex is not local
	  //here add trough an rmi the vertex in the other 
	  //processor queue of ready vertices

	  Location _n=this->Lookup(succs[v2id]);
	  stapl_assert(_n.ValidLocation(),
		       "Invalid id for the second vertex of an edge in TOPO");
	  //cout<<"remote vertex (preds)"<<succs[v2id]<<" "<<_n.locpid()<<endl;
	  _StaplPenta<VID,VISITOR,int,bool,VID> args(succs[v2id],_vis,_traversal_id,_preds,v.first->vid);
	  stapl::async_rmi<pBaseGraph_type, void, _StaplPenta<VID,VISITOR,int,bool,VID> >( _n.locpid(),
			    this->getHandle(),
			    //(void (pBaseGraph_type::*)(_StaplQuad<VID,VISITOR,int,bool>))&pBaseGraph_type::TOPO_traversal_wrapper<VISITOR>,args);
			    (void (pBaseGraph_type::* const)(_StaplPenta<VID,VISITOR,int,bool,VID>))&pBaseGraph_type::TOPO_traversal_wrapper,args);
	}
	else{
	  //here we are if the vertex is local
	  //first check the _color of the node
	  _color = vi->GetColorOut(v.second) ;
	  if (_color == STAPL_WHITE) {
	    cout<<"Check this if in TOPO_traversal preds"<<endl;
	    continue;
	  }

	  vi->DeleteXEdges(v.first->vid,1);

	  _color = _color - 1;
	  vi->SetColorOut(v.second,_color);
 
	  if(_color == STAPL_WHITE){//if _color is STAPL_WHITE
	    QT.push_back(_StaplTriple<typename pBaseGraph_type::VI,int,int>(vi,v.second,STAPL_BACK));
	  }
	}
      }//for all succs
    }//else  --> STAPL_BACK
  }//while
  return OK;
}//end method TOPO_traversal 
 
/*
*Allocate space for _tr_nr symultaneous traversals
*/
template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
SetTraversalNumber(int _tr_nr){
  typename pBaseGraph_type::VI vi;
  for(vi=this->local_begin();vi != this->local_end();vi++){    
    vi->SetTraversalNumber(_tr_nr);
  }
}


/*
 *Reset the colors to white for all traversals
*/
template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
ResetTraversals(){
  typename pBaseGraph_type::VI vi;
  for(vi=this->local_begin();vi != this->local_end();vi++){
    vi->ResetTraversals();
  }
}

/**
 *Reset the colors to white for traversal _tr_id
*/
template <class VERTEX, class WEIGHT>
int
pBaseGraph<VERTEX,WEIGHT>::
ResetTraversal(int _tr_id){
  typename pBaseGraph_type::VI vi;
  for(vi=this->local_begin();vi != this->local_end();vi++){
    vi->ResetTraversal(_tr_id);
  }
  return OK;
}

////////////////////////////////////////////////////////////////////
//   Set in/out dependencies
////////////////////////////////////////////////////////////////////
/*
 *Set the in/out dependencies number for one specific traversal;
*/
template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
SetDependencies(){
  typename pBaseGraph_type::VI vi;
  for(vi=this->local_begin();vi != this->local_end();vi++){
    vi->SetDependencies();
  }
}

/*
 *Set the in/out dependencies number for one specific traversal;
*/

template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
SetDependencies(int _tr_id){
  typename pBaseGraph_type::VI vi;
  for(vi=this->local_begin();vi != this->local_end();vi++){
    vi->SetDependencies(_tr_id);
    //if(vi->in[_tr_id]==0) in_c++;
    //if(vi->out[_tr_id]==0) out_c++;
  }
  //printf("$$ %d %d $$\n",in_c,out_c);
}

template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
GetZeroDependencies(int _tr_id,vector<VID>& start_in, vector<VID>& start_out){
  typename pBaseGraph_type::VI vi;

  start_in.clear();
  start_out.clear();

  for(vi=this->local_begin();vi != this->local_end();vi++){
    if(vi->GetColorIn(_tr_id)==0) start_in.push_back(vi->vid);
    if(vi->GetColorOut(_tr_id)==0) start_out.push_back(vi->vid);
  }
}

template <class VERTEX, class WEIGHT>
void
pBaseGraph<VERTEX,WEIGHT>::
GetZeroDependencies(vector<VID>& start_in, vector<VID>& start_out){
  typename pBaseGraph_type::VI vi;

  start_in.clear();
  start_out.clear();

  for(vi=this->local_begin();vi != this->local_end();vi++){
    if(vi->edgelist.size() == 0) 
      start_out.push_back(vi->vid);
    if(vi->predecessors.size() == 0) 
      start_in.push_back(vi->vid);
  }
}



//==============================================================================
//      Methods for getting edge weight field and vertex field
//==============================================================================
//GetVertexField with no arguments

template <class VERTEX, class WEIGHT>
template<class T> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetVertexField(_StaplPair<VID,T (VERTEX::*)()> args){
  pContainerPart_type* _part;
  VERTEX* pvt;
  PARTID _pid;
  //if local
  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      pvt = &(vi->data);
      return (pvt->*(args.second))();     
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
      T t;
      return t;
    }
  }
  cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
  T t;
  return t;
}


template <class VERTEX, class WEIGHT>
template<class T> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetVertexField(VID _vid,T (VERTEX::* method)()){
  
  typedef _StaplPair<VID,T (VERTEX::*)()> ARGS;

  pContainerPart_type* _part;
  VERTEX* pvt;
  PARTID _pid;

  //if local
  if(this->IsLocal(_vid,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_vid,&vi)){
      pvt = &(vi->data);
      return (pvt->*(method))();     
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
      T t;
      return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_vid);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField");
    ARGS args(_vid, method);
    return stapl::sync_rmi<pBaseGraph_type, T,ARGS>(_n.locpid(),
			   this->getHandle(),
			   (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetVertexField,
			   args);
  }
}


//GetVertexField with no arguments but CONST
template <class VERTEX, class WEIGHT>
template<class T> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetVertexField(_StaplPair<VID,T (VERTEX::*)() const> args){
  pContainerPart_type* _part;
  VERTEX* pvt;
  PARTID _pid;
  //if local
  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      pvt = &(vi->data);
      return (pvt->*(args.second))();     
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
      T t;
      return t;
    }
  }
  cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
  T t;
  return t;
}


template <class VERTEX, class WEIGHT>
template<class T> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetVertexField(VID _vid,T (VERTEX::* method)() const){
  typedef _StaplPair<VID,T (VERTEX::*)() const> ARGS;

  pContainerPart_type* _part;
  VERTEX* pvt;
  PARTID _pid;

  //if local
  if(this->IsLocal(_vid,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_vid,&vi)){
      pvt = &(vi->data);
      return (pvt->*(method))();     
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
      T t;
      return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_vid);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField");
    ARGS args(_vid, method);
    return stapl::sync_rmi<pBaseGraph_type,T,ARGS>(_n.locpid(),
			   this->getHandle(),
			   (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetVertexField,
			   args);
  }  
}


//GetVertexField with ONE argument
template <class VERTEX, class WEIGHT>
template<class T, class ARG> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetVertexField(_StaplTriple<VID,T (VERTEX::*)(ARG&),ARG> args){
  pContainerPart_type* _part;
  VERTEX* pvt;
  PARTID _pid;
  //if local
  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      pvt = &(vi->data);
      return (pvt->*(args.second))(args.third);     
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
      T t;
      return t;
    }
  }
  cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
  T t;
  return t;
}


template <class VERTEX, class WEIGHT>
template<class T,class ARG> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetVertexField(VID _vid,T (VERTEX::* method)(ARG&),ARG& _arg){
  
  typedef _StaplTriple<VID,T (VERTEX::*)(ARG&),ARG> ARGS;

  pContainerPart_type* _part;
  VERTEX* pvt;
  PARTID _pid;

  //if local
  if(this->IsLocal(_vid,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_vid,&vi)){
      pvt = &(vi->data);
      return (pvt->*(method))(_arg);     
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
      T t;
      return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_vid);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField");
    ARGS args(_vid, method,_arg);
    return stapl::sync_rmi<pBaseGraph_type,T,ARGS>(_n.locpid(),
			   this->getHandle(),
			   (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetVertexField,
			   args);
  }  
}

//GetVertexField with ONE argument and CONST
template <class VERTEX, class WEIGHT>
template<class T, class ARG> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetVertexField(_StaplTriple<VID,T (VERTEX::*)(ARG&) const,ARG> args){
  pContainerPart_type* _part;
  VERTEX* pvt;
  PARTID _pid;
  //if local
  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      pvt = &(vi->data);
      return (pvt->*(args.second))(args.third);     
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
      T t;
      return t;
    }
  }
  cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
  T t;
  return t;
}


template <class VERTEX, class WEIGHT>
template<class T,class ARG> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetVertexField(VID _vid,T (VERTEX::* method)(ARG&) const ,ARG& _arg){
  
  typedef _StaplTriple<VID,T (VERTEX::*)(ARG&) const,ARG> ARGS;

  pContainerPart_type* _part;
  VERTEX* pvt;
  PARTID _pid;

  //if local
  if(this->IsLocal(_vid,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_vid,&vi)){
      pvt = &(vi->data);
      return (pvt->*(method))(_arg);     
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
      T t;
      return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_vid);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField");
    ARGS args(_vid, method,_arg);
    return stapl::sync_rmi<pBaseGraph_type,T,ARGS>(_n.locpid(),
			   this->getHandle(),
			   (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetVertexField,
			   args);
  }  
}






//    GetWeightField with NO argument
template <class VERTEX, class WEIGHT>
template<class T> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetWeightField(_StaplTriple<VID,VID,T (WEIGHT::*)()> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local

  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == args.second){
	  pwt = &(ei->weight);
	  return (pwt->*(args.third))(); 
	}
	ei++;
      }
      //maybe exception
      cout<<"ERROR:: __GetWeightField: invalid vertex2 id "<<args.second<<endl;
      T t;
      return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __GetWeightField: invalid vertex1 id"<<endl;
      T t;
      return t;
    }
  }
  else{
    cout<<"ERROR:: __GetWeightField: invalid vertex1 id"<<endl;
    T t;
    return t;
  }
}


template <class VERTEX, class WEIGHT>
template<class T> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetWeightField(VID _v1id,VID _v2id, T (WEIGHT::* method)()){

  typedef _StaplTriple<VID,VID,T (WEIGHT::*)()> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == _v2id){
	  pwt = &(ei->weight);
	  return (pwt->*(method))(); 
	}
	ei++;
      }
      //maybe exception
      cout<<"ERROR:: pGetWeightField: invalid vertex2 id:"<<_v2id<<endl;
      T t;
      return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetWeightField: invalid vertex 1 id"<<endl;
      T t;
      return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField");
    ARGS args(_v1id,_v2id,method);
    return stapl::sync_rmi<pBaseGraph_type,T,ARGS>(_n.locpid(),
		    this->getHandle(),
		    (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetWeightField,args);
  }  
}

//    GetWeightField with NO argument but CONST

template <class VERTEX, class WEIGHT>
template<class T> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetWeightField(_StaplTriple<VID,VID,T (WEIGHT::*)() const> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local

  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == args.second){
	  pwt = &(ei->weight);
	  return (pwt->*(args.third))(); 
	}
	ei++;
      }
      //maybe exception
      cout<<"ERROR:: __GetWeightField: invalid vertex2 id "<<args.second<<endl;
      T t;
      return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __GetWeightField: invalid vertex1 id"<<endl;
      T t;
      return t;
    }
  }
  else{
    cout<<"ERROR:: __GetWeightField: invalid vertex1 id"<<endl;
    T t;
    return t;
  }
}


template <class VERTEX, class WEIGHT>
template<class T> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetWeightField(VID _v1id,VID _v2id, T (WEIGHT::* method)() const){

  typedef _StaplTriple<VID,VID,T (WEIGHT::*)() const > ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == _v2id){
	  pwt = &(ei->weight);
	  return (pwt->*(method))(); 
	}
	ei++;
      }
      //maybe exception
      cout<<"ERROR:: pGetWeightField: invalid vertex2 id:"<<_v2id<<endl;
      T t;
      return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetWeightField: invalid vertex 1 id"<<endl;
      T t;
      return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField");
    ARGS args(_v1id,_v2id,method);
    return stapl::sync_rmi(_n.locpid(),
		    this->getHandle(),
		    (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetWeightField,args);
  }  
}




//    GetWeightField with one argument

template <class VERTEX, class WEIGHT>
template<class T, class ARG> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetWeightField(_StaplQuad<VID,VID,T (WEIGHT::*)(ARG&),ARG> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local

  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == args.second){
	  pwt = &(ei->weight);
	  return (pwt->*(args.third))(args.fourth); 
	}
	ei++;
      }
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid vertex2 id "<<args.second<<endl;
      T t;
      return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
      T t;
      return t;
    }
  }
  else{
    cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
    T t;
    return t;
  }
}


template <class VERTEX, class WEIGHT>
template<class T,class ARG> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetWeightField(VID _v1id,VID _v2id, T (WEIGHT::* method)(ARG&),ARG& _arg){

  typedef _StaplQuad<VID,VID,T (WEIGHT::*)(ARG&),ARG> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == _v2id){
	  pwt = &(ei->weight);
	  return (pwt->*(method))(_arg); 
	}
	ei++;
      }
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid vertex2 id:"<<_v2id<<endl;
      T t;
      return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid vertex 1 id"<<endl;
      T t;
      return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField(arg)");
    ARGS args(_v1id,_v2id,method,_arg);
    return stapl::sync_rmi<pBaseGraph_type,T,ARGS>(_n.locpid(),
		    this->getHandle(),
		    (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetWeightField,args);
  }  
}


//    GetWeightField with one argument CONST

template <class VERTEX, class WEIGHT>
template<class T, class ARG> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetWeightField(_StaplQuad<VID,VID,T (WEIGHT::*)(ARG&) const,ARG> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local

  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == args.second){
	  pwt = &(ei->weight);
	  return (pwt->*(args.third))(args.fourth); 
	}
	ei++;
      }
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid vertex2 id "<<args.second<<endl;
      T t;
      return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
      T t;
      return t;
    }
  }
  else{
    cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
    T t;
    return t;
  }
}


template <class VERTEX, class WEIGHT>
template<class T,class ARG> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetWeightField(VID _v1id,VID _v2id, T (WEIGHT::* method)(ARG&) const,ARG& _arg){

  typedef _StaplQuad<VID,VID,T (WEIGHT::*)(ARG&) const,ARG> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == _v2id){
	  pwt = &(ei->weight);
	  return (pwt->*(method))(_arg); 
	}
	ei++;
      }
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid vertex2 id:"<<_v2id<<endl;
      T t;
      return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid vertex 1 id"<<endl;
      T t;
      return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField(arg)");
    ARGS args(_v1id,_v2id,method,_arg);
    return stapl::sync_rmi<pBaseGraph_type,T,ARGS>(_n.locpid(),
		    this->getHandle(),
		    (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetWeightField,args);
  }  
}


//   GetWeightField with TWO arguments

template <class VERTEX, class WEIGHT>
template<class T, class T1,class T2> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetWeightField(_StaplPenta<VID,VID,T (WEIGHT::*)(T1&,T2&),T1,T2> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local

  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == args.second){
	  pwt = &(ei->weight);
	  return (pwt->*(args.third))(args.fourth,args.fifth); 
	}
	ei++;
      }
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid vertex2 id "<<args.second<<endl;
      T t;
      return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
      T t;
      return t;
    }
  }
  else{
    cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
    T t;
    return t;
  }
}


template <class VERTEX, class WEIGHT>
template<class T,class T1,class T2> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetWeightField(VID _v1id,VID _v2id, T (WEIGHT::* method)(T1&,T2&),T1& _arg1,T2& _arg2){

  typedef _StaplPenta<VID,VID,T (WEIGHT::*)(T1&,T2&),T1,T2> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == _v2id){
	  pwt = &(ei->weight);
	  return (pwt->*(method))(_arg1,_arg2); 
	}
	ei++;
      }
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid vertex2 id:"<<_v2id<<endl;
      T t;
      return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid vertex 1 id"<<endl;
      T t;
      return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField(arg)");
    ARGS args(_v1id,_v2id,method,_arg1,_arg2);
    return stapl::sync_rmi<pBaseGraph_type,T,ARGS>(_n.locpid(),
		    this->getHandle(),
		    (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetWeightField,args);
  }  
}

//    GetWeightField with TWO arguments and const method

template <class VERTEX, class WEIGHT>
template<class T, class T1,class T2> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetWeightField(_StaplPenta<VID,VID,T (WEIGHT::*)(T1&,T2&) const ,T1,T2> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local

  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == args.second){
	  pwt = &(ei->weight);
	  return (pwt->*(args.third))(args.fourth,args.fifth); 
	}
	ei++;
      }
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid vertex2 id "<<args.second<<endl;
      T t;
      return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
      T t;
      return t;
    }
  }
  else{
    cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
    T t;
    return t;
  }
}


template <class VERTEX, class WEIGHT>
template<class T,class T1,class T2> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetWeightField(VID _v1id,VID _v2id, T (WEIGHT::* method)(T1&,T2&) const,T1& _arg1,T2& _arg2){

  typedef _StaplPenta<VID,VID,T (WEIGHT::*)(T1&,T2&) const,T1,T2> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == _v2id){
	  pwt = &(ei->weight);
	  return (pwt->*(method))(_arg1,_arg2); 
	}
	ei++;
      }
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid vertex2 id:"<<_v2id<<endl;
      T t;
      return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid vertex 1 id"<<endl;
      T t;
      return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField(arg)");
    ARGS args(_v1id,_v2id,method,_arg1,_arg2);
    return stapl::sync_rmi<pBaseGraph_type,T,ARGS>(_n.locpid(),
		    this->getHandle(),
		    (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetWeightField,args);
  }  
}

//
//                         SET methods
//

//SetVertexField with ONE argument
template <class VERTEX, class WEIGHT>
template<class T> 
void
pBaseGraph<VERTEX,WEIGHT>::
__SetVertexField(_StaplTriple<VID,int (VERTEX::*)(T&),T> args){
  pContainerPart_type* _part;
  VERTEX* pvt;
  PARTID _pid;
  //if local
  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      pvt = &(vi->data);
      //the return code is not passed back to the caller;
      //talk about this
      (pvt->*(args.second))(args.third);     
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
    }
  }
}


template <class VERTEX, class WEIGHT>
template<class T> 
int
pBaseGraph<VERTEX,WEIGHT>::
pSetVertexField(VID _vid,int (VERTEX::* method)(T&),T& _arg){
  
  typedef _StaplTriple<VID,int (VERTEX::*)(T&),T> ARGS;

  pContainerPart_type* _part;
  VERTEX* pvt;
  PARTID _pid;

  //if local
  if(this->IsLocal(_vid,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_vid,&vi)){
      pvt = &(vi->data);
      return (pvt->*(method))(_arg);     
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
      return ERROR;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_vid);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField");
    ARGS args(_vid, method,_arg);
    stapl::async_rmi<pBaseGraph_type,void,ARGS>(_n.locpid(),
		     this->getHandle(),
		     (void (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__SetVertexField,
		     args);
    return OK;
  } 
}


//SetVertexField with TWO arguments
template <class VERTEX, class WEIGHT>
template<class T1,class T2> 
void
pBaseGraph<VERTEX,WEIGHT>::
__SetVertexField(_StaplQuad<VID,int (VERTEX::*)(T1&,T2&),T1,T2> args){
  pContainerPart_type* _part;
  VERTEX* pvt;
  PARTID _pid;
  //if local
  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      pvt = &(vi->data);
      //the method is VOID
      (pvt->*(args.second))(args.third,args.fourth);     
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
    }
  }
}


template <class VERTEX, class WEIGHT>
template<class T1,class T2> 
int
pBaseGraph<VERTEX,WEIGHT>::
pSetVertexField(VID _vid,int (VERTEX::* method)(T1&,T2&),T1& _arg1,T2& _arg2){
  
  typedef _StaplQuad<VID,int (VERTEX::*)(T1&,T2&),T1,T2> ARGS;

  pContainerPart_type* _part;
  VERTEX* pvt;
  PARTID _pid;

  //if local
  if(this->IsLocal(_vid,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_vid,&vi)){
      pvt = &(vi->data);
      return (pvt->*(method))(_arg1,_arg2);     
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
      return ERROR;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_vid);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField");
    ARGS args(_vid, method,_arg1,_arg2);
    stapl::async_rmi<pBaseGraph_type,void,ARGS>(_n.locpid(),
		     this->getHandle(),
		     (void (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__SetVertexField,
		     args);
    return OK;
  } 
}

//SetVertexField with THREE arguments
template <class VERTEX, class WEIGHT>
template<class T1,class T2,class T3> 
void
pBaseGraph<VERTEX,WEIGHT>::
__SetVertexField(_StaplPenta<VID,int (VERTEX::*)(T1&,T2&,T3&),T1,T2,T3> args){
  pContainerPart_type* _part;
  VERTEX* pvt;
  PARTID _pid;
  //if local
  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      pvt = &(vi->data);
      //the method is VOID
      (pvt->*(args.second))(args.third,args.fourth,args.fifth);     
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
    }
  }
}


template <class VERTEX, class WEIGHT>
template<class T1,class T2,class T3> 
int
pBaseGraph<VERTEX,WEIGHT>::
pSetVertexField(VID _vid,int (VERTEX::* method)(T1&,T2&,T3&),T1& _arg1,T2& _arg2, T3& _arg3){
  
  typedef _StaplPenta<VID,int (VERTEX::*)(T1&,T2&,T3&),T1,T2,T3> ARGS;

  pContainerPart_type* _part;
  VERTEX* pvt;
  PARTID _pid;

  //if local
  if(this->IsLocal(_vid,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_vid,&vi)){
      pvt = &(vi->data);
      return (pvt->*(method))(_arg1,_arg2,_arg3);     
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetVertexField: invalid vertex id"<<endl;
      return ERROR;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_vid);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField");
    ARGS args(_vid, method,_arg1,_arg2,_arg3);
    stapl::async_rmi<pBaseGraph_type,void, ARGS>(_n.locpid(),
		     this->getHandle(),
		     (void (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__SetVertexField,
		     args);
    return OK;
  } 
}

//SetWeightField with ONE argument

template <class VERTEX, class WEIGHT>
template<class T> 
void
pBaseGraph<VERTEX,WEIGHT>::
__SetWeightField(_StaplQuad<VID,VID,int (WEIGHT::*)(T&),T> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == args.second){
	  pwt = &(ei->weight);
	  //apply the method to all edges <source, dest>
	  (pwt->*(args.third))(args.fourth);
	}
	ei++;
      }
      //maybe exception
      //cout<<"ERROR:: __SetWeightField: invalid vertex2 id "<<args.second<<endl;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __SetWeightField: invalid vertex1 id"<<endl;
    }
  }
  else{
    cout<<"ERROR:: __SetWeightField: invalid vertex1 id"<<endl;
  }
}


template <class VERTEX, class WEIGHT>
template<class T> 
int
pBaseGraph<VERTEX,WEIGHT>::
pSetWeightField(VID _v1id,VID _v2id, int (WEIGHT::* method)(T&), T& _arg){

  typedef _StaplQuad<VID,VID,int (WEIGHT::*)(T&),T> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	//apply the method to all edges <source, dest>
	if(ei->vertex2id == _v2id){
	  pwt = &(ei->weight);
	  (pwt->*(method))(_arg); 
	}
	ei++;
      }
      //maybe exception
      //      cout<<"ERROR:: pSetWeightField: invalid vertex2 id:"<<_v2id<<endl;
      return OK;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pSetWeightField: invalid vertex 1 id"<<endl;
      return ERROR;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pSetVertexField");
    ARGS args(_v1id,_v2id,method,_arg);
    stapl::async_rmi<pBaseGraph_type,void,ARGS>(_n.locpid(),
		     this->getHandle(),
		     (void (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__SetWeightField,args);
    return OK;
  }  
}


//SetWeightField with TWO arguments
template <class VERTEX, class WEIGHT>
template<class T1,class T2> 
void
pBaseGraph<VERTEX,WEIGHT>::
__SetWeightField(_StaplPenta<VID,VID,int (WEIGHT::*)(T1&,T2&),T1,T2> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local

  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	//apply the method to all edges <source, dest>
	if(ei->vertex2id == args.second){
	  pwt = &(ei->weight);
	  (pwt->*(args.third))(args.fourth,args.fifth); 
	}
	ei++;
      }
      //maybe exception
      //cout<<"ERROR:: __SetWeightField: invalid vertex2 id "<<args.second<<endl;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __SetWeightField: invalid vertex1 id"<<endl;
    }
  }
  else{
    cout<<"ERROR:: __SetWeightField: invalid vertex1 id"<<endl;
  }
}


template <class VERTEX, class WEIGHT>
template<class T1,class T2> 
int
pBaseGraph<VERTEX,WEIGHT>::
pSetWeightField(VID _v1id,VID _v2id, int (WEIGHT::* method)(T1&,T2&), T1& _arg1,T2& _arg2){

  typedef _StaplPenta<VID,VID,int (WEIGHT::*)(T1&,T2&),T1,T2> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	//apply the method to all edges <source, dest>
	if(ei->vertex2id == _v2id){
	  pwt = &(ei->weight);
	  (pwt->*(method))(_arg1,_arg2); 
	}
	ei++;
      }
      //maybe exception
      //cout<<"ERROR:: pSetWeightField: invalid vertex2 id:"<<_v2id<<endl;
      return OK;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pSetWeightField: invalid vertex 1 id"<<endl;
      return ERROR;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pSetVertexField");
    ARGS args(_v1id,_v2id,method,_arg1,_arg2);
    stapl::async_rmi<pBaseGraph_type,void,ARGS>(_n.locpid(),
		     this->getHandle(),
		     (void (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__SetWeightField,args);
    return OK;
  }  
}

//SetWeightField with THREE arguments
template <class VERTEX, class WEIGHT>
template<class T1,class T2,class T3> 
void
pBaseGraph<VERTEX,WEIGHT>::
__SetWeightField(_StaplSix<VID,VID,int (WEIGHT::*)(T1&,T2&,T3&),T1,T2,T3> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local

  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == args.second){
	  pwt = &(ei->weight);
	  (pwt->*(args.third))(args.fourth,args.fifth,args.sixth); 
	}
	ei++;
      }
      //maybe exception
      //cout<<"ERROR:: __SetWeightField: invalid vertex2 id "<<args.second<<endl;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __SetWeightField: invalid vertex1 id"<<endl;
    }
  }
  else{
    cout<<"ERROR:: __SetWeightField: invalid vertex1 id"<<endl;
  }
}


template <class VERTEX, class WEIGHT>
template<class T1,class T2,class T3> 
int
pBaseGraph<VERTEX,WEIGHT>::
pSetWeightField(VID _v1id,VID _v2id, int (WEIGHT::* method)(T1&,T2&,T3&), T1& _arg1,T2& _arg2,T3& _arg3){

  typedef _StaplSix<VID,VID,int (WEIGHT::*)(T1&,T2&,T3&),T1,T2,T3> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->edgelist.begin();
      while(ei != vi->edgelist.end()){
	if(ei->vertex2id == _v2id){
	  pwt = &(ei->weight);
	  (pwt->*(method))(_arg1,_arg2,_arg3); 
	}
	ei++;
      }
      //maybe exception
      //cout<<"ERROR:: pSetWeightField: invalid vertex2 id:"<<_v2id<<endl;
      return OK;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pSetWeightField: invalid vertex 1 id"<<endl;
      return ERROR;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pSetVertexField");
    ARGS args(_v1id,_v2id,method,_arg1,_arg2,_arg3);
    stapl::async_rmi<pBaseGraph_type,void,ARGS>(_n.locpid(),
		     this->getHandle(),
		     (void (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__SetWeightField,args);
    return OK;
  }  
}


// SET WEIGHT FIELD EDGE ID


//SetWeightField with ONE argument

template <class VERTEX, class WEIGHT>
template<class T> 
void
pBaseGraph<VERTEX,WEIGHT>::
__SetWeightFieldEdgeId(_StaplQuad<VID,int,int (WEIGHT::*)(T&),T> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(args.second);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	(pwt->*(args.third))(args.fourth);
	return;
      }
      //maybe exception
      cout<<"ERROR:: __SetWeightFieldEdgeId: invalid edge id "<<args.second<<endl;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __SetWeightFieldEdgeId: invalid vertex1 id"<<endl;
    }
  }
  else{
    cout<<"ERROR:: __SetWeightFieldEdgeId: invalid vertex1 id"<<endl;
  }
}


template <class VERTEX, class WEIGHT>
template<class T> 
int
pBaseGraph<VERTEX,WEIGHT>::
pSetWeightFieldEdgeId(VID _v1id,int _edgeid, int (WEIGHT::* method)(T&), T& _arg){

  typedef _StaplQuad<VID,int,int (WEIGHT::*)(T&),T> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(_edgeid);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	(pwt->*(method))(_arg);
	return OK;
      }
      //maybe exception
      //cout<<"ERROR:: pSetWeightField: invalid vertex2 id:"<<_edgeid<<endl;
      return OK;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pSetWeightField: invalid vertex 1 id"<<endl;
      return ERROR;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pSetVertexField");
    ARGS args(_v1id,_edgeid,method,_arg);
    stapl::async_rmi<pBaseGraph_type, void,ARGS>(_n.locpid(),
		     this->getHandle(),
		     (void (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__SetWeightFieldEdgeId,args);
    return OK;
  }  
}


//SetWeightField with TWO arguments
template <class VERTEX, class WEIGHT>
template<class T1,class T2> 
void
pBaseGraph<VERTEX,WEIGHT>::
__SetWeightFieldEdgeId(_StaplPenta<VID,int,int (WEIGHT::*)(T1&,T2&),T1,T2> args){
  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(args.second);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	(pwt->*(args.third))(args.fourth, args.fifth);
	return;
      }
      //maybe exception
      cout<<"ERROR:: __SetWeightFieldId: invalid edge id "<<args.second<<endl;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __SetWeightFieldId: invalid vertex1 id"<<endl;
    }
  }
  else{
    cout<<"ERROR:: __SetWeightFieldId: invalid vertex1 id"<<endl;
  }
}


template <class VERTEX, class WEIGHT>
template<class T1,class T2> 
int
pBaseGraph<VERTEX,WEIGHT>::
pSetWeightFieldEdgeId(VID _v1id,int _edgeid, int (WEIGHT::* method)(T1&,T2&), T1& _arg1,T2& _arg2){

  typedef _StaplPenta<VID,int,int (WEIGHT::*)(T1&,T2&),T1,T2> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(_edgeid);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	(pwt->*(method))(_arg1,_arg2);
	return OK;
      }
      //maybe exception
      cout<<"ERROR:: pSetWeightField: invalid edge id:"<<_edgeid<<endl;
      return ERROR;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pSetWeightField: invalid vertex 1 id"<<endl;
      return ERROR;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pSetVertexField");
    ARGS args(_v1id,_edgeid,method,_arg1,_arg2);
    stapl::async_rmi<pBaseGraph_type, void,ARGS>(_n.locpid(),
		     this->getHandle(),
		     (void (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__SetWeightFieldEdgeId,args);
    return OK;
  }  
}

//SetWeightField with THREE arguments
template <class VERTEX, class WEIGHT>
template<class T1,class T2,class T3> 
void
pBaseGraph<VERTEX,WEIGHT>::
__SetWeightFieldEdgeId(_StaplSix<VID,int,int (WEIGHT::*)(T1&,T2&,T3&),T1,T2,T3> args){
 pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(args.second);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	(pwt->*(args.third))(args.fourth,args.fifth,args.sixth); 
	return;
      }
      //maybe exception
      cout<<"ERROR:: __SetWeightFieldId: invalid edge id "<<args.second<<endl;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __SetWeightFieldId: invalid vertex1 id"<<endl;
    }
  }
  else{
    cout<<"ERROR:: __SetWeightFieldId: invalid vertex1 id"<<endl;
  }
}


template <class VERTEX, class WEIGHT>
template<class T1,class T2,class T3> 
int
pBaseGraph<VERTEX,WEIGHT>::
pSetWeightFieldEdgeId(VID _v1id,int _edgeid, int (WEIGHT::* method)(T1&,T2&,T3&), T1& _arg1,T2& _arg2,T3& _arg3){
  typedef _StaplSix<VID,int,int (WEIGHT::*)(T1&,T2&,T3&),T1,T2,T3> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(_edgeid);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	(pwt->*(method))(_arg1,_arg2,_arg3);
	return OK;
      }
      //maybe exception
      cout<<"ERROR:: pSetWeightField: invalid edge id:"<<_edgeid<<endl;
      return ERROR;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pSetWeightField: invalid vertex 1 id"<<endl;
      return ERROR;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pSetVertexField");
    ARGS args(_v1id,_edgeid,method,_arg1,_arg2,_arg3);
    stapl::async_rmi<pBaseGraph_type, void,ARGS>(_n.locpid(),
		     this->getHandle(),
		     (void (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__SetWeightFieldEdgeId,args);
    return OK;
  }  
}

//                GetWeightFieldEdgeId
//    GetWeightField with NO argument

template <class VERTEX, class WEIGHT>
template<class T> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetWeightFieldEdgeId(_StaplTriple<VID,int,T (WEIGHT::*)()> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local

  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(args.second);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	return (pwt->*(args.third))();
      }
      //maybe exception
      cout<<"ERROR:: __GetWeightField: invalid edge id "<<args.second<<endl;
      T t; return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __GetWeightField: invalid vertex1 id"<<endl;
      T t; return t;
    }
  }
  else{
    cout<<"ERROR:: __GetWeightField: invalid vertex1 id"<<endl;
    T t; return t;
  }
}


template <class VERTEX, class WEIGHT>
template<class T> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetWeightFieldEdgeId(VID _v1id,int _edgeid, T (WEIGHT::* method)()){

  typedef _StaplTriple<VID,int,T (WEIGHT::*)()> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(_edgeid);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	return (pwt->*(method))();
      }
      //maybe exception
      cout<<"ERROR:: pGetWeightField: invalid edge id:"<<_edgeid<<endl;
      T t; return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetWeightField: invalid vertex 1 id"<<endl;
      T t; return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField");
    ARGS args(_v1id,_edgeid,method);
    return stapl::sync_rmi<pBaseGraph_type,T,ARGS>(_n.locpid(),
		    this->getHandle(),
		    (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetWeightFieldEdgeId,args);
  }  
}

//    GetWeightField with NO argument but CONST

template <class VERTEX, class WEIGHT>
template<class T> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetWeightFieldEdgeId(_StaplTriple<VID,int,T (WEIGHT::*)() const> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local

  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(args.second);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	return (pwt->*(args.third))();
      }
      //maybe exception
      cout<<"ERROR:: __GetWeightField: invalid edge id "<<args.second<<endl;
      T t; return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __GetWeightField: invalid vertex1 id"<<endl;
      T t; return t;
    }
  }
  else{
    cout<<"ERROR:: __GetWeightField: invalid vertex1 id"<<endl;
    T t; return t;
  }
}


template <class VERTEX, class WEIGHT>
template<class T> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetWeightFieldEdgeId(VID _v1id,int _edgeid, T (WEIGHT::* method)() const){

  typedef _StaplTriple<VID,int,T (WEIGHT::*)() const > ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(_edgeid);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	return (pwt->*(method))();
      }
      //maybe exception
      cout<<"ERROR:: pGetWeightField: invalid edge id:"<<_edgeid<<endl;
      T t; return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetWeightField: invalid vertex 1 id"<<endl;
      T t; return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField");
    ARGS args(_v1id,_edgeid,method);
    return stapl::sync_rmi<pBaseGraph_type,T,ARGS>(_n.locpid(),
		    this->getHandle(),
		    (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetWeightFieldEdgeId,args);
  }  
}

//    GetWeightField with one argument
template <class VERTEX, class WEIGHT>
template<class T, class ARG> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetWeightFieldEdgeId(_StaplQuad<VID,int,T (WEIGHT::*)(ARG&),ARG> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(args.second);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	return (pwt->*(args.third))(args.fourth);
      }
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid edge id "<<args.second<<endl;
      T t; return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
      T t; return t;
    }
  }
  else{
    cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
    T t; return t;
  }
}


template <class VERTEX, class WEIGHT>
template<class T,class ARG> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetWeightFieldEdgeId(VID _v1id,VID _edgeid, T (WEIGHT::* method)(ARG&),ARG& _arg){

  typedef _StaplQuad<VID,int,T (WEIGHT::*)(ARG&),ARG> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(_edgeid);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	return (pwt->*(method))(_arg);
      }
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid vertex2 id:"<<_edgeid<<endl;
      T t; return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid vertex 1 id"<<endl;
      T t; return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField(arg)");
    ARGS args(_v1id,_edgeid,method,_arg);
    return stapl::sync_rmi<pBaseGraph_type,T,ARGS>(_n.locpid(),
		    this->getHandle(),
		    (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetWeightFieldEdgeId,args);
  }  
}


//   GetWeightField with one argument but const

template <class VERTEX, class WEIGHT>
template<class T, class ARG> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetWeightFieldEdgeId(_StaplQuad<VID,int,T (WEIGHT::*)(ARG&) const,ARG> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local

  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(args.second);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	return (pwt->*(args.third))(args.fourth);
      }
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid edge id "<<args.second<<endl;
      T t; return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
      T t; return t;
    }
  }
  else{
    cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
    T t; return t;
  }
}


template <class VERTEX, class WEIGHT>
template<class T,class ARG> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetWeightFieldEdgeId(VID _v1id,VID _edgeid, T (WEIGHT::* method)(ARG&) const,ARG& _arg){

  typedef _StaplQuad<VID,int,T (WEIGHT::*)(ARG&) const,ARG> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(_edgeid);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	return (pwt->*(method))(_arg);
      }
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid edge id:"<<_edgeid<<endl;
      T t; return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid vertex 1 id"<<endl;
      T t; return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField(arg)");
    ARGS args(_v1id,_edgeid,method,_arg);
    return stapl::sync_rmi<pBaseGraph_type,T,ARGS>(_n.locpid(),
		    this->getHandle(),
		    (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetWeightFieldEdgeId,args);
  }  
}


//    GetWeightField with TWO arguments
template <class VERTEX, class WEIGHT>
template<class T, class T1,class T2> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetWeightFieldEdgeId(_StaplPenta<VID,int,T (WEIGHT::*)(T1&,T2&),T1,T2> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(args.second);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	return (pwt->*(args.third))(args.fourth,args.fifth);
      }
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid edge id "<<args.second<<endl;
      T t; return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
      T t; return t;
    }
  }
  else{
    cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
    T t; return t;
  }
}


template <class VERTEX, class WEIGHT>
template<class T,class T1,class T2> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetWeightFieldEdgeId(VID _v1id,VID _edgeid, T (WEIGHT::* method)(T1&,T2&),T1& _arg1, T2& _arg2){

  typedef _StaplPenta<VID,int,T (WEIGHT::*)(T1&,T2&),T1,T2> ARGS;

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(_edgeid);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	return (pwt->*(method))(_arg1,_arg2);
      }
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid edge id:"<<_edgeid<<endl;
      T t; return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid vertex 1 id"<<endl;
      T t; return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField(arg)");
    ARGS args(_v1id,_edgeid,method,_arg1,_arg2);
    return stapl::sync_rmi<pBaseGraph_type,T,ARGS>(_n.locpid(),
		    this->getHandle(),
		    (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetWeightFieldEdgeId,args);
  }  
}


//    GetWeightField with TWO arguments but const

template <class VERTEX, class WEIGHT>
template<class T, class T1,class T2> 
T
pBaseGraph<VERTEX,WEIGHT>::
__GetWeightFieldEdgeId(_StaplPenta<VID,int,T (WEIGHT::*)(T1&,T2&) const,T1,T2> args){

  pContainerPart_type* _part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local

  if(this->IsLocal(args.first,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(args.first,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(args.second);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	return (pwt->*(args.third))(args.fourth,args.fifth);
      }
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid edge id "<<args.second<<endl;
      T t; return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
      T t; return t;
    }
  }
  else{
    cout<<"ERROR:: __GetWeightField(arg): invalid vertex1 id"<<endl;
    T t; return t;
  }
}


template <class VERTEX, class WEIGHT>
template<class T,class T1,class T2> 
T
pBaseGraph<VERTEX,WEIGHT>::
pGetWeightFieldEdgeId(VID _v1id,VID _edgeid, T (WEIGHT::* method)(T1&,T2&) const,T1& _arg1,T2& _arg2){

  typedef _StaplPenta<VID,int,T (WEIGHT::*)(T1&,T2&) const,T1,T2> ARGS;

  pContainerPart_type *_part;
  WEIGHT* pwt;
  PARTID _pid;
  //if local
  if(this->IsLocal(_v1id,_pid)){
    part_iterator vi;
    _part = this->GetPart(_pid);
    if(_part->ContainElement(_v1id,&vi)){
      //here _v1id is valid and identified by vi
      //identify the edge now
      EI ei = vi->find_edgeid_eq(_edgeid);
      if(ei != vi->edgelist.end()){
	pwt = &(ei->weight);
	return (pwt->*(method))(_arg1,_arg2);
      }
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid edge id:"<<_edgeid<<endl;
      T t; return t;
    }
    else{
      //maybe exception
      cout<<"ERROR:: pGetWeightField(arg): invalid vertex 1 id"<<endl;
      T t; return t;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_v1id);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField(arg)");
    ARGS args(_v1id,_edgeid,method,_arg1,_arg2);
    return stapl::sync_rmi<pBaseGraph_type,T,ARGS>(_n.locpid(),
		    this->getHandle(),
		    (T (pBaseGraph_type::*)(ARGS))&pBaseGraph_type::__GetWeightFieldEdgeId,args);
  }  
}
//                          End SET/GET Section
}//end namespace
//@}
#endif
