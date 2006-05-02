/* pFindCCinUG: find all CCs in a undirected graph
 * After calling it, each vertex knows which CC it belongs to (in ccids[vid])
 * and each processor knows which CCs it owns (the representer of the CC)
 *
 * typedef pGraph<PUG<Task,Weight2>, PMG<Task,Weight2>, PWG<Task,Weight2>, Task,   Weight2> PUGRAPH;
 * PUGRAPH pg;
 * map<VID,VID> ccids;
 * vector<VID> ccidlist;
 *
 * Usage: int ret = pFindCCinUG<PUGRAPH>(pg, ccids, ccidlist);
 * 
 */

#include "Defines.h"
#include "Graph.h"
#include "GraphAlgo.h"
#include "../pvector/pvector.h"
#include <map>
#include "pGraphAlgo.h"

#include <sys/time.h>

namespace stapl{

template <class PUGRAPH>
int _functor_BFS_forCC(PUGRAPH* _g, visitor_base<PUGRAPH>& _f, VID _startvid, map<VID,int>& _color, map<VID,int>& _ccid) {
  list<VID> q;
    typename PUGRAPH::VI v1, v2;
    typename PUGRAPH::EI e1;
    VID u,v;
    if (_g->IsVertex(_startvid, &v1) ) {
      q.push_back(_startvid);
      _f.vertex(v1);
      _color[_startvid] = 1; //GRAY;
    } else return(-1);
    while (!q.empty() ) {
      u = q.front();
      if (_g->IsVertex(u, &v1) ) {
	for (e1=v1->edgelist.begin(); e1!=v1->edgelist.end(); e1++) {
	  v = e1->vertex2id;
	  if (_g->IsVertex(v, &v2) ) {
  	    if (_color[v] == 0) {//WHITE
	      _f.vertex(v2);
	      _f.tree_edge(v1,e1);
	      _color[v] = GRAY;
	      _ccid[v] = _ccid[u];
	      q.push_back(v);
	    } else if (_color[v] == 1) { //GRAY
	      _f.back_edge(v1, e1);
	    } else if (_color[v] == 2) { //BLACK
	      _f.ft_edge(v1, e1);
	    } else cout<<"ERROR"<<endl;
	  }//end if v is local
	}
	_color[u] = BLACK;
	_f.finish_vertex(v1);
      }//end if u is local
      else cout<<"ERROR"<<endl;
      q.pop_front();
    }
    return 1;
}

template <class PUGRAPH>
int _functor_BFS_forpCC(PUGRAPH* _g, visitor_base<PUGRAPH>& _f, VID _startvid, map<VID,int>& _color, map<VID,int>& _ccid, map<VID,vector<VID> >* _cccontents, map<VID,VID>* _parent, vector<pair<VID,VID> >* _redges1, vector<VID>& _ccidlist) {
    list<VID> q;
    typename PUGRAPH::VI v1, v2;
    typename PUGRAPH::EI e1;
    VID u,v;
    if (_g->IsVertex(_startvid, &v1) ) {
      q.push_back(_startvid);
      _f.vertex(v1);
      _color[_startvid] = 1; //GRAY;
    } else return(-1);
    while (!q.empty() ) {
      u = q.front();
      if (_g->IsVertex(u, &v1) ) {
	for (e1=v1->edgelist.begin(); e1!=v1->edgelist.end(); e1++) {
	  v = e1->vertex2id;
	  if (_g->IsVertex(v, &v2) ) {
  	    if (_color[v] == 0) {//WHITE
	      _f.vertex(v2);
	      _f.tree_edge(v1,e1);
	      _color[v] = GRAY;
	      _ccid[v] = _ccid[u];
	      (*_cccontents)[_ccid[u]].push_back(v);
	      //cout<<"#"<<stapl::get_thread_id()<<" cccontents["<<_ccid[u]<<"].push_back("<<v<<")"<<endl;

	      	//move the "collapse to star" from StarF to here
	      (*_parent)[v] = _ccid[u];

	      q.push_back(v);
	    } else if (_color[v] == 1) { //GRAY
	      _f.back_edge(v1, e1);
	    } else if (_color[v] == 2) { //BLACK
	      _f.ft_edge(v1, e1);
	    } else cout<<"ERROR"<<endl;
	  }//end if v is local
	  else {
		//remember remote edge (u,v) in redges1
	    pair<VID,VID> re;
	    re.first = u; re.second = v; 
	    _redges1->push_back(re);
	  }
	}
	_color[u] = BLACK;
	_f.finish_vertex(v1);
      }//end if u is local
      else cout<<"ERROR"<<endl;
      q.pop_front();
    }
    return 1;
}


/*
Sparse__CC_Functor
*/
template <class PUGRAPH>
class __Sparse__CC_Functor : public BasePObject {
  int ECount;
  PUGRAPH* _g;

	//VID parent[u]: u's parent
  map<VID,VID> parent;  
  map<VID,VID> s;

	//VID ccid[u]: the vid of the root for the CC to which u belongs
  map<VID,VID>* ccid;   


  	//ccidlist: collects all the CC's ids of my processor
  vector<VID>* ccidlist;

  map<VID,vector<VID> > cccontents;

  	//origccroots: remember all the roots of the original CCs found in LocalCCSearch
  vector<VID> origccroots;
	
	//bool ccstag[r]: true if the CC(whose root is "r") is stagnant
	//		  false otherwise
  map<VID,bool> ccstag;

  	//crossedges[r]: the crossedges for CC w/ root r
  map<VID,vector<pair<VID,VID> > > crossedges;

	//redges1: all the remote edges in graph
	//bool flag: whether or not this element is still valid
	//   be TRUE initially, can be unset when the remote edge is deleted
  vector<pair<VID,VID> > redges1;
  	
  	//redges2: to remember the newly added edges in REdgeCol
  	//they may be remote (by changing remote edge (u,v) in redges1)
  	//or local (if u and parentv are on the same processor)
  	//redges2 is actually the total set of crossedges
  	//when used later in LocalCCsCheck, always check whether
  	//ccid[redges2[i].first] == ccid[redges2[i].second]?
  	//if not, it's ccid[redges2[i].first]'s crossedge
  vector<pair<VID,VID> > redges2;

	//redges3: to remember all the crossedges found in LocalCcsCheck	//note that only part of redges2 may have ccidu!=ccidv, which is
	//crossedge, that these redges2[i] are remembered in redges3
	//redges3 won't be changed in Hookings, so it can be used in 
	//SelfLoopRemoval
  vector<pair<VID,VID> > redges3;

	//redges4: to remember the left edges after SelfLoopRemoval
	// since this method deletes all the edges in the same loop, 
	// the left edges are just active crossedges before
	//it will be used by EdgelistConcatenation
	//EdgelistConcatenation will decide redges4[i] = (u,v)
	// if u is leaf, then change it to (ccidu, v), and remember
	//the (ccidu, v) in redges2, which will be used in LocalCCsCheck
	//of the next iteration of while
  vector<pair<VID,VID> > redges4;

	//todelccs: each of them is the id of the CC that should be
	//deleted from my ccidlist later (it's my own cc)
    	//used in Con(Uncon)Hook
  vector<VID> todelccs;
	
	//toupdateccs: each of them is the id of the CC that should be 
    	//set to TRUE 
    	//used in Con(Uncon)Hook
  vector<VID> toupdateccs;


  	//got in Con(Uncon)Hook
	//to remember the vertices whose parents have ever been updated in the 
	//Hooking(s) of this iter. 
	//when PointerJumping, first apply it to the vertices in changedv to 
	//make them all consistent.
  vector<VID> changedv;

  int localfinished;

public:
  __Sparse__CC_Functor() { 
        ccid->clear();
	ccidlist->clear();
	origccroots.clear();
	cccontents.clear();
	redges1.clear();
	redges2.clear();
	redges3.clear();
		//v1.9
	changedv.clear();
	localfinished = 0;
	this->register_this(this);
  }

  __Sparse__CC_Functor(PUGRAPH* pg, map<VID,VID>* _ccid, vector<VID>* _ccidlist) {
    typename PUGRAPH::VI v1;
    _g = pg; 
    ccid = _ccid;
    ccid->clear();
    ccidlist = _ccidlist;
    ccidlist->clear();
    origccroots.clear();
    cccontents.clear();
    redges1.clear();
    redges2.clear();
    redges3.clear();
    changedv.clear();
    localfinished = 0;
    for(v1=_g->local_begin(); v1!=_g->local_end(); v1++)  {
      parent[v1->vid] = v1->vid;
      (*ccid)[v1->vid] = v1->vid;
    }
    this->register_this(this);
  }

  ~__Sparse__CC_Functor() { 
    rmiHandle handle = this->getHandle();
    if (handle > -1)
      stapl::unregister_rmi_object(handle);
  }

  VID GetParentid(VID v) { 
    if (!_g->IsLocal(v) ) return ERROR; 
    else return (parent[v]); 
  }

  VID GetCCid(VID v) { 
    if (!_g->IsLocal(v) ) return ERROR; 
    else return ((*ccid)[v]); 
  }

  int sLocalCCSearch() {
	//each processor finds local CC among its nodes using BFS, 
	//only dealing with local edges in this step
    typename PUGRAPH::VI v1;
    typename PUGRAPH::EI e1;
    visitor_base<PUGRAPH> _f;
    map<VID,int> _color;
    VID u,v;
    int temp;

    timeval __start, __end;
    float elapseL = 0;
    gettimeofday(&__start, NULL);

    for(v1=_g->local_begin(); v1!=_g->local_end(); v1++) {
      _color[v1->vid] = 0;
    }
    for(v1=_g->local_begin(); v1!=_g->local_end(); v1++) {
      if (_color[v1->vid] == 0) {//WHITE
        (*ccid)[v1->vid] = v1->vid;
	cccontents[v1->vid].push_back(v1->vid);
	temp = _functor_BFS_forCC(_g, _f, v1->vid, _color, *ccid);
	if (temp==ERROR) return ERROR;
      }
    }
    _color.clear();

    gettimeofday(&__end,NULL);
    elapseL = (float)(__end.tv_sec-__start.tv_sec)*1000000 + (float)(__end.tv_usec-__start.tv_usec);
    cout<<"sLocalCCSearch TIME: "<<elapseL<<endl;

    return temp;
  }


  int LocalCCSearch() {
	//each processor finds local CC among its nodes using BFS, 
	//only dealing with local edges in this step
	//and remember all the remote edges in redges1
	// redges1 will be used by StarFormation
	//finally, copy the local CCs' roots into origccroots
    typename PUGRAPH::VI v1;
    typename PUGRAPH::EI e1;
    visitor_base<PUGRAPH> _f;
    map<VID,int> _color;
    VID u,v;
    int temp;


    timeval __start, __end;
    float elapseL=0;
    gettimeofday(&__start, NULL);

    for(v1=_g->local_begin(); v1!=_g->local_end(); v1++) {
      if (_color[v1->vid] == 0) {//WHITE
        (*ccid)[v1->vid] = v1->vid;
	cccontents[v1->vid].push_back(v1->vid);
	//cout<<"#"<<stapl::get_thread_id()<<" cccontents["<<v1->vid<<"].push_back("<<v1->vid<<")"<<endl;
        parent[v1->vid] = v1->vid;
	ccidlist->push_back(v1->vid);
	temp = _functor_BFS_forpCC(_g, _f, v1->vid, _color, *ccid, &cccontents, &parent, &redges1, *ccidlist);
	if (temp==ERROR) return ERROR;
      }
    }

    for(int i=0; i<ccidlist->size(); i++) {
      origccroots.push_back((*ccidlist)[i]);
    }

    gettimeofday(&__end,NULL);
    elapseL = (float)(__end.tv_sec-__start.tv_sec)*1000000 + (float)(__end.tv_usec-__start.tv_usec);
    cout<<"#"<<stapl::get_thread_id()<<" LocalCCSearch TIME: "<<elapseL<<endl;

/*
    {
      for(v1=_g->local_begin(); v1!=_g->local_end(); v1++) {
        cout<<"On thread #"<<stapl::get_thread_id()<<" LocalCCS: ccid["<<v1->vid<<"] = "<<(*ccid)[v1->vid]<<endl;
      } 
      for(int i=0; i<ccidlist->size(); i++) {
        cout<<"On thread #"<<stapl::get_thread_id()<<" LocalCCS: ccidlist["<<i<<"] = "<<(*ccidlist)[i]<<endl;
	VID cc=(*ccidlist)[i];
	for(int j=0; j<cccontents[cc].size(); j++) {
	  cout<<"	On thread #"<<stapl::get_thread_id()<<"LocalCCS: cccontents["<<cc<<"]["<<j<<"]="<<cccontents[cc][j]<<endl;
	}
      }
      for(int i=0; i<redges1.size(); i++) {
        cout<<"#"<<stapl::get_thread_id()<<" LocalCCS: redges1["<<i<<"] = ("<<redges1[i].first<<", "<<redges1[i].second<<")"<<endl;
      }
      stapl::rmi_fence();
    }
*/
    return temp;
  }

  void StarFormation() {
	//change each redges1[i]=(u,v) to (ccidu,ccidv)
	  //but we don't really do DeleteEdge and AddEdge
	  //only remember (ccidu,ccidv) in redges2 on ccidu's owner
	  
    typename PUGRAPH::VI v1,v2;
    typename PUGRAPH::EI e1,e2;
    VID u,v;
    int _cciduowner;
    int i=0;
    VID ccidu, ccidv;

    timeval __start, __end;
    float elapseStarF=0;
    gettimeofday(&__start, NULL);

    if (redges1.size()==0) {
      redges2.clear();
    } else {

      VID oldccidu, oldccidv;
      u = redges1[0].first; v = redges1[0].second;
      oldccidu = (*ccid)[u];	//u must be local

      if (_g->IsLocal(oldccidu) ) {
	_cciduowner = stapl::get_thread_id();
      } else {
        Location _m = _g->Lookup(oldccidu);
        stapl_assert(_m.ValidLocation(), "......");
	_cciduowner = _m.locpid();
      }

      Location _n = _g->Lookup(v);	//v must be remote
      stapl_assert(_n.ValidLocation(), "......");
      oldccidv = stapl::sync_rmi(_n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetCCid, v);

      stapl::async_rmi(_cciduowner, this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::AddRedges2,oldccidu,oldccidv);

     for(i=1; i<redges1.size(); i++) {
	u = redges1[i].first; v = redges1[i].second;
	ccidu = (*ccid)[u];

        if (_g->IsLocal(ccidu) ) {
  	  _cciduowner = stapl::get_thread_id();
        } else {
          Location _m = _g->Lookup(ccidu);
          stapl_assert(_m.ValidLocation(), "......");
	  _cciduowner = _m.locpid();
        }

        Location _n = _g->Lookup(v);
        stapl_assert(_n.ValidLocation(), "......");
        ccidv = stapl::sync_rmi(_n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetCCid, v);

	if (ccidu!=oldccidu || ccidv!=oldccidv) {
	  stapl::async_rmi(_cciduowner, this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::AddRedges2, ccidu,ccidv);
	}

	oldccidu = ccidu; oldccidv = ccidv;
     }//end for i=1...

    } //end if redges1.size()==0

    stapl::rmi_fence();

    gettimeofday(&__end,NULL);     
    elapseStarF = (float)(__end.tv_sec-__start.tv_sec)*1000000 + (float)(__end.tv_usec-__start.tv_usec);
    cout<<"#"<<stapl::get_thread_id()<<" StarFormation TIME: "<<elapseStarF<<endl;     


    return;
}

  	//auxiliary, called by REdgeCollapse
  void  AddRedges2(VID u,VID v) {
    pair<VID,VID> redge2;
    redge2.first = u; 
    redge2.second = v;
    redges2.push_back(redge2);
    return;
  }

	/* only when each processor has localfinished==1
	 * return true 
	 */
  bool TerminateCheck() {
    typename PUGRAPH::VI v1;

    typename PUGRAPH::EI e1;
    int mycount, wholecount;
    int myid=stapl::get_thread_id();
    int NP = stapl::get_num_threads();

    //mycount=redges2.size(); wholecount=0;
    mycount=localfinished; wholecount=0;
    stapl::rmi_fence();
    reduce_rmi(&mycount, &wholecount, this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::sumcount, true);
    stapl::rmi_fence();

    //return (wholecount==0);
    return (wholecount==NP);
  }

  void sumcount(int* in, int* inout) {*inout += *in; }


  //auxiliary, called by LocalCCsCheck
  void  AddCrossedges(_StaplTriple<VID,VID ,VID> _toadd) {
    VID ccidu=_toadd.first;
    VID u=_toadd.second;
    VID v=_toadd.third;
    pair<VID,VID> crossedge;
    crossedge.first = u;
    crossedge.second = v;
    crossedges[ccidu].push_back(crossedge); 
    return;
  }

  void LocalCCsCheck() {
	//0. redges3.clear();
 	//1. if (redges2.size()==0)
	  //    nothing to do (no remote edge at all, thus no crossedge)
	  //    set each CC in myccidlist as TRUE;
	  // else if (ccidlist->size() == 0)
	  //    nothing to do 
	  // else step2
  	//2. init:
	  // for each CC in my ccidlist
	  //	set its tag as TRUE
	  //	clear its crossedges
	  //	go on with step3
	//3. check each edge in redges2 (u,v)
	  //    if ccidu!=ccidv (it's a crossedge)
	  //    SetCCStag(ccidu, false) on ccidu's owner (maybe not me)
	  // 			(sync)
	  //    remember (u,v) in crossedges[ccidu] on ccidu's owner
	  //    		(sync)
	  //    remember (u,v) in redges3 on u's owner
	  //    		(sync)
	//4. redges2.clear();
	  
    typename PUGRAPH::VI v1;
    typename PUGRAPH::EI e1;
    VID cci,u,v,ccidu,ccidv;

    
    timeval __start, __end;
    float elapseLCheck=0;
    gettimeofday(&__start, NULL);

    redges3.clear();

    if (redges2.size()==0) {
      for(int i=0; i<ccidlist->size();i++) 
	ccstag[(*ccidlist)[i]] = true;
    } else if (!(ccidlist->size() == 0)) {
      for(int i=0; i<ccidlist->size();i++) {
	ccstag[(*ccidlist)[i]] = true;
	crossedges[(*ccidlist)[i]].clear();
      }
    }

    stapl::rmi_fence();

      /**init.**/
      //for(int i=0; i<ccidlist->size();i++) {
      //ccstag[(*ccidlist)[i]] = true;
      //crossedges[(*ccidlist)[i]].clear();
      //}
    
      
      /**decide ccstag for all CCs**/
      for(int i=0; i<redges2.size(); i++) {
	u = redges2[i].first; v = redges2[i].second;
	ccidu = (*ccid)[u]; 
	if (_g->IsLocal(v) ) {
	  ccidv = (*ccid)[v];
	} else {
	  Location _n = _g->Lookup(v);
	  stapl_assert(_n.ValidLocation(), "......");
	  ccidv = stapl::sync_rmi(_n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetCCid, v);
	}
	
	
	/*** (u,v) is not crossedge, do noop, goto next redges[i] ***/
	if (ccidu==ccidv) continue;
	
	/*** ccidu has crossedge, set its tag false on its owner 
	     remember (u,v) in crossedges[ccidu] on ccidu's owner ***/
	if (_g->IsLocal(ccidu) ) {
	  ccstag[ccidu] = false;
	  pair<VID,VID> crossedge;
	  crossedge.first=u; crossedge.second=v;
	  crossedges[ccidu].push_back(crossedge);
	} else {
	  _StaplTriple<VID,VID,VID> _toadd;
	  _toadd.first = ccidu;
	  _toadd.second = u;
	  _toadd.third = v;
	  Location _n = _g->Lookup(v);
	  stapl_assert(_n.ValidLocation(), "......");
	  stapl::async_rmi(_n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::SetCCStag, ccidu, false);
	  stapl::async_rmi(_n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::AddCrossedges, _toadd);
	}     
	/*** remember the crossedge (u,v) on u's owner ***/
	redges3.push_back(redges2[i]);
      } //for

    stapl::rmi_fence();
    redges2.clear();
    stapl::rmi_fence();
    
    gettimeofday(&__end, NULL);
    elapseLCheck = (float)(__end.tv_sec-__start.tv_sec)*1000000 + (float)(__end.tv_usec-__start.tv_usec);
    cout<<"#"<<stapl::get_thread_id()<<" LocalCCsCheck TIME: "<<elapseLCheck<<endl;

    return;
  }

  //auxiliary method, called by ConditionalHook and Uncon...
  void  SetCCStag(VID cc, bool tag) {
	//if the given CC exists in my ccidlist, set its stag to be true
    for(int i=0; i<ccidlist->size(); i++) {
      if ((*ccidlist)[i] == cc) {
	ccstag[cc] = tag;
      }
    }
    //if doesn't exist, do noop
    return;
  }

  bool GetCCStag(VID cc) {
    return ccstag[cc];
  }

  //auxiliary method, called by ConditionalHook, Uncon..., and PointerJumping
  void SetParentid(VID u, VID v) {
    parent[u] = v;
    return;
  }

  //auxiliary method, called by PointerJumping
  void SetCCid(VID u, VID ccidu) {
    //cout<<"#"<<stapl::get_thread_id()<<" Set CCid of "<<u<<" tobe "<<ccidu<<endl;
    (*ccid)[u] = ccidu;
    return;
  }

  //auxiliary method, called by ConditionalHook
  void Addtoupdateccs(VID cc) {
    toupdateccs.push_back(cc);
    return;
  }

  //auxiliary method, called by ConditionalHook and Uncon...
  void Addchangedv(VID u) {
    changedv.push_back(u);
    return;
  }

  void ConditionalStarHooking() {
	  //if (my ccidlist is empty) {
	  //  do noop;
	  //} else {
	  //  for (each ccidlist[i]=cc on me) {
	  //   if cc is true, continue with next cc;
	  //   int j=0;
	  //   while (j<crossedges[cc].size() ) {
	  //    u = crossedges[cc][j].first; v=crossedges[cc][j].second;
	  //    if (cc >= ccid[v]) { 
	  //      j++; 
	  //    } else {
	  //	  remember cc in todelccs on me
	  //	  		(local)
	  //	  remember parent[u] in changedv and set parent[parent[u]] <-- v on parent[u]'s owner
	  //      		(async)
	  //	  remember ccid[v] in toupdateccs on ccid[v]'s owner
	  //	  		(async)
	  //	  break; 
	  //     }//end while
	  //    }//each cc
	  //  }//end for
	  //}//end if else
	  //
	  //rmi_fence();
	  //
  	  //for each ccid in my todelccs {
  	  //  erase it from my ccidlist
  	  //}
	  //
	  //rmi_fence();
	  //
	  //for each ccid in my toupdateccs {
	  //  set its tag as TRUE
	  //}
	  //rmi_fence();
	  
    VID u,v,cc,ccidv,pu;
    Location _m, _n;
    int temp;

    timeval __start, __end;
    float elapseConHook=0;
    gettimeofday(&__start, NULL);
          
    todelccs.clear();
    toupdateccs.clear();

    if (ccidlist->size()!=0 ){
     for(int i=0; i<ccidlist->size(); i++) {
      cc = (*ccidlist)[i];
      if (ccstag[cc] == true) continue;
      int j=0;
      while (j<crossedges[cc].size() ) {
        u = crossedges[cc][j].first;
        v = crossedges[cc][j].second;

        _m = _g->Lookup(v);
        stapl_assert(_m.ValidLocation(), "......");
        ccidv = stapl::sync_rmi(_m.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetCCid, v);

	  /** if (cc >= ccidv) {
	   *    j++;
	   *    continue with while loop;
	   *  } else {
	   *     hook(ccidu, ccidv)
	   *  i.e.:
	   *      remember cc in todelccs on me  (cc's owner)
	   *      	(local)
	   *      remember parent[u] in changedv and set parent[parent[u] ] = v 
	   *      	(on parent[u]'s owner)      	(async)
	   *      remember ccid[v] in toupdateccs on ccid[v]'s owner
	   *      	(async)
	   **/
        if (cc >= ccidv) {
  	  j++;
        } else {
	  	/*** rememeber cc in todelccs on me (cc's owner)
	      		local					***/
	  todelccs.push_back(cc);

		/*** remember parent[u] in changedv 
		 *   and set parent[parent[u] ] <- v 
		 *   on parent[u]'s owner 
	  		async					***/ 
	  _m = _g->Lookup(u);
          stapl_assert(_m.ValidLocation(), "......");
	  pu = stapl::sync_rmi( _m.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetParentid, u);

          Location _n = _g->Lookup(pu);
          stapl_assert(_n.ValidLocation(), "......");
          //cout<<"ConHOOK("<<u<<","<<v<<") with ccidu="<<cc<<" and ccidv="<<ccidv<<endl;

          stapl::async_rmi(_n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::Addchangedv, pu);
          stapl::async_rmi(_n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::SetParentid, pu, v);

	 	/***  remember ccid[v] in toupdateccs on ccid[v]'s owner
	 		      	sync				***/
          _m = _g->Lookup(ccidv);
          stapl_assert(_m.ValidLocation(), "......");
          stapl::async_rmi(_m.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::Addtoupdateccs, ccidv);

          break;//go out of while loop
        } //end cc<ccidv
      } //end while
     } //end for each cc in my ccidlist
    } //end if 

    stapl::rmi_fence();

	/**erase each of todelccs**/
    vector<VID>::iterator loc;
    for(int i=0; i<todelccs.size(); i++) {
      for(int j=0; j<ccidlist->size(); j++) {
	if (todelccs[i] == (*ccidlist)[j] ) {
	  loc = &((*ccidlist)[j]);
	}
      }
      ccidlist->erase(loc);
    }

    stapl::rmi_fence();

	/**for each toupdateccs[i].first
	     set the CC's stag as TRUE - if not exist, ignore**/
	
    for(int i=0; i<toupdateccs.size(); i++) {
      VID cc = toupdateccs[i];
      SetCCStag(cc,true);
    }
    stapl::rmi_fence();

   gettimeofday(&__end, NULL);
   elapseConHook = (float)(__end.tv_sec-__start.tv_sec)*1000000 + (float)(__end.tv_usec-__start.tv_usec);
   cout<<"#"<<stapl::get_thread_id()<<" ConditionalHook TIME: "<<elapseConHook<<endl;

    return;
  }

  void UnconditionalStarHooking() {
	  //if (my ccidlist is empty) {
	  //  do noop;
	  //} else {
	  //  for (each ccidlist[i]=cc on me) {
	  //   if (cc is true) continue with next cc;
	  //   int j=0; 
	  //   while (j<crossedges[cc].size() ) {
	  //    u = crossedges[cc][j].first; v=crossedges[cc][j].second;
	  //    if (cc == ccid[v]) { 
	  //      j++;
	  //    } else {
	  //	  remember cc in todelccs on me
	  //	  		(local)
	  //	  remember parent[u] in changedv and set parent[parent[u]] <-- v 
	  //	  	on parent[u]'s owner
	  //      		(async)
	  //	  break;
	  //     }//end while
	  //    }//each cc
	  //  }//end for
	  //}//end if else
	  //
	  //rmi_fence();
	  //
  	  //for each ccid in my todelccs {
  	  //  erase it from my ccidlist
  	  //}
	  //
	  //rmi_fence();
	  //}
	  
    VID u,v,cc,ccidv,pu;
    Location _m, _n;
    int temp;

    timeval __start, __end;
    float elapseUnconHook=0;
    gettimeofday(&__start, NULL);

    todelccs.clear();
    toupdateccs.clear();

    if (ccidlist->size()!=0 ){
     for(int i=0; i<ccidlist->size(); i++) {
      cc = (*ccidlist)[i];
      if (ccstag[cc]==true) continue;
      int j=0;
      while (j<crossedges[cc].size() ) {
        u = crossedges[cc][j].first;
        v = crossedges[cc][j].second;

        _m = _g->Lookup(v);
        stapl_assert(_m.ValidLocation(), "......");
        ccidv = stapl::sync_rmi(_m.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetCCid, v);

	/** if (cc == ccidv) {
	 *    j++;
	 *    continue with while loop;
	 *  } else {
	 *     hook(ccidu, ccidv)
	 *  i.e.:
	 *      remember cc in todelccs on me  (cc's owner)
	 *      	(local)
	 *      remember u in changedv on u's owner
	 *      	(async)
	 *      parent[parent[u] ] = v (on parent[u]'s owner)
	 *      	(async)
	 **/
        if (cc == ccidv) {
  	  j++;
        } else {
	  	/*** rememeber cc in todelccs on me (cc's owner)
	      		local					***/
	  todelccs.push_back(cc);

		/*** remember parent[u] in changedv 
		 *   and set parent[parent[u] ] <- v 
		 *   on parent[u]'s owner
	  		async					***/
            _m = _g->Lookup(u);
            stapl_assert(_m.ValidLocation(), "......");
	    pu = stapl::sync_rmi( _m.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetParentid, u);

	    _n = _g->Lookup(pu);
            stapl_assert(_n.ValidLocation(), "......");
            //cout<<"UnconHOOK("<<u<<","<<v<<") with ccidu="<<cc<<" and ccidv="<<ccidv<<endl;
            stapl::async_rmi(_n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::Addchangedv, pu);
            stapl::async_rmi(_n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::SetParentid, pu, v);

            break;//go out of while loop
        } //end cc==ccidv

      } //end while

     } //end for each cc in my ccidlist

    } //end if 

    stapl::rmi_fence();
	/**erase each of todelccs**/
    vector<VID>::iterator loc;
    for(int i=0; i<todelccs.size(); i++) {
      for(int j=0; j<ccidlist->size(); j++) {
	if (todelccs[i] == (*ccidlist)[j] ) {
	  loc = &((*ccidlist)[j]);
	}
      }
      ccidlist->erase(loc);
    }
    stapl::rmi_fence();

   gettimeofday(&__end, NULL);
   elapseUnconHook = (float)(__end.tv_sec-__start.tv_sec)*1000000 + (float)(__end.tv_usec-__start.tv_usec);
   cout<<"#"<<stapl::get_thread_id()<<" UnconditionalHook TIME: "<<elapseUnconHook<<endl;

    return;
  }

  //auxiliary method, called by PointerJumping.
  void AddtoCCcontents(VID cc, VID v) {
    //cout<<"#"<<stapl::get_thread_id()<<" cccontents["<<cc<<"] add ("<<v<<")"<<endl;
    cccontents[cc].push_back(v);
    return;
  }

  void PointerJumping() { 
	//1. update the parent for the vertices remembered in changedv (on me)
	//	do while loop of pointerjumping
	//	until each of them has p[p[u]]==p[u]
	//	then set ccid[u]=ccid[p[u]]
	//2. update the vertices w/ the same CCid as the old CCid of the vertices in changedv (on me)
	//	no while loop, simply update
	//3. update the parent for the vertices involved in redges3 (on me)
	//	why? - b/c later on, SelfLoopRemoval only cares about the edges
	//		in redges3, decide whether or not they should be kept
	//		by ccidu==ccidv??. so only the vertices in redges should 	//		have their ccid(s) updated by pointerjumping.
	//	(may include the vertices in changedv)
	//	howto? - if someone here needs to have its parent updated, that 	//		must b/c its old parent has its own parent updated
	//		that means, its old parent must be in changedv
	//		so here we only need one step: p[u] <- p[p[u]]
	//					and then ccid[u]<-ccid[p[u]]
	// notice: in step2, update each vertex, must do updating p[u] and 
	  // 	ccid[u] in a single sync_rmi

	//only update the parent and ccid of nodes involved in redges3
    typename PUGRAPH::VI v1;
    VID u, oldcu, pu, ppu, newcc;
    bool done;


    timeval __start, __end;
    float elapsePtJ=0;
    gettimeofday(&__start, NULL);


	/* nodes in changedv have their parent updated to final value */
    do {
      done = true;
      for(int i=0; i<changedv.size(); i++) {
	u = changedv[i];
	if (!_g->IsLocal(u) ) continue;
  	pu = parent[u];
	if (_g->IsLocal(pu) ) {
	  ppu = parent[pu];
	} else {
	  Location _n = _g->Lookup(pu);
          stapl_assert(_n.ValidLocation(), "......");
	  ppu = stapl::sync_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetParentid, pu);
	}

	while (pu!=ppu) {
	  done = false;
	  parent[u] = ppu;
	  	//new pu, and get the current parent of pu: ppu
	  pu = parent[u];
	  if (_g->IsLocal(pu) ) {
	    ppu = parent[pu];
	  } else {
	    Location _n = _g->Lookup(pu);
            stapl_assert(_n.ValidLocation(), "......");
	    ppu = stapl::sync_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetParentid, pu);
	  }
	}//end while
	//after done with one vertex, go on to the next one
        //cout<<"#"<<stapl::get_thread_id()<<" PTJ: changedv["<<i<<"] = "<<changedv[i]<<" has its parent set to "<<parent[u]<<endl;
      }//end for
    }while (done==false);


    stapl::rmi_fence();
    //cout<<"#"<<stapl::get_thread_id()<<" PTJ: COMPLETE WITH UPDATING PARENT FIELD OF CHANGEDV"<<endl;
    stapl::rmi_fence();

	/* nodes in changedv have their ccid updated to final value 
         * and the other guys who are in the same CC as a vertex in
	 * changedv have their ccid updated to the same as its */
    for(int i=0; i<changedv.size(); i++) {
      u = changedv[i];
      oldcu = (*ccid)[u];
      pu = parent[u];
      Location _n = _g->Lookup(pu);
      stapl_assert(_n.ValidLocation(), "......");
      newcc = stapl::sync_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetCCid, pu);
      (*ccid)[u] = newcc;
      //cout<<"#"<<stapl::get_thread_id()<<" PTJ: changedv["<<i<<"] = "<<changedv[i]<<" has its ccid set to "<<(*ccid)[u]<<endl;
      stapl::async_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::AddtoCCcontents, newcc, u);


      for(int j=0; j<cccontents[oldcu].size(); j++) {
        VID sameu = cccontents[oldcu][j];
        Location _m = _g->Lookup(sameu);
        VID oldccid = stapl::sync_rmi( _m.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetCCid, sameu);
        //cout<<"#"<<stapl::get_thread_id()<<" PTJ: processing sameu="<<sameu<<"whose oldccid["<<sameu<<"]="<<oldccid<<" for u="<<u<<" oldcu="<<oldcu<<" newcc="<<newcc<<endl;
        if (oldccid!=oldcu) continue;
        stapl::async_rmi( _m.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::SetCCid, sameu, newcc);
        stapl::async_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::AddtoCCcontents, newcc, sameu);
      }
    }

    stapl::rmi_fence();

    Location _m, _n;
    for (int i=0; i<redges3.size(); i++) {
     {
      u = redges3[i].first; //u must be local, v is not necessarily local/remote
      //cout<<"#"<<stapl::get_thread_id()<<" PTJ: processing u="<<u<<" in redges3["<<i<<"].first whose oldpu="<<parent[u]<<endl;
      pu = parent[u];
      _n = _g->Lookup(pu); 
      stapl_assert(_n.ValidLocation(), "......");
      newcc = stapl::sync_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetCCid, pu);
      ppu = stapl::sync_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetParentid, pu);
      if (pu!=ppu) {
	parent[u] = ppu;
        (*ccid)[u] = newcc;
        stapl::async_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::AddtoCCcontents, newcc, u);
      }
     }

     {
      u = redges3[i].second; //u can be remote or local
      _n = _g->Lookup(u);
      stapl_assert(_n.ValidLocation(), "......");
      pu = stapl::sync_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetParentid, u);
      if (_g->IsLocal(pu) ) {
	newcc = (*ccid)[pu];
	ppu = parent[pu];
      } else {
	_m = _g->Lookup(pu);
        stapl_assert(_m.ValidLocation(), "......");
	newcc = stapl::sync_rmi( _m.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetCCid, pu);
	ppu = stapl::sync_rmi( _m.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetParentid, pu);
      }
      if (pu!=ppu) {
        stapl::async_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::SetParentid, u, ppu);
        stapl::async_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::SetCCid, u, newcc);
        stapl::async_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::AddtoCCcontents, newcc, u);
      }
     }
    }

    changedv.clear(); 
    stapl::rmi_fence();

    gettimeofday(&__end, NULL);
    elapsePtJ = (float)(__end.tv_sec-__start.tv_sec)*1000000 + (float)(__end.tv_usec-__start.tv_usec);
    cout<<"#"<<stapl::get_thread_id()<<" PointerJumping TIME: "<<elapsePtJ<<endl;
/*
    {
      for(int i=0; i<ccidlist->size(); i++) {
        cout<<"On thread #"<<stapl::get_thread_id()<<" PTJ: ccidlist["<<i<<"] = "<<(*ccidlist)[i]<<endl;
	VID cc=(*ccidlist)[i];
	for(int j=0; j<cccontents[cc].size(); j++) {
	  cout<<"	On thread #"<<stapl::get_thread_id()<<"PTJ: cccontents["<<cc<<"]["<<j<<"]="<<cccontents[cc][j]<<endl;
	}
      }
    }
*/
    return;
  }

  void SelfLoopRemoval() { 
	//redges4.clear();
	//
	//delete all the edges in graph//don't do real deleteedge
	//  
	//for (each redges3[i]=(u,v) ) {
	//   if (ccid[u] != ccid[v]) {
	//     AddEdge(u->v)	//don't do real addedge
	//     remember (u,v) in redges4 on me (u's owner)
	//   }
	//}
	  
	  
    VID u,v,cci,ccidu, ccidv;

    timeval __start, __end;
    float elapseSelfLoopR=0;
    gettimeofday(&__start, NULL);

    redges4.clear();
    stapl::rmi_fence();

   if (redges3.size()!=0 ) {
    for (int i=0; i<redges3.size(); i++) {
      u = redges3[i].first;
      v = redges3[i].second;
      ccidu = (*ccid)[u];
      Location _m = _g->Lookup(v);
      stapl_assert(_m.ValidLocation(), "wrong......");
      ccidv = stapl::sync_rmi( _m.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetCCid, v);
      if (ccidu != ccidv) {
        pair<VID,VID> redge4;
        redge4.first = u;
        redge4.second = v;
        redges4.push_back(redge4);
      }
     } //end for redges3[i]
    }//end if (redges3.size()!=0)
    stapl::rmi_fence();

    gettimeofday(&__end, NULL);
    elapseSelfLoopR = (float)(__end.tv_sec-__start.tv_sec)*1000000 + (float)(__end.tv_usec-__start.tv_usec);
    cout<<"#"<<stapl::get_thread_id()<<" SelfLoopRemoval TIME: "<<elapseSelfLoopR<<endl; 

    return;
  }

  void EdgelistConcatenation() {
	//for each redges4[i]:(u,v) {
 	//    if (u!=parent[u]) {	//u is leaf
	//      change edge (u,v) to edge (ccidu, v)
	//      remember (ccidu,v) in redges2 on ccidu's owner
	// 		(sync)
	//	remember (v,ccidu) in redges2 on v's owner
	//		(sync)
	//    } else {
	//      remember (u,v) in redges2 on me (u's owner)
	//		(local)
	//	remember (v,u) in redges2 on v's owner
	//		(sync)
	//    }
	//}
	//rmi_fence();
	//
	//if (redges2.size()==0) {
	//  localfinished=true;
	//else localfinished=false;
	

    vector<VID> tochangevs;
    VID u,v,ccidu,ccidv;

    timeval __start, __end;
    float elapseEdgelistCon=0;
    gettimeofday(&__start, NULL);

    redges2.clear();

    for(int i=0; i<redges4.size(); i++) {
        u = redges4[i].first;
 	v = redges4[i].second;
        ccidu = (*ccid)[u];

        if (u!=parent[u]) {
		/** u is leaf, change (u,v) to (ccidu,v)
		    remember (ccidu, v) in redges2 on ccidu's owner 
		    remember (v,ccidu) in redges2 on v's owner      **/
	  Location _n = _g->Lookup(ccidu);
          stapl_assert(_n.ValidLocation(), "......");
	  stapl::async_rmi(_n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::AddRedges2, ccidu, v);
	  Location _m = _g->Lookup(v);
          stapl_assert(_m.ValidLocation(), "......");
	  stapl::async_rmi(_m.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::AddRedges2, v, ccidu);
	} else {
		/** u isn't leaf, no change
		    remember (u,v) in redges2 on me (u's owner) 
		    remember (v,u) in redges2 on v's owner          **/
	  pair<VID,VID> redge2;
	  redge2.first = u;
	  redge2.second = v;
	  redges2.push_back(redge2);
	  Location _m = _g->Lookup(v);
          stapl_assert(_m.ValidLocation(), "......");
	  stapl::async_rmi(_m.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::AddRedges2, v, u);
        }//end if u!=parentu

    }//end for redges4[i]

    stapl::rmi_fence();
    if (redges2.size()==0) localfinished=1;
    else localfinished=0;
    stapl::rmi_fence();

   gettimeofday(&__end, NULL);
   elapseEdgelistCon = (float)(__end.tv_sec-__start.tv_sec)*1000000 + (float)(__end.tv_usec-__start.tv_usec);
   cout<<"#"<<stapl::get_thread_id()<<" EdgelistConcatenation TIME: "<<elapseEdgelistCon<<endl;

    return;
  }  


    //finally, clear up all the nodes' ccids
  void FinalUpdateCCids() {
    typename PUGRAPH::VI v1;
    //VID u, pu, ppu, newcc;
    VID u, cc, oldcu;
    bool done;


    timeval __start, __end;
    float elapseFinalUpdate=0;
    gettimeofday(&__start, NULL);


    for(int i=0; i<ccidlist->size(); i++) {
      cc = (*ccidlist)[i];
      for(int j=0; j<cccontents[cc].size(); j++) {
	u = cccontents[cc][j];
	Location _n = _g->Lookup(u);
        stapl_assert(_n.ValidLocation(), "......");
	oldcu = stapl::sync_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetCCid, u);
	if (oldcu!=cc) 
	  stapl::async_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::SetCCid, u, cc);
      }
    }
    /*
    do {
      done = true;
      for(int i=0; i<origccroots.size(); i++) {
        u = origccroots[i];
	if (!_g->IsLocal(u) ) continue;
  	cu = (*ccid)[u];
	Location _n = _g->Lookup(cu);
        stapl_assert(_n.ValidLocation(), "......");
	ccu = stapl::sync_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetCCid, cu);

	while (cu!=ccu) {
	  done = false;
	  (*ccid)[u] = ccu;
	  	//new cu, and get the current CCid of cu: ccu
	  cu = ccu;
	  Location _n = _g->Lookup(cu);
          stapl_assert(_n.ValidLocation(), "......");
	  ccu = stapl::sync_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetCCid, cu);
	}//end while
      }//end for
    }while (done==false);

    stapl::rmi_fence();

   gettimeofday(&__end, NULL);
   elapseFinalUpdate = (float)(__end.tv_sec-__start.tv_sec)*1000000 + (float)(__end.tv_usec-__start.tv_usec);
   cout<<"#"<<stapl::get_thread_id()<<" FinalUpdateCCids while TIME: "<<elapseFinalUpdate<<endl;
   gettimeofday(&__start, NULL);

    for(v1=_g->local_begin(); v1!=_g->local_end(); v1++) {
        u = v1->vid;
  	cu = (*ccid)[u];
	Location _n = _g->Lookup(cu);
        stapl_assert(_n.ValidLocation(), "......");
	ccu = stapl::sync_rmi( _n.locpid(), this->getHandle(), &__Sparse__CC_Functor<PUGRAPH>::GetCCid, cu);

	if (cu!=ccu) (*ccid)[u] = ccu;
    }//end for
    */

    stapl::rmi_fence();

   gettimeofday(&__end, NULL);
   elapseFinalUpdate = (float)(__end.tv_sec-__start.tv_sec)*1000000 + (float)(__end.tv_usec-__start.tv_usec);
   cout<<"#"<<stapl::get_thread_id()<<" FinalUpdateCCids TIME: "<<elapseFinalUpdate<<endl;


    return;
  }


  int findCC() {
	  //LocalCCSearch();   get ccid for each vertex (local BFS)
	  //StarFormation();   produce many stars
	  //while(1) {
	  //  0. rmi_fence()
	  //  1. bool done = TerminateCheck();
	  //  2. if (done) break;
  	  //  3. 
	  //  	3.1 ConditionalStarHooking
	  //    3.2 UnconditionalStarHooking
	  //    3.3 PointerJumping();
	  //    3.4 SelfLoopRemoval();
	  //    3.5 EdgelistConcatenation();
	  //  } 
	  //} 
	  //rmi_fence();
	  //FinalUpdateCCids;
	  //rmi_fence();
	  //
	  //Check the result ccid
	  
    typename PUGRAPH::VI v1;
    typename PUGRAPH::EI e1;
    int temp;
    bool done=false;

    timeval tv1, tv2, elapsed;
    gettimeofday(&tv1, NULL);

     
    temp = LocalCCSearch();
    if (temp==ERROR) { cout<<"On thread #"<<stapl::get_thread_id()<<" LocalCCSearch() returns ERROR"<<endl; return ERROR; }
    StarFormation();

    while(1) {
      stapl::rmi_fence();
      done = TerminateCheck();
      if (done) break;
      LocalCCsCheck();
      ConditionalStarHooking();
      UnconditionalStarHooking();
      PointerJumping();
      SelfLoopRemoval();
      EdgelistConcatenation();
    }//end while(1)

    stapl::rmi_fence();
   // FinalUpdateCCids();
    stapl::rmi_fence();

    gettimeofday(&tv2, NULL);
    if (tv1.tv_usec > tv2.tv_usec) {
      tv2.tv_usec += 10000000; tv2.tv_sec--;
    }
    elapsed.tv_usec = tv2.tv_usec - tv1.tv_usec;
    elapsed.tv_sec = tv2.tv_sec - tv1.tv_sec;
    cout<<"# "<<get_thread_id()<<" WHILE TIME: "<<(elapsed.tv_sec*1000000+elapsed.tv_usec)<<endl;


    return 1;
  }
};

template<class PUGRAPH>
int pFindCCinUG(PUGRAPH& ug, map<VID,int>& ccids, vector<VID>& ccidlist)
{
__Sparse__CC_Functor<PUGRAPH> sccf(&ug, &ccids, &ccidlist);
timeval tvp1, tvp2;
float ptime=0;
gettimeofday(&tvp1,NULL);


stapl::rmi_fence();
stapl::rmi_fence();

int temp = sccf.findCC();
stapl::rmi_fence();

gettimeofday(&tvp2, NULL);
ptime = (float)(tvp2.tv_sec - tvp1.tv_sec)*1000000 + (float)(tvp2.tv_usec - tvp1.tv_usec);
cout<<"#"<<stapl::get_thread_id()<<" pFindCCinUG TIME: "<<ptime<<endl;

return 0;
}

/* sFindCCinUG is only used when only using 1 processor
 * we simply use BFS traversal to do localCCSearch and get all the CCs
 * it won't appear in final version
 * we need to compare the results of pFind and sFind to check p's correctness
 */
template<class PUGRAPH>
int sFindCCinUG(PUGRAPH& ug, map<VID,int>& ccids, vector<VID>& ccidlist)
{

if (stapl::get_num_threads() > 1) return ERROR;
__Sparse__CC_Functor<PUGRAPH> sccf(&ug, &ccids, &ccidlist);
timeval tvs1, tvs2;
float stime=0;
gettimeofday(&tvs1,NULL);

stapl::rmi_fence();
int temp = sccf.sLocalCCSearch();
if (temp==ERROR) return ERROR;
gettimeofday(&tvs2, NULL);
stime = (float)(tvs2.tv_sec - tvs1.tv_sec)*1000000 + (float)(tvs2.tv_usec - tvs1.tv_usec);
cout<<"sFindCCinUG TIME: "<<stime<<endl;

return 0;
}


} //end namespace stapl
