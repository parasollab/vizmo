/*!
        \file pGraph.h
	\author Gabriel Tanase	
        \date Jan. 9, 03
        \ingroup stapl
        \brief The parallel counterparts for Directed/Undirected, Weighted/Unweighted
        Multi/Non Multi classes; The pgraph will be composed by inheritting from
        the appropriate clasess
        The class hierarchy at this point is the following
                           BaseGraph
                 DG  UG     WG  NWG     MG   NMG
                PDG  PUG   PWG  PNWG   PMG   PNMG
                            PGRAPH
*/

#ifndef PGRAPH_H
#define PGRAPH_H


#include <runtime.h>
#include "Graph.h"
#include "BasePContainer.h"
#include "GraphPart.h"

#include "../distribution/DistributionDefines.h"
#include "../distribution/BaseDistribution.h"
#include "../distribution/GraphDistribution.h"

#include "pBaseGraph.h"


namespace stapl{
/**
 * @ingroup pgraph
 * 
 * @{
*/

//==============================================================================
//                PARALLEL DIRECTED / UNDIRECTED
//==============================================================================
/**Parallel Undirected Graph class. Here we declare methods that have to handle the fact that the graph
 *is undirected. for example AddEdge, DeleteEdge are different from similar methods for Directed
 *Graph.All these methods are inheritted by Graph when the class is used. 
*/
template<class VERTEX, class WEIGHT=int>
class PUG : public UG<VERTEX, WEIGHT, pBaseGraph<VERTEX,WEIGHT> >{
  public:
  typedef pBaseGraph<VERTEX,WEIGHT>  pBaseGraph_type;
  typedef UG<VERTEX,WEIGHT,pBaseGraph<VERTEX,WEIGHT> > UG_type;

  typedef typename pBaseGraph_type::VI   VI;   ///<VI Vertex Iterator
  typedef typename pBaseGraph_type::CVI  CVI;  ///<CVI Constant Vertex Iterator
  typedef typename pBaseGraph_type::RVI  RVI;  ///<RVI Reverse Vertex Iterator
  typedef typename pBaseGraph_type::CRVI CRVI; ///<CRVI Constant Reverse Vertex Iterator

  typedef typename pBaseGraph_type::EI   EI;   ///<EI Edge Iterator
  typedef typename pBaseGraph_type::CEI  CEI;  ///<CEI Constant Edge Iterator
  typedef typename pBaseGraph_type::REI  REI;  ///<REI Reverse Edge Iterator
  typedef typename pBaseGraph_type::CREI CREI; ///<CREI Constant Reverse Edge Iterator

  typedef  pBaseGraph<VERTEX,WEIGHT> Base;

  /**Add edge v1->v2, and v2->v1 to graph with same weight.
   *@param VERTEX v1 user data for first vertex
   *@param VERTEX v2 is any vertex contains user data in the second parameter.
   *@return ERROR if v1 and/or v2 are not in graph.
   *@return ERROR if v1 and v2 have been connected.
   *@note enven 2 edges are created, but they are counted as one edge.
   */
  inline int AddEdge(VERTEX& _v1, VERTEX& _v2, WEIGHT _weight,bool _multi) {
    VID vid1 = this->GetVID(_v1);
    VID vid2 = this->GetVID(_v2);
    return this->AddEdge(vid1,vid2,_weight,_multi);
  }
 
  /**Add edge vid1->vid2, and vid2->vid1 to graph with same weight.
   *@return ERROR if vid1 and/or vid2 are not in graph.
   *@return ERROR if vid1 and vid2 have been connected.
   *@note enven 2 edges are created, but they are counted as one edge.
   */
  inline int AddEdge(VID _v1, VID _v2, WEIGHT _weight,bool _multi) {
    if(Base::AddEdge(_v1,_v2,_weight,_multi)==ERROR) return ERROR;
    return Base::AddEdge(_v2,_v1,_weight,_multi);
  }

  /**Add edge vid1->vid2, and vid2->vid1 to graph with 2 different weights.
   *@return ERROR if vid1 and/or vid2 are not in graph.
   *@return ERROR if vid1 and vid2 have been connected.
   *@note enven 2 edges are created, but they are counted as one edge.
   */
  inline int  AddEdge(VID _v1, VID _v2, pair<WEIGHT,WEIGHT>& _p){
    if(Base::AddEdge(_v1,_v2,_p.first)==ERROR) return ERROR;
    return Base::AddEdge(_v2,_v1,_p.second);
  }
  /**Add edge v1->v2, and v2->v1 to graph with 2 different weights.
   *@param VERTEX v1 user data for first vertex
   *@param VERTEX v2 is any vertex contains user data in the second parameter.
   *@return ERROR if v1 and/or v2 are not in graph.
   *@return ERROR if v1 and v2 have been connected.
   *@note enven 2 edges are created, but they are counted as one edge.
   */
  inline int  AddEdge(VERTEX& _v1, VERTEX& _v2, pair<WEIGHT,WEIGHT>& _p){
    VID vid1 = this->GetVID(_v1);
    VID vid2 = this->GetVID(_v2);
    return this->AddEdge(vid1,vid2,_p);
  }
  /**Delete all edges from vid1 to vid2 and from vid2 to vid1.
   *No matter what n is given, it always chage n to -1.
   *
   *@param n number of edges will be delete.
   *@note vid1 an vid2 should be in this graph
   *@note enven 2 edges are deleted, but they are counted as one edge.
   *@return ERROR if vid1 and/or vid2 are not found. OK if ok.
   */
  inline int DeleteEdge(VID _v1id, VID _v2id, int _n=-1) {
    Base::DeleteEdge(_v1id, _v2id);
    return Base::DeleteEdge(_v2id, _v1id);
  }

  /**Delete n edges from vid1 to vid2 and from vid2 to vid1 
   *of specified weight.
   *
   *No matter what n is given, it always chage n to -1.
   *
   *@param WEIGHT the edges of this weught will be deleted
   *@note vid1 an vid2 should be in this graph
   *@note enven 2 edges are deleted, but they are counted as one edge.
   *@return ERROR if vid1 and/or vid2 are not found. OK if ok.
   */
  int DeleteWtEdge(VID _v1id, VID _v2id, WEIGHT& _w, int _n=-1) {
    cout<<"This method is not checked for correctness"<<endl;
    return Base::DeleteWtEdge(_v1id,_v2id,_w);
  }

  /**Delete all edges from v1 to v2 and from vid2 to vid1.
   *@param VERTEX v1 user data for first vertex
   *@param VERTEX v2 is any vertex contains user data in the second parameter.
   *No matter what n is given, it always chage n to -1.
   *
   *@param n number of edges will be delete.
   *@note v1 an v2 should be in this graph
   *@note enven 2 edges are deleted, but they are counted as one edge.
   *@return ERROR if v1 and/or v2 are not found. OK if ok.
   */  
  int DeleteEdge(VERTEX& _v1, VERTEX& _v2, int _n=-1) {
    VID vid1 = this->GetVID(_v1);
    VID vid2 = this->GetVID(_v2);
    return this->DeleteEdge(vid1,vid2,_n);
  }
  
  /**Delete all edges from v1 to v2 and from vid2 to vid1
   *of specified weight.
   *@param VERTEX v1 user data for first vertex
   *@param VERTEX v2 is any vertex contains user data in the second parameter.
   *No matter what n is given, it always chage n to -1.
   *
   *@param WEIGHT the edges of this weught will be deleted
   *@note v1 an v2 should be in this graph
   *@note enven 2 edges are deleted, but they are counted as one edge.
   *@return ERROR if v1 and/or v2 are not found. OK if ok.
   */
  int DeleteWtEdge(VERTEX& _v1, VERTEX& _v2, WEIGHT _w, int _n=-1) {
    VID vid1 = this->GetVID(_v1);
    VID vid2 = this->GetVID(_v2);
    return this->DeleteEdge(vid1,vid2,_w,_n);
  }

  /**Get Degree of this specified Vertex.
   *@note For undirected graph, outgoing degree is the same as incoming degree.
   *@return Degree if specified VID is found in graph. Otherwise ERROR will be returned.
   */
  int GetVertexOutDegree(VID _v1) {
    return Base::GetVertexOutDegree(_v1);
  }

  /**Get vertices which are adjacent to this specified vertex.
   *@param _v1id the vertex id for which we want adjacent information
   *@param _succ vector of VIDs which are the VIDs of those who are next to the specified vertex.
   *@return the number of elements pushed in _succ
   *If this specified VID is not found, an empty list will be returned.
   *@see GetSuccessors
   */
  int GetAdjacentVertices(VID _v1id, vector<VID>& _succ) /*const*/{
    return  Base::GetAdjacentVertices(_v1id,_succ);
  }

  /**Get vertices which are adjacent to this specified vertex.
   *@param _v1 the user data for the vertex  for which we want adjacent information
   *@param _succ vector of VIDs which are the VIDs of those who are next to the specified vertex.
   *@return the number of elements pushed in _succ
   *If this specified VID is not found, an empty list will be returned.
   *@see GetSuccessors
   */
  int GetAdjacentVertices(VERTEX _v1, vector<VID>& _succ) /*const*/{
    VID _v1id = this->GetVID(_v1);
    return  Base::GetAdjacentVertices(_v1id,_succ);
  }

  int GetAdjacentVertices(VI _vi, vector<VID>& _succ) const{
    return  Base::GetAdjacentVertices(_vi,_succ);
  }

  /**Get data for vertices which are adjacent to this specified vertex(user data).
   *@param _v1id the vertex id for which we want adjacent information
   *@param _succ vector of user data which is the data of those who are next to the specified vertex.
   *@return the number of elements pushed in _succ
   *If the specified VID is not found, an empty list will be returned and error message
   *will be in standard output.
   *@see GetSuccessors
   */
  int GetAdjacentVerticesDATA(VID _v1id, vector<VERTEX>& _succ) /*const */{
    return Base::GetAdjacentVerticesDATA(_v1id,_succ);
  }

  /**Get data for vertices which are adjacent to this specified vertex(user data).
   *@param _v1 the vertex for which we want adjacent information
   *@param _succ vector of user data which is the data of those who are next to the specified vertex.
   *@return the number of elements pushed in _succ
   *If the specified VERTEX is not found, an empty list will be returned and error message
   *will be in standard output.
   *@see GetSuccessors
   */
  int GetAdjacentVerticesDATA(VERTEX& _v1, vector<VERTEX>& _succ) /*const*/ {
    return Base::GetAdjacentVerticesDATA( GetVID(_v1), _succ );
  }

  /**Get Edges which is incident to this specified vertex.
   *Incident edges are edges around this specified vertex.
   *@return a list of edges which is defined by 2 VIDs of its endpoints.
   *Empty list will be returned if this specified VID 
   *is not in graph and error message will be in standard output.
   */
  int GetIncidentEdges(VID _v1id,vector< pair<VID,VID> >& iedges) /*const*/ {
    return Base::GetOutgoingEdges(_v1id, iedges);
  }
  /**Get Edges which is incident to this specified vertex and user data associated with
   *these edges.
   *Incident edges are edges around this specified vertex.
   *@return a list of edges which is defined by 2 user data of its endpoints.
   *Empty list will be returned if this specified VID 
   *is not in graph and error message will be in standard output.
   */
  int GetIncidentEdgesVData(VID _v1id, vector< pair<VERTEX,VERTEX> >& iedges) /*const*/ {
    return Base::GetOutgoingEdgesVData(_v1id, iedges);
  }

  /**Get Edges which is incident to this specified vertex.
   *Incident edges are edges around this specified vertex.
   *@return a list of edges which is defined by 2 VIDs of its endpoints and the edge weight.
   *Empty list will be returned if this specified VID 
   *is not in graph and error message will be in standard output.
   */
  int GetIncidentEdges(VID _v1id,vector< pair<pair<VID,VID>,WEIGHT> >& iedges) /*const*/ {
    return Base::GetOutgoingEdges(_v1id, iedges);
  }
  
  /**Get Edges which is incident to this specified vertex and user data associated with
   *these edges.
   *Incident edges are edges around this specified vertex.
   *@return a list of edges which is defined by 2 user data of its endpoints and
   *the weight of edge. Empty list will be returned if this specified VID 
   *is not in graph and error message will be in standard output.
   */
  int GetIncidentEdgesVData(VID _v1id,vector< pair<pair<VERTEX,VERTEX>,WEIGHT> >& iedges) /*const*/ {
    return Base::GetOutgoingEdgesVData(_v1id, iedges);
  }

  /** Used internally by  Dikstra; It has the same functionality as the above 
   *two functions.
   */
  int GetDijkstraInfo(VID _v1id, vector<VID>& _succ) const {
    return Base::GetAdjacentVertices(_v1id,_succ);
  }

};


/**
 * @ingroup pgraph
 *Parallel directed Graph class. This class encapsulates the methods related with the fact the graph is directed. 
 *We have methods to handle the successors and predecessors.All these methods are inheritted by 
 *Graph when the class is used as one of the templates.
*/
template<class VERTEX, class WEIGHT=int>
class PDG : public DG<VERTEX, WEIGHT, pBaseGraph<VERTEX,WEIGHT> >{
  public:
  typedef WtVertexType<VERTEX,WEIGHT> Vertex;
  typedef WtEdgeType<VERTEX,WEIGHT> WtEdge;

  typedef DG<VERTEX,WEIGHT,pBaseGraph<VERTEX,WEIGHT> > DG_type;
  typedef pBaseGraph<VERTEX,WEIGHT> pBaseGraph_type;

  typedef typename pBaseGraph_type::VI   VI;   ///<VI Vertex Iterator
  typedef typename pBaseGraph_type::CVI  CVI;  ///<CVI Constant Vertex Iterator
  typedef typename pBaseGraph_type::RVI  RVI;  ///<RVI Reverse Vertex Iterator
  typedef typename pBaseGraph_type::CRVI CRVI; ///<CRVI Constant Reverse Vertex Iterator

  typedef typename pBaseGraph_type::EI   EI;   ///<EI Edge Iterator
  typedef typename pBaseGraph_type::CEI  CEI;  ///<CEI Constant Edge Iterator
  typedef typename pBaseGraph_type::REI  REI;  ///<REI Reverse Edge Iterator
  typedef typename pBaseGraph_type::CREI CREI; ///<CREI Constant Reverse Edge Iterator

  typedef  pBaseGraph<VERTEX,WEIGHT> Base;

  void PDGfoo(){
    cout<<"PDG foo"<<endl;
  }
  void PDGrmi(){
    async_rmi(1,this->getHandle(),&PDG<VERTEX,WEIGHT>::PDGfoo);
  }

  /**Initialize predecessors in the data field of Vertex.
   *Predecessors tells client where the edges that 
   *connected to this vertex are from. Return false if the predecessors 
   *already set.
   */
  inline void SetPredecessors() {
    //this method will contain the code that right now is in the base;
    //after the virtual inherittance is fixed
    Base::pSetPredecessors();
  }

  /**Get out degree of a given vertex.
   *out degree of a vertex is number of edges
   *that are from this vertex to any other vertex in graph.
   *@return the out degree for the specifed vertex or ERROR if the vertex doesn't exist.
   */
  int GetVertexOutDegree(VID _v1) /*const*/ {
    return Base::GetVertexOutDegree(_v1);
  }

  /**Get VIDs which are connected to by edges going out from
   *specified VID.
   *@return empty vector If there is no such vertex found and
   *error message will be output to standard output.
   */  
  int GetSuccessors(VID _v1id, vector<VID>& _succ) /*const*/ {
    return Base::GetAdjacentVertices(_v1id,_succ);
  }

  /**Get VIDs which are connected to by edges going out from
   *specified VID.
   *@return empty vector If there is no such vertex found and
   *error message will be output to standard output.
   */  
  int GetSuccessors(VI _vi, vector<VID>& _succ) const {
    return Base::GetAdjacentVertices(_vi,_succ);
  }


  /**Get VERTEXs which are connected to by edges going out from
   *specified VID.
   *@return empty vector If there is no such vertex found and
   *error message will be output to standard output.
   */
  int GetSuccessorsDATA(VID _v1id, vector<VERTEX>& _succ) /*const*/ {
    return Base::GetAdjacentVerticesDATA(_v1id,_succ);
  }

  /**Get VIDs which are connected to by edges going out from
   *specified v.
   *Here v is any vertex contains user data in the parameter
   *if there are more than one, then the first will be applied.
   *@return empty vector If there is no such vertex found 
   */
  int GetSuccessors(VERTEX& _v1, vector<VID>& _succ) /*const*/ {
    VID  vid1 = this->GetVID(_v1);
    return Base::GetAdjacentVertices(vid1, _succ);
  }

  /**Get VERTEXs which are connected to by edges going out from
   *specified _v1.
   *Here _v1 is any vertex contains user data in the parameter
   *if there are more than one, then the first will be applied.
   *return empty vector If there is no such vertex found and
   *error message will be output to standard output.
   *@return the number of elements pushed in _succ
   */
  int GetSuccessorsDATA(VERTEX& _v1, vector<VERTEX>& _succ) /*const*/ {
    VID  vid1 = this->GetVID(_v1);
    return Base::GetAdjacentVerticesDATA(vid1,_succ);
  }

  /**Get all predecessors of a specified VID as a vector<VID>.
   *@note Need to call SetPredecessors() first to initialize predecessor vector
   *before using this method.
   *
   *@return empty vector If there is no such vertex found and
   *error message will be output to standard output.
   *@see SetPredecessors
   */
  int GetPredecessors(VID _v1id, vector<VID>& _pred) /*const*/ {
    return Base::GetPredVertices(_v1id,_pred);
  }

  /**Get all predecessors of a specified VID as a vector<VID>.
   *@note Need to call SetPredecessors() first to initialize predecessor vector
   *before using this method.
   *
   *@return empty vector If there is no such vertex found.
   *@see SetPredecessors
   */
  int GetPredecessors(VI _vi, vector<VID>& _pred) const {
    return Base::GetPredVertices(_vi,_pred);
  }


  /**Get all predecessors of a specified VID as a vector<VERTEX>.
   *@note Need to call SetPredecessors() first to initialize predecessor vector
   *before using this method.
   *
   *@return empty vector If there is no such vertex found and
   *error message will be output to standard output.
   *@see SetPredecessors
   */
  int GetPredecessorsDATA(VID _v1id,vector<VERTEX>& _pred) /*const*/ {
    return Base::GetPredVerticesDATA(_v1id,_pred);
  }

  /**Get all predecessors of a specified v as a vector<VID>.
   *Here v is any vertex contains user data in the parameter
   *if there are more than one, then the first will be applied.
   *
   *@note Need to call SetPredecessors() first to initialize predecessor vector
   *before using this method.
   *
   *@return empty vector If there is no such vertex found.
   */
  int GetPredecessors(VERTEX& _v1,vector<VID>& _pred) /*const*/ {
    return this->GetPredecessors( GetVID(_v1), _pred );
  }

  /**Get all predecessors of a specified v as a vector<VERTEX>.
   *Here v is any vertex contains user data in the parameter
   *if there are more than one, then the first will be applied.
   *
   *@note Need to call SetPredecessors() first to initialize predecessor vector
   *before using this method.
   *
   *@return empty vector If there is no such vertex found and
   *error message will be output to standard output.
   */
  int GetPredecessorsDATA(VERTEX& _v1,vector<VERTEX>& _pred) /*const*/ {
    return this->GetPredecessorsDATA( GetVID(_v1), _pred );
  }

  /** internal used by Dijkstra SSP
   * Same functionality as the above functions
   */
  int GetDijkstraInfo(VID _v1id, vector<VID>& _succ) const {
    return this->GetSuccessors(_v1id,_succ);
  }
  

  /**Get Edges which are incident to this specified vertex.
   *Outgoing edges are edges around this specified vertex.
   *@return a list of edges which is defined by 2 VIDs of its endpoints.
   *Empty list will be returned if this specified VID 
   *is not in graph and error message will be in standard output.
   */
  int GetOutgoingEdges(VID _v1id,vector< pair<VID,VID> >& iedges) /*const*/ {
    return Base::GetOutgoingEdges(_v1id, iedges);
  }
  
  /**Get Edges which are incident to this specified vertex.
   *Outgoing edges are edges around this specified vertex.
   *@return a list of edges which is defined by 2 VIDs of its endpoints and the edge weight.
   *Empty list will be returned if this specified VID 
   *is not in graph and error message will be in standard output.
   */
  int GetOutgoingEdges(VID _v1id,vector< pair<pair<VID,VID>,WEIGHT> >& iedges) /*const*/ {
    return Base::GetOutgoingEdges(_v1id, iedges);
  }
  
    /**Get Edges which are incident to this specified vertex and user data associated with
   *these edges.
   *Outgoing edges are edges around this specified vertex.
   *@return a list of edges which is defined by 2 user data of its endpoints.
   *Empty list will be returned if this specified VID 
   *is not in graph and error message will be in standard output.
   */
  int GetOutgoingEdgesVData(VID _v1id, vector< pair<VERTEX,VERTEX> >& iedges) /*const*/{
    return Base::GetOutgoingEdgesVData(_v1id,iedges);
  }

 /**Get Edges which are incident to this specified vertex and user data associated with
   *these edges.
   *Outgoing edges are edges around this specified vertex.
   *@return a list of edges which is defined by 2 user data of its endpoints and
   *the weight of edge. Empty list will be returned if this specified VID 
   *is not in graph and error message will be in standard output.
   */
  int GetOutgoingEdgesVData(VID _v1id,vector< pair<pair<VERTEX,VERTEX>,WEIGHT> >& iedges) /*const*/ {
    return Base::GetOutgoingEdgesVData(_v1id,iedges);
  }

};

//==============================================================================
//                PARALLEL WEIGHTED / UNWEIGHTED
//==============================================================================
/**
 * @ingroup pgraph
 *Parallel Weighted Graph class. Here we have methods that deal with the fact is weighted or not. these 
 *methods are inheritted by Graph when used. The methods are inheritted by graph.
*/
template<class VERTEX, class WEIGHT=int>
class PWG : public WG<VERTEX, WEIGHT, pBaseGraph<VERTEX,WEIGHT> >{

  typedef pBaseGraph<VERTEX,WEIGHT>  pBaseGraph_type;
  typedef WG<VERTEX,WEIGHT,pBaseGraph<VERTEX,WEIGHT> > WG_type;

  typedef typename pBaseGraph_type::VI   VI;   ///<VI Vertex Iterator
  typedef typename pBaseGraph_type::CVI  CVI;  ///<CVI Constant Vertex Iterator
  typedef typename pBaseGraph_type::RVI  RVI;  ///<RVI Reverse Vertex Iterator
  typedef typename pBaseGraph_type::CRVI CRVI; ///<CRVI Constant Reverse Vertex Iterator

  typedef typename pBaseGraph_type::EI   EI;   ///<EI Edge Iterator
  typedef typename pBaseGraph_type::CEI  CEI;  ///<CEI Constant Edge Iterator
  typedef typename pBaseGraph_type::REI  REI;  ///<REI Reverse Edge Iterator
  typedef typename pBaseGraph_type::CREI CREI; ///<CREI Constant Reverse Edge Iterator

  typedef  pBaseGraph<VERTEX,WEIGHT> Base;

  public:
  /**Check if there is any edge connected from vid1 to vid2.
   *@return false if vid1 or vid2 are not in graph, or
   *there is no edge from vid1 to vid2.
   */
  bool IsEdge(VID _v1id, VID _v2id)  {  
    return (Base::IsEdge(_v1id,_v2id) );
  }   

  /**Check if there is any edge connected from v1 to v2.
   *@param VERTEX v1 user data for first vertex
   *@param VERTEX v2 is any vertex contains user data in the second parameter.
   *@return false if v1 or v2 are not in graph, or
   *there is no edge from v1 to v2.
   */
  bool IsEdge(VERTEX& _v1, VERTEX& _v2)  {
    return (Base::IsEdge(_v1,_v2) );
  }

  /**Check if there is any edge connected from vid1 to vid2 of specified weight.
   *@return false if vid1 or vid2 are not in graph, or
   *there is no edge from vid1 to vid2.
   */
  bool IsEdge(VID _v1id, VID _v2id, WEIGHT _weight)  {
    return ( Base::IsEdge(_v1id,_v2id,_weight) );
  }

  /**Check if there is any edge connected from v1 to v2 of specified weight.
   *@param VERTEX v1 user data for first vertex
   *@param VERTEX v2 is any vertex contains user data in the second parameter.
   *@return false if v1 or v2 are not in graph, or
   *there is no edge from v1 to v2.
   */
  bool IsEdge(VERTEX& _v1, VERTEX& _v2, WEIGHT _weight) {
    VID vid1 = this->GetVID(_v1);
    VID vid2 = this->GetVID(_v2);
    return ( Base::IsEdge(vid1,vid2,_weight) );
  }

  /**Check if there is any edge connected from vid1 to vid2.
   *@return false if vid1 or vid2 are not in graph, or
   *there is no edge from vid1 to vid2.Also the _vi will point to the
   *vertex 1 location and _ei will point to the actual edge;
   */
  bool IsEdge(VID _v1id, VID _v2id,VI* _vi,EI* _ei) {
    return (Base::IsEdge(_v1id,_v2id,_vi,_ei));
  }
  
  /**Check if there is any edge connected from vid1 to vid2.
   *@return false if _v1 or _v2 are not in graph, or
   *there is no edge from vid1 to vid2.Also the _vi will point to the
   *vertex 1 location and _ei will point to the actual edge;
   */
  bool IsEdge(VERTEX _v1, VERTEX _v2,VI* _vi,EI* _ei) {
    return (Base::IsEdge(_v1,_v2,_vi,_ei));
  }

  /**@brief 
   *Check if the corresponding edge exist locally;
   *@param VERTEX& _v1 data of the source vertex.
   *@param VERTEX& _v2 data of the destination vertex.
   *@param WEIGHT _weight the weight for the edge we are looking for.
   *@param VI* pointer to a vertex iterator. If the edge is found this iterator 
   *will be initialized with the location of source vertex.
   *@param EI* pointer to an edge iterator. If the edge is found this iterator 
   *will be initialized with the location of the edge.
   *@return true if the edge exist locally and false otherwise;Also Iterators
   *to vertex and edge are initialized if the edge is local;
   */
  bool IsEdge(VERTEX& _v1, VERTEX& _v2, WEIGHT& _weight, VI* _vit, EI* _ei){
    VID vid1 = this->GetVID(_v1);
    VID vid2 = this->GetVID(_v2);
    return (Base::IsEdge(vid1,vid2,_weight,_vit,_ei));
  }

  /**Get weight of edge (vid1->vid2).
   *if no such edge WEIGHT(-1) will be returned.
   */
  WEIGHT GetEdgeWeight(VID _v1id, VID _v2id) {
    return (Base::GetEdgeWeight(_v1id,_v2id));
  }
  
#ifdef _PGRAPH
  /**Get weight of edge (vid1->vid2).
   *if no such edge WEIGHT(-1) will be returned.
   */
  WEIGHT GetEdgeIdWeight(VID _v1id, int _edgeid) {
    VI v1;
    EI e12;
    if (Base::IsEdgeId(_v1id,_edgeid,&v1,&e12)) {
      return  e12->weight;
    } else {
      return WEIGHT(-1);
    }
  }
#endif

  /**Get weight of edge (_v1->_v2).
   *if no such edge WEIGHT(-1) will be returned.
   */
  WEIGHT GetEdgeWeight(VERTEX& _v1, VERTEX& _v2)  {
    VID vid1 = this->GetVID(_v1);
    VID vid2 = this->GetVID(_v2);
    return GetEdgeWeight(vid1,vid2);
  }

};

/**
 * @ingroup pgraph
 *Parallel Non Weighted Graph class. This class will be inheritted if we don't want to use 
 *weights for edges. The weights are still in the graph and they are of type int
 *but the user doesn't have to be concerned with them. Also some methods related with 
 *weight are not implemented in this class as opposed to WG.The methods are inheritted by graph.
*/
template<class VERTEX, class WEIGHT=int>
class PNWG : public NWG<VERTEX, WEIGHT, pBaseGraph<VERTEX,WEIGHT> >{

  typedef pBaseGraph<VERTEX,WEIGHT>  pBaseGraph_type;
  typedef NWG<VERTEX,WEIGHT,pBaseGraph<VERTEX,WEIGHT> > NWG_type;

  typedef typename pBaseGraph_type::VI   VI;   ///<VI Vertex Iterator
  typedef typename pBaseGraph_type::CVI  CVI;  ///<CVI Constant Vertex Iterator
  typedef typename pBaseGraph_type::RVI  RVI;  ///<RVI Reverse Vertex Iterator
  typedef typename pBaseGraph_type::CRVI CRVI; ///<CRVI Constant Reverse Vertex Iterator

  typedef typename pBaseGraph_type::EI   EI;   ///<EI Edge Iterator
  typedef typename pBaseGraph_type::CEI  CEI;  ///<CEI Constant Edge Iterator
  typedef typename pBaseGraph_type::REI  REI;  ///<REI Reverse Edge Iterator
  typedef typename pBaseGraph_type::CREI CREI; ///<CREI Constant Reverse Edge Iterator
  typedef  pBaseGraph<VERTEX,WEIGHT> Base;

  /**Check if there is any edge connected from vid1 to vid2.
   *@return false if vid1 or vid2 are not in graph, or
   *there is no edge from vid1 to vid2.
   */
  bool IsEdge(VID _v1id, VID _v2id)  {  
    return (Base::IsEdge(_v1id,_v2id) );
  }   

  /**Check if there is any edge connected from v1 to v2.
   *Here v1 is any vertex contains user data in the first parameter,
   *and v2 is any vertex contains user data in the second parameter.
   *if there are more than one, then the first will be applied.
   *
   *@return false if v1 or v2 are not in graph, or
   *there is no edge from v1 to v2.
   */
  bool IsEdge(VERTEX& _v1, VERTEX& _v2)  {
    return (Base::IsEdge(_v1,_v2) );
  }

  /**Check if there is any edge connected from vid1 to vid2.
   *@return false if vid1 or vid2 are not in graph, or
   *there is no edge from vid1 to vid2.Also the _vi will point to the
   *vertex 1 location and _ei will point to the actual edge;
   */
  bool IsEdge(VID _v1id, VID _v2id,VI* _vi,EI* _ei) {
    return (Base::IsEdge(_v1id,_v2id,_vi,_ei));
  }
  
  /**Check if there is any edge connected from vid1 to vid2.
   *@return false if _v1 or _v2 are not in graph, or
   *there is no edge from vid1 to vid2.Also the _vi will point to the
   *vertex 1 location and _ei will point to the actual edge;
   */
  bool IsEdge(VERTEX _v1, VERTEX _v2,VI* _vi,EI* _ei) {
    return (Base::IsEdge(_v1,_v2,_vi,_ei));
  }
};


//==============================================================================
//                PARALLEL MULTI / NONMULTI
//==============================================================================
/**
 * @ingroup pgraph
 *Parallel Multiple Edge Graph class. Here for right now we have only chacks but in the feature
 *new methods related with multiplicity will be added. These methods are inheritted by graph.
*/
template<class VERTEX, class WEIGHT=int>
class PMG : public MG<VERTEX, WEIGHT, pBaseGraph<VERTEX,WEIGHT> >{
  public:
  typedef pBaseGraph<VERTEX,WEIGHT>  pBaseGraph_type;
  typedef MG<VERTEX,WEIGHT,pBaseGraph<VERTEX,WEIGHT> > MG_type;

  typedef typename MG_type::VI   VI;   ///<VI Vertex Iterator
  typedef typename MG_type::CVI  CVI;  ///<CVI Constant Vertex Iterator
  typedef typename MG_type::RVI  RVI;  ///<RVI Reverse Vertex Iterator
  typedef typename MG_type::CRVI CRVI; ///<CRVI Constant Reverse Vertex Iterator

  typedef typename MG_type::EI   EI;   ///<EI Edge Iterator
  typedef typename MG_type::CEI  CEI;  ///<CEI Constant Edge Iterator
  typedef typename MG_type::REI  REI;  ///<REI Reverse Edge Iterator
  typedef typename MG_type::CREI CREI; ///<CREI Constant Reverse Edge Iterator

  typedef  pBaseGraph<VERTEX,WEIGHT> Base;

  //all methods are inheritted from MG
};

/**
 * @ingroup pgraph
 *Parallel Non Multiple Edge Graph class. Here for right now we have only chacks but in the feature
 *new methods related with multiplicity will be added. These methods are inheritted by graph.
*/
template<class VERTEX, class WEIGHT=int>
class PNMG : public NMG<VERTEX, WEIGHT, pBaseGraph<VERTEX,WEIGHT> >{
  public:
  typedef NMG<VERTEX,WEIGHT,pBaseGraph<VERTEX,WEIGHT> > Multi_type;
  typedef NMG<VERTEX,WEIGHT,pBaseGraph<VERTEX,WEIGHT> > NMG_type;

  typedef typename NMG_type::VI   VI;   ///<VI Vertex Iterator
  typedef typename NMG_type::CVI  CVI;  ///<CVI Constant Vertex Iterator
  typedef typename NMG_type::RVI  RVI;  ///<RVI Reverse Vertex Iterator
  typedef typename NMG_type::CRVI CRVI; ///<CRVI Constant Reverse Vertex Iterator

  typedef typename NMG_type::EI   EI;   ///<EI Edge Iterator
  typedef typename NMG_type::CEI  CEI;  ///<CEI Constant Edge Iterator
  typedef typename NMG_type::REI  REI;  ///<REI Reverse Edge Iterator
  typedef typename NMG_type::CREI CREI; ///<CREI Constant Reverse Edge Iterator

  typedef  pBaseGraph<VERTEX,WEIGHT> Base;

  /**
   *This function check if the edge _v1->_v2 exist already in the graph and returns 
   *zero if not and one if the edge is in the graph.
   */
  inline int check_edge(VID _v1, VID _v2){
    //cout<<"NM::check_edge"<<endl;
    if(Base::IsEdge(_v1, _v2)) return 0;
    else return 1;
  }

  /**
   *This function check if the edge _v1->_v2 exist already in the graph and returns 
   *zero if not and one if the edge is in the graph.
   */
  inline int check_edge(VERTEX& _v1, VERTEX& _v2){
    cout<<"NM::check_edge with user data as vertex identifier; NOT implemented"<<endl;
    return 0;
  }

};


/*
                            ===================
                            |   PGRAPH class  |
                            ===================
*/
/**
 * @ingroup pgraph
 *The pGraph class will be instantiated by the user.
 *It is templated with five arguments. The first one describes the directness,
 *the second describe if the pGraph will be with multiple edges or not
 *and the last describes if the pGraph will be weighted or not. In other
 *words PD can be one of PDG (ParallelDirectedGraph) or
 *PUG (ParallelUndirectedGraph), PM can be one of PMG (ParallelMultipleGraph- allows
 *multiple edges) or PNMG (Paralle Non Multiple Edges Graph) and PW can be
 *either PWG (methods related with weight enabled) or
 *PNWG (methods related with weights disabled(missing); if the user will try to use them for
 *non-weighted graphs it will generate compiler errors);
 * The last two templates specifies the VERTEX type and the edge WEIGHT type.
*/
template <class PD, class PM, class PW, class __VERTEX,class __WEIGHT=int>
class pGraph:public PD, public PM, public PW{
  public:

  typedef __VERTEX VERTEX;
  typedef __WEIGHT WEIGHT;

  typedef GraphPart<VERTEX,WEIGHT> PCONTAINERPART;

  typedef typename PD::VI   VI;   ///<VI Vertex Iterator
  typedef typename PD::CVI  CVI;  ///<CVI Constant Vertex Iterator
  typedef typename PD::RVI  RVI;  ///<RVI Reverse Vertex Iterator
  typedef typename PD::CRVI CRVI; ///<CRVI Constant Reverse Vertex Iterator

  typedef typename PD::EI   EI;   ///<EI Edge Iterator
  typedef typename PD::CEI  CEI;  ///<CEI Constant Edge Iterator
  typedef typename PD::REI  REI;  ///<REI Reverse Edge Iterator
  typedef typename PD::CREI CREI; ///<CREI Constant Reverse Edge Iterator

  typedef  pBaseGraph<VERTEX,WEIGHT> Base;


  /**@brief 
   *Default constructor
   */
  pGraph(){
  }

  /**Define type for pGraph.
   */
  void define_type(stapl::typer& t) {
    stapl_assert(1,"ERROR: pGraph define_type used.\n");
  }


  /**
   *Check if the graph is directed or not; 
   *@return one if the graph is directed zero if the graph is undirected; 
   */
  inline int IsDirected() const {
    return PD::check_directed();
  }

  /**
   *Check if the graph is weighted or not; 
   *@return one if the graph is weighted zero if the graph is unweighted; 
   */
  inline int IsWeighted() const {
    return PW::check_weighted();
  }
  
  /**
   *Check if the graph support multiple edges or not; 
   *@return one if the graph supports and zero if the graph doesn't support; 
   */
  inline int IsMulti() const {
    return PM::check_multi();
  }

  /**
   *Add an edge into a graph. The functions takes care about directness, multiplicity,
   *weightness. If the graph is unweighted the user doesn't have to specify the weight.
   *@param _v1 the first vertex
   *@param _v2 the second vertex
   *@param _weight the weight of the edge.
   *@note The weight of the edge doesn't haveto be specified if the graph is unweighted. 
   *@return OK/ERROR depending if the operation succeded or not
   */
  inline int AddEdge(VID _v1, VID _v2, WEIGHT _weight=WEIGHT(-1)) {
    //if(PM::check_edge(_v1,_v2)) return PD::AddEdge(_v1,_v2,_weight);
    //else return ERROR;
    return PD::AddEdge(_v1,_v2,_weight,PM::check_multi());
  }

  inline void AddEdgeAsync(VID _v1, VID _v2, WEIGHT _weight=WEIGHT(-1)) {
    //if(PM::check_edge(_v1,_v2)) return PD::AddEdge(_v1,_v2,_weight);
    //else return ERROR;
    PD::AddEdgeAsync(_v1,_v2,_weight,PM::check_multi());
  }


  /**
   *Add an edge into a graph. The functions takes care about directness, multiplicity,
   *weightness. If the graph is unweighted the user doesn't have to specify the weight.
   *@param _v1 the first vertex(user data)
   *@param _v2 the second vertex(user data)
   *@param _weight the weight of the edge.
   *@note The weight of the edge doesn't haveto be specified if the graph is unweighted. 
   *@return OK/ERROR depending if the operation succeded or not
   */
  inline int AddEdge(VERTEX& _v1, VERTEX& _v2, WEIGHT _weight=WEIGHT(-1)) {
    VID _v1id = this->GetVID(_v1);
    VID _v2id = this->GetVID(_v2);    
    //if(PM::check_edge(_v1id,_v2id)) return PD::AddEdge(_v1id,_v2id,_weight);
    //else return ERROR;
    return PD::AddEdge(_v1id,_v2id,_weight,PM::check_multi());
  }
  
  inline void AddEdgeAsync(VERTEX& _v1, VERTEX& _v2, WEIGHT _weight=WEIGHT(-1)) {
    VID _v1id = this->GetVID(_v1);
    VID _v2id = this->GetVID(_v2);    
    //if(PM::check_edge(_v1id,_v2id)) return PD::AddEdge(_v1id,_v2id,_weight);
    //else return ERROR;
    return PD::AddEdgeAsync(_v1id,_v2id,_weight,PM::check_multi());
  }


  /**
   *This functions is for weighted graphs. 
   *This functions has different functionality for Directed and Undirected graphs.
   *If the graph is directed this function will add two edges into the graph. The first
   *one is from _v1 to _v2 and has the weight _p.first and the second one is from 
   *_v2 to _v1 and has weight _p.second. for the undirected graphs this function will add 
   *one edge  from _v1 to _v2 but the weight associated with this edge is different 
   *depending on the way you traverse the edge.  
   *@param _v1 the first vertex
   *@param _v2 the second vertex
   *@param _p a pair of weights for the edge(edges) that will be added.
   *@return OK/ERROR depending if the operation succeded or not
   */
  int  AddEdge(VID _v1, VID _v2, pair<WEIGHT,WEIGHT>& _p){
    //  !!!! !   cout<<"not optimized to use async"<<endl;
    if(PM::check_edge(_v1,_v2) && PM::check_edge(_v2,_v1)) return PD::AddEdge(_v1,_v2,_p, IsMulti());
    else return ERROR;
  }

  int  AddEdgeAsync(VID _v1, VID _v2, pair<WEIGHT,WEIGHT>& _p){
    //  !!!! !   cout<<"not optimized to use async"<<endl;
    if(PM::check_edge(_v1,_v2) && PM::check_edge(_v2,_v1)) PD::AddEdgeAsync(_v1,_v2,_p, IsMulti());
  }


  /**
   *This functions is for weighted graphs. 
   *This functions has different functionality for Directed and Undirected graphs.
   *If the graph is directed this function will add two edges into the graph. The first
   *one is from _v1 to _v2 and has the weight _p.first and the second one is from 
   *_v2 to _v1 and has weight _p.second. for the undirected graphs this function will add 
   *one edge  from _v1 to _v2 but the weight associated with this edge is different 
   *depending on the way you traverse the edge.  
   *@param _v1 the first vertex(user data)
   *@param _v2 the second vertex(user data)
   *@param _p a pair of weights for the edge(edges) that will be added.
   *@return OK/ERROR depending if the operation succeded or not
   */
  int  AddEdge(VERTEX& _v1, VERTEX& _v2, pair<WEIGHT,WEIGHT>& _p){
    cout<<"not optimized to use async"<<endl;
    VID _v1id = this->GetVID(_v1);
    VID _v2id = this->GetVID(_v2);    
    if(PM::check_edge(_v1id,_v2id) && PM::check_edge(_v2id,_v1id)) return PD::AddEdge(_v1,_v2,_p);
    else return ERROR;
  }

  /**Get All edges in this graph.
   *@param edges vector where the edges are stored
   *@return A edge list. One edge is defined as 2 VIDs.
   *@note This methods works both for weighted and unweighted graphs.
   */
  int GetEdges(vector< pair<VID,VID> >& edges)  {
    //vector< pair<VID,VID> > edges;    
    edges.clear();
    //    edges.reserve(this->GetEdgeCount());
    //VI in the next line can be done CVI and the whole method const
    for (VI vi = this->local_begin(); vi != this->local_end(); ++vi) {
      for (CEI ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ++ei) {
	if(!(IsDirected()) && 
	   (vi->vid > ei->vertex2id)) continue;
	pair<VID,VID> newedge(vi->vid, ei->vertex2id);
	edges.push_back( newedge );
      }
    }
    return edges.size();
  }
  
  /**Get All edges in this graph.
   *@param edges vector where the edges are stored
   *@return A edge list. One edge is defined as 2 VERTEX data structure(user data).
   *@note This methods works both for weighted and unweighted graphs.
   */
  int GetEdgesVData(vector< pair<VERTEX,VERTEX> >& edges) {
    //vector< pair<VERTEX,VERTEX> > edges; 
    edges.clear();
    //    edges.reserve(this->GetEdgeCount());
    //VI in the next line can be done CVI
    for (VI vi = this->local_begin(); vi != this->local_end(); ++vi) {
      for (CEI ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ++ei) {
	if(!(IsDirected()) && 
	   (vi->vid > ei->vertex2id)) continue;
	VERTEX v2data = GetData(ei->vertex2id);
	pair<VERTEX,VERTEX> newedge(vi->data, v2data);
	edges.push_back( newedge );
      }
    }
    return edges.size();
  }

  /**Get All edges in this graph.
   *@param edges vector where the edges are stored
   *@return A edge list. One edge is defined as 2 VIDs and weight.
   *@note This methods works only for weighted graphs.
   */
  int  GetEdges(vector< pair< pair<VID,VID>, WEIGHT> >& edges)  {
    //vector< pair< pair<VID,VID>, WEIGHT> > edges;    
    edges.clear();
    //    edges.reserve(this->GetEdgeCount());
    for (VI vi = this->local_begin(); vi != this->local_end(); ++vi) {
      for (CEI ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++ ) {
	if(!(IsDirected()) && 
	   (vi->vid > ei->vertex2id)) continue;
	pair<VID,VID> newedge(vi->vid, ei->vertex2id);
	pair<pair<VID,VID>,WEIGHT> newedgewt(newedge, ei->weight);
	edges.push_back( newedgewt );
      }
    }
    return edges.size();
  }

  /**Get All edges in this graph.
   *@param edges vector where the edges are stored
   *@return A edge list. One edge is defined as 2 VERTEX data structure(user data).
   *@note This methods works only for weighted graphs.
   */
  int GetEdgesVData(vector< pair< pair<VERTEX,VERTEX>, WEIGHT> >& edges)  {
    //vector< pair< pair<VERTEX,VERTEX>, WEIGHT> > edges; 
    edges.clear();
    //    edges.reserve(this->GetEdgeCount());
    for (VI vi = this->local_begin(); vi != this->local_end(); vi++) {
      for (CEI ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++ ) {
	if(!(IsDirected()) && 
	   (vi->vid > ei->vertex2id)) continue;
	VERTEX v2data = GetData(ei->vertex2id);
	pair<VERTEX,VERTEX> newedge(vi->data, v2data);
	pair<pair<VERTEX,VERTEX>,WEIGHT> newedgewt(newedge, ei->weight);
	edges.push_back( newedgewt );
      }
    }
    return edges.size();
  }

  //----------------------------------
  /**Get All edges in this graph.
   *@param edges vector where the edges are stored
   *@return A edge list. One edge is defined as 2 VIDs.
   *@note This methods works both for weighted and unweighted graphs.
   */
  /*
  template <class PCONTAINER>
  int GetAllEdges(PCONTAINER& edges)  {
    typename PCONTAINER::iterator pi;
    edges.resize(this->GetLocalEdgeCount());
    pi = edges.local_begin();

    for (VI vi = this->local_begin(); vi != this->local_end(); ++vi) {
      for (CEI ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ++ei) {
	if(!(IsDirected()) && 
	   (vi->vid > ei->vertex2id)) continue;
	pair<VID,VID> newedge(vi->vid, ei->vertex2id);
	*pi = newedge;
	++pi;
      }
    }
    stapl::rmi_fence();
    return edges.size();
  }
  */
  /**Get All edges in this graph.
   *@param edges vector where the edges are stored
   *@return A edge list. One edge is defined as 2 VERTEX data structure(user data).
   *@note This methods works both for weighted and unweighted graphs.
   */
  /*
  template <class PCONTAINER>
  int GetAllEdgesVData(PCONTAINER& edges) {
    typename PCONTAINER::iterator pi;
    edges.resize(this->GetLocalEdgeCount());
    pi = edges.local_begin();

    for (VI vi = this->local_begin(); vi != this->local_end(); ++vi) {
      for (CEI ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ++ei) {
	if(!(IsDirected()) && 
	   (vi->vid > ei->vertex2id)) continue;
	VERTEX v2data = GetData(ei->vertex2id);
	pair<VERTEX,VERTEX> newedge(vi->data, v2data);
	*pi = newedge;
	++pi;
      }
    }
    return edges.size();
  }
  */

  /**Get All edges in this graph.
   *@param edges vector where the edges are stored
   *@return A edge list. One edge is defined as 2 VIDs and weight.
   *@note This methods works only for weighted graphs.
   */
  template <class PCONTAINER>
  int GetAllEdges(PCONTAINER& edges)  {
    typename PCONTAINER::iterator pi;
    edges.resize(this->GetLocalEdgeCount());
    pi = edges.local_begin();
//added by thuang - to handle edges like 8-8, 9-9... in undirected graphs
    int equaledges = 0;
    
    for (VI vi = this->local_begin(); vi != this->local_end(); ++vi) {
      for (CEI ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++ ) {
//	if(!(IsDirected()) && 
//	   (vi->vid > ei->vertex2id)) continue;
//added by thuang - to handle edges like 8-8, 9-9... in undirected graphs
	if(!(IsDirected())) {
	  if (vi->vid > ei->vertex2id) continue;
	  if (vi->vid == ei->vertex2id) {
	    equaledges++; if (equaledges % 2 == 0) continue;
	  }
	}
	pair<VID,VID> newedge(vi->vid, ei->vertex2id);
	pair<pair<VID,VID>,WEIGHT> newedgewt(newedge, ei->weight);
	*pi = newedgewt;
	++pi;
      }
    }
    stapl::rmi_fence();
    return edges.size();
  }

  /**Get All edges in this graph.
   *@param edges vector where the edges are stored
   *@return A edge list. One edge is defined as 2 VERTEX data structure(user data).
   *@note This methods works only for weighted graphs.
   */
  template <class PCONTAINER>
  int GetAllEdgesVData(PCONTAINER& edges)  {
    typename PCONTAINER::iterator pi;
    edges.resize(this->GetLocalEdgeCount());
    pi = edges.local_begin();
//added by thuang - to handle edges like 8-8, 9-9... in undirected graphs
    int equaledges = 0;

    for (VI vi = this->local_begin(); vi != this->local_end(); vi++) {
      for (CEI ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++ ) {
//	if(!(IsDirected()) && 
//	   (vi->vid > ei->vertex2id)) continue;
//added by thuang - to handle edges like 8-8, 9-9... in undirected graphs
	if(!(IsDirected())) {
	  if (vi->vid > ei->vertex2id) continue;
	  if (vi->vid == ei->vertex2id)  {
	    equaledges++; if (equaledges % 2 == 0) continue;
	  }
	}
	VERTEX v2data = GetData(ei->vertex2id);
	pair<VERTEX,VERTEX> newedge(vi->data, v2data);
	pair<pair<VERTEX,VERTEX>,WEIGHT> newedgewt(newedge, ei->weight);
	*pi = newedgewt;
	++pi;
      }
    }
    stapl::rmi_fence();
    return edges.size();
  }
  //----------------------------------
 
  template <class PCONTAINER>
  int GetVertices(PCONTAINER& pc){
    typename PCONTAINER::iterator pi;
    pc.resize(this->local_size());
    pi = pc.local_begin();
    for(int p=0;p<this->GetPartsCount();++p){
      typename Base::pContainerPart_type* pp = this->pcontainer_parts[p];
      for(typename Base::pContainerPart_type::iterator vi=pp->begin();vi != pp->end();++vi){
	*pi = vi->vid;
	pi++;
      }
    }
    stapl::rmi_fence();
    return pc.size();
  }

  template <class PCONTAINER>
  int GetVerticesDATA(PCONTAINER& pc){
    typename PCONTAINER::iterator pi;
    int ic=0;
    pc.resize(this->local_size());
    pi = pc.local_begin();
    for(int p=0;p<this->GetPartsCount();++p){
      typename Base::pContainerPart_type* pp = this->pcontainer_parts[p];
      for(typename Base::pContainerPart_type::iterator vi=pp->begin();vi != pp->end();++vi){
	*pi = vi->data;
	pi++;
	ic++;
      }
    }
    cout<<stapl::get_thread_id()<<":copied into parray:"<<ic<<endl;
    stapl::rmi_fence();
    int sz = pc.size();
    stapl::rmi_fence();
    return sz;
  }

  /**@brief
   *Initialize the pGraph with information from the sequential graph.
   *@param GRAPH squential graph
   *@note the sequential graph will be all contained by the main thread.
   *distribute method should be called to spread the graph across available threads.
   */
  template <class GRAPH>
  void SetPGraphwGraph(GRAPH& _g){
    typename GRAPH::VI it;
    typename GRAPH::EI ei;

    stapl::rmi_fence();

    if(this->myid== 0){
      for(it = _g.v.begin();it != _g.v.end();it++){
	this->AddVertex(it->data, it->vid, this->myid);
      }
    }

    stapl::rmi_fence();

    if(this->myid == 0){
      for(it = _g.v.begin();it != _g.v.end();it++){
	for(ei = it->edgelist.begin();ei!=it->edgelist.end();ei++){
	  this->AddEdge(it->vid, ei->vertex2id,ei->weight);
	}
      }   
    }
    stapl::rmi_fence();
  }

  /**@brief
   *Initialize the pGraph with information from the sequential graph.
   *@param GRAPH squential graph.
   *@param const map<PID,vector<VID> > the way we want the sequential graph to be partitioned.
   */
  template <class GRAPH>
  void SetPGraphwGraph(GRAPH& _g,const map<PID,vector<VID> >& data_map){
    typename GRAPH::VI it;
    typename GRAPH::EI ei;
    
    stapl::rmi_fence();
    map<PID,vector<VID> >::const_iterator its;
    int aggf = stapl::get_aggregation();
    //cout<<"START adding vertices"<<endl;
    if(this->myid== 0){
      for(its=data_map.begin(); its!=data_map.end(); ++its) {
	for(vector<VID>::const_iterator vit = its->second.begin();vit != its->second.end();++vit){
	  if(_g.IsVertex(*vit, &it)){
	     this->AddVertex(it->data, it->vid, its->first);
	  }
	}
      }
    }
    stapl::rmi_fence();
    //cout<<"START adding edges"<<endl;
    stapl::set_aggregation(10);    
    if(this->myid == 0){
      for(it = _g.v.begin();it != _g.v.end();it++){
	for(ei = it->edgelist.begin();ei!=it->edgelist.end();ei++){
	  this->AddEdge(it->vid, ei->vertex2id,ei->weight);
	}
      }   
    }
    stapl::rmi_fence();
    stapl::set_aggregation(aggf);
    //cout<<"SetPGraph done"<<endl;
  }

  /**@brief
   *Check if the current pGraph has the same structure as the sequential graph
   *specified as argument.
   *@param GRAPH squential graph.
   */  
  template <class GRAPH>
  void CheckPGraphwGraph(GRAPH& _g){
    typename GRAPH::VI it;
    typename GRAPH::EI ei;

    stapl::rmi_fence();
    //checking vertices
    //cout<<"start checking vertices"<<endl;
    if(this->myid== 0){
      for(it = _g.v.begin();it != _g.v.end();it++){
	if(this->IsVertex(it->vid) == false){
	  cout<<"ERROR while performing the distribution.VID = "<<it->vid<<"not found"<<endl;
	}
      }
    }
    stapl::rmi_fence();
    //cout<<"start checking edges"<<endl;
    if(this->myid == 0){
      for(it = _g.v.begin();it != _g.v.end();it++){
	for(ei = it->edgelist.begin();ei!=it->edgelist.end();ei++){
	  if(this->IsEdge(it->vid, ei->vertex2id) == false)
	    cout<<"ERROR while performing the distribution.Edge:"<<it->vid<<" "<<ei->vertex2id<<"not found"<<endl;
	}
      }   
    }
    stapl::rmi_fence();
  }



  /***********************************************************************
                     SIMPLE IO procedures
     The rest of the methods needs to  be refined and implemented
   ***********************************************************************/
  

  /**Simple split. Takes as an input a file and a vector
   *Reads the contents of the file into a sequential graph and 
   *then sends parts of the sequential graph into different processors
   *the third argument "in_charge" determines which processor is in 
   *charge of sending out the nodes of the graph
   *@param char* file containing the sequential graph.
   *@param map<VID,PID>& map containing the distribution<vertex, thread owner>
   *@param PID thread id that will be in charge with the distribution.
  */
  template <class GRAPH>
  void Split_Crude_Dot(char* file,map<VID,PID> &where_to_go_list,PID in_charge=0){

    GRAPH g;
    typename GRAPH::VI vi;
    map<VID,VID> m;
    cout<<"start split crude "<<this->myid<<" "<<in_charge<<endl;
    
    //int max_vid=0; 
    
    if(this->myid == in_charge){
      //g.ReadExoGraph(file);
      g.ReadDotGraph(file);
      //max_vid = g.GetNextVID();
      
      printf("crude started inside\n");
      
      //g.DisplayGraph();
      int temp;
      printf("crude inside %d \n",this->myid);
      
      VERTEX data;
      int num_verts=0;
      for(vi=g.begin();vi!=g.end();vi++){
	//stapl::rmi_poll();
	//printf("%d\n",vi->vid);
	temp = where_to_go_list[num_verts];			
	num_verts++;
	data.SetWeight(vi->vid);      
	this->AddVertex(data, vi->vid,temp);
	
	//the next two steps are not necessary
	//but there are some RMI problems
	Location _l(temp,0);//temp is dest proc and 0 is the partid
	this->dist.Add2Cache(vi->vid,_l);
      }
    }
    
    stapl::rmi_fence();
    //return;
    if(this->myid == in_charge){
      printf("adding edges from [  %d   ]\n",this->myid);
      
      //dist.DisplayElementLocationCache();
      
      vector<pair<pair<VID,VID>,WEIGHT> > edges;
      g.GetEdges(edges);
      g.EraseGraph();
      for(int i=0;i<edges.size();i++){
	int eid = this->AddEdge(edges[i].first.first, 
		edges[i].first.second ,
		edges[i].second);
	//cout<<edges[i].first.first<<"->"<<edges[i].first.second<<"::"<<eid<<endl;
      }//for every edge
    }//if in_charge
    
#ifdef STAPL_DEBUG
  printf("crude inside %d \n",this->myid);
#endif
  
  
  //BUG FIX
  stapl::rmi_fence(); //BUG : The broadcast does not work without the rmi_fence
  //stapl::broadcast(in_charge,&max_vid,1);
  //stapl::rmi_fence();
  //this->SetStartID(max_vid);
#ifdef STAPL_DEBUG
  cout<<"me : "<<stapl::get_thread_id()<<endl;   
#endif  
#ifdef _OPTIONAL_SORT_
  //this->SimpleSort();
#endif
   
   stapl::rmi_fence();
}


  template <class GRAPH>
  void Split_Crude_Exo(char* file,map<VID,PID> &where_to_go_list,PID in_charge=0){

    GRAPH g;
    typename GRAPH::VI vi;
    map<VID,VID> m;
    
    cout<<"start split crude exo"<<this->myid<<" "<<in_charge<<endl;
    
    int max_vid=0; 
    
    if(this->myid == in_charge){
      g.ReadExoGraph(file);
      max_vid = g.GetNextVID();
      
      int temp;
      printf("crude inside %d \n",this->myid);
      
      VERTEX data;
      int num_verts=0;
      for(vi=g.begin();vi!=g.end();vi++){
	//stapl::rmi_poll();
	//printf("%d\n",vi->vid);
	temp = where_to_go_list[num_verts];			
	num_verts++;
	data.SetWeight(vi->vid);      
	this->AddVertex(data, vi->vid,temp);
	
	//the next two steps are not necessary
	//but there are some RMI problems
	Location _l(temp,0);//temp is dest proc and 0 is the partid
	this->dist.Add2Cache(vi->vid,_l);
      }
    }
    
    stapl::rmi_fence();
    //return;
    if(this->myid == in_charge){
      printf("adding edges from [  %d   ]\n",this->myid);
      
      //dist.DisplayElementLocationCache();
      
      vector<pair<pair<VID,VID>,WEIGHT> > edges;
      g.GetEdges(edges);
      g.EraseGraph();
      for(int i=0;i<edges.size();i++){
	this->AddEdge(edges[i].first.first, 
		edges[i].first.second ,
		edges[i].second);
      }//for every edge
    }//if in_charge
    
#ifdef STAPL_DEBUG	
  printf("crude inside %d \n",this->myid);
#endif
  
  
  //BUG FIX
  stapl::rmi_fence(); //BUG : The broadcast does not work without the rmi_fence
  //stapl::broadcast(in_charge,&max_vid,1);
  //stapl::rmi_fence();
  //this->SetStartID(max_vid);
#ifdef STAPL_DEBUG	
  cout<<"me : "<<stapl::get_thread_id()<<"  max: "<<max_vid<<endl;   
#endif  
#ifdef _OPTIONAL_SORT_
  //this->SimpleSort();
#endif
   
   stapl::rmi_fence();
}

void ReadDotGraph(const char*  _fname) {  
  this->ErasePGraph(); // empty graph before filling it in
  stapl::rmi_fence();
  if(this->myid == 0){
    ifstream  myifstream(_fname);
    if (!myifstream) {
      cout << "\nIn ReadGraph: can't open infile: " << _fname ;
      return;
    }
    ReadDotGraph(myifstream);
    myifstream.close();
  }
  stapl::rmi_fence();
}

/**
 *This function will read a graph from a file with sequential format for the graph;
 *The graph will be read on processor 0 and after that
 *it is distributed according to partition;  
*/
void ReadDotGraph(const char*  _fname, map<PID,vector<VID> >& partition){  
  this->ErasePGraph(); // empty graph before filling it in
  stapl::rmi_fence();
  if(this->myid == 0){
    ifstream  myifstream(_fname);
    if (!myifstream) {
      cout << "\nIn ReadGraph: can't open infile: " << _fname ;
      return;
    }
    ReadDotGraph(myifstream);
    myifstream.close();
  }
  if(this->myid != 0) partition.clear();
  stapl::rmi_fence();
  this->pGraphDistribute(partition);
  stapl::rmi_fence();
}

void ReadDotGraph(istream& _myistream) {
  VID v1id, v2id, maxVID;
  VI  v1;
  VERTEX data;
  WEIGHT weight;
  int nVerts=0, nEdges=0;
  char tagstring[100];

  _myistream >> tagstring;
  if ( !strstr(tagstring,"GRAPHSTART") ) {
    cout << endl << "In ReadGraph: didn't read GRAPHSTART tag right";
    return;
  }
  
  _myistream >> v1id;
  while(v1id != -1){
    this->AddVertex(data,v1id,0);
    if ( !IsVertex(v1id,&v1) ) {
      cout << "\nIn ReadGraph: didn't add v1...";
      //return -1;
    }
    _myistream >> v1id;
  }
  
  _myistream >> v1id;
  while(v1id != -1){
    
    _myistream >> v2id;
    if(this->AddEdge(v1id,v2id,weight) < 0) {
      cout<<"Error while trying to insert edge "<<v1id<<" "<<v2id<<endl;
    }
    nEdges++;
    _myistream >> v1id;
  }
  //maxVID = nVerts;
  //this->numVerts = nVerts;
  
  //if(this->IsDirected()) this->numEdges = nEdges;
  //else this->numEdges = nEdges*2;
  //this->vertIDs = maxVID; // set the maximum VID used so far...
  _myistream >> tagstring;
  if ( !strstr(tagstring,"GRAPHSTOP") ) {
    cout << endl << "In ReadGraph: didn't read GRAPHSTOP tag right";
    return;
  }
}
 

/**
 *This function will read a graph from a file with sequential format for the graph;
 *The graph will be read on processor 0 and after that
 *it is distributed according to partition;  
*/
void ReadExoGraph(const char*  _fname, map<PID,vector<VID> >& partition){  
  this->ErasePGraph(); // empty graph before filling it in
  stapl::rmi_fence();
  if(this->myid == 0){
    ifstream  myifstream(_fname);
    if (!myifstream) {
      cout << "\nIn ReadGraph: can't open infile: " << _fname ;
      return;
    }
    ReadExoGraph(myifstream);
    myifstream.close();
  }
  if(this->myid != 0) partition.clear();
  stapl::rmi_fence();
  this->pGraphDistribute(partition);
  stapl::rmi_fence();
}



void ReadExoGraph(const char*  _fname) {
  this->ErasePGraph(); // empty graph before filling it in
  stapl::rmi_fence();
 
  if(this->myid == 0){
    ifstream  myifstream(_fname);
    if (!myifstream) {
      cout << "\nIn ReadGraph: can't open infile: " << _fname ;
      return;
    }
    ReadExoGraph(myifstream);
    myifstream.close();
  }
  stapl::rmi_fence();
}
  
void ReadExoGraph(istream& _myistream) {
  VID v1id, v2id, maxVID;
  VI  v1;
  VERTEX data;
  WEIGHT weight;
  int nVerts, nEdges, nedges;
  int tr;
  double dtr;
  char tagstring[100];
  
  _myistream >> tagstring;
  if ( !strstr(tagstring,"GRAPHSTART") ) {
    cout << endl << "In ReadGraph: didn't read GRAPHSTART tag right";
    return;
  }
  
  _myistream >> nVerts >> nEdges >> maxVID;
  
  //first add vertices; our graph checks when an edge 
  //is added for both source and destination;
  int i;
  for (i = 0; i < nVerts; i++){
    this->AddVertex(data,i,0);
    if ( !IsVertex(i,&v1) ) {
      cout << "\nIn ReadGraph: didn't add v1...";
    }
  }

  for (i = 0; i < nVerts; i++){
    _myistream >> v1id ;             // read and add vertex 
    _myistream >> tr >> dtr >>dtr >>dtr >> tr;
    //this->AddVertex(data,v1id,0);
    if ( !IsVertex(v1id,&v1) ) {
      cout << "\nIn ReadGraph: didn't add v1...";
    }
    //when it will be fixed I need to add here the code to update 
    //the weight of the vertex
    //........

    _myistream >> nedges;               // read and add its edges
    for (int j = 0; j < nedges; j++){
      if(this->check_weighted()){
	_myistream >> v2id;
	_myistream >> tr;
      }
      else _myistream >> v2id;//read only the id and put a default weight	  
      this->AddEdge(v1id,v2id,weight);
    }
  }
  
  /*
  this->numVerts = nVerts;
  //internally we keep trace to all the edges in the graph;
  //which for undirected is double
  if(this->IsDirected()) this->numEdges = nEdges;
  else this->numEdges = nEdges*2;
  this->vertIDs = maxVID; // set the maximum VID used so far...
  // should sort verts & find biggest used...
  */

  _myistream >> tagstring;
  if ( !strstr(tagstring,"GRAPHSTOP") ) {
    cout << endl << "In ReadGraph: didn't read GRAPHSTOP tag right";
    return;
  }  
}

  /**
   *Write a pBaseGraph to a file to the disk; The distribution maps are
   *also written for faster reconstruction of the pBaseGraph;
   *@param file_name The name of the file will be produced;
   *@param num_chars The maximum length for the file name; 
   */
  void pFileWrite(const char * file_name,int num_chars){
  
    stapl::rmi_fence();
    char * actual_file_name=new char[num_chars];
    sprintf(actual_file_name,"%s_%d_%d",
	    file_name,
	    stapl::get_num_threads(),
	    stapl::get_thread_id());
    
    ofstream  myofstream(actual_file_name);
    if (!myofstream) {
      cout << "\nIn pFileWrite: can't open outfile: " << actual_file_name ; 	
      return;
    }
    
    iFileWrite(myofstream);
    this->dist.LocationMapWrite(myofstream);
    myofstream.close();
    stapl::rmi_fence();
  }

  void iFileWrite(ostream& _myostream) {
    
    _myostream <<"GRAPHSTART";
    
    if(this->IsDirected())
    _myostream << endl << this->local_size() << " " << this->GetLocalEdgeCount();
    else
      _myostream << endl << this->local_size() << " " << this->GetLocalEdgeCount()/2; 
    
    for (VI vi = this->local_begin(); vi != this->local_end(); vi++){
      _myostream << endl;  
      //We should also write the information about the Ghost Node and normal node
      vi->WriteEdgelist(_myostream,this->IsWeighted());
    } 
    
    _myostream << endl << "GRAPHSTOP";
    _myostream << endl; 
  }

  /**
   *Read a pBaseGraph from a file; The distribution maps are
   *also read such that the pBaseGraph is fully functional after this call   
   *Note that the pBaseGraph will be read in one part; 
   *@param file_name The name of the file will be produced;
   *@param num_chars The maximum length for the file name; 
   */
void pFileRead(const char * file_name,int num_chars){
  stapl::rmi_fence();
  char * actual_file_name=new char[num_chars];
  //sprintf(actual_file_name,"%s%d",file_name,stapl::get_thread_id());
  sprintf(actual_file_name,"%s_%d_%d",
	  file_name,
	  stapl::get_num_threads(),
	  stapl::get_thread_id());

  ifstream _myistream(actual_file_name);

  if(!_myistream.is_open()){
    cout<<"Error Opening Input File "<<actual_file_name<<endl;
    return;
  }

  iFileRead(_myistream);
  this->dist.LocationMapRead(_myistream);
  
  _myistream.close();
  stapl::rmi_fence();
}

void iFileRead(istream& _myistream) {

  VID v1id, v2id;
  typename PCONTAINERPART::VI  v1;

  VERTEX data;
  WEIGHT weight;

  int i,nVerts, nEdges, nedges;
  char tagstring[20];

  //this->AddPart();
  PCONTAINERPART* p = this->GetPart(0);

  int ne =0;

  _myistream >> tagstring;
  if ( !strstr(tagstring,"GRAPHSTART") ) {
    cout << endl << "In ReadGraph: didn't read GRAPHSTART tag right";
    return;
  }
  
  if (this->local_size() != 0) {
    this->ErasePGraph(); // empty graph before filling it in
  }
  stapl::rmi_fence();

  _myistream >> nVerts >> nEdges;
  
  for (i = 0; i < nVerts; i++){
    _myistream >> v1id >> data;             // read and add vertex 
    p->AddElement(data,v1id);
    this->dist.Add2PartIDMap(v1id,0);
    if ( !p->IsVertex(v1id,&v1) ) {
      cout << "\nERROR:: In ReadGraph: didn't add v1...";
      return;
    }
    
    _myistream >> nedges;               // read and add its edges
    for (int j = 0; j < nedges; j++){
      _myistream >> v2id >> weight; 
      v1->AddEdge(v2id,weight);
      ne++;
    }
  }
  
  p->SetnumVerts(nVerts);
  p->SetnumEdges(ne); //BUG ???????-Check Directed????
  // should sort verts & find biggest used...
  _myistream >> tagstring;
  if ( !strstr(tagstring,"GRAPHSTOP") ) {
    cout << endl << "In ReadGraph: didn't read GRAPHSTOP tag right";
    return;
  } 
 }

  /**Read graph info from the given input stream. Assign vid with given values.
   *Read data which were written by WriteGraph.
   @note Error message will be outputed if something wrong
   *during processing.
   */
  void ReadGraph(istream& _myistream) {
    VID v1id, v2id, maxVID;
    VI  v1;
    VERTEX data;
    WEIGHT weight;
    int nVerts, nEdges, nedges;
    char tagstring[100];
    
    _myistream >> tagstring;
    if ( !strstr(tagstring,"GRAPHSTART") ) {
      cout << endl << "In ReadGraph: didn't read GRAPHSTART tag right";
      return;
    }
    
    _myistream >> nVerts >> nEdges >> maxVID;
    
    for (int i = 0; i < nVerts; i++){
      _myistream >> v1id >> data;             // read and add vertex 
      AddVertex(data,v1id,0);
      if ( !IsVertex(v1id,&v1) ) {
	cout << "\nIn ReadGraph: didn't add v1...";
      }
      
      _myistream >> nedges;               // read and add its edges
      for (int j = 0; j < nedges; j++){
	if(this->check_weighted())
	  _myistream >> v2id >> weight; 
	else _myistream >> v2id;//read only the id and put a default weight	  
	v1->AddEdge(v2id,weight);
      }
    }
    
    /*this->numVerts = nVerts;*/
    //internally we keep trace to all the edges in the graph;
    //which for undirected is double
    /*
    if(this->IsDirected()) this->numEdges = nEdges;
    else this->numEdges = nEdges*2;
    this->vertIDs = maxVID; // set the maximum VID used so far...
    */
    // should sort verts & find biggest used...
    
    _myistream >> tagstring;
    if ( !strstr(tagstring,"GRAPHSTOP") ) {
      cout << endl << "In ReadGraph: didn't read GRAPHSTOP tag right";
      return;
    }
  }

  /**Read graph info from the file of file name, _filename.
   *This method calls ReadGraph(istream& _myistream).
   */
  void ReadGraph(const char*  _fname) {
    this->ErasePGraph();
    stapl::rmi_fence();
    if(this->myid == 0) {
      ifstream  myifstream(_fname);
      if (!myifstream) {
	cout << "\nIn ReadGraph: can't open infile: " << _fname ;
	return;
      }
      ReadGraph(myifstream);
      myifstream.close();
    }
    stapl::rmi_fence();
  }

  /**Write graph info to the given output stream.
   *This method outputs numVerts, numEdges, vertIDs
   *to the output stream and calls 
   *WtVertexType::WriteEdgelist for each vertex in 
   *this graph.
   *@see WtVertexType::WriteEdgelist
   */
  void WriteGraph(const char* _fname, bool append=false) /*const*/ { 
    ofstream _myostream;

    if(stapl::get_thread_id() == 0) {  
      if(append)
	_myostream.open(_fname, ios::out|ios::app);
      else
	_myostream.open(_fname);

#ifdef _ASCI_
      _myostream << endl << "GRAPHSTART";
#else
      _myostream << endl << "#####GRAPHSTART#####";
#endif
      int vertNum = GetVertexCount();
      int edgeNum = GetEdgeCount();
      if(this->IsDirected())
        _myostream << endl << vertNum << " " << edgeNum << " " << vertNum; 
      else 
        _myostream << endl << vertNum << " " << edgeNum / 2 << " " << vertNum; 

      _myostream.close();
    }
    stapl::rmi_fence();
 
    //format: VID VERTEX #edges VID WEIGHT VID WEIGHT ... 
    for(int i=0; i<stapl::get_num_threads(); ++i) {
      if(i == stapl::get_thread_id()) {
	_myostream.open(_fname, ios::out|ios::app);
        for (VI vi = this->local_begin(); vi != this->local_end(); vi++) {
          _myostream << endl;
          vi->WriteEdgelist(_myostream,this->check_weighted());
        } 
	_myostream.close();
      }
      stapl::rmi_fence();
    }
    stapl::rmi_fence();

    if(stapl::get_thread_id() == 0) {
      _myostream.open(_fname, ios::out|ios::app);
#ifdef _ASCI_
      _myostream << endl << "GRAPHSTOP";
#else
      _myostream << endl << "#####GRAPHSTOP#####";
#endif
      _myostream << endl; 
      _myostream.close();
    }
  }
};//end class pGraph
//@}
}//end namespace stapl
#endif
