#ifndef _P_GRAPH_ALGO_H
#define _P_GRAPH_ALGO_H

#include "Defines.h"
#include "pBaseGraph.h"
#include "Graph.h"
#include "GraphAlgo.h"
#include "pvector.h"

////////////////////
//pgraph algorithms 
////////////////////

namespace stapl{

#ifndef STAPL_P_FWD
#define STAPL_P_FWD 0
#endif

#ifndef STAPL_P_BACK
#define STAPL_P_BACK 1
#endif


#define STAPL_SET_TOKEN 1
#define STAPL_UNSET_TOKEN 0


/*
DFS_Functor internal function --> distributed version
*/
template <class PGRAPH,class VISITOR>
class __DFS_Functor : public BasePObject{
  
  PGRAPH* _g;
  VISITOR* _f;
  int tt;
  int token;
public:

   map<VID, int> _color;

  __DFS_Functor(){
    this->register_this(this);
  }

  ~__DFS_Functor(){
    rmiHandle handle = this->getHandle();
    if (handle > -1)
      stapl::unregister_rmi_object(handle);
    _color.clear();
  }

  __DFS_Functor(PGRAPH* pg, VISITOR* f){
    _g = pg;
    _f = f;
    this->register_this(this);
  }

  int SetTT(int _tt){
    //printf("REmote set tt %d\n",_tt);
    tt = _tt;
    return tt;
  }
  
  int GetTT(){
    return tt;
  }

  int SetToken(int _tt){

    printf("##########%d\n",stapl::get_thread_id());

    int temp = token; 
    token    = STAPL_SET_TOKEN;
    tt = _tt;
    return   temp;
  }

  int UnsetToken(){
    int temp = token; 
    token    = STAPL_UNSET_TOKEN;
    return   temp;
  }

  int GetToken(){
    return token;
  }


  int __p_recursive_DFS_SET(VID _v1id,int _tt){
    tt=_tt;
    return __p_recursive_DFS(_v1id);
  }

  //arguments:: vertex id and termination times
  int __p_recursive_DFS(VID _v1id){
    VID _v2id;
    typename PGRAPH::VI v1,v2;
    typename PGRAPH::EI ei;
    map<VID, int>::iterator mit;
    int temp;
    
    mit = _color.find(_v1id);
    if(_color.size() != 0 && mit != _color.end()){//if color is not WHITE
      return tt;
    }
    
    _color[_v1id] = GRAY;
    
    if ( !_g->IsVertex(_v1id, &v1)){
      //here _v1id is ghost
      //...
      printf("ERROR::__p_recursive_DFS redundant check\n");
      return ERROR;
    }
    //printf("Local %d\n",_v1id);
    _f->vertex(v1);
    ei = v1->edgelist.begin(); 
    while ( ei != v1->edgelist.end() ) {
      _v2id = ei->vertex2id;
      mit = _color.find(_v2id);
      if(mit == _color.end()){//if color is WHITE
	_f->tree_edge(v1,ei);
	//_color[_v2id]=GRAY;//this add _v2id into current color map
	//printf("going to %d from %d  :- local tt %d\n",_v2id,_v1id, tt);
	if ( _g->IsVertex(_v2id, &v2) ) {
	  temp =  __p_recursive_DFS(_v2id);
	  tt = temp;
	}
	else{
	  //find where _v2id lives and call async there
	  Location _n=_g->Lookup(_v2id);
	  if(!_n.ValidLocation()){
	    cout<<"ERROR::: "<<_v2id<<" "<<_v1id<<" " <<stapl::get_thread_id()<<endl;
	  }
	  stapl_assert(_n.ValidLocation(),"Invalid id for the second vertex of an edge in DFS");
	  //cout<<"DEST:"<<_n.locpid()<<"::::"<<this->getHandle()<<endl;

	  temp = stapl::sync_rmi( _n.locpid(), 
				  this->getHandle(),
				  &__DFS_Functor::__p_recursive_DFS_SET, _v2id,tt);
	  tt = temp;
	}
	if(temp == ERROR) return ERROR;
      }
      else if(mit->second == GRAY){
	_f->back_edge(v1,ei);
      }
      else if(mit->second == BLACK){
	_f->ft_edge(v1,ei);
      }
      else cout<<"ERROR: colors in DFS functor"<<endl;
      ei++; 
    } 
    _color[_v1id] = BLACK;
    _f->finish_vertex(v1,++tt);

    return tt;  

  }//end __p_recursize_DFS

};//end class definition
  

/**DFS functor; Apply visitor _f to all the nodes in _g accesible
 * from _startVid in a BFS manner
 *@param _g the graph for which we apply DFS
 *@param _f the visitor that will be used for traversal
 *@param _startVid start vertex for traversal
 * @return ERROR if something wrong happened
 */
template<class PGRAPH,class VISITOR>
int pDFSfunctor (PGRAPH& _g, 
		 VISITOR& _f, 
		 VID _startVid) {
  int temp;
  
  __DFS_Functor<PGRAPH,VISITOR> _dfsf(&_g , &_f);
  stapl::rmi_fence();

  _dfsf.SetTT(0);

  if(_g.IsLocal(_startVid) ) {//only one of them start the DFS
    printf("Started  on thread ---------> %d\n",stapl::get_thread_id());
    temp = _dfsf.__p_recursive_DFS(_startVid);
  }
  
  //next line can be moved out if we don't want all the threads to
  //run this call
  stapl::rmi_fence();

  //temp has to be broadcasted to everybody before return
  return temp;
}



/**DFS functor; Apply visitor _f to all the nodes in _g accesible
 * from _startVid in a BFS manner
 *@param _g the graph for which we apply DFS
 *@param _f the visitor that will be used for traversal
 *@param _startVid start vertex for traversal
 * @return ERROR if something wrong happened
 */

template<class PGRAPH,class VISITOR>
int pDFSAllFunctor (PGRAPH& _g, 
		    VISITOR& _f) {

  int temp;
  int myid = stapl::get_thread_id(); 
  int NP   = stapl::get_num_threads();
  map<VID, int>::iterator mit;
  int i;

  __DFS_Functor<PGRAPH,VISITOR> _dfsf(&_g , &_f);

  if(myid == 0){
    //printf("Set\n");
    _dfsf.SetToken(0);    
  }
  else {
    //printf("Unset\n");
    _dfsf.UnsetToken();
  }
  stapl::rmi_fence();
 
  //if it is not your turn wait
  while(_dfsf.GetToken() == STAPL_UNSET_TOKEN)
    stapl::rmi_wait();

  //printf("AFFFTTERRRRR %d\n",myid);

  //here I have the token
  for(typename PGRAPH::VI vi = _g.local_begin(); vi != _g.local_end(); vi++) {	 
    mit = _dfsf._color.find(vi->vid);
    if(mit == _dfsf._color.end()){
      //printf("Started  on thread ---------> %d\n",stapl::get_thread_id());
      temp = _dfsf.__p_recursive_DFS(vi->vid);
    }
  }

  //here all local traversals are finished
  // reset the token 
  //pass the token to the next processor and also the current termination time;
  if(myid+1 < NP){
    temp = stapl::sync_rmi( myid+1, 
			    _dfsf.getHandle(),
			    &__DFS_Functor<PGRAPH,VISITOR>::SetToken,_dfsf.GetTT());
  }
  stapl::rmi_fence();

  //temp has to be broadcasted to everybody before return
  return temp;
}


////////////////////////////////////////////////////
//       DFS traversal on the transpose graph;
// the algorithm assumes that the user has called SetPredecessors before 
// invocation
////////////////////////////////////////////////////
/*
DFS_Functor internal function --> distributed version
*/
template <class PGRAPH,class VISITOR>
class __DFS_TransposeFunctor : public BasePObject{
  
  PGRAPH* _g;
  int tt;
  int token;
public:
  VISITOR* _f;

   map<VID, int> _color;

  __DFS_TransposeFunctor(){
    this->register_this(this);
  }

  ~__DFS_TransposeFunctor(){
    rmiHandle handle = this->getHandle();
    if (handle > -1)
      stapl::unregister_rmi_object(handle);
    _color.clear();
  }

  __DFS_TransposeFunctor(PGRAPH* pg, VISITOR* f){
    _g = pg;
    _f = f;
    this->register_this(this);
  }

  int SetTT(int _tt){
    //printf("REmote set tt %d\n",_tt);
    tt = _tt;
    return tt;
  }
  
  int GetTT(){
    return tt;
  }

  int SetToken(int _tt){

    printf("##########%d\n",stapl::get_thread_id());

    int temp = token; 
    token    = STAPL_SET_TOKEN;
    tt = _tt;
    return   temp;
  }

  int UnsetToken(){
    int temp = token; 
    token    = STAPL_UNSET_TOKEN;
    return   temp;
  }

  int GetToken(){
    return token;
  }


  int __p_recursive_DFS_SET(_StaplTriple<VID,int,VISITOR> _arg){
    tt=_arg.second;
    *_f = _arg.third;//assign the visitor
    return __p_recursive_DFS(_arg.first);
  }
 
  //arguments:: vertex id and termination times
  int __p_recursive_DFS(VID _v1id){
    VID _v2id;
    typename PGRAPH::VI v1,v2;
    typename PGRAPH::EI ei;
    map<VID, int>::iterator mit;
    int temp;
    
    mit = _color.find(_v1id);
    if(_color.size() != 0 && mit != _color.end()){//if color is not WHITE
      return tt;
    }
    
    _color[_v1id] = GRAY;
    
    if ( !_g->IsVertex(_v1id, &v1)){
      cout<<"ERROR::__p_recursive_DFS redundant check; the transpose version\n";
      return ERROR;
    }

    _f->vertex(v1);

    ei = v1->predecessors.begin(); 
    while ( ei != v1->predecessors.end() ) {
      _v2id = ei->vertex2id;
      mit = _color.find(_v2id);
      if(mit == _color.end()){//if color is WHITE
	_f->tree_edge(v1,ei);
	//_color[_v2id]=GRAY;//this add _v2id into current color map
	//printf("going to %d from %d  :- local tt %d\n",_v2id,_v1id, tt);
	if ( _g->IsVertex(_v2id, &v2) ) {
	  temp =  __p_recursive_DFS(_v2id);
	  tt = temp;
	}
	else{
	  //find where _v2id lives and call async there
	  Location _n=_g->Lookup(_v2id);
	  if(!_n.ValidLocation()){
	    cout<<"ERROR::__p_recursive_DFS transpose "<<_v2id<<" "<<_v1id<<" "<<stapl::get_thread_id()<<endl;
	  }
	  stapl_assert(_n.ValidLocation(),"Invalid id for the second vertex of an edge in DFS transpose");
	  //cout<<"DEST:"<<_n.locpid()<<"::::"<<this->getHandle()<<endl;
	  _StaplTriple<VID,int,VISITOR> _arg(_v2id,tt,*_f);
	  temp = stapl::sync_rmi( _n.locpid(), 
				  this->getHandle(),
				  &__DFS_TransposeFunctor::__p_recursive_DFS_SET, _arg);
	  tt = temp;
	}
	if(temp == ERROR) return ERROR;
      }
      else if(mit->second == GRAY){
	_f->back_edge(v1,ei);
      }
      else if(mit->second == BLACK){
	_f->ft_edge(v1,ei);
      }
      else cout<<"ERROR: colors in DFS functor"<<endl;
      ei++; 
    } 
    _color[_v1id] = BLACK;
    _f->finish_vertex(v1,++tt);

    return tt;  

  }//end __p_recursive_DFS

};//end class definition
  

/**DFS functor; Apply visitor _f to all the nodes in _g accesible
 * from _startVid in a BFS manner
 *@param _g the graph for which we apply DFS
 *@param _f the visitor that will be used for traversal
 *@param _startVid start vertex for traversal
 * @return ERROR if something wrong happened
 */
template<class PGRAPH,class VISITOR>
int pDFSTransposeFunctor (PGRAPH& _g, 
		 VISITOR& _f, 
		 VID _startVid) {
  int temp;
  
  __DFS_TransposeFunctor<PGRAPH,VISITOR> _dfsf(&_g , &_f);
  stapl::rmi_fence();

  _dfsf.SetTT(0);

  if(_g.IsLocal(_startVid) ) {//only one of them start the DFS
    temp = _dfsf.__p_recursive_DFS(_startVid);
  }
  
  //next line can be moved out if we don't want all the threads to
  //run this call
  stapl::rmi_fence();

  //temp has to be broadcasted to everybody before return
  return temp;
}



/**DFS functor; Apply visitor _f to all the nodes in _g accesible
 * from _startVid in a BFS manner
 *@param _g the graph for which we apply DFS
 *@param _f the visitor that will be used for traversal
 *@param _startVid start vertex for traversal
 * @return ERROR if something wrong happened
 */

template<class PGRAPH,class VISITOR>
int pDFSAllTransposeFunctor (PGRAPH& _g, 
		    VISITOR& _f) {

  int temp;
  int myid = stapl::get_thread_id(); 
  int NP   = stapl::get_num_threads();
  map<VID, int>::iterator mit;
  int i;

  __DFS_TransposeFunctor<PGRAPH,VISITOR> _dfsf(&_g , &_f);

  if(myid == 0){
    printf("Set\n");
    _dfsf.SetToken(0);    
  }
  else {
    printf("Unset\n");
    _dfsf.UnsetToken();
  }
  stapl::rmi_fence();
 
  //if it is not your turn wait
  while(_dfsf.GetToken() == STAPL_UNSET_TOKEN)
    stapl::rmi_wait();

  printf("AFFFTTERRRRR %d\n",myid);

  //here I have the token
  for(typename PGRAPH::VI vi = _g.local_begin(); vi != _g.local_end(); vi++) {	 
    mit = _dfsf._color.find(vi->vid);
    if(mit == _dfsf._color.end()){
      printf("Started  on thread ---------> %d\n",stapl::get_thread_id());
      temp = _dfsf.__p_recursive_DFS(vi->vid);
    }
  }

  //here all local traversals are finished
  // reset the token 
  //pass the token to the next processor and also the current termination time;
  if(myid+1 < NP){
    temp = stapl::sync_rmi( myid+1, 
			    _dfsf.getHandle(),
			    &__DFS_TransposeFunctor<PGRAPH,VISITOR>::SetToken,_dfsf.GetTT());
  }

  stapl::rmi_fence();
  return temp;
}


//this version of the DFS takes as input a vector specifying the order 
//in which the vertices to be processed
template<class PGRAPH,class VISITOR>
int pDFSAllTransposeFunctor (PGRAPH& _g, 
			     VISITOR& _f , 
			     pvector<int>& _order) {

  int temp;
  int myid = stapl::get_thread_id(); 
  int NP   = stapl::get_num_threads();
  map<VID, int>::iterator mit;
  int i;

  __DFS_TransposeFunctor<PGRAPH,VISITOR> _dfsf(&_g , &_f);
  if(myid == 0){
    _dfsf.SetToken(0);    
  }
  else {
    _dfsf.UnsetToken();
  }
  stapl::rmi_fence();
 
  //if it is not your turn wait
  while(_dfsf.GetToken() == STAPL_UNSET_TOKEN)
    stapl::rmi_wait();

  for(pvector<int>::iterator pvi=_order.local_begin();pvi!=_order.local_end();pvi++){
    mit = _dfsf._color.find(*pvi);
    if(mit == _dfsf._color.end()){
      //find where pvi lives and call async there

      _dfsf._f->SetSCCID(*pvi);

      if(_g.IsLocal(*pvi)){
	temp = _dfsf.__p_recursive_DFS(*pvi);
      }
      else{
	Location _n=_g.Lookup(*pvi);
	if(!_n.ValidLocation()){
	  cout<<"ERROR::__p_recursive_DFS transpose "<<*pvi<<" "<<stapl::get_thread_id()<<endl;
	}
	stapl_assert(_n.ValidLocation(),"Invalid id for the second vertex of an edge in DFS transpose");
	//cout<<"DFS will start on transpose :"<<_n.locpid()<<"::::"<<endl;
	_StaplTriple<VID,int,VISITOR> _arg(*pvi, _dfsf.GetTT() , *(_dfsf._f));
	temp = stapl::sync_rmi( _n.locpid(), 
				_dfsf.getHandle(),
				&__DFS_TransposeFunctor<PGRAPH,VISITOR>::__p_recursive_DFS_SET, _arg);
	_dfsf.SetTT(temp);
      }
    }
  }

  //here all local traversals are finished
  //pass the token to the next processor and also the current termination time;
  if(myid+1 < NP){
    temp = stapl::sync_rmi( myid+1, 
			    _dfsf.getHandle(),
			    &__DFS_TransposeFunctor<PGRAPH,VISITOR>::SetToken,_dfsf.GetTT());
  }

  stapl::rmi_fence();
  return temp;
}



//////////////////////////////////////////////////////////////////////////////
//                   BFS like traversal (multiple graphs)
//////////////////////////////////////////////////////////////////////////////

template <class PG,class VISITOR>
class BFS_traversal_multi: public BasePObject{  
 public:

  typedef map<VID,int> MAP;

  vector<MAP> _color_fwd;
  vector<MAP> _color_back;
  vector<PG*> _pgraphs;
  vector<VISITOR*> _f_fwd;
  vector<VISITOR*> _f_back;

  //the next triplet includes vertex vi , index and direction(fwd,back)
  list<_StaplTriple<typename PG::VI,int,int> > QT;//queue used for trimer traversals
  bool inside;

  BFS_traversal_multi(){
    SetInside(false);
    register_this(this);
  }

  ~BFS_traversal_multi(){
    rmiHandle handle = this->getHandle();
    if (handle > -1)
      stapl::unregister_rmi_object(handle);
  }

  BFS_traversal_multi(vector<PG*>& _pgs, 
		      vector<VISITOR*> _vvis_fwd, 
		      vector<VISITOR*> _vvis_back) {
    _pgraphs = _pgs;
    _f_fwd = _vvis_fwd;
    _f_back = _vvis_back;
    _color_fwd.resize(_pgs.size());
    _color_back.resize(_pgs.size());
    SetInside(false);
    register_this(this);
  }

  void SetInside(bool _i){
    inside = _i;
  }

  void addQueue(typename PG::VI vi,int index){
    QT.push_back(_StaplTriple<typename PG::VI,int,int>(vi,index,STAPL_P_FWD));
  }

  void addQueueFront(typename PG::VI vi,int index){
    QT.push_front(_StaplTriple<typename PG::VI,int,int>(vi,index,STAPL_P_FWD));
  }

  void addQueueReverse(typename PG::VI vi,int index){
    QT.push_back(_StaplTriple<typename PG::VI,int,int>(vi,index,STAPL_P_BACK));
  }

  void addQueueFrontReverse(typename PG::VI vi,int index){
    QT.push_front(_StaplTriple<typename PG::VI,int,int>(vi,index,STAPL_P_BACK));
  }

  _StaplTriple<typename PG::VI,int,int> popQueue(){
    _StaplTriple<typename PG::VI,int,int> temp = QT.front();
    QT.pop_front();
    return temp;
  }
  
  inline int size(){
    return QT.size();
  }

  void remoteAdd(_StaplTriple<VID,int,VISITOR> _arg){
    typename PG::VI vi;
    map<VID, int>::iterator mit;
    //cout<<"remote function called"<<endl;

    if(!_pgraphs[_arg.second]->IsVertex(_arg.first,&vi)){
      return;
    }
    //here vi is initialized properly;
    //check if it's color is GRAY; if so don't added
    mit = _color_fwd[_arg.second].find(_arg.first);

    if(mit == _color_fwd[_arg.second].end()){//if color is WHITE

      *(_f_fwd[_arg.second]) = _arg.third;

      _color_fwd[_arg.second][vi->vid] = GRAY;
      addQueue(vi,_arg.second);
      if (inside == false) BFSLoop();
    }
  }

  void remoteAddReverse(_StaplTriple<VID,int,VISITOR> _arg){
    typename PG::VI vi;
    map<VID, int>::iterator mit;
    //cout<<"remote function called"<<endl;

    if(!_pgraphs[_arg.second]->IsVertex(_arg.first,&vi)){
      return;
    }
    //here vi is initialized properly;
    //check if it's color is GRAY; if so don't added
    mit = _color_back[_arg.second].find(_arg.first);

    if(mit == _color_back[_arg.second].end()){//if color is WHITE

      *(_f_back[_arg.second]) = _arg.third;

      _color_back[_arg.second][vi->vid] = GRAY;
      addQueueReverse(vi,_arg.second);
      if (inside == false) BFSLoop();
    }
  }

  int BFSLoop(){
    //sweep to the graph associated with this trimer
    typename PG::EI ei;
    typename PG::VI vi;
    //vi , index, direction
    _StaplTriple<typename PG::VI,int,int> v;
    map<VID, int>::iterator mit;    
    vector<VID> succs;

    SetInside(true);

    while(size() > 0){//while there are more vertices
      //pop a vertex from Q
      v = popQueue();

      if(v.third == STAPL_P_FWD)
	_f_fwd[v.second]->vertex(v.first);
      else
	_f_back[v.second]->vertex(v.first);
      
      //for every child u of v
      if(v.third == STAPL_P_FWD){
	int t = _pgraphs[v.second]->GetAdjacentVertices(v.first,succs);
	cout<<"Adjacency for "<<v.first->vid<<" "<< t <<" "<<succs.size()<<endl;
	for(int v2id=0;v2id<t;v2id++){
	  if(!_pgraphs[v.second]->IsVertex(succs[v2id],&vi)){
	    //the vertex is not local
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    
	    //but first checked if not touched(GRAY)
	    mit = _color_fwd[v.second].find(succs[v2id]);
	    if(mit == _color_fwd[v.second].end()){//if color is WHITE
	      //cout<<"remote vertex"<<endl;
	      Location _n=_pgraphs[v.second]->Lookup(succs[v2id]);
	      stapl_assert(_n.ValidLocation(),
			   "Invalid id for the second vertex of an edge in BFS");
	      _color_fwd[v.second][succs[v2id]] = GRAY;
	      _StaplTriple<VID,int,VISITOR> _arg(succs[v2id],v.second,*(_f_fwd[v.second]));
	      stapl::async_rmi( _n.locpid(), 
				getHandle(),
				&BFS_traversal_multi::remoteAdd,_arg);
	    }
	  }
	  else{
	    //here we are if the vertex is local
	    //first check the color of the node
	    mit = _color_fwd[v.second].find(succs[v2id]);
	    if(mit == _color_fwd[v.second].end()){//if color is WHITE
	      //addQueueFront(vi,v.second);
	      _color_fwd[v.second][succs[v2id]] = GRAY;
	      addQueue(vi,v.second);
	    }
	  }
	}//for all succs
      }//if fwd
      else{
	//reverse traversal
	int t = _pgraphs[v.second]->GetPredVertices(v.first,succs);
	for(int v2id=0;v2id < t;v2id++){
	  if(!_pgraphs[v.second]->IsVertex(succs[v2id],&vi)){
	    //the vertex is not local
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    
	    //but first checked if not touched(GRAY)
	    mit = _color_back[v.second].find(succs[v2id]);
	    if(mit == _color_back[v.second].end()){//if color is WHITE
	      //cout<<"remote vertex"<<endl;
	      Location _n=_pgraphs[v.second]->Lookup(succs[v2id]);
	      stapl_assert(_n.ValidLocation(),
			   "Invalid id for the second vertex of an edge in BFS");
	      _color_back[v.second][succs[v2id]] = GRAY;
	      _StaplTriple<VID,int,VISITOR> _arg(succs[v2id],v.second,*(_f_back[v.second]));
	      stapl::async_rmi( _n.locpid(), 
				getHandle(),
				&BFS_traversal_multi::remoteAddReverse,_arg);
	    }
	  }
	  else{
	    //here we are if the vertex is local
	    //first check the color of the node
	    mit = _color_back[v.second].find(succs[v2id]);
	    if(mit == _color_back[v.second].end()){//if color is WHITE
	      //addQueueFront(vi,v.second);
	      _color_back[v.second][succs[v2id]] = GRAY;
	      addQueueReverse(vi,v.second);
	    }
	  }
	}//for all succs
      }//else
    }//while
    SetInside(false);
    return OK;
  }//end method BFSLoop 

};

/**
 *BFS for a vector of pGraphs and a vector of Visitors; The starting point for BFS is 
 *specified by _startVid; The BFS is applied going trough succesors edges;
*/

template<class PGRAPH, class VISITOR>
int pBFS (vector<PGRAPH*>& _vpg , 
	  vector<VISITOR*> _vvis, 
	  vector<vector<VID> > _startVids) {
  int temp;
  typename PGRAPH::VI vi;
  vector<VISITOR*> empty;

  BFS_traversal_multi<PGRAPH,VISITOR> _bfsf(_vpg , _vvis, empty);
  stapl::rmi_fence();

  for(int i=0;i<_vpg.size();i++){
    for(int j=0;j<_startVids[i].size();j++){
      if(_vpg[i]->IsVertex(_startVids[i][j],&vi)){
	_bfsf.addQueue(vi,i);
	_bfsf._color_fwd[i][vi->vid] = GRAY;
      }
    }
  }

  _bfsf.SetInside(false);
  temp = _bfsf.BFSLoop();
  
  //next line can be moved out if we don't want all the threads to
  //run this call
  stapl::rmi_fence();
  stapl::rmi_fence();
  //temp has to be broadcasted to everybody before return
  return temp;
}

/**
 *pBFSReverse for a vector of pGraphs and a vector of Visitors; 
 *The starting point for BFS is 
 *specified by _startVid; The BFS is applied going trough predecessors edges;
*/
template<class PGRAPH, class VISITOR>
int pBFSReverse (vector<PGRAPH*>& _vpg , 
		 vector<VISITOR*> _vvis, 
		 vector<vector<VID> > _startVids) {
  int temp;
  typename PGRAPH::VI vi;
  vector<VISITOR*> empty;
  
  BFS_traversal_multi<PGRAPH,VISITOR> _bfsf(_vpg , empty, _vvis);
  stapl::rmi_fence();

  for(int i=0;i<_vpg.size();i++){
    for(int j=0;j<_startVids[i].size();j++){
      if(_vpg[i]->IsVertex(_startVids[i][j],&vi)){
	_bfsf.addQueueReverse(vi,i);
	_bfsf._color_back[i][vi->vid] = GRAY;
      }
    }
  }

  _bfsf.SetInside(false);
  temp = _bfsf.BFSLoop();

  stapl::rmi_fence();
  //stapl::rmi_fence();
  return temp;
}

/**
 *pBFSReverse for a vector of pGraphs 
 *The starting point for BFS is 
 *specified by _startVid; The BFS is applied going trough succesors 
 *and applying _vvis_fwd and going trough predecessors and 
 *applying _vvis_back;
*/
template<class PGRAPH, class VISITOR>
int pBFSFwdBack (vector<PGRAPH*>& _vpg , 
		 vector<VISITOR*> _vvis_fwd, 
		 vector<VISITOR*> _vvis_back,
		 vector<vector<VID> > _startVids) {
  int temp;
  typename PGRAPH::VI vi;
  vector<VISITOR*> empty;
  
  BFS_traversal_multi<PGRAPH,VISITOR> _bfsf(_vpg , empty, _vvis);
  stapl::rmi_fence();

  for(int i=0;i<_vpg.size();i++){
    for(int j=0;j<_startVids[i].size();j++){
      if(_vpg[i]->IsVertex(_startVids[i][j],&vi)){
	_bfsf.addQueue(vi,i);
	_bfsf.addQueueReverse(vi,i);
	_bfsf._color_fwd[i][vi->vid] = GRAY;
	_bfsf._color_back[i][vi->vid] = GRAY;
      }
    }
  }

  _bfsf.SetInside(false);
  temp = _bfsf.BFSLoop();
  
  stapl::rmi_fence();
  stapl::rmi_fence();
  return temp;
}

///////////////////////////////////////
//       BFS like traversal (one graph)
///////////////////////////////////////
template <class PG,class VISITOR>
class BFS_traversal_one: public BasePObject{  


  typedef map<VID,int> MAP;

  PG* _pgraphs;
  VISITOR* _f_fwd;
  VISITOR* _f_back;

  //vertex vi , index and direction(fwd,back)
  list<_StaplPair<typename PG::VI,int> > QT;//queue used for trimer traversals
 public:

  bool inside;
  MAP _color_fwd;
  MAP _color_back;

  BFS_traversal_one(){
    SetInside(false);
    register_this(this);
  }

  ~BFS_traversal_one(){
    rmiHandle handle = this->getHandle();
    if (handle > -1)
      stapl::unregister_rmi_object(handle);
  }

  BFS_traversal_one(PG* _pgs, VISITOR* _vvis_fwd,VISITOR* _vvis_back) {
    _pgraphs = _pgs;
    _f_fwd = _vvis_fwd;
    _f_back = _vvis_back;
    SetInside(false);
    register_this(this);
  }

  void SetInside(bool _i){
    inside = _i;
  }

  void addQueue(typename PG::VI vi){
    QT.push_back(_StaplPair<typename PG::VI,int>(vi,STAPL_P_FWD));
  }

  void addQueueFront(typename PG::VI vi){
    QT.push_front(_StaplPair<typename PG::VI,int>(vi,STAPL_P_FWD));
  }

  void addQueueReverse(typename PG::VI vi){
    QT.push_back(_StaplPair<typename PG::VI,int>(vi,STAPL_P_BACK));
  }

  void addQueueFrontReverse(typename PG::VI vi){
    QT.push_front(_StaplPair<typename PG::VI,int>(vi,STAPL_P_BACK));
  }

  _StaplPair<typename PG::VI,int> popQueue(){
    _StaplPair<typename PG::VI,int> temp = QT.front();
    QT.pop_front();
    return temp;
  }
  
  inline int size(){
    return QT.size();
  }

  void remoteAdd(_StaplPair<VID,VISITOR> _arg){
    typename PG::VI vi;
    map<VID, int>::iterator mit;
    
    if(!_pgraphs->IsVertex(_arg.first,&vi)){
      return;
    }
    //here vi is initialized properly;
    //check if it's color is GRAY; if so don't added
    mit = _color_fwd.find(_arg.first);
    if(mit == _color_fwd.end()){//if color is WHITE

      *_f_fwd = _arg.second;

      _color_fwd[vi->vid] = GRAY;
      addQueue(vi);
      if (inside == false) BFSLoop();
    }
  }

  void remoteAddReverse(_StaplPair<VID,VISITOR> _arg){
    typename PG::VI vi;
    map<VID, int>::iterator mit;
    
    if(!_pgraphs->IsVertex(_arg.first,&vi)){
      return;
    }
    //here vi is initialized properly;
    //check if it's color is GRAY; if so don't added
    mit = _color_back.find(_arg.first);
    if(mit == _color_back.end()){//if color is WHITE

      *_f_back = _arg.second;

      _color_back[vi->vid] = GRAY;
      addQueueReverse(vi);
      if (inside == false) BFSLoop();
    }
  }

  int BFSLoop(){
    //sweep to the graph associated with this trimer
    typename PG::EI ei;
    typename PG::VI vi;
    //vi , index, direction
    _StaplPair<typename PG::VI,int> v;
    map<VID, int>::iterator mit;    
    vector<VID> succs;

    SetInside(true);

    while(size() > 0){//while there are more vertices
      //pop a vertex from Q
      v = popQueue();
      if(v.second == STAPL_P_FWD)
	_f_fwd->vertex(v.first);
      else
	_f_back->vertex(v.first);

      //for every child u of v
      if(v.second == STAPL_P_FWD){
	int t = _pgraphs->GetAdjacentVertices(v.first,succs);
	for(int v2id=0;v2id < t;v2id++){
	  if(!_pgraphs->IsVertex(succs[v2id],&vi)){
	    //the vertex is not local
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    
	    //but first checked if not touched(GRAY)
	    mit = _color_fwd.find(succs[v2id]);
	    if(mit == _color_fwd.end()){//if color is WHITE

	      Location _n=_pgraphs->Lookup(succs[v2id]);
	      stapl_assert(_n.ValidLocation(),
			   "Invalid id for the second vertex of an edge in BFS");
	      _color_fwd[succs[v2id]] = GRAY;
	      //cout<<"remote vertex "<<succs[v2id]<<" "<<_n.locpid()<<endl;
	      _StaplPair<VID,VISITOR> _arg(succs[v2id],*_f_fwd);
	      stapl::async_rmi( _n.locpid(), 
				getHandle(),
				&BFS_traversal_one::remoteAdd,_arg);
	    }
	  }
	  else{
	    //here we are if the vertex is local
	    //first check the color of the node
	    mit = _color_fwd.find(succs[v2id]);
	    if(mit == _color_fwd.end()){//if color is WHITE
	      //addQueueFront(vi,v.second);
	      _color_fwd[succs[v2id]] = GRAY;
	      addQueue(vi);
	    }
	  }
	}//for all succs
      }//if fwd
      else{
	int t = _pgraphs->GetPredVertices(v.first,succs);
	for(int v2id=0;v2id < t;v2id++){
	  if(!_pgraphs->IsVertex(succs[v2id],&vi)){
	    //the vertex is not local
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    
	    //but first checked if not touched(GRAY)
	    mit = _color_back.find(succs[v2id]);
	    if(mit == _color_back.end()){//if color is WHITE

	      Location _n=_pgraphs->Lookup(succs[v2id]);
	      stapl_assert(_n.ValidLocation(),
			   "Invalid id for the second vertex of an edge in BFS");
	      _color_back[succs[v2id]] = GRAY;
	      //cout<<"remote vertex "<<succs[v2id]<<" "<<_n.locpid()<<endl;
	      _StaplPair<VID,VISITOR> _arg(succs[v2id],*_f_fwd);
	      stapl::async_rmi( _n.locpid(), 
				getHandle(),
				&BFS_traversal_one::remoteAddReverse,_arg);
	    }
	  }
	  else{
	    //here we are if the vertex is local
	    //first check the color of the node
	    mit = _color_back.find(succs[v2id]);
	    if(mit == _color_back.end()){//if color is WHITE
	      //addQueueFront(vi,v.second);
	      _color_back[succs[v2id]] = GRAY;
	      addQueueReverse(vi);
	    }
	  }
	}//for all succs
      }
    }//while
    SetInside(false);
    return OK;
  }//end method BFSLoop 
};

 template<class PGRAPH, class VISITOR>
  int pBFS(PGRAPH& _vpg , 
	   VISITOR& _vvis, 
	   VID _startVid) {
  int temp;
  int i;
  typename PGRAPH::VI vi;
  VISITOR* empty;
  BFS_traversal_one<PGRAPH,VISITOR> _bfsf(&_vpg , &_vvis, empty);
  stapl::rmi_fence();

  if(_vpg.IsLocal(_startVid) ) {//only one of them start the BFS
    printf("Started  on thread ---------> %d\n",stapl::get_thread_id());
    _vpg.IsVertex(_startVid,&vi);
    _bfsf.addQueue(vi);
    _bfsf._color_fwd[vi->vid] = GRAY;
    _bfsf.SetInside(false);
    temp = _bfsf.BFSLoop();
  }

  stapl::rmi_fence();
  //temp has to be broadcasted to everybody before return
  return temp;
 }

 template<class PGRAPH, class VISITOR>
  int pBFSReverse(PGRAPH& _vpg , 
	   VISITOR& _vvis, 
	   VID _startVid) {
  int temp;
  int i;
  typename PGRAPH::VI vi;
  VISITOR* empty;
  BFS_traversal_one<PGRAPH,VISITOR> _bfsf(&_vpg , empty, &_vvis);
  stapl::rmi_fence();

  if(_vpg.IsLocal(_startVid) ) {//only one of them start the DFS
    printf("One graph BFS  on thread ---------> %d\n",stapl::get_thread_id());
    _vpg.IsVertex(_startVid,&vi);
    _bfsf.addQueueReverse(vi);
    _bfsf._color_back[vi->vid] = GRAY;
    _bfsf.SetInside(false);
    temp = _bfsf.BFSLoop();
  }
  stapl::rmi_fence();
  //temp has to be broadcasted to everybody before return
  return temp;
 }

 template<class PGRAPH, class VISITOR>
  int pBFSFwdBack(PGRAPH& _vpg , 
		  VISITOR& _vvis_fwd,VISITOR& _vvis_back, 
		  VID _startVid) {
  int temp;
  int i;
  typename PGRAPH::VI vi;
  BFS_traversal_one<PGRAPH,VISITOR> _bfsf(&_vpg , &_vvis_fwd, &_vvis_back);
  stapl::rmi_fence();

  if(_vpg.IsLocal(_startVid) ) {//only one of them start the DFS
    printf("Started  on thread ---------> %d\n",stapl::get_thread_id());
    _vpg.IsVertex(_startVid,&vi);
    _bfsf.addQueueReverse(vi,0);
    _bfsf._color_back[vi->vid] = GRAY;
    _bfsf.SetInside(false);
    temp = _bfsf.BFSLoop();
  }
  stapl::rmi_fence();
  //temp has to be broadcasted to everybody before return
  return temp;
}






//////////////////////////////////////////////////////////////////////////////
//                   BFS like traversal (multiple graphs)
//                   Early Quit version
// The BFS early quit traversals will cooperate with the visitor and
// will stop traversing certain areas of the pGraph; For example you can have
// some vertices of the pGraph marked as "dead". With this mechanism you can
// avoid traversing them using BFS. 
//////////////////////////////////////////////////////////////////////////////

template <class PG,class VISITORFWD,class VISITORBACK>
class BFSEQ_traversal_multi: public BasePObject{  
 public:

  typedef map<VID,int> MAP;

  vector<MAP> _color_fwd;
  vector<MAP> _color_back;
  vector<PG*> _pgraphs;
  vector<VISITORFWD*>  _f_fwd;
  vector<VISITORBACK*> _f_back;

  //the next triplet includes vertex vi , index and direction(fwd,back)
  list<_StaplTriple<typename PG::VI,int,int> > QT;//queue used for trimer traversals
  bool inside;

  BFSEQ_traversal_multi(){
    SetInside(false);
    register_this(this);
  }

  ~BFSEQ_traversal_multi(){
    rmiHandle handle = this->getHandle();
    if (handle > -1)
      stapl::unregister_rmi_object(handle);
  }

  BFSEQ_traversal_multi(vector<PG*>& _pgs, 
		      vector<VISITORFWD*>  _vvis_fwd, 
		      vector<VISITORBACK*> _vvis_back) {
    _pgraphs = _pgs;
    _f_fwd = _vvis_fwd;
    _f_back = _vvis_back;
    _color_fwd.resize(_pgs.size());
    _color_back.resize(_pgs.size());
    SetInside(false);
    register_this(this);
  }

  void SetInside(bool _i){
    inside = _i;
  }

  void addQueue(typename PG::VI vi,int index){
    QT.push_back(_StaplTriple<typename PG::VI,int,int>(vi,index,STAPL_P_FWD));
  }

  void addQueueFront(typename PG::VI vi,int index){
    QT.push_front(_StaplTriple<typename PG::VI,int,int>(vi,index,STAPL_P_FWD));
  }

  void addQueueReverse(typename PG::VI vi,int index){
    QT.push_back(_StaplTriple<typename PG::VI,int,int>(vi,index,STAPL_P_BACK));
  }

  void addQueueFrontReverse(typename PG::VI vi,int index){
    QT.push_front(_StaplTriple<typename PG::VI,int,int>(vi,index,STAPL_P_BACK));
  }

  _StaplTriple<typename PG::VI,int,int> popQueue(){
    _StaplTriple<typename PG::VI,int,int> temp = QT.front();
    QT.pop_front();
    return temp;
  }
  
  inline int size(){
    return QT.size();
  }

  void remoteAdd(_StaplTriple<VID,int,VISITORFWD> _arg){
    typename PG::VI vi;
    map<VID, int>::iterator mit;
    //cout<<"remote function called"<<endl;

    if(!_pgraphs[_arg.second]->IsVertex(_arg.first,&vi)){
      return;
    }
    //here vi is initialized properly;
    //check if it's color is GRAY; if so don't added
    mit = _color_fwd[_arg.second].find(_arg.first);

    if(mit == _color_fwd[_arg.second].end()){//if color is WHITE

      //*(_f_fwd[_arg.second]) = _arg.third;

      _color_fwd[_arg.second][vi->vid] = GRAY;
      addQueue(vi,_arg.second);
      if (inside == false) BFSEQLoop();
    }
  }

  void remoteAddReverse(_StaplTriple<VID,int,VISITORBACK> _arg){
    typename PG::VI vi;
    map<VID, int>::iterator mit;
    //cout<<"remote function called"<<endl;

    if(!_pgraphs[_arg.second]->IsVertex(_arg.first,&vi)){
      return;
    }
    //here vi is initialized properly;
    //check if it's color is GRAY; if so don't added
    mit = _color_back[_arg.second].find(_arg.first);

    if(mit == _color_back[_arg.second].end()){//if color is WHITE

      //*(_f_back[_arg.second]) = _arg.third;

      _color_back[_arg.second][vi->vid] = GRAY;
      addQueueReverse(vi,_arg.second);
      if (inside == false) BFSEQLoop();
    }
  }

  int BFSEQLoop(){
    //sweep to the graph associated with this trimer
    typename PG::EI ei;
    typename PG::VI vi;
    //vi , index, direction
    _StaplTriple<typename PG::VI,int,int> v;
    map<VID, int>::iterator mit;    
    vector<VID> succs;

    SetInside(true);

    while(size() > 0){//while there are more vertices
      //pop a vertex from Q
      v = popQueue();

      if(v.third == STAPL_P_FWD){
	if(_f_fwd[v.second]->vertex(v.first) < 0) continue;
      }
      else{
	if(_f_back[v.second]->vertex(v.first) < 0) continue;
      }
      
      //for every child u of v
      if(v.third == STAPL_P_FWD){
	int t = _pgraphs[v.second]->GetAdjacentVertices(v.first,succs);
	//cout<<"Adjacency for "<<v.first->vid<<" "<< t <<" "<<succs.size()<<endl;
	for(int v2id=0;v2id<t;v2id++){
	  if(!_pgraphs[v.second]->IsVertex(succs[v2id],&vi)){
	    //the vertex is not local
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    
	    //but first checked if not touched(GRAY)
	    mit = _color_fwd[v.second].find(succs[v2id]);
	    if(mit == _color_fwd[v.second].end()){//if color is WHITE
	      Location _n=_pgraphs[v.second]->Lookup(succs[v2id]);
	      stapl_assert(_n.ValidLocation(),
			   "Invalid id for the second vertex of an edge in BFSEQ");
	      _color_fwd[v.second][succs[v2id]] = GRAY;
	      _StaplTriple<VID,int,VISITORFWD> _arg(succs[v2id],v.second,*(_f_fwd[v.second]));
	      stapl::async_rmi( _n.locpid(), 
				getHandle(),
				&BFSEQ_traversal_multi::remoteAdd,_arg);
	    }
	  }
	  else{
	    //here we are if the vertex is local
	    //first check the color of the node
	    mit = _color_fwd[v.second].find(succs[v2id]);
	    if(mit == _color_fwd[v.second].end()){//if color is WHITE
	      //addQueueFront(vi,v.second);
	      _color_fwd[v.second][succs[v2id]] = GRAY;
	      addQueue(vi,v.second);
	    }
	  }
	}//for all succs
      }//if fwd
      else{
	//reverse traversal
	int t = _pgraphs[v.second]->GetPredVertices(v.first,succs);
	for(int v2id=0;v2id < t;v2id++){
	  if(!_pgraphs[v.second]->IsVertex(succs[v2id],&vi)){
	    //the vertex is not local
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    
	    //but first checked if not touched(GRAY)
	    mit = _color_back[v.second].find(succs[v2id]);
	    if(mit == _color_back[v.second].end()){//if color is WHITE
	      Location _n=_pgraphs[v.second]->Lookup(succs[v2id]);
	      stapl_assert(_n.ValidLocation(),
			   "Invalid id for the second vertex of an edge in BFSEQ");
	      _color_back[v.second][succs[v2id]] = GRAY;
	      _StaplTriple<VID,int,VISITORBACK> _arg(succs[v2id],v.second,*(_f_back[v.second]));
	      stapl::async_rmi( _n.locpid(), 
				getHandle(),
				&BFSEQ_traversal_multi::remoteAddReverse,_arg);
	    }
	  }
	  else{
	    //here we are if the vertex is local
	    //first check the color of the node
	    mit = _color_back[v.second].find(succs[v2id]);
	    if(mit == _color_back[v.second].end()){//if color is WHITE
	      //addQueueFront(vi,v.second);
	      _color_back[v.second][succs[v2id]] = GRAY;
	      addQueueReverse(vi,v.second);
	    }
	  }
	}//for all succs
      }//else
    }//while
    SetInside(false);
    return OK;
  }//end method BFSEQLoop 

};

/**
 *BFSEQ for a vector of pGraphs and a vector of Visitors; The starting point for BFSEQ is 
 *specified by _startVid; The BFSEQ is applied going trough succesors edges;
*/

template<class PGRAPH, class VISITOR>
int pBFSEQ (vector<PGRAPH*>& _vpg , 
	  vector<VISITOR*> _vvis, 
	  vector<vector<VID> >_startVids) {
  int temp;
  typename PGRAPH::VI vi;
  vector<VISITOR*> empty;

  BFSEQ_traversal_multi<PGRAPH,VISITOR,VISITOR> _bfsf(_vpg , _vvis, empty);
  stapl::rmi_fence();

  for(int i=0;i<_vpg.size();i++){
    for(int j=0;j<_startVids[i].size();j++){
      if(_vpg[i]->IsVertex(_startVids[i][j],&vi)){
	_bfsf.addQueue(vi,i);
	_bfsf._color_fwd[i][vi->vid] = GRAY;
      }
    }
  }

  _bfsf.SetInside(false);
  temp = _bfsf.BFSEQLoop();

  //next line can be moved out if we don't want all the threads to
  //run this call
  stapl::rmi_fence();
  stapl::rmi_fence();
  //temp has to be broadcasted to everybody before return
  return temp;
}

/**
 *pBFSEQReverse for a vector of pGraphs and a vector of Visitors; 
 *The starting point for BFSEQ is 
 *specified by _startVid; The BFSEQ is applied going trough predecessors edges;
*/
template<class PGRAPH, class VISITOR>
int pBFSEQReverse (vector<PGRAPH*>& _vpg , 
		   vector<VISITOR*> _vvis, 
		   vector<vector<VID> > _startVids) {
  int temp;
  typename PGRAPH::VI vi;
  vector<VISITOR*> empty;
  
  BFSEQ_traversal_multi<PGRAPH,VISITOR,VISITOR> _bfsf(_vpg , empty, _vvis);
  stapl::rmi_fence();

  for(int i=0;i<_vpg.size();i++){
    for(int j=0;j<_startVids[i].size();j++){
      if(_vpg[i]->IsVertex(_startVids[i][j],&vi)){
	_bfsf.addQueueReverse(vi,i);
	_bfsf._color_back[i][vi->vid] = GRAY;
      }
    }
  }

  _bfsf.SetInside(false);
  temp = _bfsf.BFSEQLoop();

  stapl::rmi_fence();
  //stapl::rmi_fence();
  return temp;
}

/**
 *pBFSEQReverse for a vector of pGraphs 
 *The starting point for BFSEQ is 
 *specified by _startVid; The BFSEQ is applied going trough succesors 
 *and applying _vvis_fwd and going trough predecessors and 
 *applying _vvis_back;
*/
template<class PGRAPH, class VISITORFWD,class VISITORBACK>
int pBFSEQFwdBack (vector<PGRAPH*>& _vpg , 
		 vector<VISITORFWD*> _vvis_fwd, 
		 vector<VISITORBACK*> _vvis_back,
		 vector<vector<VID> >_startVids) {
  int temp;
  typename PGRAPH::VI vi;
  
  BFSEQ_traversal_multi<PGRAPH,VISITORFWD,VISITORBACK> _bfsf(_vpg , _vvis_fwd, _vvis_back);
  stapl::rmi_fence();

  for(int i=0;i<_vpg.size();i++){
    for(int j=0;j<_startVids[i].size();j++){
      if(_vpg[i]->IsVertex(_startVids[i][j],&vi)){
	_bfsf.addQueue(vi,i);
	_bfsf.addQueueReverse(vi,i);
	_bfsf._color_fwd[i][vi->vid] = GRAY;
	_bfsf._color_back[i][vi->vid] = GRAY;
      }
    }
  }

  _bfsf.SetInside(false);
  temp = _bfsf.BFSEQLoop();

  stapl::rmi_fence();
  stapl::rmi_fence();
  return temp;
}

///////////////////////////////////////
//       BFSEQ like traversal (one graph)
///////////////////////////////////////
template <class PG,class VISITOR>
class BFSEQ_traversal_one: public BasePObject{  


  typedef map<VID,int> MAP;

  PG* _pgraphs;
  VISITOR* _f_fwd;
  VISITOR* _f_back;

  //vertex vi , index and direction(fwd,back)
  list<_StaplPair<typename PG::VI,int> > QT;//queue used for trimer traversals
 public:

  bool inside;
  MAP _color_fwd;
  MAP _color_back;

  BFSEQ_traversal_one(){
    SetInside(false);
    register_this(this);
  }

  ~BFSEQ_traversal_one(){
    rmiHandle handle = this->getHandle();
    if (this->handle > -1)
      stapl::unregister_rmi_object(handle);
  }

  BFSEQ_traversal_one(PG* _pgs, VISITOR* _vvis_fwd,VISITOR* _vvis_back) {
    _pgraphs = _pgs;
    _f_fwd = _vvis_fwd;
    _f_back = _vvis_back;
    SetInside(false);
    register_this(this);
  }

  void SetInside(bool _i){
    inside = _i;
  }

  void addQueue(typename PG::VI vi){
    QT.push_back(_StaplPair<typename PG::VI,int>(vi,STAPL_P_FWD));
  }

  void addQueueFront(typename PG::VI vi){
    QT.push_front(_StaplPair<typename PG::VI,int>(vi,STAPL_P_FWD));
  }

  void addQueueReverse(typename PG::VI vi){
    QT.push_back(_StaplPair<typename PG::VI,int>(vi,STAPL_P_BACK));
  }

  void addQueueFrontReverse(typename PG::VI vi){
    QT.push_front(_StaplPair<typename PG::VI,int>(vi,STAPL_P_BACK));
  }

  _StaplPair<typename PG::VI,int> popQueue(){
    _StaplPair<typename PG::VI,int> temp = QT.front();
    QT.pop_front();
    return temp;
  }
  
  inline int size(){
    return QT.size();
  }

  void remoteAdd(_StaplPair<VID,VISITOR> _arg){
    typename PG::VI vi;
    map<VID, int>::iterator mit;
    
    if(!_pgraphs->IsVertex(_arg.first,&vi)){
      return;
    }
    //here vi is initialized properly;
    //check if it's color is GRAY; if so don't added
    mit = _color_fwd.find(_arg.first);
    if(mit == _color_fwd.end()){//if color is WHITE

      *_f_fwd = _arg.second;

      _color_fwd[vi->vid] = GRAY;
      addQueue(vi);
      if (inside == false) BFSEQLoop();
    }
  }

  void remoteAddReverse(_StaplPair<VID,VISITOR> _arg){
    typename PG::VI vi;
    map<VID, int>::iterator mit;
    
    if(!_pgraphs->IsVertex(_arg.first,&vi)){
      return;
    }
    //here vi is initialized properly;
    //check if it's color is GRAY; if so don't added
    mit = _color_back.find(_arg.first);
    if(mit == _color_back.end()){//if color is WHITE

      *_f_back = _arg.second;

      _color_back[vi->vid] = GRAY;
      addQueueReverse(vi);
      if (inside == false) BFSEQLoop();
    }
  }

  int BFSEQLoop(){
    //sweep to the graph associated with this trimer
    typename PG::EI ei;
    typename PG::VI vi;
    //vi , index, direction
    _StaplPair<typename PG::VI,int> v;
    map<VID, int>::iterator mit;    
    vector<VID> succs;

    SetInside(true);

    while(size() > 0){//while there are more vertices
      //pop a vertex from Q
      v = popQueue();

      if(v.second == STAPL_P_FWD){
	if(_f_fwd->vertex(v.first) < 0) continue;
      }
      else{
	if(_f_back->vertex(v.first) < 0) continue;
      }

      //for every child u of v
      if(v.second == STAPL_P_FWD){
	int t = _pgraphs->GetAdjacentVertices(v.first,succs);
	//cout<<"Adjacency for "<<v.first->vid<<" "<< t <<" "<<succs.size()<<endl;
	for(int v2id=0;v2id < t;v2id++){
	  if(!_pgraphs->IsVertex(succs[v2id],&vi)){
	    //the vertex is not local
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    
	    //but first checked if not touched(GRAY)
	    mit = _color_fwd.find(succs[v2id]);
	    if(mit == _color_fwd.end()){//if color is WHITE

	      Location _n=_pgraphs->Lookup(succs[v2id]);
	      stapl_assert(_n.ValidLocation(),
			   "Invalid id for the second vertex of an edge in BFSEQ");
	      _color_fwd[succs[v2id]] = GRAY;
	      //cout<<"remote vertex "<<succs[v2id]<<" "<<_n.locpid()<<endl;
	      _StaplPair<VID,VISITOR> _arg(succs[v2id],*_f_fwd);
	      stapl::async_rmi( _n.locpid(), 
				getHandle(),
				&BFSEQ_traversal_one::remoteAdd,_arg);
	    }
	  }
	  else{
	    //here we are if the vertex is local
	    //first check the color of the node
	    mit = _color_fwd.find(succs[v2id]);
	    if(mit == _color_fwd.end()){//if color is WHITE
	      //addQueueFront(vi,v.second);
	      _color_fwd[succs[v2id]] = GRAY;
	      addQueue(vi);
	    }
	  }
	}//for all succs
      }//if fwd
      else{
	int t = _pgraphs->GetPredVertices(v.first,succs);
	//cout<<"Preds for "<<v.first->vid<<" "<< t <<" "<<succs.size()<<endl;
	for(int v2id=0;v2id<t;v2id++){
	  if(!_pgraphs->IsVertex(succs[v2id],&vi)){
	    //the vertex is not local
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    
	    //but first checked if not touched(GRAY)
	    mit = _color_back.find(succs[v2id]);
	    if(mit == _color_back.end()){//if color is WHITE

	      Location _n=_pgraphs->Lookup(succs[v2id]);
	      stapl_assert(_n.ValidLocation(),
			   "Invalid id for the second vertex of an edge in BFSEQ");
	      _color_back[succs[v2id]] = GRAY;
	      //cout<<"remote vertex "<<succs[v2id]<<" "<<_n.locpid()<<endl;
	      _StaplPair<VID,VISITOR> _arg(succs[v2id],*_f_fwd);
	      stapl::async_rmi( _n.locpid(), 
				getHandle(),
				&BFSEQ_traversal_one::remoteAddReverse,_arg);
	    }
	  }
	  else{
	    //here we are if the vertex is local
	    //first check the color of the node
	    mit = _color_back.find(succs[v2id]);
	    if(mit == _color_back.end()){//if color is WHITE
	      _color_back[succs[v2id]] = GRAY;
	      addQueueReverse(vi);
	    }
	  }
	}//for all succs
      }
    }//while
    SetInside(false);
    return OK;
  }//end method BFSEQLoop 
};

 template<class PGRAPH, class VISITOR>
  int pBFSEQ(PGRAPH& _vpg , 
	   VISITOR& _vvis, 
	   VID _startVid) {
  int temp;
  int i;
  typename PGRAPH::VI vi;
  VISITOR* empty;
  BFSEQ_traversal_one<PGRAPH,VISITOR> _bfsf(&_vpg , &_vvis, empty);
  stapl::rmi_fence();

  if(_vpg.IsLocal(_startVid) ) {//only one of them start the DFS
    //printf("Started  on thread ---------> %d\n",stapl::get_thread_id());
    _vpg.IsVertex(_startVid,&vi);
    _bfsf.addQueue(vi);
    _bfsf._color_fwd[vi->vid] = GRAY;
    _bfsf.SetInside(false);
    temp = _bfsf.BFSEQLoop();
  }

  stapl::rmi_fence();
  //temp has to be broadcasted to everybody before return
  return temp;
 }

 template<class PGRAPH, class VISITOR>
  int pBFSEQReverse(PGRAPH& _vpg , 
	   VISITOR& _vvis, 
	   VID _startVid) {
  int temp;
  int i;
  typename PGRAPH::VI vi;
  VISITOR* empty;
  BFSEQ_traversal_one<PGRAPH,VISITOR> _bfsf(&_vpg , empty, &_vvis);
  stapl::rmi_fence();

  if(_vpg.IsLocal(_startVid) ) {//only one of them start the DFS
    //printf("One graph BFSEQ  on thread ---------> %d\n",stapl::get_thread_id());
    _vpg.IsVertex(_startVid,&vi);
    _bfsf.addQueueReverse(vi);
    _bfsf._color_back[vi->vid] = GRAY;
    _bfsf.SetInside(false);
    temp = _bfsf.BFSEQLoop();
  }
  stapl::rmi_fence();
  //temp has to be broadcasted to everybody before return
  return temp;
 }

 template<class PGRAPH, class VISITOR>
  int pBFSEQFwdBack(PGRAPH& _vpg , 
		  VISITOR& _vvis_fwd,VISITOR& _vvis_back, 
		  VID _startVid) {
  int temp;
  int i;
  typename PGRAPH::VI vi;
  BFSEQ_traversal_one<PGRAPH,VISITOR> _bfsf(&_vpg , &_vvis_fwd, &_vvis_back);
  stapl::rmi_fence();

  if(_vpg.IsLocal(_startVid) ) {//only one of them start the DFS
    //printf("Started  on thread ---------> %d\n",stapl::get_thread_id());
    _vpg.IsVertex(_startVid,&vi);
    _bfsf.addQueueReverse(vi,0);
    _bfsf._color_back[vi->vid] = GRAY;
    _bfsf.SetInside(false);
    temp = _bfsf.BFSEQLoop();
  }
  stapl::rmi_fence();
  //temp has to be broadcasted to everybody before return
  return temp;
}


 //////////////////////////////////////////////////////////////////////////
 //                    TOPOLOGICAL traversal
 //
 //////////////////////////////////////////////////////////////////////////

template <class PG,class VISITORFWD,class VISITORBACK>
class TOPO_traversal_multi: public BasePObject{  
 public:

  typedef map<VID,int> MAP;

  vector<MAP> _color_fwd;
  vector<MAP> _color_back;
  vector<PG*> _pgraphs;
  vector<VISITORFWD*> _f_fwd;
  vector<VISITORBACK*> _f_back;

  //the next triplet includes vertex vi , index and direction(fwd,back)
  list<_StaplTriple<typename PG::VI,int,int> > QT;//queue used for trimer traversals
  bool inside;

  TOPO_traversal_multi(){
    SetInside(false);
    register_this(this);
  }

  ~TOPO_traversal_multi(){
    rmiHandle handle = this->getHandle();
    if (handle > -1)
      stapl::unregister_rmi_object(handle);
  }

  TOPO_traversal_multi(vector<PG*>& _pgs, 
		      vector<VISITORFWD*> _vvis_fwd, 
		      vector<VISITORBACK*> _vvis_back) {
    _pgraphs = _pgs;
    _f_fwd = _vvis_fwd;
    _f_back = _vvis_back;
    _color_fwd.resize(_pgs.size());
    _color_back.resize(_pgs.size());
    bool fwd = (_vvis_fwd.size() != 0);
    bool back = (_vvis_back.size() != 0);

    //initialize the color map with in/out degree
    typename PG::VI vi;
    for(int i=0;i<_pgs.size();i++){
      for(vi=_pgs[i]->local_begin();vi != _pgs[i]->local_end();vi++){
	if(fwd)  {
	  _color_fwd[i][vi->vid]  = vi->predecessors.size();
	  if (vi->predecessors.size() == 0)
	    addQueue(vi,i);
	}
	if(back) {
	  _color_back[i][vi->vid] = vi->edgelist.size();
	  if (vi->edgelist.size() == 0)
	    addQueueReverse(vi,i);
	}
      }
    }
    SetInside(false);
    register_this(this);
  }

  void SetInside(bool _i){
    inside = _i;
  }

  void addQueue(typename PG::VI vi,int index){
    QT.push_back(_StaplTriple<typename PG::VI,int,int>(vi,index,STAPL_P_FWD));
  }

  void addQueueFront(typename PG::VI vi,int index){
    QT.push_front(_StaplTriple<typename PG::VI,int,int>(vi,index,STAPL_P_FWD));
  }

  void addQueueReverse(typename PG::VI vi,int index){
    QT.push_back(_StaplTriple<typename PG::VI,int,int>(vi,index,STAPL_P_BACK));
  }

  void addQueueFrontReverse(typename PG::VI vi,int index){
    QT.push_front(_StaplTriple<typename PG::VI,int,int>(vi,index,STAPL_P_BACK));
  }

  _StaplTriple<typename PG::VI,int,int> popQueue(){
    _StaplTriple<typename PG::VI,int,int> temp = QT.front();
    QT.pop_front();
    return temp;
  }
  
  inline int size(){
    return QT.size();
  }

  void remoteAdd(_StaplTriple<VID,int,VISITORFWD> _arg){
    typename PG::VI vi;
    map<VID, int>::iterator mit;
    //cout<<"remote function called"<<endl;

    if(!_pgraphs[_arg.second]->IsVertex(_arg.first,&vi)){
      return;
    }
    //here vi is initialized properly;
    //check if it's color is GRAY; if so don't added
    mit = _color_fwd[_arg.second].find(_arg.first);
    if(mit == _color_fwd[_arg.second].end())//if color is WHITE
      cout<<"ERROR: Inside topological traversal: RAR uninitialized vertex"<<endl;
    int _temp = mit->second;
    if(_temp == 0){
      cout<<"Warning in topo traversal RAR: while decrementing the in count"<<endl;
      return;
    }
    _temp = _temp - 1;
    mit->second = _temp;
    if(_temp == 0){//all incoming dependencies are satisfied
      *(_f_fwd[_arg.second]) = _arg.third;
      addQueue(vi,_arg.second);
      if (inside == false) TOPOLoop();
    }
  }

  void remoteAddReverse(_StaplTriple<VID,int,VISITORBACK> _arg){
    typename PG::VI vi;
    map<VID, int>::iterator mit;
    //cout<<"remote function called"<<endl;

    if(!_pgraphs[_arg.second]->IsVertex(_arg.first,&vi)){
      return;
    }
    //here vi is initialized properly;
    //check if it's color is GRAY; if so don't added
    mit = _color_back[_arg.second].find(_arg.first);

    if(mit == _color_back[_arg.second].end())//if color is WHITE
      cout<<"ERROR: Inside topological traversal: RAR uninitialized vertex"<<endl;
    int _temp = mit->second;
    if(_temp == 0){
      cout<<"Warning in topo traversal RAR: while decrementing the in count"<<endl;
      return;
    }
    _temp = _temp - 1;
    mit->second = _temp;
    if(_temp == 0){//all incoming dependencies are satisfied
      *(_f_back[_arg.second]) = _arg.third;
      addQueueReverse(vi,_arg.second);
      if (inside == false) TOPOLoop();
    }
  }

  int TOPOLoop(){
    //sweep to the graph associated with this trimer
    typename PG::EI ei;
    typename PG::VI vi;
    //vi , index, direction
    _StaplTriple<typename PG::VI,int,int> v;
    map<VID, int>::iterator mit;    
    vector<VID> succs;

    SetInside(true);

    while(size() > 0){//while there are more vertices
      //pop a vertex from Q
      v = popQueue();

      if(v.third == STAPL_P_FWD)
	_f_fwd[v.second]->vertex(v.first);
      else
	_f_back[v.second]->vertex(v.first);
      
      //for every child u of v
      if(v.third == STAPL_P_FWD){
	int t = _pgraphs[v.second]->GetAdjacentVertices(v.first,succs);
	//cout<<"Adjacency for "<<v.first->vid<<" "<< t <<" "<<succs.size()<<endl;
	for(int v2id=0;v2id<t;v2id++){
	  if(!_pgraphs[v.second]->IsVertex(succs[v2id],&vi)){
	    //the vertex is not local
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    //cout<<"remote vertex"<<endl;
	    Location _n=_pgraphs[v.second]->Lookup(succs[v2id]);
	    stapl_assert(_n.ValidLocation(),
			 "Invalid id for the second vertex of an edge in TOPO");
	    _StaplTriple<VID,int,VISITORFWD> _arg(succs[v2id],v.second,*(_f_fwd[v.second]));
	    stapl::async_rmi( _n.locpid(), 
			      getHandle(),
			      &TOPO_traversal_multi::remoteAdd,_arg);
       	  }
	  else{
	    //here we are if the vertex is local
	    //first check the color of the node
	    mit = _color_fwd[v.second].find(succs[v2id]);
	    if(mit == _color_fwd[v.second].end())
	      cout<<"ERROR: Inside topological traversal: uninitialized vertex"<<endl;
	    int _temp = mit->second;
	    if(_temp == 0){
	      cout<<"Warning in topo traversal while decrementing the in count"<<endl;
	      continue;
	    }
	    _temp = _temp - 1;
	    mit->second = _temp;
	    if(_temp == 0){//all incoming dependencies are satisfied
	      addQueue(vi,v.second);
	    }
	  }
	}//for all succs
      }//if fwd
      else{
	//reverse traversal
	int t = _pgraphs[v.second]->GetPredVertices(v.first,succs);
	//cout<<"Preds for "<<v.first->vid<<" "<< t <<" "<<succs.size()<<endl;
	for(int v2id=0;v2id<t;v2id++){
	  if(!_pgraphs[v.second]->IsVertex(succs[v2id],&vi)){
	    //the vertex is not local
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    //cout<<"remote vertex"<<endl;
	    Location _n=_pgraphs[v.second]->Lookup(succs[v2id]);
	    stapl_assert(_n.ValidLocation(),
			 "Invalid id for the second vertex of an edge in TOPO");
	    _StaplTriple<VID,int,VISITORBACK> _arg(succs[v2id],v.second,*(_f_back[v.second]));
	    stapl::async_rmi( _n.locpid(), 
			      getHandle(),
			      &TOPO_traversal_multi::remoteAddReverse,_arg);
       	  }
	  else{
	    //here we are if the vertex is local
	    //first check the color of the node
	    mit = _color_back[v.second].find(succs[v2id]);
	    if(mit == _color_back[v.second].end())
	      cout<<"ERROR: Inside topological traversal: uninitialized vertex"<<endl;
	    int _temp = mit->second;
	    if(_temp == 0){
	      cout<<"Warning in topo traversal while decrementing the in count"<<endl;
	      continue;
	    }
	    _temp = _temp - 1;
	    mit->second = _temp;
	    if(_temp == 0){//all incoming dependencies are satisfied
	      addQueueReverse(vi,v.second);
	    }
	  }
	}
      }//else
    }//while
    SetInside(false);
    return OK;
  }//end method TOPOLoop 

};

/**
 *TOPO for a vector of pGraphs and a vector of Visitors; The starting point for TOPO is 
 *specified by _startVid; The TOPO is applied going trough succesors edges;
*/

template<class PGRAPH, class VISITOR>
int pTOPO (vector<PGRAPH*>& _vpg , 
	  vector<VISITOR*> _vvis) {
  int temp;
  typename PGRAPH::VI vi;
  vector<VISITOR*> empty;

  TOPO_traversal_multi<PGRAPH,VISITOR,VISITOR> _bfsf(_vpg , _vvis, empty);
  stapl::rmi_fence();

  _bfsf.SetInside(false);
  temp = _bfsf.TOPOLoop();
    
  //next line can be moved out if we don't want all the threads to
  //run this call
  stapl::rmi_fence();
  stapl::rmi_fence();
  //temp has to be broadcasted to everybody before return
  return temp;
}

/**
 *pTOPOReverse for a vector of pGraphs and a vector of Visitors; 
 *The starting point for TOPO is 
 *specified by _startVid; The TOPO is applied going trough predecessors edges;
*/
template<class PGRAPH, class VISITOR>
int pTOPOReverse (vector<PGRAPH*>& _vpg , 
		 vector<VISITOR*> _vvis) {
  int temp;
  typename PGRAPH::VI vi;
  vector<VISITOR*> empty;
  
  TOPO_traversal_multi<PGRAPH,VISITOR,VISITOR> _bfsf(_vpg , empty, _vvis);
  stapl::rmi_fence();

   _bfsf.SetInside(false);
  temp = _bfsf.TOPOLoop();
    
  stapl::rmi_fence();
  //stapl::rmi_fence();
  return temp;
}

/**
 *pTOPOReverse for a vector of pGraphs 
 *The starting point for TOPO is 
 *specified by _startVid; The TOPO is applied going trough succesors 
 *and applying _vvis_fwd and going trough predecessors and 
 *applying _vvis_back;
*/
template<class PGRAPH, class VISITORFWD, class VISITORBACK>
int pTOPOFwdBack (vector<PGRAPH*>& _vpg , 
		 vector<VISITORFWD*> _vvis_fwd, 
		 vector<VISITORBACK*> _vvis_back) {
  int temp;
  typename PGRAPH::VI vi;
  
  TOPO_traversal_multi<PGRAPH,VISITORFWD,VISITORBACK> _bfsf(_vpg , _vvis_fwd, _vvis_back);
  stapl::rmi_fence();

  _bfsf.SetInside(false);
  temp = _bfsf.TOPOLoop();
  
  stapl::rmi_fence();
  stapl::rmi_fence();
  return temp;
}



template<class PGRAPH, class VISITOR>
int p_SimpleTraversal(PGRAPH& _pg , 
		   VISITOR& _vis) {
  int i;
  for(typename PGRAPH::VI vi = _pg.local_begin();vi!=_pg.local_end();vi++){
    _vis.vertex(vi);
    for(typename PGRAPH::EI ei = vi->edgelist.begin();ei != vi->edgelist.end();ei++){
      _vis.tree_edge(vi,ei);
    }
  }  
  return OK;
}



//-----------------------------------------------------------
//      Connected components algorithms
//-----------------------------------------------------------

//------ IsSameCC
 class pIsSameCC; 
 
 template <class PGRAPH>
   class _visitorpIsSameCC: public visitor_base<PGRAPH>{
    stapl::rmiHandle Handle;//the handle of the pIsSameCC object
    VID v2id;
    PID procid;
   public:  
   _visitorpIsSameCC(stapl::rmiHandle _h, VID _v2){
     v2id = _v2;
     Handle = _h;
     procid = stapl::get_thread_id();
   }

   inline int vertex(typename PGRAPH::VI v) {
     if(v->vid == v2id) {
       //here send an async to the thread that initiated the search
       stapl::sync_rmi( procid, 
			Handle,
			&pIsSameCC::SetResult,true);
       return EARLY_QUIT;
     }
     else return 1;
   }

   void define_type(stapl::typer &t) {
     t.local(Handle);
     t.local(v2id);
     t.local(procid);
   }
 };
 
 class pIsSameCC : public BasePObject{
   bool result;
 public:
   pIsSameCC(){
     //register the object
     this->register_this(this);
     stapl::rmi_fence();
   }
   ~pIsSameCC(){
     //unregister the object
     stapl::unregister_rmi_object( Handle );
   }
   bool SetResult(bool _r){
     result = _r;
     return true;
   }
   bool GetResult(){
     return result;
   }
   template <class PGRAPH>
   bool IsSameCC(PGRAPH& pg, VID _vid1,VID _vid2){
     int i;
     int myid = stapl::get_thread_id();
     result = false;
     //reset the data structure that store the colors
     //this is used to color the graph while we perform DFS
     //we have to do this because tehre can be multiple calls 
     //of this method
     for(i=0;i<stapl::get_num_threads();++i){
       if(i == myid){
	 pg.ResetTraversal(myid);
       }
       else stapl::sync_rmi( i, 
			     pg.getHandle(),
			     &PGRAPH::ResetTraversal, myid);
     }
     //now we have to start the DFS traversal
     _visitorpIsSameCC<PGRAPH> _vis(this->getHandle(),_vid2);
     if(pg.IsLocal(_vid1))
       pg.DFS_traversal(_vid1,_vis,myid,false);
     return result;
   }

   template <class PGRAPH>
   bool IsSameCCData(PGRAPH& pg, 
		     typename PGRAPH::VERTEX _v1,
		     typename PGRAPH::VERTEX _v2){
     VID vid1 = pg.GetVID(_v1);
     VID vid2 = pg.GetVID(_v2);
     return IsSameCC(pg,vid1,vid2);
   }
 };


 //------ GetCC
 template <class PGRAPH>
 class pGetCC; 

 template <class PGRAPH>
   class _visitorpGetCC: public visitor_base<PGRAPH>{
    stapl::rmiHandle Handle;//the handle of the pGetCC object
    PID procid;
   public:  
   _visitorpGetCC(stapl::rmiHandle _h){
     Handle = _h;
     procid = stapl::get_thread_id();
   }

   inline int vertex(typename PGRAPH::VI v) {
     //here send an async to the thread that initiated the search
     stapl::sync_rmi( procid, 
		      Handle,
		      &pGetCC<PGRAPH>::AddVertex,v->vid);
     return 1;
   }

   void define_type(stapl::typer &t) {
     t.local(Handle);
     t.local(procid);
   }
 };
 
 template <class PGRAPH>
   class _visitorpGetCCData: public visitor_base<PGRAPH>{
    stapl::rmiHandle Handle;//the handle of the pGetCC object
    PID procid;
   public:  
   _visitorpGetCCData(stapl::rmiHandle _h){
     Handle = _h;
     procid = stapl::get_thread_id();
   }

   inline int vertex(typename PGRAPH::VI v) {
     //here send an async to the thread that initiated the search
     stapl::sync_rmi( procid, 
		      Handle,
		      &pGetCC<PGRAPH>::AddVertexData,v->data);
     return 1;
   }

   void define_type(stapl::typer &t) {
     t.local(Handle);
     t.local(procid);
   }
 };

 template <class PGRAPH>
 class pGetCC : public BasePObject{
   vector<VID>* result;
   vector<typename PGRAPH::VERTEX>* resultData;
 public:
   pGetCC(){
     //register the object
     this->register_this(this);
     stapl::rmi_fence();
   }
   ~pGetCC(){
     //unregister the object
     stapl::unregister_rmi_object( Handle );
   }

   bool AddVertex(VID _vid){
     result->push_back(_vid);
     return true;
   }

   bool AddVertexData(typename PGRAPH::VERTEX _v){
     resultData->push_back(_v);
     return true;
   }


   int GetCCNoReset(PGRAPH& pg, VID _vid1,vector<VID>& _CC){
     int i;
     int myid = stapl::get_thread_id();
     result = &_CC;
     result->clear();
     //now we have to start the DFS traversal
     _visitorpGetCC<PGRAPH> _vis(this->getHandle());
     if(pg.IsLocal(_vid1))
       pg.DFS_traversal(_vid1,_vis,0,false);
     return result->size();
   }


   int GetCC(PGRAPH& pg, VID _vid1,vector<VID>& _CC){
     int i;
     int myid = stapl::get_thread_id();
     result = &_CC;
     result->clear();
     //reset the data structure that store the colors
     //this is used to color the graph while we perform DFS
     //we have to do this because tehre can be multiple calls 
     //of this method
     for(i=0;i<stapl::get_num_threads();++i){
       if(i == myid){
	 pg.ResetTraversal(myid);
       }
       else stapl::sync_rmi( i, 
			     pg.getHandle(),
			     &PGRAPH::ResetTraversal, myid);
     }
     //now we have to start the DFS traversal
     _visitorpGetCC<PGRAPH> _vis(this->getHandle());
     if(pg.IsLocal(_vid1))
       pg.DFS_traversal(_vid1,_vis,myid,false);
     return result->size();
   }

   int GetCC(PGRAPH& pg, 
	     typename PGRAPH::VERTEX _v1,
	     vector<typename PGRAPH::VERTEX>& _CC){
     int i;
     int myid = stapl::get_thread_id();
     resultData = &_CC;
     resultData->clear();
     VID _vid1 = pg.GetVID(_v1);
     //reset the data structure that store the colors
     //this is used to color the graph while we perform DFS
     //we have to do this because tehre can be multiple calls 
     //of this method
     for(i=0;i<stapl::get_num_threads();++i){
       if(i == myid){
	 pg.ResetTraversal(myid);
       }
       else stapl::sync_rmi( i, 
			     pg.getHandle(),
			     &PGRAPH::ResetTraversal, myid);
     }
     //now we have to start the DFS traversal
     _visitorpGetCCData<PGRAPH> _vis(this->getHandle());
     if(pg.IsLocal(_vid1))
       pg.DFS_traversal(_vid1,_vis,myid,false);
     return result->size();
   }

 };
 //--------------------------------------------
 //GetCCCount
 //--------------------------------------------
 template <class PGRAPH>
   class _visitorpGetCCcount: public visitor_base<PGRAPH>{
   public:  
   _visitorpGetCCcount(){
   }
   inline int vertex(typename PGRAPH::VI v) {
     return 1;
   }
   void define_type(stapl::typer &t) {
   }
 };

 template <class PGRAPH>
 class pGetCCcount : public BasePObject{
  int token;
  int myid;
public:

  pGetCCcount(){
    this->register_this(this);
    myid = stapl::get_thread_id();
    stapl::rmi_fence();
  }

  ~pGetCCcount(){
    stapl::unregister_rmi_object( Handle );
  }

  int SetTT(int _tt){
    //printf("REmote set tt %d\n",_tt);
    tt = _tt;
    return tt;
  }
  
  int GetTT(){
    return tt;
  }

  void SetToken(){
    printf("##########%d\n",stapl::get_thread_id());
    token    = STAPL_SET_TOKEN;
  }

  int UnsetToken(){
    int temp = token; 
    token    = STAPL_UNSET_TOKEN;
    return   temp;
  }
  int GetToken(){
    return token;
  }
  void sum(int* in, int* inout) { *inout += *in; }

  int GetCCcount(PGRAPH& _pg){
    int lcccount=0;
    int result = 0;
    STAPL_GRAPH_COLOR color;
    
    if(myid == 0){
      //printf("Set\n");
      SetToken();    
    }
    else {
      //printf("Unset\n");
      UnsetToken();
    }
    _pg.ResetTraversal(0);
    stapl::rmi_fence(); 
    //if it is not your turn wait
    while(GetToken() == STAPL_UNSET_TOKEN)
      stapl::rmi_wait();
    
    //here I have the token
    typename PGRAPH::VI pend = _pg.local_end();
    for(typename PGRAPH::VI vi = _pg.local_begin(); vi != pend; ++vi) {
      color = vi->GetColorIn(0);
      if(color == STAPL_WHITE){
	//printf("Started  on thread ---------> %d\n",stapl::get_thread_id());
	lcccount++;
	_visitorpGetCCcount<PGRAPH> _vis;
	_pg.DFS_traversal(vi->vid,_vis,0,false);
      }
    }
    //here all local traversals are finished
    // reset the token 
    //pass the token to the next processor and also the current termination time;
    if(myid + 1 < stapl::get_num_threads()){
      stapl::async_rmi( myid+1, 
			this->getHandle(),
			&pGetCCcount<PGRAPH>::SetToken);
    }
    stapl::rmi_fence();
    //now perform a reduce on the ccount;
    stapl::reduce_rmi( &lcccount, &result, this->getHandle(), &pGetCCcount<PGRAPH>::sum, true);
    return result;
  }//end GetCCcount method
 };//end pGetCCcount class

 //--------------------------------------------
 //GetCCStats
 //--------------------------------------------

 template <class PGRAPH>
 class pGetCCStats : public BasePObject{
  int token;
  int myid;
public:

  pGetCCStats(){
    this->register_this(this);
    myid = stapl::get_thread_id();
    stapl::rmi_fence();
  }

  ~pGetCCStats(){
    stapl::unregister_rmi_object( Handle );
  }

  int SetTT(int _tt){
    //printf("REmote set tt %d\n",_tt);
    tt = _tt;
    return tt;
  }
  
  int GetTT(){
    return tt;
  }

  void SetToken(){
    token    = STAPL_SET_TOKEN;
  }

  int UnsetToken(){
    int temp = token; 
    token    = STAPL_UNSET_TOKEN;
    return   temp;
  }
  int GetToken(){
    return token;
  }
  void sum(int* in, int* inout) { *inout += *in; }

  template <class PCONTAINER>
  int GetCCStats(PGRAPH& _pg, PCONTAINER& _stats){
    int lcccount=0;
    int result = 0;
    pGetCC<PGRAPH> getCC;
    STAPL_GRAPH_COLOR color;
    vector<VID> _ccverts;
    if(myid == 0){
      //printf("Set\n");
      SetToken();    
    }
    else {
      //printf("Unset\n");
      UnsetToken();
    }
    _pg.ResetTraversal(0);
    stapl::rmi_fence(); 
    //if it is not your turn wait
    while(GetToken() == STAPL_UNSET_TOKEN)
      stapl::rmi_wait();
    
    //here I have the token
    typename PGRAPH::VI pend = _pg.local_end();
    for(typename PGRAPH::VI vi = _pg.local_begin(); vi != pend; ++vi) {
      int tmp=0;
      color = vi->GetColorIn(0);
      if(color == STAPL_WHITE){
	//printf("Started  on thread ---------> %d\n",stapl::get_thread_id());
	lcccount++;
	tmp = getCC.GetCCNoReset(_pg,vi->vid,_ccverts);
	_stats.push_back(pair<int,VID>(tmp,vi->vid));
      }
    }
    //here all local traversals are finished
    // reset the token 
    //pass the token to the next processor and also the current termination time;
    if(myid + 1 < stapl::get_num_threads()){
      stapl::async_rmi( myid+1, 
			this->getHandle(),
			&pGetCCcount<PGRAPH>::SetToken);
    }
    stapl::rmi_fence();
    _stats.update();
    //now perform a sort on the ccount;
    stapl::rmi_fence();
    stapl::rmi_fence();
    stapl::reduce_rmi( &lcccount, &result, this->getHandle(), &pGetCCcount<PGRAPH>::sum, true);
    stapl::rmi_fence();
    return result;
  }//end GetCCcount method
 };//end pGetCCcount class


template <class PGRAPH>
void pDisplayCC ( PGRAPH& pg, pGetCC<PGRAPH>& pgcc, VID _v1id) {
  typedef vector<VID>::iterator VI;
  vector<VID> ccverts;
  pgcc.GetCC(pg, _v1id, ccverts);
  cout << "\nCC[" << _v1id << "] = {";
  for (VI vi = ccverts.begin(); vi < ccverts.end(); vi++ ) {
    cout << *vi; 
    if (vi != ccverts.end() -1 ) cout << ", ";
  }
  cout << "}\n";
}

 class _getResults : public BasePObject{
  typedef pvector< pair<int,VID>,no_trace,stapl::vector_distribution> _PV_STATS;
  typedef vector< pair<int,VID> > _V_STATS;
 public:
  _PV_STATS* ccstats;
  _getResults(_PV_STATS* pv){
    ccstats = pv;
    this->register_this(this);
    stapl::rmi_fence();
  }

  ~_getResults(){
    stapl::unregister_rmi_object( Handle );
  }

  _V_STATS _getFromThread(){
    _V_STATS temp(ccstats->local_size());
    _V_STATS::iterator sit = temp.begin();
    for(_PV_STATS::iterator it = ccstats->local_begin();it != ccstats->local_end(); ++it){
      *sit = *it;
      sit++;
    }
    return temp;
  }

  _V_STATS fromThread(int tid){
    return stapl::sync_rmi(tid,this->getHandle(),&_getResults::_getFromThread);
  }
 };


template <class PGRAPH>
void pDisplayCCStats(PGRAPH& pg, pGetCCStats<PGRAPH>& pgccs, int _maxCCprint=-1)  {
    
    ///Modified for VC
    typedef vector< pair<int,VID> > PAIR_VECTOR;
    typedef PAIR_VECTOR::const_iterator CCI; 
    typedef vector< pair<int,VID> > _V_STATS;

    int maxCCprint;
    int i,j;

    pvector< pair<int,VID>,no_trace,stapl::vector_distribution> ccstats;
    pvector< pair<int,VID>,no_trace,stapl::vector_distribution>::iterator pvit;
    _getResults gr(&ccstats);
    stapl::rmi_fence();
    pgccs.GetCCStats(pg, ccstats);

    //temp fix until we figure it out what is wrong with pvector
    

    if(stapl::get_thread_id() == 0){
      int sz = ccstats.size();
      if (_maxCCprint == -1) {
        maxCCprint = sz;
      } else {
        maxCCprint = _maxCCprint;
      }
      int ccnum = 1;
      cout << "\nThere are " << sz << " connected components:";

      //print local ones first
      for (pvit = ccstats.local_begin();pvit<ccstats.local_end();pvit++){
	cout << "\nCC[" << ccnum << "]: " << (*pvit).first ; 
	cout << " (vid=" << (*pvit).second << ")";
	ccnum++;
	if (ccnum > maxCCprint) break; 
      }
      if(ccnum <= maxCCprint){
	for(i=1;i<stapl::get_num_threads();i++){
	  _V_STATS tv = gr.fromThread(i);
	  for (j=0;j<tv.size();++j) { 
	    cout << "\nCC[" << ccnum << "]: " << tv[j].first ; 
	    cout << " (vid=" << tv[j].second << ")";
	    ccnum++;
	    if (ccnum > maxCCprint) break; 
	  }
	}
      }
      /*
       //to be uncommented when pvector works
      for (int i=0;i<sz;++i) {
	pair<int,VID> ccs = ccstats.Get(i); 
        cout << "\nCC[" << ccnum << "]: " << ccs.first ; 
        cout << " (vid=" << ccs.second << ")";
        ccnum++;
        if (ccnum > maxCCprint) break; 
      }
      */
    }
    stapl::rmi_fence();
}

} //end namespace stapl
#endif
