/* this is not committed:
 *	1. support for boolean "wantranks"
 * 	2. support EP template instead of pList<euleredge>
 * 	3. move comm_tellstartowner inside of Link_Build
/*
 *  pET.h: finds an Euler Tour in given tree
 *         the returned tour is represented as a pList of edges, and each edge has its rank on the tour
 * 	   using algo. of Thm3.3 on JaJa's P114;
 *  Definition of tree: an acyclic undirected connected graph
 *  B/c there's currently no dedicated data structure for tree, simply use a directed graph to replace it
 *  there is Tree-gen/tree_gen to produce  a random tree
 *
 *  Main function is pET, it has 3 steps:
 *  1. Local_Build: each thread works on its local edges, and build some pList components 
 *  2. Link_Build: link the components built in step1 according to the order of edges
 *  3. call Recurrence function (a pAlgorithm) to compute rank values for each edge on the tour
 *
 *  testing (with correctness check): test_pET.cc (works in dir: stapl/test_suite_Stapl/pcontainers/pGraph) 
 *  or tester: stapl/test_suite_Stapl/pcontainers/pGraph/ET-Testers/pET_Tester
 *
 */
#include "Defines.h"
#include "Graph.h"
#include "GraphAlgo.h"
#include <runtime.h>
#include <map>
#include "pAlgorithms/p_Recurrence.h"

#include <sys/time.h>

namespace stapl{

#ifndef Forward
#define Forward true
#endif
#ifndef Backward
#define Backward false
#endif

/* each element of returned pList is an euleredge object 
The euler tour is an list of euler edges
*/
class euleredge {
    VID u; VID v; int rank; 
  public:
    euleredge() : u(-1),v(-1),rank(-1){}
    euleredge(VID _u, VID _v) : u(_u), v(_v), rank(-1) {}
    euleredge(VID _u, VID _v, int _r) : u(_u), v(_v), rank(_r) {}

    void SetVids(VID _u, VID _v) {u = _u; v = _v;}
    void SetRank(int _r) {rank = _r;}

    VID GetFirstVid() const { return u; }
    VID GetSecondVid() const { return v; }
    int GetRank() const {return rank; }

    bool operator==(const euleredge e) const {
      if ( (u==e.GetFirstVid()) && (v==e.GetSecondVid() )  )
	return true; 
      else return false;
    }
    bool operator<(const euleredge e) const {
      VID u1 = e.GetFirstVid(); VID v1 = e.GetSecondVid();
      if (u<u1) return true;
      if (u>u1) return false;
      if (u==u1) {
	if (v<v1) return true;
	else return false;
      } 
    }
    ~euleredge() {};
    void define_type(stapl::typer &t) { t.local(u); t.local(v); t.local(rank); }
};

ostream& operator<< ( ostream& os, const euleredge& e) {
  os <<e.GetFirstVid()<<"->"<<e.GetSecondVid()<<"["<<e.GetRank()<<"] ";
  return os;
}

/* step1 Local_Build
 * follow local edges and build local parts as long as possible
 * use a temporary vector<vector<euleredge> > eparts to store the euleredges found during loop
 * and when one part is finished, save that part
 */
template <class PTREE, class EP>
void Local_Build(PTREE& ptree, EP& eulerpath, VID& startvid, PID& startowner, euleredge& startedge, PID& endowner, euleredge& endedge, vector<euleredge>& mytails) {
  typename PTREE::VI vi;
  typename PTREE::EI ei;
  PARTID partid=0;
  euleredge e;
  VID u,v,w;

  PARTID rem_part = INVALID_PART;
  PID myid = stapl::get_thread_id();
  if (startvid!=-1 && ptree.IsLocal(startvid) )
    endowner = myid;  //the owner of startvid is the owner of endedge

  set< pair<VID,VID> > cutedges;
  /* Get all incoming cut edges */
  for (vi = ptree.local_begin(); vi!=ptree.local_end(); ++vi) {
    for (ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ++ei) {
      if (!ptree.IsLocal(ei->vertex2id) ) {
	cutedges.insert(pair<VID,VID>(ei->vertex2id, vi->vid) );
      }
    }
  }
  set<pair<VID,VID> >::iterator sit;

  /* starting from each incoming cut edge, get a string of edges as long as possible */
  int index=0;
  bool endflag=false;//Is this the first iteration in which startvid is met?

  for(sit = cutedges.begin(); sit != cutedges.end(); ++sit ){
    u = sit->first; v=sit->second;
    e.SetVids(u,v); e.SetRank(1);
    if (ptree.IsVertex(v, &vi)) ;

    //go along predecessors for edges until meeting an incoming edge (w is remote)
    //or going back to startedge
    //while (ptree.IsVertex(v, &vi) ) {
    while (1) {
      if (myid == endowner && v == startvid && rem_part==INVALID_PART) {
	  //the first time pushinging some edge as (?? -> startvid) into eulerpath
	  //so look at this edge as "endedge", then its successor edge is "startedge"
	  //Notice that startedge may be on another thread
	rem_part = partid; 
	e.SetRank(0);
	endedge = e;
	endflag=true;
      }
      eulerpath.AddElement2Part(e, partid); 

      //find successor edge (v->w) for (u->v)
      for (ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ++ei) {
        if (ei->vertex2id == u) break;
      } 
      ++ei;
      if (ei == vi->edgelist.end() )
        w = vi->edgelist.begin()->vertex2id;
      else
        w = ei->vertex2id;
      e.SetVids(v,w); e.SetRank(1);
      u = e.GetFirstVid(); v = e.GetSecondVid();

      if (endflag) {
	if (ptree.IsLocal(v) ) {
  	  partid = eulerpath.AddPart(); 
          mytails.push_back(e);
        }
	startedge = e;
	startowner = ptree.Lookup(v).locpid(); //maybe me, maybe not
	endflag=false; //no longer first time
      }
      
      if (!ptree.IsVertex(v, &vi)) {
        mytails.push_back(e);
	break;
      }
    }

    //add a new part for the string starting from another incoming cutedge
    if (index < cutedges.size()-1 ) //not the last cutedge
      partid = eulerpath.AddPart();
    index++;
  }
  //stapl::rmi_fence();
  if (startvid==-1 && myid==0) {
    //always choose the last local vertex on the last local part as startvid
    //so that endedge is always the last edge on the part and it won't cause a new part
    startedge = mytails[partid];
    endowner = myid;
    typename EP::pContainer_Part_type::iterator lastit = eulerpath.GetPart(partid)->end();
    --lastit;
    //endedge = *lastit; endedge.SetRank(0);
    endedge = lastit->GetUserData(); endedge.SetRank(0);
    //*lastit = endedge;
    lastit->SetUserData(endedge);
    startowner = ptree.Lookup(startedge.GetSecondVid()).locpid();
  }
  stapl::rmi_fence();
}


/* step2 Link_Build 
 * each part sends its last edge (must be cross edge, or endedge) to its ending point's owner thread
 * the receiver thread checks its own parts, finding a part whose first edge is the successor for the received
 * tailedge, then sets the received partloc as this matching part's prev, and then tells the matching part's loc 
 * back to the sender thread of this message, such that the sender can then set the matching part as its that 
 * part's next.
 */
//communicate class for tellall
class comm_tellstartowner : public BasePObject {
    PID* startowner;
    euleredge* startedge;
 public:
  comm_tellstartowner (PID& _startowner, euleredge& _startedge) {
    startowner = &_startowner; startedge = &_startedge;
    register_this(this);
  }
  ~comm_tellstartowner() {
    rmiHandle handle = this->getHandle();
    if (handle > -1) stapl::unregister_rmi_object(handle);
  }
  void tellstartowner(PID dest, euleredge _startedge) {
    stapl::async_rmi(dest, this->getHandle(), &comm_tellstartowner::gettoknow, _startedge);
  }
  void gettoknow(euleredge _startedge) {
    *startowner = stapl::get_thread_id();
    *startedge = _startedge;
  }
};

//communicate class used by Link_Build
template <class EP>
class comm_links : public BasePObject {
    EP *eulerpath;
    vector<Location> *befores, *nexts;
  public:
    comm_links (EP& _eulerpath, vector<Location>& _befores, vector<Location>& _nexts ) {
      eulerpath = &_eulerpath; befores = &_befores; nexts = &_nexts; register_this(this);
    }
    ~comm_links () {
      rmiHandle handle = this->getHandle();
      if (handle > -1) unregister_rmi_object(handle);
    }
    void AskforMatch (PID nextpid, Location mypartloc, euleredge& tomatch ) {
      async_rmi ( nextpid, this->getHandle(), &comm_links::TrytoMatch, mypartloc, tomatch);
    }
    void TrytoMatch (Location prevpartloc, euleredge& tomatchhead) {
      //receive the tail edge of prevpart, try to find the edge's successor, and then match it with one local part's head
      PARTID partid;
      for (partid=0; partid<eulerpath->get_num_parts(); partid++) {
        if ( (eulerpath->GetPart(partid))->begin()->GetUserData() == tomatchhead)
	//if (* ( (eulerpath->GetPart(partid) )->begin()) == tomatchhead) 
	  break;
      }
      Location matchingpartloc = Location(stapl::get_thread_id(), partid);
      (*befores)[partid] = prevpartloc;
      AnswerMatched ( prevpartloc.locpid(), matchingpartloc, prevpartloc.partid() );
    }
    void AnswerMatched (PID prevpid, Location matchingpartloc, PARTID prevpartid) {
      async_rmi(prevpid, this->getHandle(), &comm_links::GetAnswer, matchingpartloc, prevpartid);
    }
    void GetAnswer (Location matchingpartloc, PARTID mypartid) {
      (*nexts)[mypartid] = matchingpartloc;
    }
};

template <class PTREE, class EP>
void Link_Build (PTREE& ptree, EP& eulerpath, PID& startowner, PID& endowner, euleredge& startedge, euleredge& endedge, vector<euleredge>& mytails) {
  int npart = eulerpath.get_num_parts();
  PID myid = stapl::get_thread_id();
  PARTID partid;
  vector<Location> befores(npart), nexts(npart);
  typename PTREE::VI vi;
  typename PTREE::EI ei;
  VID u,v,w;
  
  /* Now, only endedge's owner (i.e. startvid's owner knows the correct info. about startedge, startowner, endedge, endowner 
   * if endowner is not startowner, should also tell startowner s.t. startowner can know which part is the beginning of whole pList when Link_Build
   */
  stapl::rmi_fence();
  comm_tellstartowner teller(startowner, startedge);
  if (myid==endowner && startowner!=endowner) {
    teller.tellstartowner(startowner, startedge); 
  }
  stapl::rmi_fence();

  comm_links<EP> linkcomm(eulerpath, befores, nexts);
  stapl::rmi_fence();
  //cout<<"#"<<get_thread_id()<<" startowner="<<startowner<<", endowner="<<endowner<<", startedge="<<startedge<<", endedge="<<endedge<<endl;
 
  for (partid=0; partid<npart; partid++) {
    euleredge head = (eulerpath.GetPart(partid)->begin())->GetUserData();
    //cout<<"#"<<get_thread_id()<<", partid#"<<partid<<" head = "<<head<<endl;
    //euleredge head = *(eulerpath.GetPart(partid)->begin());
    if (myid==startowner && head == startedge) {
      //cout<<"#"<<get_thread_id()<<", partid#"<<partid<<" before = invalid"<<endl;
      befores[partid] = Location(INVALID_PID, INVALID_PART);
    }
    euleredge tail = mytails[partid];
    //cout<<"#"<<get_thread_id()<<", partid#"<<partid<<" tail = "<<tail<<endl;
    if (myid==endowner && tail == startedge) {
      //the last part of whole pList does not send message to AskforMatch
      //then the first part of whole pList should not receive any message
      //cout<<"#"<<get_thread_id()<<", partid#"<<partid<<" next = invalid"<<endl;
      nexts[partid] = Location(INVALID_PID, INVALID_PART);
    } else {
      w = tail.GetSecondVid();
      PID wowner = ptree.Lookup(w).locpid();
      //if this part should have next part,
      //then send its tail edge to w's owner thread, ask for matching part
      //cout<<"#"<<get_thread_id()<<", partid#"<<partid<<" Ask #"<<wowner<<" for edge "<<tail<<endl;
      linkcomm.AskforMatch(wowner, Location(myid,partid), tail);
    }
  }
  stapl::rmi_fence();
  for (partid=0; partid<npart; partid++) {
    eulerpath.InitBoundaryInfo(partid, befores[partid], nexts[partid]);
  }
  stapl::rmi_fence();
}

/* step3 call Linear Recurrence algorithm
 * to compute ranks for each euleredge in the pList
 */
//recurrence function : input for Recurrence1
class IncreaseRank {
  public:
    euleredge identity() { return euleredge(-1, -1, 0); }
    euleredge operator()(euleredge& x1, euleredge& x2) {
      //cout<<"#"<<get_thread_id()<<" IncreaseRank x1="<<x1<<" x2="<<x2<<" return rank="<<x1.GetRank()+x2.GetRank()<<endl;
      //the associative plus operator for Recurrence is defined as
      //the VIDs are the same, but the rank increases 
      return euleredge (x2.GetFirstVid(), x2.GetSecondVid(), x1.GetRank()+x2.GetRank() );
    }
};
 
template<class EP>
void ComputeRank(EP& eulerpath) {
  typename EP::reverse_PRange epr(&eulerpath);
  eulerpath.get_prange(epr);
  stapl::rmi_fence();

  IncreaseRank increase_rank;
  Recurrence1<typename EP::reverse_PRange, IncreaseRank>(epr, increase_rank, Backward);
  stapl::rmi_fence();
}


/* main function */
 
template <class PTREE, class EP>
void pET(PTREE& ptree, EP& eulerpath, VID startvid=-1, bool wantranks=true) {
  timer tv; double elapsed;
  typename PTREE::VI vi;
  typename PTREE::EI ei;

  /* If #threads==1, we use simple sequential algorithm to find the Euler Tour
   * b/c we don't have cut edges at all which we use in Local_Build step 
   */
  tv = start_timer();
  if (stapl::get_num_threads() == 1) {
    VID u,v,w;
    euleredge e,nexte, startedge, endedge;
    int edgecounter = 2*(ptree.size()-1)-1;
    //decide the startedge
    if (startvid != -1) {
      if (ptree.IsVertex(startvid, &vi) ) {
	ei = vi->edgelist.begin();
	v = ei->vertex2id;
	startedge.SetVids(startvid, v);
	startedge.SetRank(edgecounter); 
      }
    } else {
      vi = ptree.local_begin();
      u = vi->vid;
      ei = vi->edgelist.begin();
      v = ei->vertex2id;
      startedge.SetVids(u, v);
      startedge.SetRank(edgecounter);
    }

    //decide the endedge - the pred. of startedge
    //since startedge is always the leftmost in startvid's edgelist
    //endedge is always the rightmost one
    u = startedge.GetFirstVid();
    if (ptree.IsVertex(u, &vi) ) {
      ei = vi->edgelist.end(); ei--;
      w = ei->vertex2id;
    }
    endedge.SetVids(w,u);
    endedge.SetRank(0);

    //then starting form the ending point of startedge
    e = startedge;
    while(1) {
      u = e.GetFirstVid();
      v = e.GetSecondVid();
      if (e == endedge) { 
        eulerpath.AddElement2Part(endedge,0); break; 
      }
      if (! (ptree.IsVertex(v, &vi) )  )
        ;
      for (ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ++ei) {
        if (ei->vertex2id == u) break;
      }
      if ((++ei) != vi->edgelist.end() ) //not the last 
	w = ei->vertex2id; //then the right neighbor
      else  //the last
	w = vi->edgelist.begin()->vertex2id; //then the first
      nexte.SetVids(v, w);
      e.SetVids(u, v);
      e.SetRank(edgecounter);
      eulerpath.AddElement2Part(e,0);
      e = nexte;
      edgecounter--;
    } 
   
    elapsed = stop_timer(tv);
    cout<<"internal time "<<elapsed<<endl;
    return;
  }

  euleredge startedge(-1, -1, -1), endedge(-1,-1,-1);
  PID startowner=INVALID_PID, endowner=INVALID_PID;
  PID myid = stapl::get_thread_id();
  vector<euleredge> mytails;


  stapl::rmi_fence();
  tv = start_timer();
  Local_Build(ptree, eulerpath, startvid, startowner, startedge, endowner, endedge, mytails);
  stapl::rmi_fence();
  elapsed = stop_timer(tv);
  if (myid==0) cout<<"Local_Build time "<<elapsed<<endl;
 

  tv = start_timer();
  Link_Build(ptree, eulerpath, startowner,endowner, startedge, endedge, mytails);
  stapl::rmi_fence();
  elapsed = stop_timer(tv);
  if (myid==0) cout<<"Link_Build time "<<elapsed<<endl;
  
  if (wantranks) {
  tv = start_timer();
  ComputeRank(eulerpath);
  stapl::rmi_fence();
  elapsed = stop_timer(tv);
  if (myid==0) cout<<"ComputeRank time "<<elapsed<<endl;
  }
} 


}
