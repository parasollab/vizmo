#ifndef _P_GRAPH_TRAVERSAL_H
#define _P_GRAPH_TRAVERSAL_H

#include "Defines.h"
#include "Graph.h"
#include "GraphAlgo.h"
#include "../pvector/pvector.h"

#include <pRange.h>
#include "common/rmitools.h"
////////////////////
//pgraph traversals
////////////////////

namespace stapl{

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//              BFS generic and with prange/executor
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


// !!! I need to inherit from timmie;s abstract interface; processed is changed
template <class PGRAPH>
class BFSDDG: public BasePObject{

  map<VID, int> color;  
  vector<int>   ready;
  vector<int>   start;
  PGRAPH* pg;

public:
  BFSDDG(){
    this->register_this(this);
    //cout<<"default object registered:"<<this->getHandle()<<endl;
  }

  BFSDDG(PGRAPH& p,VID _start){
    typename PGRAPH::VI vi;
    pg = &p;

    //if(p.IsVertex(start,&vi)){
    //ready.push_back(_start);
    start.push_back(_start);
    //}
    // else{
    //  cout<<"ERROR::BFSDDG constructor:Invalid start vertex"<<endl;
    //}
    this->register_this(this);
    //cout<<"Normal object registered:"<<this->getHandle()<<endl;
  }

  BFSDDG(PGRAPH& p,vector<VID>& _start){
    typename PGRAPH::VI vi;
    pg = &p;
    start = _start;
    if(start.size() == 0){
      //cout<<"Warning::BFSDDG constructor vector of starting points is empty"<<endl;
    }

    //for(int i=0;i<start.size();i++){
    //if(p.IsVertex(start[i],&vi)){
    //ready.push_back(start[i]);
    //}
    //else{
    //cout<<"ERROR::BFSDDG constructor:Invalid start vertex"<<endl;
    //}
    //}
    this->register_this(this);
    //cout<<"Normal object registered:"<<this->getHandle()<<endl;
  }

  BFSDDG(const BFSDDG& other) {
    color = other.color;
    ready = other.ready;
    start = other.start;
    pg = other.pg;
    this->register_this(this);
    //cout<<"copy constructor  object registered:"<<this->getHandle()<<endl;
  }

  void Name(){
    cout<<"BFSDDG"<<endl;
  }

  ~BFSDDG(){
    this->color.clear();
    this->ready.clear();
    this->start.clear();
    rmiHandle handle = this->getHandle();
    if (handle > -1)
      stapl::unregister_rmi_object(handle);
  }

  BFSDDG& operator=(const BFSDDG& other) {
    color = other.color;
    ready = other.ready;
    start = other.start;
    pg = other.pg;
  }

  void clearStart(){
    start.clear();
  }

  void reset(){
    typename PGRAPH::VI vi;
    color.clear();
    ready.clear();
    for(int i=0;i<start.size();i++){
      if(pg->IsVertex(start[i],&vi)){
	ready.push_back(start[i]);
      }
      else{
	cout<<"ERROR::BFSDDG::reset():Invalid start vertex"<<endl;
      } 
    }
  }

  bool finished(){
    //cout<<"finished"<<ready.size()<<endl;
    if(ready.size() == 0) return true;
    else return false;
  }

  std::vector<int> get_ready(){
    vector<int> temp = ready;
    ready.clear();

    //if(stapl::get_thread_id() == 0)
    //if(temp.size() > 0) cout<<"DG"<<this<<endl; 

    //for(int c=0;c < temp.size(); c++){
    //if(temp[c] == 4816 || temp[c] == 4838 || temp[c] == 4354)
    //cout<<"returned to executor"<<temp[c]<<endl;; 
    //}

    return temp;
  }
  
  void remoteMark(VID _v2){
    //if(_v2 == 4140 || _v2== 4600)
    //cout<<"REMOTEMARK"<<_v2<<endl; 

    if(pg->IsLocal(_v2)){
      map<VID, int>::iterator mit = color.find(_v2);
      if(mit == color.end()){
	//if(_v2 == 4140 || _v2== 4600)
	//cout<<"ADDED in REMOTEMARK"<<_v2<<" "<<this<<endl; 

	color[_v2] = GRAY;
	ready.push_back(_v2);
      }
    }
    else{
      cout<<"Warning vertex 2 of a remote edge doesn't exist BFSDDG::remoteMark"<<endl;
    }
  }

  void processed(const int& vid, set<int>& remote,int execution_status){
    typename PGRAPH::VI vi;
    map<VID, int>::iterator mit;
    color[vid] = GRAY;

    //if early quit has been signalled just return
    //if(vi->vid == 4140||vi->vid == 4600)
    //cout<<"DDG"<<vi->vid<<"::es"<<execution_status<<endl;
   
    if(execution_status < 0) return;

    if(pg->IsVertex(vid, &vi)){
      for(typename PGRAPH::EI ei = vi->edgelist.begin();ei != vi->edgelist.end(); ei++){
	//if(ei->vertex2id == 4140||ei->vertex2id == 4600||ei->vertex2id == 4602)
	//cout<<"-->DDG "<<vi->vid<<"to"<<ei->vertex2id<<"]"<<endl;
	
	mit = color.find(ei->vertex2id);
	if(mit == color.end()){
	  
	
	  //cout<<"processed edge from "<<vi->vid<<" to "<<ei->vertex2id<<endl;
	  //the node is not touched
	  if(pg->IsLocal(ei->vertex2id)){

	    //if(ei->vertex2id == 4140||ei->vertex2id == 4600||ei->vertex2id == 4602)
	    //cout<<"-->LOCALDDG--> "<<vi->vid<<"to"<<ei->vertex2id<<"]"<<endl;

	    color[ei->vertex2id] = GRAY;
	    ready.push_back(ei->vertex2id);
	  }
	  else{

	    //if(ei->vertex2id == 4140||ei->vertex2id == 4600||ei->vertex2id == 4602)
	    //cout<<"-->REMOTDDG--> "<<vi->vid<<"to"<<ei->vertex2id<<"]"<<endl;

	    Location _n=pg->Lookup(ei->vertex2id);
	    stapl_assert(_n.ValidLocation(),"Invalid id for the second vertex in BFSDDG::processed()");    
	    //here I have to add remotely to the ready queue
	     stapl::async_rmi( _n.locpid(), 
			       this->getHandle(),
			       &BFSDDG::remoteMark,ei->vertex2id);
	    //signal the remote processor to check for work;
	    remote.insert(_n.locpid());
	  }
	}
      }
    }
    else{
      cout<<"ERROR:: BFSDDG::processed() invalid vertex"<<vid<<endl;
    }
  }

  void define_type(stapl::typer& t) {
    stapl_assert(1,"ERROR: pDDG define_type used.\n");
  }
};//end BFS DDG class

template <class PGRAPH>
class BFSDDGReverse: public BasePObject{

  map<VID, int> color;  
  vector<int>   ready;
  vector<int>   start;
  PGRAPH* pg;

public:

  BFSDDGReverse(){
    this->register_this(this);
    cout<<"BFSDDGReverse object registered:"<<this->getHandle()<<endl;
  }

  BFSDDGReverse(PGRAPH& p,VID start){
    typename PGRAPH::VI vi;
    pg = &p;
    //if(p.IsVertex(start,&vi)){
    //ready.push_back(_start);
    start.push_back(_start);
    //}
    //else{
    //cout<<"ERROR::BFSDDGReverse constructor:Invalid start vertex"<<endl;
    //}
    this->register_this(this);
    //cout<<"BFSDDGReverse:: Normal object registered:"<<this->getHandle()<<endl;
  }

  BFSDDGReverse(PGRAPH& p,vector<VID>& _start){
    typename PGRAPH::VI vi;
    pg = &p;
    start = _start;
    if(start.size() == 0){
      //cout<<"Warning::BFSDDGReverse constructor vector of starting points is empty"<<endl;
    }
    //for(int i=0;i<start.size();i++){
    //  if(p.IsVertex(start[i],&vi)){
    //	ready.push_back(start[i]);
    //    }
    //  else{
    //	cout<<"ERROR::BFSDDGReverse constructor:Invalid start vertex"<<endl;
    //  }
    //}
    this->register_this(this);
    //cout<<"Normal object registered:"<<this->getHandle()<<endl;
  }

  BFSDDGReverse(const BFSDDGReverse& other) {
    color = other.color;
    ready = other.ready;
    start = other.start;
    pg = other.pg;
    this->register_this(this);
    //cout<<"copy constructor  object registered:"<<this->getHandle()<<endl;
  }

  ~BFSDDGReverse(){
    this->color.clear();
    this->ready.clear();
    this->start.clear();
    rmiHandle handle = this->getHandle();
    if (handle > -1)
      stapl::unregister_rmi_object(handle);
  }

  void Name(){
    cout<<"BFSDDGReverse"<<endl;
  }

  BFSDDGReverse& operator=(const BFSDDGReverse& other) {
    color = other.color;
    ready = other.ready;
    start = other.start;
    pg = other.pg;
  }

  void clearStart(){
    start.clear();
  }

  void reset(){
    typename PGRAPH::VI vi;
    color.clear();
    ready.clear();
    for(int i=0;i<start.size();i++){
      if(pg->IsVertex(start[i],&vi)){
	ready.push_back(start[i]);
      }
      else{
	cout<<"ERROR::BFSDDG::reset():Invalid start vertex"<<endl;
      } 
    }
  }

  bool finished(){
    if(ready.size() == 0) return true;
    else return false;
  }

  std::vector<int> get_ready(){
    vector<int> temp = ready;
    ready.clear();
    return temp;
  }
  
  void remoteMark(VID _v2){

    if(pg->IsLocal(_v2)){
      map<VID, int>::iterator mit = color.find(_v2);
      if(mit == color.end()){
	//cout<<"Remote mark called on "<< stapl::get_thread_id()<<"for "<<_v2<<endl;
	color[_v2] = GRAY;
	ready.push_back(_v2);
      }
    }
    else{
      cout<<"Warning vertex 2 of a remote edge doesn't exist BFSDDG::remoteMark"<<endl;
    }

    //if(stapl::get_thread_id()==0){
    //this->Name();
    //printf("rm-->%x\n",this);
    //cout<<"REmote ready size"<<ready.size()<<endl;
    //}

  }

  void processed(const int& vid, set<int>& remote,int execution_status){
    typename PGRAPH::VI vi;
    map<VID, int>::iterator mit;
    this->color[vid] = GRAY;

    //if early quit has been signalled just return
    if(execution_status < 0) return;

    if(pg->IsVertex(vid, &vi)){
      for(typename PGRAPH::EI ei = vi->predecessors.begin();
	  ei != vi->predecessors.end(); ei++){
	mit = color.find(ei->vertex2id);
	if(mit == color.end()){
	  //the node is not touched
	  if(pg->IsLocal(ei->vertex2id)){
	    color[ei->vertex2id] = GRAY;
	    ready.push_back(ei->vertex2id);
	  }
	  else{
	    Location _n=pg->Lookup(ei->vertex2id);
	    stapl_assert(_n.ValidLocation(),"Invalid id for the second vertex in BFSDDGReverse::processed()");    
	    //here I have to add remotely to the ready queue
	     stapl::async_rmi( _n.locpid(), 
			       this->getHandle(),
			       &BFSDDGReverse::remoteMark,ei->vertex2id);
	    //signal the remote processor to check for work;
	    remote.insert(_n.locpid());
	  }
	}
      }
    }
    else{
      cout<<"ERROR:: BFSDDGReverse::processed() invalid vertex"<<vid<<endl;
    }
  }
};//end class BFSDDGReverse

template <class PGRAPH>
class TOPODDG: public BasePObject{

  map<VID, int> color;
  
  vector<int>   ready;
  PGRAPH* pg;

public:
  TOPODDG(){
    this->register_this(this);
    cout<<"default object registered:"<<this->getHandle()<<endl;
  }

  TOPODDG(PGRAPH& p){
    typename PGRAPH::VI vi;
    pg = &p;
    this->register_this(this);
    //cout<<"Normal object registered:"<<this->getHandle()<<endl;
  }

  TOPODDG(const TOPODDG& other) {
    color = other.color;
    ready = other.ready;
    pg = other.pg;
    this->register_this(this);
    //cout<<"copy constructor  object registered:"<<this->getHandle()<<endl;
  }

  ~TOPODDG(){
    this->color.clear();
    this->ready.clear();
    rmiHandle handle = this->getHandle();
    if (handle > -1)
      stapl::unregister_rmi_object(handle);
  }

  void Name(){
    cout<<"TOPO normal"<<endl;
  }

  TOPODDG& operator=(const TOPODDG& other) {
    color = other.color;
    ready = other.ready;
    pg = other.pg;
  }

  void reset(){
    typename PGRAPH::VI vi;
    color.clear();
    //this method assumes that we have called set predecessors
    for(vi=pg->local_begin();vi != pg->local_end();vi++){
      
      color[vi->vid] = vi->predecessors.size();
      
      if(vi->predecessors.size() == 0)
	ready.push_back(vi->vid);
    }
  }

  bool finished(){
    //cout<<"finished"<<ready.size()<<endl;
    if(ready.size() == 0) return true;
    else return false;
  }

  std::vector<int> get_ready(){
    vector<int> temp = ready;
    ready.clear();
    return temp;
  }
  
  void remoteMark(VID _v2){
    if(pg->IsLocal(_v2)){
      map<VID, int>::iterator mit = color.find(_v2);
      mit->second = mit->second - 1;
      if(mit->second == 0) 
	ready.push_back(_v2);
    }
    else{
      cout<<"Warning vertex 2 of a remote edge doesn't exist TOPODDG::remoteMark"<<endl;
    }
  }

  void processed(const int& vid, set<int>& remote, int execution_status){
    typename PGRAPH::VI vi;
    map<VID, int>::iterator mit;

    mit = color.find(vid); 
    if(mit == color.end()){
      cout<<"TOPODDG::processed() Vertex not found:"<<vid<<endl; 
      return;
    }

    if(pg->IsVertex(vid, &vi)){
      for(typename PGRAPH::EI ei = vi->edgelist.begin();ei != vi->edgelist.end(); ei++){
	mit = color.find(ei->vertex2id);
	if(mit != color.end()){
	  mit->second = mit->second - 1;
	  if(mit->second == 0)
	    ready.push_back(ei->vertex2id);
	}
	else{
	  Location _n=pg->Lookup(ei->vertex2id);
	  stapl_assert(_n.ValidLocation(),"Invalid id for the second vertex in TOPODDG::processed()");    
	  //here I have to add remotely to the ready queue
	  stapl::async_rmi( _n.locpid(), 
			    this->getHandle(),
			    &TOPODDG::remoteMark,ei->vertex2id);
	  //signal the remote processor to check for work;
	  remote.insert(_n.locpid());
	}
      }
    }
    else{
      cout<<"ERROR:: TOPODDG::processed() invalid vertex"<<vid<<endl;
    }
  }

  void define_type(stapl::typer& t) {
    stapl_assert(1,"ERROR: pDDG define_type used.\n");
  }
};//end TOPODDG class


template <class PGRAPH>
class TOPODDGReverse: public BasePObject{

  map<VID, int> color;
  
  vector<int>   ready;
  PGRAPH* pg;

public:
  TOPODDGReverse(){
    this->register_this(this);
    //cout<<"default object registered:"<<this->getHandle()<<endl;
  }

  TOPODDGReverse(PGRAPH& p){
    typename PGRAPH::VI vi;
    pg = &p;
    this->register_this(this);
    //cout<<"Normal object registered:"<<this->getHandle()<<endl;
  }

  TOPODDGReverse(const TOPODDGReverse& other) {
    color = other.color;
    ready = other.ready;
    pg = other.pg;
    this->register_this(this);
    //cout<<"copy constructor  object registered:"<<this->getHandle()<<endl;
  }

  ~TOPODDGReverse(){
    this->color.clear();
    this->ready.clear();
    rmiHandle handle = this->getHandle();
    if (handle > -1)
      stapl::unregister_rmi_object(handle);
  }

  void Name(){
    cout<<"TOPO normal"<<endl;
  }

  TOPODDGReverse& operator=(const TOPODDGReverse& other) {
    color = other.color;
    ready = other.ready;
    pg = other.pg;
  }

  void reset(){
    typename PGRAPH::VI vi;
    color.clear();
    //this method assumes that we have called set predecessors
    for(vi=pg->local_begin();vi != pg->local_end();vi++){
      
      color[vi->vid] = vi->edgelist.size();
      
      if(vi->edgelist.size() == 0)
	ready.push_back(vi->vid);
    }
  }

  bool finished(){
    //cout<<"finished"<<ready.size()<<endl;
    if(ready.size() == 0) return true;
    else return false;
  }

  std::vector<int> get_ready(){
    vector<int> temp = ready;
    ready.clear();
    return temp;
  }
  
  void remoteMark(VID _v2){
    if(pg->IsLocal(_v2)){
      map<VID, int>::iterator mit = color.find(_v2);
      mit->second = mit->second - 1;
      if(mit->second == 0) 
	ready.push_back(_v2);
    }
    else{
      cout<<"Warning vertex 2 of a remote edge doesn't exist TOPODDGReverse::remoteMark"<<endl;
    }
  }
  void processed(const int& vid, set<int>& remote, int execution_status){
    typename PGRAPH::VI vi;
    map<VID, int>::iterator mit;

    mit = color.find(vid); 
    if(mit == color.end()){
      cout<<"TOPODDGReverse::processed() Vertex not found:"<<vid<<endl; 
      return;
    }

    if(pg->IsVertex(vid, &vi)){
      for(typename PGRAPH::EI ei = vi->predecessors.begin();
	  ei != vi->predecessors.end(); ei++){
	mit = color.find(ei->vertex2id);
	if(mit != color.end()){
	  mit->second = mit->second - 1;
	  if(mit->second == 0)
	    ready.push_back(ei->vertex2id);
	}
	else{
	  Location _n=pg->Lookup(ei->vertex2id);
	  stapl_assert(_n.ValidLocation(),"Invalid id for the second vertex in TOPODDGReverse::processed()");    
	  //here I have to add remotely to the ready queue
	  stapl::async_rmi( _n.locpid(), 
			    this->getHandle(),
			    &TOPODDGReverse::remoteMark,ei->vertex2id);
	  //signal the remote processor to check for work;
	  remote.insert(_n.locpid());
	}
      }
    }
    else{
      cout<<"ERROR:: TOPODDGReverse::processed() invalid vertex"<<vid<<endl;
    }
  }

  void define_type(stapl::typer& t) {
    stapl_assert(1,"ERROR: pDDGReverse define_type used.\n");
  }
};//end TOPODDG REverse class

///////////////////////////////////////////////////////////////////////
//                         WORK FUNCTIONS
///////////////////////////////////////////////////////////////////////
template <class PGRAPH,class VISITOR>
class _BFS_traversal_prange{

  typedef stapl::linear_boundary<typename PGRAPH::VI> linear_boundary;
  typedef stapl::pRange<linear_boundary,PGRAPH,BFSDDG<PGRAPH> > pGraphpRange;
  VISITOR vis; 
 public:

  _BFS_traversal_prange(VISITOR& _v){
    vis = _v;
  }

  int operator()(pGraphpRange& _prange){
    
    typename PGRAPH::VI vi = _prange.get_boundary().start();
    typename PGRAPH::EI ei;

    int sz =0;
    for(;vi != _prange.get_boundary().finish();vi++){
      if(vis.vertex(vi) < 0) return -1;
      //for all edges call visitor on them
      for(ei=vi->edgelist.begin();ei != vi->edgelist.end();ei++){
	vis.tree_edge(vi,ei);
      }
      sz++;
    }
    return OK;
  }

  void define_type(stapl::typer& t) {
    stapl_assert(1,"ERROR:_BFS_traversal_prange define type used.\n");
  }
}; //end BFS visitor

template <class PGRAPH,class VISITOR>
class _BFSReverse_traversal_prange{

  typedef stapl::linear_boundary<typename PGRAPH::VI> linear_boundary;
  typedef stapl::pRange<linear_boundary,PGRAPH,BFSDDGReverse<PGRAPH> > pGraphpRange;
  VISITOR vis; 
 public:

  _BFSReverse_traversal_prange(VISITOR& _v){
    vis = _v;
  }

  int operator()(pGraphpRange& _prange){
    
    typename PGRAPH::VI vi = _prange.get_boundary().start();
    typename PGRAPH::EI ei;

    int sz =0;
    for(;vi != _prange.get_boundary().finish();vi++){
      if(vis.vertex(vi) < 0) return -1;
      for(ei=vi->edgelist.begin();ei != vi->edgelist.end();ei++){
	vis.tree_edge(vi,ei);
      }
      sz++;
    }

    return OK;
  }

  void define_type(stapl::typer& t) {
    stapl_assert(1,"ERROR:_BFS_traversal_prange define type used.\n");
  }
}; //end BFSReverse visitor

template <class PGRAPH,class VISITOR>
class _TOPO_traversal_prange{

  typedef stapl::linear_boundary<typename PGRAPH::VI> linear_boundary;
  typedef stapl::pRange<linear_boundary,PGRAPH,TOPODDG<PGRAPH> > pGraphpRange;
  VISITOR vis; 
 public:

  _TOPO_traversal_prange(VISITOR& _v){
    vis = _v;
  }

  void operator()(pGraphpRange& _prange){
    
    typename PGRAPH::VI vi = _prange.get_boundary().start();
    typename PGRAPH::EI ei;

    int sz =0;
    for(;vi != _prange.get_boundary().finish();vi++){

      vis.vertex(vi);

      for(ei=vi->edgelist.begin();ei != vi->edgelist.end();ei++){
	vis.tree_edge(vi,ei);
      }
      sz++;
    }
  }

  void define_type(stapl::typer& t) {
    stapl_assert(1,"ERROR:_TOPO_traversal_prange define type used.\n");
  }
}; //end TOPO visitor

template <class PGRAPH,class VISITOR>
class _TOPOReverse_traversal_prange{
  typedef stapl::linear_boundary<typename PGRAPH::VI> linear_boundary;
  typedef stapl::pRange<linear_boundary,PGRAPH,TOPODDGReverse<PGRAPH> > pGraphpRange;

  VISITOR vis; 

 public:

  _TOPOReverse_traversal_prange(VISITOR& _v){
    vis = _v;
  }

  int operator()(pGraphpRange& _prange){
    
    typename PGRAPH::VI vi = _prange.get_boundary().start();
    typename PGRAPH::EI ei;
	
    int sz =0;
    for(;vi != _prange.get_boundary().finish();vi++){
      vis.vertex(vi);
      for(ei=vi->edgelist.begin();ei != vi->edgelist.end();ei++){
	vis.tree_edge(vi,ei);
      }
      sz++;
    }
    return OK;
  }

  void define_type(stapl::typer& t) {
    stapl_assert(1,"ERROR:_TOPO_traversal_prange define type used.\n");
  }
}; //end TOPOReverse visitor


////////////////////////////////////////////////////////////////////////////
//                  User Accessible Traversal Functions
////////////////////////////////////////////////////////////////////////////
//                ONE  GRAPH
template<class PGRAPH, class VISITOR>
int p_BFS_pRange (PGRAPH& _pg ,
		  VISITOR& _vis,
		  VID _startVid) {

  typedef stapl::linear_boundary<typename PGRAPH::VI> linear_boundary;
  typedef stapl::pRange<linear_boundary,PGRAPH,BFSDDG<PGRAPH> > pGraphpRange;
  typename PGRAPH::VI vi;

  BFSDDG<PGRAPH> ddg(_pg,_startVid);
  pGraphpRange pgpr(&_pg);
  pgpr.set_ddg(&ddg);
  stapl::rmi_fence();

  for(vi = _pg.local_begin();vi != _pg.local_end(); vi++){
    pGraphpRange* ppr = new pGraphpRange(vi->vid, stapl::get_thread_id(), &_pg,linear_boundary(vi,vi+1),10000,&pgpr);
    pgpr.add_subrange(*ppr);
  }

  vector<VISITOR> vvis;
  vvis.push_back(_vis);

  vector<pGraphpRange> vpr;
  vpr.push_back(pgpr);

  p_BFS_pRange<PGRAPH,pGraphpRange,VISITOR>(vpr, vvis);

  return OK;
}// end BFS traversal

template<class PGRAPH, class VISITOR>
int p_BFSReverse_pRange (PGRAPH&  _pg ,
			 VISITOR& _vis,
			 VID      _startVid) {

  typedef stapl::linear_boundary<typename PGRAPH::VI> linear_boundary;
  typedef stapl::pRange<linear_boundary,PGRAPH,BFSDDGReverse<PGRAPH> > pGraphpRange;
  typename PGRAPH::VI vi;

  BFSDDGReverse<PGRAPH> ddg(_pg,_startVid);
  pGraphpRange pgpr(&_pg);
  pgpr.set_ddg(&ddg);
  stapl::rmi_fence();

  for(vi = _pg.local_begin();vi != _pg.local_end(); vi++){
    pGraphpRange* ppr = new pGraphpRange(vi->vid, stapl::get_thread_id(), &_pg,linear_boundary(vi,vi+1),10000,&pgpr);
    pgpr.add_subrange(*ppr);
  }

  vector<VISITOR> vvis;
  vvis.push_back(_vis);

  vector<pGraphpRange> vpr;
  vpr.push_back(pgpr);

  p_BFSReverse_pRange<PGRAPH,pGraphpRange,VISITOR>(vpr, vvis);

  return OK;
}// end BFSReverse traversal

template<class PGRAPH, class VISITOR>
int p_TOPO_pRange (PGRAPH& _pg ,
		  VISITOR& _vis,
		  VID _startVid) {

  typedef stapl::linear_boundary<typename PGRAPH::VI> linear_boundary;
  typedef stapl::pRange<linear_boundary,PGRAPH,TOPODDG<PGRAPH> > pGraphpRange;
  typename PGRAPH::VI vi;

  TOPODDG<PGRAPH> ddg(_pg);
  pGraphpRange pgpr(&_pg);
  pgpr.set_ddg(&ddg);
  stapl::rmi_fence();

  for(vi = _pg.local_begin();vi != _pg.local_end(); vi++){
    pGraphpRange* ppr = new pGraphpRange(vi->vid, stapl::get_thread_id(), &_pg,linear_boundary(vi,vi+1),10000,&pgpr);
    pgpr.add_subrange(*ppr);
  }

  vector<VISITOR> vvis;
  vvis.push_back(_vis);

  vector<pGraphpRange> vpr;
  vpr.push_back(pgpr);

  p_TOPO_pRange<PGRAPH,pGraphpRange,VISITOR>(vpr, vvis);

  return OK;
}// end TOPO traversal

template<class PGRAPH, class VISITOR>
int p_TOPOReverse_pRange (PGRAPH&  _pg ,
			 VISITOR& _vis,
			 VID      _startVid) {

  typedef stapl::linear_boundary<typename PGRAPH::VI> linear_boundary;
  typedef stapl::pRange<linear_boundary,PGRAPH,TOPODDGReverse<PGRAPH> > pGraphpRange;
  typename PGRAPH::VI vi;

  TOPODDGReverse<PGRAPH> ddg(_pg);
  pGraphpRange pgpr(&_pg);
  pgpr.set_ddg(&ddg);
  stapl::rmi_fence();

  for(vi = _pg.local_begin();vi != _pg.local_end(); vi++){
    pGraphpRange* ppr = new pGraphpRange(vi->vid, stapl::get_thread_id(), &_pg,linear_boundary(vi,vi+1),10000,&pgpr);
    pgpr.add_subrange(*ppr);
  }

  vector<VISITOR> vvis;
  vvis.push_back(_vis);

  vector<pGraphpRange> vpr;
  vpr.push_back(pgpr);

  p_TOPOReverse_pRange<PGRAPH,pGraphpRange,VISITOR>(vpr, vvis);

  return OK;
}// end TOPOReverse traversal

////////////////////////////////////////////////////////////////////////
//                               VECTOR of  GRAPHS
////////////////////////////////////////////////////////////////////////
template<class PGRAPH, class VISITOR>
int p_BFS_pRange (vector<PGRAPH>&  _vpg ,
		  vector<VISITOR>& _vvis,vector<VID>& _startVID) {

  typename PGRAPH::VI vi, tvi;
  int myid = stapl::get_thread_id();

  typedef stapl::linear_boundary<typename PGRAPH::VI> linear_boundary;
  typedef stapl::pRange<linear_boundary,PGRAPH,BFSDDG<PGRAPH> > pGraphpRange;
  typedef stapl::defaultScheduler scheduler;

  vector<pGraphpRange> vpr(_vpg.size());
  stapl::rmi_fence();

  for(int i=0;i<_vpg.size();i++){
    BFSDDG<PGRAPH>* ddg = new BFSDDG<PGRAPH>(_vpg[i],_startVID[i]);
    stapl::rmi_fence();

    pGraphpRange pgpr(&_vpg[i]);
    stapl::rmi_fence();

    pgpr.set_ddg(ddg);

    for(vi = _vpg[i].local_begin();vi != _vpg[i].local_end(); vi++){
      pGraphpRange* ppr = new pGraphpRange(vi->vid, myid, &_vpg[i],linear_boundary(vi,vi+1),10000,&pgpr);
      pgpr.add_subrange(*ppr);
    }
    vpr[i] = pgpr;
  }

  p_BFS_pRange<PGRAPH,pGraphpRange,VISITOR>(vpr,_vvis);

  return OK;
}// end BFS traversal

template<class PGRAPH, class VISITOR>
int p_BFSReverse_pRange (vector<PGRAPH>&  _vpg ,
			  vector<VISITOR>& _vvis,vector<VID>& _startVID) {

  typename PGRAPH::VI vi, tvi;
  int myid = stapl::get_thread_id();

  typedef stapl::linear_boundary<typename PGRAPH::VI> linear_boundary;
  typedef stapl::pRange<linear_boundary,PGRAPH,BFSDDGReverse<PGRAPH> > pGraphpRange;
  typedef stapl::defaultScheduler scheduler;

  vector<pGraphpRange> vpr(_vpg.size());
  stapl::rmi_fence();

  for(int i=0;i<_vpg.size();i++){
    BFSDDGReverse<PGRAPH>* ddg = new BFSDDGReverse<PGRAPH>(_vpg[i],_startVID[i]);
    stapl::rmi_fence();

    pGraphpRange pgpr(&_vpg[i]);
    stapl::rmi_fence();

    pgpr.set_ddg(ddg);

    for(vi = _vpg[i].local_begin();vi != _vpg[i].local_end(); vi++){
      pGraphpRange* ppr = new pGraphpRange(vi->vid, myid, &_vpg[i],linear_boundary(vi,vi+1),10000,&pgpr);
      pgpr.add_subrange(*ppr);
    }
    vpr[i] = pgpr;
  }

  p_BFSReverse_pRange<PGRAPH,pGraphpRange,VISITOR>(vpr,_vvis);

  return OK;
}// end BFSReverse traversal


//                        EARLY QUIT

template<class PGRAPH, class VISITOR>
int p_BFS_pRange_EQ (vector<PGRAPH>&  _vpg ,
		     vector<VISITOR>& _vvis,vector<VID>& _startVID) {

  typename PGRAPH::VI vi, tvi;
  int myid = stapl::get_thread_id();

  typedef stapl::linear_boundary<typename PGRAPH::VI> linear_boundary;
  typedef stapl::pRange<linear_boundary,PGRAPH,BFSDDG<PGRAPH> > pGraphpRange;
  typedef stapl::defaultScheduler scheduler;

  vector<pGraphpRange> vpr(_vpg.size());
  stapl::rmi_fence();

  for(int i=0;i<_vpg.size();i++){
    BFSDDG<PGRAPH>* ddg = new BFSDDG<PGRAPH>(_vpg[i],_startVID[i]);
    stapl::rmi_fence();

    pGraphpRange pgpr(&_vpg[i]);
    stapl::rmi_fence();

    pgpr.set_ddg(ddg);

    for(vi = _vpg[i].local_begin();vi != _vpg[i].local_end(); vi++){
      pGraphpRange* ppr = new pGraphpRange(vi->vid, myid, &_vpg[i],linear_boundary(vi,vi+1),10000,&pgpr);
      pgpr.add_subrange(*ppr);
    }
    vpr[i] = pgpr;
  }

  p_BFS_pRange_EQ<PGRAPH,pGraphpRange,VISITOR>(vpr,_vvis);

  return OK;
}// end BFS_EQ traversal

template<class PGRAPH, class VISITOR>
int p_BFSReverse_pRange_EQ (vector<PGRAPH>&  _vpg ,
			    vector<VISITOR>& _vvis) {

  typename PGRAPH::VI vi, tvi;
  int myid = stapl::get_thread_id();

  typedef stapl::linear_boundary<typename PGRAPH::VI> linear_boundary;
  typedef stapl::pRange<linear_boundary,PGRAPH,BFSDDGReverse<PGRAPH> > pGraphpRange;
  typedef stapl::defaultScheduler scheduler;

  vector<pGraphpRange> vpr(_vpg.size());
  stapl::rmi_fence();

  for(int i=0;i<_vpg.size();i++){
    BFSDDGReverse<PGRAPH>* ddg = new BFSDDGReverse<PGRAPH>(_vpg[i],_startVID[i]);
    stapl::rmi_fence();

    pGraphpRange pgpr(&_vpg[i]);
    stapl::rmi_fence();

    pgpr.set_ddg(ddg);

    for(vi = _vpg[i].local_begin();vi != _vpg[i].local_end(); vi++){
      pGraphpRange* ppr = new pGraphpRange(vi->vid, myid, &_vpg[i],linear_boundary(vi,vi+1),10000,&pgpr);
      pgpr.add_subrange(*ppr);
    }
    vpr[i] = pgpr;
  }

  p_BFSReverse_pRange_EQ<PGRAPH,pGraphpRange,VISITOR>(vpr,_vvis);

  return OK;
}// end BFSReverse_EQ traversal

//                             TOPOLOGICAL traversal
template<class PGRAPH, class VISITOR>
int p_TOPO_pRange (vector<PGRAPH>&  _vpg ,
		  vector<VISITOR>& _vvis) {

  typename PGRAPH::VI vi, tvi;
  int myid = stapl::get_thread_id();

  typedef stapl::linear_boundary<typename PGRAPH::VI> linear_boundary;
  typedef stapl::pRange<linear_boundary,PGRAPH,TOPODDG<PGRAPH> > pGraphpRange;
  typedef stapl::defaultScheduler scheduler;

  vector<pGraphpRange> vpr(_vpg.size());
  stapl::rmi_fence();

  for(int i=0;i<_vpg.size();i++){
    TOPODDG<PGRAPH>* ddg = new TOPODDG<PGRAPH>(_vpg[i]);
    stapl::rmi_fence();

    pGraphpRange pgpr(&_vpg[i]);
    stapl::rmi_fence();

    pgpr.set_ddg(ddg);

    for(vi = _vpg[i].local_begin();vi != _vpg[i].local_end(); vi++){
      pGraphpRange* ppr = new pGraphpRange(vi->vid, myid, &_vpg[i],linear_boundary(vi,vi+1),10000,&pgpr);
      pgpr.add_subrange(*ppr);
    }
    vpr[i] = pgpr;
  }

  p_TOPO_pRange<PGRAPH,pGraphpRange,VISITOR>(vpr,_vvis);

  return OK;
}// end TOPO traversal

template<class PGRAPH, class VISITOR>
int p_TOPOReverse_pRange (vector<PGRAPH>&  _vpg ,
			  vector<VISITOR>& _vvis) {

  typename PGRAPH::VI vi, tvi;
  int myid = stapl::get_thread_id();

  typedef stapl::linear_boundary<typename PGRAPH::VI> linear_boundary;
  typedef stapl::pRange<linear_boundary,PGRAPH,TOPODDGReverse<PGRAPH> > pGraphpRange;
  typedef stapl::defaultScheduler scheduler;

  vector<pGraphpRange> vpr(_vpg.size());
  stapl::rmi_fence();

  for(int i=0;i<_vpg.size();i++){
    TOPODDGReverse<PGRAPH>* ddg = new TOPODDGReverse<PGRAPH>(_vpg[i]);
    stapl::rmi_fence();

    pGraphpRange pgpr(&_vpg[i]);
    stapl::rmi_fence();

    pgpr.set_ddg(ddg);

    for(vi = _vpg[i].local_begin();vi != _vpg[i].local_end(); vi++){
      pGraphpRange* ppr = new pGraphpRange(vi->vid, myid, &_vpg[i],linear_boundary(vi,vi+1),10000,&pgpr);
      pgpr.add_subrange(*ppr);
    }
    vpr[i] = pgpr;
  }

  p_TOPOReverse_pRange<PGRAPH,pGraphpRange,VISITOR>(vpr,_vvis);

  return OK;
}// end TOPOReverse traversal


////////////////////////////////////////////////////////////////////////
//                               VECTOR of  PRANGES
////////////////////////////////////////////////////////////////////////

template<class PGRAPH, class PRANGE, class VISITOR>
int p_BFS_pRange (vector<PRANGE>& _vpr, vector<VISITOR>& _vvis) {

  typedef stapl::defaultScheduler scheduler;

  vector<_BFS_traversal_prange<PGRAPH,VISITOR> > vf;
  scheduler s;

  for(int i=0;i<_vpr.size();i++){
    _BFS_traversal_prange<PGRAPH,VISITOR> _bfst(_vvis[i]);
    vf.push_back(_bfst);
  }

  stapl::rmi_fence();
  p_for_all_fence<PRANGE, _BFS_traversal_prange<PGRAPH,VISITOR>,scheduler>(_vpr, vf, s);
  return OK;
}// end BFS traversal

template<class PGRAPH, class PRANGE, class VISITOR>
int p_BFSReverse_pRange (vector<PRANGE>& _vpr, vector<VISITOR>& _vvis) {

  typedef stapl::defaultScheduler scheduler;

  vector<_BFSReverse_traversal_prange<PGRAPH,VISITOR> > vf;
  scheduler s;

  for(int i=0;i<_vpr.size();i++){
    _BFSReverse_traversal_prange<PGRAPH,VISITOR> _bfst(_vvis[i]);
    vf.push_back(_bfst);
  }

  stapl::rmi_fence();
  p_for_all_fence<PRANGE, _BFSReverse_traversal_prange<PGRAPH,VISITOR>,scheduler>(_vpr, vf, s);
  return OK;
}// end BFSReverse traversal


//                                 EARLY_QUIT

template<class PGRAPH, class PRANGE, class VISITOR>
int p_BFS_pRange_EQ (vector<PRANGE>& _vpr, vector<VISITOR>& _vvis) {

  typedef stapl::defaultScheduler scheduler;

  vector<_BFS_traversal_prange<PGRAPH,VISITOR> > vf;
  scheduler s;

  for(int i=0;i<_vpr.size();i++){
    _BFS_traversal_prange<PGRAPH,VISITOR> _bfst(_vvis[i]);
    vf.push_back(_bfst);
  }

  stapl::rmi_fence();
  p_for_all_fence_EQ<PRANGE, _BFS_traversal_prange<PGRAPH,VISITOR>,scheduler>(_vpr, vf, s);
  return OK;
}// end BFS traversal

template<class PGRAPH, class PRANGE, class VISITOR>
int p_BFSReverse_pRange_EQ (vector<PRANGE>& _vpr, vector<VISITOR>& _vvis) {

  typedef stapl::defaultScheduler scheduler;

  vector<_BFSReverse_traversal_prange<PGRAPH,VISITOR> > vf;
  scheduler s;

  for(int i=0;i<_vpr.size();i++){
    _BFSReverse_traversal_prange<PGRAPH,VISITOR> _bfst(_vvis[i]);
    vf.push_back(_bfst);
  }

  stapl::rmi_fence();
  p_for_all_fence_EQ<PRANGE, _BFSReverse_traversal_prange<PGRAPH,VISITOR>,scheduler>(_vpr, vf, s);
  return OK;
}// end BFSReverse traversal



//                          TOPOLOGICAL traversal

template<class PGRAPH, class PRANGE, class VISITOR>
int p_TOPO_pRange (vector<PRANGE>& _vpr, vector<VISITOR>& _vvis) {

  typedef stapl::defaultScheduler scheduler;

  vector<_TOPO_traversal_prange<PGRAPH,VISITOR> > vf;
  scheduler s;

  for(int i=0;i<_vpr.size();i++){
    _TOPO_traversal_prange<PGRAPH,VISITOR> _bfst(_vvis[i]);
    vf.push_back(_bfst);
  }

  stapl::rmi_fence();
  p_for_all_fence<PRANGE, _TOPO_traversal_prange<PGRAPH,VISITOR>,scheduler>(_vpr, vf, s);
  return OK;
}// end TOPO traversal

template<class PGRAPH, class PRANGE, class VISITOR>
int p_TOPOReverse_pRange (vector<PRANGE>& _vpr, vector<VISITOR>& _vvis) {

  typedef stapl::defaultScheduler scheduler;

  vector<_TOPOReverse_traversal_prange<PGRAPH,VISITOR> > vf;
  scheduler s;

  for(int i=0;i<_vpr.size();i++){
    _TOPOReverse_traversal_prange<PGRAPH,VISITOR> _bfst(_vvis[i]);
    vf.push_back(_bfst);
  }

  stapl::rmi_fence();
  p_for_all_fence<PRANGE, _TOPOReverse_traversal_prange<PGRAPH,VISITOR>,scheduler>(_vpr, vf, s);
  return OK;
}// end TOPO traversal

} //end namespace stapl
#endif
