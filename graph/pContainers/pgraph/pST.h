#include "Defines.h"
#include "Graph.h"
#include "pGraphAlgo.h"
#include <runtime.h>
#include <map>

namespace stapl {

///////////////////////////////////////
//       BFS like traversal (one graph) to find a spanning tree for an assumed connected  graph
///////////////////////////////////////
template <class PG>
class BFS_findST: public BasePObject{  
 public:

  typedef map<VID,int> MAP;

  MAP _color;

  PG* _pgraphs;
  PG* _st;

  int mystartvid;
  //vertex vi , index and direction(fwd,back)
  list<_StaplTriple<typename PG::VI,int,int> > QT;//queue used for trimer traversals
  bool inside;

  BFS_findST(){
    SetInside(false);
    register_this(this);
  }

  BFS_findST(PG* _pgs, PG* _spanningtree) {
    typename PG::VI vi;
    for (vi=_pgs->local_begin(); vi!=_pgs->local_end(); vi++) {
      _spanningtree->AddVertex(vi->data, vi->vid );
    }
    stapl::rmi_fence();
    _pgraphs = _pgs;
    _st = _spanningtree;
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

  _StaplTriple<typename PG::VI,int,int> popQueue(){
    _StaplTriple<typename PG::VI,int,int> temp = QT.front();
    QT.pop_front();
    return temp;
  }
  
  inline int size(){
    return QT.size();
  }

  //void remoteAdd(VID fromvid, VID vid,int index){
  void remoteAdd(_StaplTriple<VID,VID,int> _arg) {
    VID fromvid = _arg.first;
    VID vid = _arg.second;
    int index = _arg.third;  
    typename PG::VI vi;
    map<VID, int>::iterator mit;
    //cout<<"remote function called on thread#"<<stapl::get_thread_id()<<" with fromvid="<<fromvid<<" vid="<<vid<<endl;

    if(!_pgraphs->IsVertex(vid,&vi)){
      //printf("ERROR: while an rmi method invoked: trim::remoteAdd\n");
      return;
    }
    //here vi is initialized properly;
    //check if it's color is GRAY; if so don't added
    mit = _color.find(vid);

    if(mit != _color.end()){//if color is WHITE
       //cout<<"in remoteAdd on #"<<stapl::get_thread_id()<<" vertex "<<vid<<" is NOT WHITE"<<endl;
    } else {
      _color[vi->vid] = GRAY;
      _st->AddEdge(vid, fromvid, Weight2(0));	//added into spanningtree
      cout<<"on#"<<stapl::get_thread_id()<<" in remoteAdd: ST.AddEdge("<<fromvid<<","<<vid<<")"<<endl;
      addQueue(vi,0);
      if (inside == false) BFSLoop();
    }
    return;
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

    //cout<<"thread#"<<stapl::get_thread_id()<<" enters BFSLoop()"<<endl;
    while(size() > 0){//while there are more vertices
      //pop a vertex from Q
      v = popQueue();

      //for every child u of v
      if(v.third == STAPL_P_FWD){
	int t = _pgraphs->GetAdjacentVertices(v.first,succs);
	//cout<<"Adjacency for "<<v.first->vid<<" "<< t <<" "<<succs.size()<<endl;
	//cout<<"Adjacency for "<<v.first->vid<<" ("<<succs.size()<<") succs"<<endl;
	for(int v2id=0;v2id<succs.size();v2id++){
	  if(!_pgraphs->IsVertex(succs[v2id],&vi)){
	    //the vertex is not local
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    
	    //but first checked if not touched(GRAY)
	    mit = _color.find(succs[v2id]);
	    if(mit == _color.end()){//if color is WHITE
	      Location _n=_pgraphs->Lookup(succs[v2id]);
	      stapl_assert(_n.ValidLocation(),
			   "Invalid id for the second vertex of an edge in BFS");
	      _color[succs[v2id]] = GRAY;
	      _StaplTriple<VID, VID, int> _arg(v.first->vid, succs[v2id], v.second);
	      stapl::async_rmi( _n.locpid(), getHandle(), &BFS_findST::remoteAdd, _arg);
	    }
	  }
	  else{
	    //here we are if the vertex is local
	    //first check the color of the node
	    mit = _color.find(succs[v2id]);
	    if(mit == _color.end()){//if color is WHITE
	      //addQueueFront(vi,v.second);
	      _color[succs[v2id]] = GRAY;
	      //cout<<"local vertex"<<succs[v2id]<<" "<<stapl::get_thread_id()<<endl;
              _st->AddEdge(v.first->vid, succs[v2id], Weight2(0));	//added into spanningtree
  	      cout<<"ST.AddEdge("<<v.first->vid<<","<<succs[v2id]<<")"<<endl;
	      addQueue(vi,v.second);
	    }
	  }
	}//for all succs
      }//if fwd
    }//while
    SetInside(false);
    return OK;
  }//end method BFSLoop 

void decidestartvid(VID* in, VID* inout) {
  //cout<<"#"<<stapl::get_thread_id()<<"*in="<<*in<<", *inout="<<*inout<<endl;
  if (*in==-1) ;
  else 
    if (*in < *inout) *inout = *in;
}

int Getmystartvid (int caller) {
	cout<<"#"<<stapl::get_thread_id()<<" return mystartvid="<<mystartvid<<" to caller=#"<<caller<<endl;
	return mystartvid;
}

int GetStartVid() {
  typename PG::VI vi;
 // int mystartvid, finalstartvid;
  int finalstartvid;
  int myid = stapl::get_thread_id();
  int NP = stapl::get_num_threads();
  
  if (_pgraphs->local_size()==0)
    {	  mystartvid = -1; finalstartvid = -1;}
  else {  mystartvid = _pgraphs->local_begin()->vid;
	  finalstartvid = mystartvid; }
 
  stapl::rmi_fence();
  stapl::rmi_fence();
  cout<<"trying to reduce_rmi on thread #"<<stapl::get_thread_id()<<" with mystartvid="<<mystartvid<<endl;
  stapl::rmi_fence();
  stapl::rmi_fence();
  for(int i=0; i<NP; i++) {
      int temp = sync_rmi( i, getHandle(), &BFS_findST::Getmystartvid, myid);
      if (temp < 0) continue;
      if (temp < finalstartvid ) finalstartvid = temp;
  }
  //reduce_rmi(&mystartvid, &finalstartvid, this->getHandle(), &BFS_findST<PG>::decidestartvid, true);
  stapl::rmi_fence();
  stapl::rmi_fence();
  cout<<"reduce_rmi on thread #"<<stapl::get_thread_id()<<" get StartVid as "<<finalstartvid<<endl;
  return (finalstartvid);
}

};


template<class PGRAPH>
int pFindST (PGRAPH& _vpg , PGRAPH& _st) {

  int temp;
  int i;
  typename PGRAPH::VI vi;

cout<<"#"<<stapl::get_thread_id()<<" get called"<<endl;
  _vpg.pDisplayGraph();
  BFS_findST<PGRAPH> _bfsf(&_vpg , &_st);
  stapl::rmi_fence();
  stapl::rmi_fence();

  int _startVid;
  _startVid = _bfsf.GetStartVid();

  if(_vpg.IsVertex(_startVid,&vi) ) {//only one of them start the DFS
    cout<<"Started  on thread ---------> #"<<stapl::get_thread_id()<<endl;
    _bfsf.addQueue(vi,0);
    _bfsf._color[vi->vid] = GRAY;
    _bfsf.SetInside(false);
    temp = _bfsf.BFSLoop();
  } else {
	cout<<_startVid<<" isn't local to me: #"<<stapl::get_thread_id()<<endl;
  }
  stapl::rmi_fence();
  _st.pDisplayGraph();
  stapl::rmi_fence();
  stapl::rmi_fence();
  
  //temp has to be broadcasted to everybody before return
  return temp;
}

}
