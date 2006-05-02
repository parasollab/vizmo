#include "Defines.h"
#include "Graph.h"
#include "GraphAlgo.h"
#include <runtime.h>
#include <map>
#include "./pCC.h"
#include "./pST.h"


namespace stapl {

template <class PGRAPH, class PUGRAPH>
class __EC_D_Functor: public BasePObject {
  PGRAPH* _g;

  	//succs[i].first: u*_g->size() + v, that is, edge (u,v)
	//succs[i].second:x*_g->size() + y, that is, edge (x,y)
	//we remember: succ(edge(u,v) ) = edge(x,y)
  	//calculate - in findsuccs() method
  	//	    - for each v of g
  	//	    -   for each pair of incoming edge ei and 
  	//	    -                    outgoing edge ei'
 	//	    -      succs.push_back(ei, ei') 
  vector<pair<VID,VID> > succs;

  	//swips[vid][i]: the ith incoming edge to vid
  map<VID, vector<VID> > swips;

  	//the undirected graph whose vertex is an edge in g,
	//and edge means a succ relation in g
	//it's also a partition
  PUGRAPH ug;

  	//the ccids for each vertex of ug
  	//      so each vertex of ug can know which CC it belongs to
	//i.e., means the id of the circuits of g
  	//      so each edge of g can know which circuit it belongs to
	//calculate - get from a parallel connectivity algorithm (pCC)
  map<VID,VID> ccids;

  	//ccidlist: remember all the # of the CCs
	//calculate- get from a parallel connectivity algorithm (pCC)
  vector<VID> ccidlist;

  	//the undirected graph whose vertex is a CC in ug,
	//                     also a cuicuit in g
	//and for each pair of incoming edges of g into one vertex v,
  	//  say (u1,v) and (u2,v) if they're in different CCs in ug,
  	//  then add an edge (u1,v) - (u2,v) in cg
  PUGRAPH cg;
	
  	//for (u1,v) and (u2,v) in g:
  	//  since (u1*_g->size()+v) and (u2*_g->size()+v) are in diff. CCs in ug
  	//  there exists an edge between them in cg, say (x,y)
  	//we use cgedgs to remember the original u1,v,u2 info as follows:
  	//	cgedges[x*cg.size()+y].first = (u1*_g->size()+v);
  	//	cgedges[x*cg.size()+y].second = (u2*_g->size()+v);
  	//In this way, we can go back to original edges in g by traversing each
  	//	edge in Spanning Tree of cg
  map<VID, pair<VID, VID> > cgedges;

  map<VID, bool> ChosenFlag;

  	//the spanning tree of cg
  PUGRAPH st;

  	//for each vertex v of g
  	//  all the swipindecies that need to be switched
  	//calculate - findtoswitch()
  	//          - for each edge (ccid1, ccid2) in st
 	//	    -   index = ccid1 * cg.size() + ccid2 (edge of cg)
	//	    -   gedge1 = cgedges[index].first
  	//	    -   gedge2 = cgedges[index].second
        //	    -   vid = gedges1 % _g->size();
 	//	    -   toswitchs[vid].push_back(gedge1);
 	//	    -   toswitchs[vid].push_back(gedge2);
  map<VID, vector<int> > toswitchs;

public:
  __EC_D_Functor() {register_this(this);}
  __EC_D_Functor(PGRAPH* g) {_g=g; register_this(this);}
  ~__EC_D_Functor() {}

  	/* auxiliary method 
	 * the given vid is for a vertex of ug (i.e., an edge of g)
	 * only when it's local 
	 * we can get its successor vertex (in ug)
	 */
  VID __GetSucc(VID vid) {
      vector<pair<VID,VID> >::iterator succit;
      for(succit=succs.begin(); succit!=succs.end(); succit++) {
        if (succit->first == vid) 
  	  return (succit->second);
      }
  }

  	/* auxiliary method 
	 * the given vid is for a vertex of ug (i.e., an edge of g)
	 * only when it's local 
	 * we can set its successor vertex (in ug) to svid
	 */
  int __SetSucc(VID vid, VID svid) {
      vector<pair<VID,VID> >::iterator succit;
      for(succit=succs.begin(); succit!=succs.end(); succit++) {
        if (succit->first == vid)  {
    	  succit->second = svid;
	  return 1;
        }
      }
      return ERROR;
  }//end __SetSucc

    	/* auxiliary method
	 * the given vid is for a vertex of ug
	 * only when it's local, we can get its ccid
	 */
  int __GetCCid(VID vid) {
    if (ug.IsLocal(vid) ) {
      return ccids[vid];
    } else return ERROR;
  }
  
  	/* Step 1.1: findsuccs
	 *        (every processor does in parallel)
	 *        (no need for fence, b/c step1.2 doesn't need remote info)
	 *   for each v of g (local)	
	 *     for each pair of incoming edge ei and
	 *     	                outgoing edge eo
	 *	  succs.push_back(pair(ei, eo))
	 */
  int findsuccs() {
    typename PGRAPH::VI v;
    typename PGRAPH::EI ei, eo;
    int NG = _g->size();

    	//make succ pairs and store them in succs
    for(v=_g->local_begin(); v!=_g->local_end(); v++) {
      ei=v->predecessors.begin(); 
      eo=v->edgelist.begin();
      swips[v->vid].clear();

      while (ei!=v->predecessors.end()) {
        pair<VID,VID> succelem;
	succelem.first = ei->vertex2id * NG + v->vid;
	succelem.second = v->vid * NG + eo->vertex2id;
	succs.push_back(succelem);
	swips[v->vid].push_back(ei->vertex2id * NG + v->vid);
	ei++; eo++;
      }//end while


    }//end for

/***/
    for(int i=0; i<succs.size(); i++) {
	cout<<"findsuccs - thread #"<<stapl::get_thread_id()<<" succs["<<i<<"] = ("<<succs[i].first<<","<<succs[i].second<<")"<<" i.e. "<<succs[i].first/_g->size()<<" -> "<<succs[i].first%_g->size()<<" -> "<<succs[i].second%_g->size()<<endl;
    }
    for(v=_g->local_begin(); v!=_g->local_end(); v++) {
      for(int i=0; i<swips[v->vid].size(); i++) {
        cout<<"findsuccs - thread #"<<stapl::get_thread_id()<<" swips["<<v->vid<<"]["<<i<<"] = "<<swips[v->vid][i]<<endl;
      }
    }
/***/

/***/
    for(v=ug.local_begin(); v!=ug.local_end(); v++) {
	cout<<"findsuccs complete on thread #"<<stapl::get_thread_id()<<" "<<v->vid<<"\'s succ is "<<__GetSucc(v->vid)<<endl;
    }
/***/
    return 1;
  } //end findsuccs()

  	/* Step 1.2: findswips
	 * 	    (every processor does in parallel)
	 *        (no need for fence, b/c step2.1 doesn't need remote info)
	 * for each v of g (local)
	 *   for each incoming edge ei (other than the last one)
	 *      ej = ei.next;
	 *      swips[v->vid].push_back(pair(ei, ej));
	 */
  int findswips() {
    typename PGRAPH::VI v;
    typename PGRAPH::EI ei, ej;
    int NG = _g->size();

    for (v = _g->local_begin(); v != _g->local_end(); v++) {
      for (ei = v->predecessors.begin(); ei < v->predecessors.end()-1; ei++) {
        ej = ei++;
        pair<VID, VID> swipelem;
	swipelem.first = ei->vertex2id * NG + v->vid;
	swipelem.second = ej->vertex2id * NG + v->vid;
	swips[v->vid].push_back(swipelem);
      }
    }

/***/
    for (v = _g->local_begin(); v != _g->local_end(); v++) {
      for(int i=0; i<swips[v->vid].size(); i++) {
	cout<<"findswips - thread #"<<stapl::get_thread_id()<<" swips["<<v->vid<<"]["<<i<<"] = ("<<swips[v->vid][i].first<<","<<swips[v->vid][i].second<<")"<<endl;
      }
    }
/***/
  } //end findswips()

  int __RemoteAddEdge(VID u, VID v) {
    if (!ug.IsLocal(u) ) return ERROR;
    if (!ug.IsEdge(u,v) )
      ug.AddEdge(u, v,Weight2(0));
    return 1;
  }
	/* Step 2.1: createug()
	 *	     after all processors done, fence
	 *	     (b/c 2.2 needs find CC which needs remote info)
	 * Create a undirected graph (E, SE) 
	 * where (ei,ej) is in SE iff ej=succ(ei) or ei=succ(ej);
	 */
  int createug() {
    typename PGRAPH::VI vi;
    typename PGRAPH::EI ei;
    VID uvid1,uvid2;
    vector<pair<VID,VID> >::iterator succit;
    int NG = _g->size();

	    //construct V for ug - each vertex is an edge in original g
    for(vi=_g->local_begin(); vi!=_g->local_end(); vi++) {
      for(ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ei++) {
	uvid1 = (vi->vid) * NG + ei->vertex2id;
	ug.AddVertex(vi->data, uvid1);
      }
    }

    stapl::rmi_fence();

	    //construct E for ug - each edge is a succpair in original g
    for(succit=succs.begin(); succit!=succs.end(); succit++) {
      uvid1 = succit->first;
      uvid2 = succit->second;
      if (!ug.IsEdge(uvid1, uvid2) ) {
	if (ug.IsLocal(uvid1) )
      	  ug.AddEdge(uvid1, uvid2, Weight2(0) );
	else {
          Location _n = ug.Lookup(uvid1);
           stapl_assert(_n.ValidLocation(), "Invalid id for uvid1 in __eulerpartition");
           int temp = stapl::sync_rmi(_n.locpid(), this->getHandle(), &__EC_D_Functor<PGRAPH,PUGRAPH>::__RemoteAddEdge, uvid1, uvid2);
	}
//	cout<<"#"<<get_thread_id()<<" createug: add edge ("<<uvid1<<","<<uvid2<<")"<<endl;
      }
    }

    stapl::rmi_fence();
/***
    ug.pDisplayGraph();
***/
    return 1;
  } //end createug()

	/* Step 2.2: findccids()
	 *	     after all processors done, fence
	 *	     (b/c 3.1 needs all new ccids and ccidlist
	 * Apply a parallel connectivity algo to the undirected graph
	 * and make every vertex in the ug knows its ccid
	 *
	 * each vertex in ug knows its own ccid
	 * i.e.,
	 * each edge in g knows which circuit it belongs to
	 */
  int findccids() {
    stapl::rmi_fence();
    int ret = pFindCCinUG<PUGRAPH>(ug, ccids, ccidlist);
    stapl::rmi_fence();
    stapl::rmi_fence();

/***/
    typename PUGRAPH::VI vi;
    cout<<"#"<<stapl::get_thread_id()<<" findccids gets ccidlist.size()="<<ccidlist.size()<<endl;
    /*
    for(vi=ug.local_begin(); vi!=ug.local_end(); vi++) {
      cout<<"findccids thread #"<<stapl::get_thread_id()<<" ccids["<<vi->vid<<"]= "<<ccids[vi->vid]<<endl;
    }
    for(int i=0; i<ccidlist.size(); i++) {
      cout<<"findccids thread #"<<stapl::get_thread_id()<<" ccidlist["<<i<<"] = "<<ccidlist[i]<<endl;
    }
    */
    stapl::rmi_fence();
/***/

    return 1;
  } //end findccids()


  	/* auxiliary method 
	 * called by createcg
	 */
int __AddEdge3(_StaplTriple<VID,VID,pair<VID,VID> > _arg) {
  VID ccidx, ccidy, x, y;
  int cgindex;

  ccidx = _arg.first;
  ccidy = _arg.second;
  x = _arg.third.first;
  y = _arg.third.second;
  cout<<"#"<<stapl::get_thread_id()<<" __AddEdge3 processing: ccidx="<<ccidx<<" ccidy="<<ccidy<<" ugpair=("<<x<<","<<y<<")"<<endl;

  if (!cg.IsEdge(ccidx, ccidy) ) {

		//add edge (ccidx,ccidy) to cg
    cg.AddEdge(ccidx, ccidy, Weight2(0));
    cout<<"#"<<stapl::get_thread_id()<<" add edge ("<<ccidx<<","<<ccidy<<") to cg"<<endl;
		//set cgedges for (ccidx->ccidy)
    cgindex = ccidx * cg.size() + ccidy;
    cgedges[cgindex] = _arg.third; 
    cout<<"#"<<stapl::get_thread_id()<<" change cgedges["<<cgindex<<"] = ("<<x<<","<<y<<")"<<endl;

  } //if !isedge
  return 1;
}

  	/* auxiliary method 
	 * called by createcg
	 */
int __Updatecgedges(_StaplTriple<VID,VID,pair<VID,VID> > _arg) {
  VID ccidx = _arg.first;
  VID ccidy = _arg.second;
  int cgindex = ccidy*cg.size()+ccidx;
  pair<VID,VID> ugpair = _arg.third; 
  cgedges[cgindex] = ugpair;
  cout<<"#"<<stapl::get_thread_id()<<" change cgedges["<<cgindex<<"] = ("<<ugpair.first<<","<<ugpair.second<<")"<<endl;
  return 1;
}

	/* Step 3.1: createcg()
	 * Create another undirect graph cg
	 * whose vertex is a CC given in Step2, 
	 * whose edge is a pair of adjacent incoming edges into some vertex v of _g, iff the two edges are in different CCs in ug
	 *
	 * also, remember the used pair in cgedges[v]
	 */
  int createcg() {
    typename PGRAPH::VI vi;
    typename PUGRAPH::VI uv1, uv2, uv;
    typename PUGRAPH::EI ue;

	    //create V for cg, each vertex is a circuit in ug
    cout<<"#"<<stapl::get_thread_id()<<" createcg: ccidlist.size()="<<ccidlist.size()<<endl;
    stapl::rmi_fence();
    stapl::rmi_fence();
    for(int i=0; i<ccidlist.size(); i++) {
      VID vid = ccidlist[i];
      cout<<"#"<<stapl::get_thread_id()<<" createcg processing Vertex "<<vid<<endl; 
      if (ug.IsVertex(vid, &uv1) ) {
	if ( !cg.IsVertex(vid, &uv2) ) {
	      //the root of the CC is local in ug
	      //the vertex for the CC hasn't been in cg
 	  cout<<"#"<<stapl::get_thread_id()<<" createcg add Vertex "<<vid<<endl;
	  cg.AddVertex(uv1->data, vid);
        }
      }
    }
    stapl::rmi_fence();
    stapl::rmi_fence();

	    //create E for cg
    	    //for each vertex v of g
    	    //  for each adj. pair(x,y) in swips[v]
    	    //    if (ccidx != ccidy) {
    	    //	    if (!cg.IsEdge(ccidx,ccidy) ) {
    	    //        cg.AddEdge(ccidx, ccidy) - no multi. edges
	    //	      cgedges[ccidx*cg.size()+ccidy].push_back(pairof(x,y))
	    //	      cgedges[ccidy*cg.size()+ccidx].push_back(pairof(x,y))
	    //      }
	    //    }
   	    //  } 

    VID x, y, ccidx, ccidy;
    pair<VID,VID> ugpair;
    Location _n, _m;

    for(vi=_g->local_begin(); vi!=_g->local_end(); vi++) {
		//if only 1 incoming edge to vi->vid, then no need to switch
		//simply continue;
      if (swips[vi->vid].size()==1) continue;

      for (int i=0; i<swips[vi->vid].size()-1; i++) {
		//x,y are a pair of incoming edges into vi->vid, 
		//which are adjacent in swips[vi->vid]
	x = swips[vi->vid][i];
	y = swips[vi->vid][i+1];
	ChosenFlag[x*ug.size()+y] = false;
	ugpair.first = x;
	ugpair.second = y;
		//b/c swips are for incoming edges to v,
		//x and y could be remote
		//find ccidx and ccidy
  	_m = ug.Lookup(x);
	stapl_assert(_m.ValidLocation(), "Invalid id for uvid1 in __eulerpartition");
	ccidx = stapl::sync_rmi(_m.locpid(), this->getHandle(), &__EC_D_Functor<PGRAPH,PUGRAPH>::__GetCCid, x);

  	_n = ug.Lookup(y);
	stapl_assert(_n.ValidLocation(), "Invalid id for uvid2 in __eulerpartition");
	ccidy = stapl::sync_rmi(_n.locpid(), this->getHandle(), &__EC_D_Functor<PGRAPH,PUGRAPH>::__GetCCid, y);
 
	cout<<"createcg on #"<<get_thread_id()<<" x="<<x<<" y="<<y<<" ccidx="<<ccidx<<" ccidy="<<ccidy<<endl;

		//if (ccidx==ccidy), they're in same CC in ug, ignore
	if (ccidx == ccidy) continue; 

		//if (ccidx!=ccidy) {
		//  if (!_g->IsEdge(ccidx, ccidy) ) {
		//	we add edge (ccidx,ccidy) into cg
		//	remember (x,y) in cgedges[ccidx*cg.size()+ccidy]
		//      remember (x,y) in cgedges[ccidy*cg.size()+ccidx]

	if (!cg.IsEdge(ccidx, ccidy) ) {	
		//__AddEdges3: 
		//	add edge to cg - on ccidx's owner (_m)
		//      cgedges[ccidx*cg.size()+ccidy] = (x,y) - on _n
	  _StaplTriple<VID,VID,pair<VID,VID> > _arg(ccidx, ccidy, ugpair);
    	  _m = ug.Lookup(ccidx);
	  stapl_assert(_m.ValidLocation(), "Invalid id for uvid2 in __eulerpartition");
	  int temp = stapl::sync_rmi(_m.locpid(), this->getHandle(), &__EC_D_Functor<PGRAPH,PUGRAPH>::__AddEdge3, _arg);
	  if (temp==ERROR) return ERROR;

		//__Updatecgedges:
		//      cgedges[ccidy*cg.size()+ccidx] = (x,y) - on _n
  	  _n = ug.Lookup(ccidy);
	  stapl_assert(_n.ValidLocation(), "Invalid id for uvid2 in __eulerpartition");
	  temp = stapl::sync_rmi(_n.locpid(), this->getHandle(), &__EC_D_Functor<PGRAPH,PUGRAPH>::__Updatecgedges, _arg);
	  if (temp==ERROR) return ERROR;
	}
      }//end for i

    }//end for vi

    stapl::rmi_fence();
    stapl::rmi_fence();
    //cg.pDisplayGraph();
    stapl::rmi_fence();
    stapl::rmi_fence();
    /***/
    for(uv=cg.local_begin(); uv!=cg.local_end(); uv++) {
      for(ue=uv->edgelist.begin(); ue!=uv->edgelist.end(); ue++) {
	if (uv->vid < ue->vertex2id)  {
          VID vid = uv->vid * cg.size() + ue->vertex2id;
          cout<<"after createcg, thread #"<<stapl::get_thread_id()<<" cgedges["<<vid<<"] = ("<<cgedges[vid].first<<","<<cgedges[vid].second<<")"<<endl;
        }
      }
    }
    /***/
    stapl::rmi_fence();
    stapl::rmi_fence();
    return 1;
  }// end __createcg()

	/* Step 3.2: findspanningtree()
	 * find a spanning tree tr for cg
	 * B/c each vertex of cg means a circuits in g
	 * the spanning tree tr connects all the circuits in g
	 * we need be able to convert edge in tr back to the swip in g
	 */	
  int findspanningtree() {
    stapl::rmi_fence();
    cg.pDisplayGraph();
    int ret = pFindST<PUGRAPH>(cg, st);

    cout<<"#"<<stapl::get_thread_id()<<" complete ST"<<endl;
    stapl::rmi_fence();
    stapl::rmi_fence();
/***/
    //st.pDisplayGraph();
/***/
    stapl::rmi_fence();
    stapl::rmi_fence();
    return ret;
  }//end findspanningtree()

  int SetToSwitchs(VID vid, int swipindex) {
    if (!_g->IsLocal(vid) ) return ERROR;
    int i;
    for(i=0; i<toswitchs[vid].size(); i++) {
      if (toswitchs[vid][i] == swipindex) return 1;
    }
    if (i==toswitchs[vid].size())
      toswitchs[vid].push_back(swipindex);
    return 1;
  }

  	/* auxiliary method 
	 * called by findtoswitch
	 */
  int __SetChosenflag(VID e1, VID e2) {
    ChosenFlag[e1*ug.size()+e2] = true;
    cout<<"#"<<stapl::get_thread_id()<<" locally set ChosenFlag["<<e1*ug.size()+e2<<"] as TRUE!"<<endl;
    return 1;
  }

	/* Step 4.1: findtoswich()
	 * for each edge (ccid1, ccid2) in st
	 *   index = ccid1 * cg.size() + ccid2
	 *   vid = cgedges[index].first
	 *   swipindex = cgedges[index].second
	 *     //that means,
	 *     //the edge in tr is for the swips[vid][swipindex] in g
	 *   remember swipindex in toswitch[vid]
	 */
  int findtoswitch() {
    typename PUGRAPH::VI vi;
    typename PUGRAPH::EI ei;
    int NC = cg.size();
    
    for(vi=st.local_begin(); vi!=st.local_end(); vi++) {
      for(ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ei++) {
	VID ccid1 = vi->vid;
	VID ccid2 = ei->vertex2id;
	if (ccid1>ccid2) continue;
	int index = ccid1 * NC + ccid2;
	VID e1 = cgedges[index].first;
	VID e2 = cgedges[index].second;
	VID vid = e1 % _g->size();
	cout<<"#"<<stapl::get_thread_id()<<"Findtoswitch: processing ccid1="<<ccid1<<" ccid2="<<ccid2<<" whose cgedges is: ("<<e1<<","<<e2<<") and the common vid for e1 and e2 is "<<vid<<endl;
	Location _m = _g->Lookup(vid);
        stapl_assert(_m.ValidLocation(), "Invalid id for vid in _g");
	cout<<"#"<<stapl::get_thread_id()<<" Findtoswitch: set ChosenFlag[("<<e1<<","<<e2<<")] = true on #"<<_m.locpid()<<endl;
        int temp = stapl::sync_rmi(_m.locpid(), this->getHandle(), &__EC_D_Functor<PGRAPH,PUGRAPH>::__SetChosenflag, e1, e2);
	if (temp==ERROR) return ERROR;
      } //for ei
    } //for vi

    stapl::rmi_fence();
    stapl::rmi_fence();
    return 1;
  }

	/* Step 4.2: doswitch()
	 * for each vertex v of g
	 *   do {
	 *    find a sublist [s,t)
	 *    exchange for the sublist
	 *   }
	 *
	 *   remember: the succs[es] always locates on owner of the vertex 
	 *   es%_g->size()  - es is incoming to the vertex
	 */		
  int doswitch() {
    typename PGRAPH::VI vi;
    int i,j,s,t,len;
    VID vid, e1, e2;
    VID es, ej, ej1, et, succes, succet, succej1;
    Location _ms, _mt, _mj, _mj1;
    int temp;
	
    for (vi=_g->local_begin(); vi!=_g->local_end(); vi++) {
      vid = vi->vid;
      len = swips[vid].size();
      if (len <=1) continue;

      i = 0;
      while (i<len-1) {
	e1 = swips[vid][i];
	e2 = swips[vid][i+1];

	cout<<"#"<<stapl::get_thread_id()<<"Doswitch: processing e1="<<e1<<" e2="<<e2<<endl;
	while (!ChosenFlag[e1*ug.size()+e2] ) {
	  i++;
	  if (i == len-1) break;//last one, no iter any more
	  e1 = swips[vid][i];
	  e2 = swips[vid][i+1];
	  cout<<"#"<<stapl::get_thread_id()<<"Doswitch: processing e1="<<e1<<" e2="<<e2<<" with ChosenFlag as "<<ChosenFlag[e1*ug.size()+e2]<<endl;
	};

	if (i==len-1) break;

	s = i;
	es = swips[vid][s];
	_ms = _g->Lookup(es%_g->size());
        stapl_assert(_ms.ValidLocation(), "Invalid id for es in _g");
        succes = stapl::sync_rmi(_ms.locpid(), this->getHandle(), &__EC_D_Functor<PGRAPH,PUGRAPH>::__GetSucc, es);
	cout<<"#"<<stapl::get_thread_id()<<"Doswitch: for vid="<<vid<<", s="<<s<<" es="<<es<<" succes="<<succes<<endl;

	while (ChosenFlag[e1*ug.size()+e2] ) {
	  i++;
	  if (i == len-1) break;//last one, no iter any more
	  e1 = swips[vid][i];
	  e2 = swips[vid][i+1];
	};
		//update succs[et] := old-succs[es]
	t = i;
	et = swips[vid][t];
	_mt = _g->Lookup(et%_g->size());
        stapl_assert(_mt.ValidLocation(), "Invalid id for et in _g");
        succet = stapl::sync_rmi(_mt.locpid(), this->getHandle(), &__EC_D_Functor<PGRAPH,PUGRAPH>::__GetSucc, et);
        temp = stapl::sync_rmi(_mt.locpid(), this->getHandle(), &__EC_D_Functor<PGRAPH,PUGRAPH>::__SetSucc, et, succes);

	cout<<"#"<<stapl::get_thread_id()<<"Doswitch: for vid="<<vid<<", t="<<t<<" et="<<et<<" set its succ as "<<succes<<endl;
		//update sublist [s,t)
	for (j=s; j<t; j++) {
	  ej = swips[vid][j];
	  ej1 = swips[vid][j+1];

	  if (j==t-1) {
  	    succej1 = succet;
	  } else {
	    _mj1 = _g->Lookup(ej1%_g->size());
            stapl_assert(_mj1.ValidLocation(), "Invalid id for ej1 in _g");
            succej1 = stapl::sync_rmi(_mj1.locpid(), this->getHandle(), &__EC_D_Functor<PGRAPH,PUGRAPH>::__GetSucc, ej1);
	  }

	  _mj = _g->Lookup(ej%_g->size());
          stapl_assert(_mj.ValidLocation(), "Invalid id for ej in _g");
	  temp = stapl::sync_rmi(_mj.locpid(), this->getHandle(), &__EC_D_Functor<PGRAPH,PUGRAPH>::__SetSucc, ej, succej1);
	  if (temp==ERROR) return ERROR;
	  cout<<"doswitch: for vid="<<vid<<", j="<<j<<" ej="<<ej<<" set its succ as "<<succej1<<endl;
	} //end updating sublist [s,t)

      }
    }

    stapl::rmi_fence();
    stapl::rmi_fence();
    cout<<"#"<<stapl::get_thread_id()<<"finished doswitch"<<endl;
    stapl::rmi_fence();
    stapl::rmi_fence();
  }

  int __SendToZero(pair<VID,VID> succelem) {
  //void __merge(pair<VID,VID> succelem) {
    succs.push_back(succelem);
    cout<<"#"<<get_thread_id()<<" succs.push_back: ("<<succelem.first<<","<<succelem.second<<")"<<endl;
    return 1;
  }

  int __zero_reorder(vector<pair<VID,VID> >* et) {

    vector<pair<VID,VID> >::iterator etit = et->begin();
    VID u,v, v1id, v2id;
    pair<VID,VID> newet;
    int NG = _g->size();

    v1id = _g->local_begin()->vid; u = v1id;
    v2id = _g->local_begin()->edgelist.begin()->vertex2id; v = v2id;
	//(u->v) is the first element of et
	//when comes back to (u->v), et is complete
    newet.first = v1id; newet.second = v2id;
    cout<<"et->push_back("<<newet.first<<"--->"<<newet.second<<")"<<endl;
    et->push_back(newet);

    do {
	for(int i=0; i<succs.size(); i++) {
	  if (succs[i].first==v1id*NG+v2id) {
	    newet.first = succs[i].second/NG;
	    newet.second = succs[i].second%NG;
            cout<<"et->push_back("<<newet.first<<"--->"<<newet.second<<")"<<endl;
    	    et->push_back(newet);
	    v1id = newet.first;
  	    v2id = newet.second;
            cout<<"et: current v1id="<<v1id<<" v2id="<<v2id<<endl;
	    break;
	  }
	}
    } while (!(v1id==u && v2id==v));

    cout<<"Euler Circuit on thread#0 : ";
    for (etit=et->begin(); etit<et->end()-1; etit++) {
      cout<<etit->first<<" ---> "<<etit->second<<";      ";
    }
    cout<<endl;

  }

  //void showresults() {
  void showresults(vector<pair<VID,VID> >* et) {
/*
    for(int i=0; i<succs.size(); i++) {
	cout<<"#"<<stapl::get_thread_id()<<" succs["<<i<<"]=("<<succs[i].first<<","<<succs[i].second<<")"<<endl;
    } 
*/
    typename PUGRAPH::VI vi; 
    typename PUGRAPH::EI ei;
    int myid = stapl::get_thread_id();

    if (myid > 0) {
      for(int i=0; i<succs.size(); i++) {
	//cout<<"#"<<stapl::get_thread_id()<<" trying to send ("<<succs[i].first<<","<<succs[i].second<<") to #0"<<endl;
        int temp = stapl::sync_rmi(0, this->getHandle(), &__EC_D_Functor<PGRAPH,PUGRAPH>::__SendToZero, succs[i]);
	if (temp==ERROR) cout<<"#"<<stapl::get_thread_id()<<" sending ("<<succs[i].first<<","<<succs[i].second<<") Fail!"<<endl;
	else cout<<"#"<<stapl::get_thread_id()<<" sending ("<<succs[i].first<<","<<succs[i].second<<") Succeed!"<<endl;
        //stapl::async_rmi(0, this->getHandle(), &__EC_D_Functor<PGRAPH,PUGRAPH>::__merge, succs[i]);
	//cout<<"#"<<stapl::get_thread_id()<<" OK sent!"<<endl;
      } 
    }

    stapl::rmi_fence();
    stapl::rmi_fence();

    if (myid == 0) { 
	__zero_reorder(et);
    }


    if (myid == 0) {
      for(int i=0; i<succs.size(); i++) {
	cout<<"succs["<<i<<"]=("<<succs[i].first<<","<<succs[i].second<<")"<<endl;
      }
      int NG = _g->size();
      VID u = _g->local_begin()->vid;
      VID v = _g->local_begin()->edgelist.begin()->vertex2id;
      VID u0=u, v0=v;
      vector<pair<VID,VID> > evector;
      evector.clear();

     while(1) {
      pair<VID,VID> evelem;
      evelem.first = u;
      evelem.second = v;
      evector.push_back(evelem);
      VID edgeid = u * NG + v;
      VID nextedgeid = __GetSucc(edgeid);
      u = nextedgeid / NG;
      v = nextedgeid % NG;
      if (u==u0 && v==v0) break;
     }

      for(int i=0; i<evector.size(); i++) {
	cout<<evector[i].first<<"->"<<evector[i].second<<" "<<endl;
      }
    } 

  }//end showresults

};

/**Decide whether the PGRAPH g has Euler Circuit.
 * If yes, find one EC.
 * Esp., when g is a tree, use pFindET instead.	
 *@param g the original graph to be decided.
 * @return 0 if there exists Euler Circuit, -1 otherwise.
 */
template<class PGRAPH>
int pFindEC(PGRAPH& g)
{
  /* DG, if for each vertex v, in-deg(v)==out-deg(v) then TRUE
   * UG, if for each vertex v, deg(v) % 2 == 0 then TRUE
   */
  typename PGRAPH::VI v1,v2;
  typename PGRAPH::EI e1,e2;
  typedef pGraph<PUG<Task,Weight2>, PMG<Task,Weight2>, PWG<Task,Weight2>, Task, Weight2> PUGRAPH;
  vector<pair<VID,VID> > ees;

  g.pSetPredecessors();

  if (g.IsDirected() ) {
   {
     int indeg, outdeg;

    for(v1=g.local_begin(); v1!=g.local_end(); v1++) {
      outdeg = v1->edgelist.size();
      indeg = v1->predecessors.size();
      if (outdeg<=0 || indeg<=0) {
	cout<<"degree of vertex "<<v1->vid<<"on thread #"<<stapl::get_thread_id()<<" is 0"<<endl;
	return -1;
      }
      if (outdeg != indeg) {
	cout<<"degree of vertex "<<v1->vid<<"on thread #"<<stapl::get_thread_id()<<" outdeg="<<outdeg<<" indeg="<<indeg<<endl;
	return -1;
      }
    }    
   } 

    __EC_D_Functor<PGRAPH,PUGRAPH> ecdf(&g);
    int ret = ecdf.findsuccs();
    //ecdf.findswips();
    ecdf.createug();
    stapl::rmi_fence();
    ecdf.findccids();
    stapl::rmi_fence();
    stapl::rmi_fence();
    ecdf.createcg();
    stapl::rmi_fence();
    ecdf.findspanningtree();
    stapl::rmi_fence();
    ecdf.findtoswitch();
    stapl::rmi_fence();
    stapl::rmi_fence();
    ecdf.doswitch();
    ecdf.showresults(&ees);
    stapl::rmi_fence();
    stapl::rmi_fence();
    cout<<"#"<<stapl::get_thread_id()<<" complete pEC return 0"<<endl;
    stapl::rmi_fence();
    stapl::rmi_fence();
    return 0;
  } else {
	//undirected graph!
  }
}

}
