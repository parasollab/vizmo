#include "./pET.h"

namespace stapl {
#ifndef Forward
#define Forward true
#endif
#ifndef Backward
#define Backward false
#endif

template<class EP>
class __setranks : public BasePObject {
  int callerid;
  map<VID,VID> parentmap;
  rmiHandle handle;
public:
  __setranks(int _callerid) { callerid = _callerid; this->register_this(this); }
  __setranks(int _callerid, map<VID,VID>& _parentmap) { callerid = _callerid; parentmap=_parentmap; this->register_this(this); }
  ~__setranks() { handle = this->getHandle(); if (handle>-1) unregister_rmi_object(handle); }
  void operator() (typename EP::PRange::subrangeType pr) {
    typename EP::PRange::subrangeType::iteratorType it;
    euleredge ee;
    for(it = pr.get_boundary().start(); it != pr.get_boundary().finish(); ++it){
      switch(callerid) {
	case 1: //RootingTree: set all euleredge's rank as 1
	  ee = it->GetUserData(); ee.SetRank(1); it->SetUserData(ee); break;
	case 2: //PostorderNumbering: rank(v->p[v])=1, rank(p[v]->v)=0
	  ee = it->GetUserData(); 
	  if (parentmap[ee.GetFirstVid()] == ee.GetSecondVid() ) ee.SetRank(1); 
	  else ee.SetRank(0);
	  it->SetUserData(ee);
 	  break;
  	case 3: //VertexLeveling: rank(v->p[v])=-1, rank(p[v]->v)=1
	  ee = it->GetUserData(); 
	  if (parentmap[ee.GetFirstVid()] == ee.GetSecondVid() ) ee.SetRank(-1); 
	  else ee.SetRank(1);
	  it->SetUserData(ee);
	  break;
	case 4: //NumberDesc: rank(v->p[v])=1, rank(p[v]->v)=0
	  ee = it->GetUserData(); 
	  if (parentmap[ee.GetFirstVid()] == ee.GetSecondVid() ) ee.SetRank(1); 
	   else ee.SetRank(0);
	  it->SetUserData(ee);
	  break;
      }//switch...
    }//for...
  }
};

typedef hash_map<int, int, hash<int>, equal_to<int> > Pair_Hashmap;

template <class PTREE, class EP>
void ComparePairRanks(PTREE& ptree, EP& eulerpath, vector<euleredge>& tailv, map<VID,int>& resultmap, int callerid) {
  int id, reverseid, tmp, prefixsumofee; VID u,v,w;
  euleredge ee;
  typename PTREE::VI vi;
  typename PTREE::EI ei;
  int SZ = ptree.size();
  bool findreverse;
  Pair_Hashmap pairhashmap;

  for (PARTID partid=0; partid<eulerpath.get_num_parts(); partid++) {
    typename EP::pContainer_Part_type::iterator partit = eulerpath.GetPart(partid)->begin(); 
    while (1) { //current edge is u->v
      if (partit == eulerpath.GetPart(partid)->end() ) {
        if (get_num_threads() > 1) ee = tailv[partid];
	else break;
      } else {
	ee = partit->GetUserData(); 
      }
      u = ee.GetFirstVid(); v = ee.GetSecondVid(); prefixsumofee = ee.GetRank();
      id = u*SZ+v; reverseid = v*SZ+u;
      findreverse = (pairhashmap.find(reverseid) != pairhashmap.end() );
      //cout<<"#"<<get_thread_id()<<" current ee="<<ee<<" id="<<id<<" reverseid="<<reverseid<<" findreverse="<<findreverse<<endl;
      if (!findreverse) {
        pairhashmap[id] = prefixsumofee; //first time meet u->v or v->u, remember its rank in hashmap
	//cout<<"#"<<get_thread_id()<<" set pairhashmap["<<id<<"]="<<prefixsumofee<<endl;
      } else {
	tmp = pairhashmap[reverseid] - prefixsumofee; //meet its reverse euleredge, remember prefix_sums[v->u] - prefix_sums[u->v]
	if (callerid==1) {// this method is called by rooting a tree, resultmap is the parentmap
	  if (tmp<0) {
	    resultmap[u] = v;//prefix_sums[v->u]<prefix_sums[u->v], so parent[u]=v
	    //cout<<"#"<<get_thread_id()<<" find pairhashmap["<<reverseid<<"]="<<pairhashmap[reverseid]<<" tmp="<<tmp<<"<0 so parent["<<u<<"]="<<v<<endl;
	  } else {
	    resultmap[v] = u;
	    //cout<<"#"<<get_thread_id()<<" find pairhashmap["<<reverseid<<"]="<<pairhashmap[reverseid]<<" tmp="<<tmp<<"<0 so parent["<<v<<"]="<<u<<endl;
	  }
	} else if (callerid==4) {//this method is called by Number of Descendants, resultmap is sizemap
	  if (tmp>0) {
	    resultmap[v] = tmp; 
	    //cout<<"#"<<get_thread_id()<<" find pairhashmap["<<reverseid<<"]="<<pairhashmap[reverseid]<<" tmp="<<tmp<<" size["<<v<<"]="<<tmp<<endl;
	  } else {
	    resultmap[u] = 0-tmp; 
	    //cout<<"#"<<get_thread_id()<<" find pairhashmap["<<reverseid<<"]="<<pairhashmap[reverseid]<<" tmp="<<tmp<<" size["<<u<<"]="<<0-tmp<<endl;
	  }
	}
      }
      if (partit == eulerpath.GetPart(partid)->end() ) break;
      ++partit;
    }//end while(1)...
    
  }
}

//communicate class to collect tailv
template<class EP>
class comm_telltails : public BasePObject {
  EP *eulerpath;
  vector<euleredge> *tailv;
public:
  comm_telltails(EP& _eulerpath, vector<euleredge>& _tailv) { eulerpath=&_eulerpath; tailv=&_tailv; register_this(this); }
  ~comm_telltails() { rmiHandle handle = this->getHandle(); if (handle>-1) unregister_rmi_object(handle);}
  void GetTail (PARTID mypartid, euleredge intail) {
    //cout<<"#"<<get_thread_id()<<" GetTail mypartid="<<mypartid<<" intail="<<intail<<endl;
    (*tailv)[mypartid] = intail;
    //cout<<"#"<<get_thread_id()<<" old value is "<<(*tailv)[mypartid]<<endl;
  }
  void TellBefore (Location beforepart, euleredge myhead) {
    //cout<<"#"<<get_thread_id()<<" TellBefore beforepart=("<<beforepart.locpid()<<","<<beforepart.partid()<<") myhead="<<myhead<<endl;
    async_rmi(beforepart.locpid(), this->getHandle(), &comm_telltails<EP>::GetTail, beforepart.partid(), myhead);
  }
  void GetFromStart (euleredge intail) {
    Location nextpart;
    for (PARTID partid=0; partid<eulerpath->get_num_parts(); ++partid) {
      nextpart = eulerpath->GetDistribution().GetPartBdry(partid).second;
      if (nextpart.locpid()==INVALID_PID || nextpart.partid()==INVALID_PART) {
	//cout<<"#"<<get_thread_id()<<" #part"<<partid<<" GetFromStart intail="<<intail<<endl;
	(*tailv)[partid] = intail;
	break;
      }
    }
  }
  void TellEndPart (PID rootowner, euleredge myhead) {
    //cout<<"#"<<get_thread_id()<<" TellEndPart rootowner="<<rootowner<<" myhead="<<myhead<<endl;
    async_rmi(rootowner, this->getHandle(), &comm_telltails<EP>::GetFromStart, myhead);
  }
};

template<class EP>
void TellTails ( EP& eulerpath, PID rootowner, vector<euleredge>& tailv) {
  Location beforepart, nextpart; 
  comm_telltails<EP> commtail(eulerpath, tailv);
  stapl::rmi_fence();

  for (PARTID partid=0; partid<eulerpath.get_num_parts(); ++partid) {
    beforepart = eulerpath.GetDistribution().GetPartBdry(partid).first;
    nextpart = eulerpath.GetDistribution().GetPartBdry(partid).second;
    euleredge myhead = eulerpath.GetPart(partid)->begin()->GetUserData();
    //cout<<"#"<<get_thread_id()<<" part"<<partid<<" myhead="<<myhead<<" before=("<<beforepart.locpid()<<","<<beforepart.partid()<<")"<<endl;
    if (beforepart.locpid()==INVALID_PID || beforepart.partid()==INVALID_PART) {
      //first part, sends its firstee to last part by sending the ee to rootowner, 
      //and rootowner finds the last part(by deciding whether next link is invalid)
      if (nextpart.locpid()!=INVALID_PID && nextpart.partid()!=INVALID_PART) {
        commtail.TellEndPart(rootowner, myhead);
      }
    } else {
      commtail.TellBefore(beforepart, myhead);
    }
  }
  stapl::rmi_fence();
}

template <class PTREE, class EP>
void RootingTree(PTREE& ptree, map<VID, VID>& parentmap, EP& eulerpath, VID root=-1){
  /* 1. call pET to build eulerpath, without setting ranks */
  pET<PTREE,EP>(ptree, eulerpath, root, false);
  stapl::rmi_fence();

  /* 2. set all euleredge's "weight"(its rank value) to be 1 */
  typename EP::PRange epr(&eulerpath);
  eulerpath.get_prange(epr);
  stapl::rmi_fence();
  defaultScheduler s1;
  __setranks<EP>  setranks(1);
  p_for_all<typename EP::PRange, __setranks<EP>, defaultScheduler>(epr, setranks, s1);
  stapl::rmi_fence();
    
  /* 3. compute prefix sums on the pList, using Recurrence */
  IncreaseRank increase_rank; 
  Recurrence1<typename EP::PRange, IncreaseRank> (epr, increase_rank);
  stapl::rmi_fence();

  /* 4. Tell tails to befores */
  vector<euleredge> tailv(eulerpath.get_num_parts() );
  PID rootowner = ptree.Lookup(root).locpid(); //the owner of last part
  TellTails(eulerpath, rootowner, tailv);
  stapl::rmi_fence();
  //for (PARTID partid=0; partid<eulerpath.get_num_parts(); partid++) {
   // cout<<"#"<<get_thread_id()<<" tailv["<<partid<<"]="<<tailv[partid]<<endl;
  //} 
  //stapl::rmi_fence();

  /* 5. for each pair euleredges (u->v) and (v->u), compute prefix_sums(u->v) - prefix_sums(v->u) and put it in a hash map
   * pairhashmp which lets (u->v) and (v->u) mapped to the same position
   * and the value is set to prefix_sums(u->v) if meeting u->v first
   * and then set to prefix_sums(v->u)-prefix_sums(u->v)
   * for Rooting a Tree:
   *    we care about whether prefix_sums(u->v) < prefix_sums(v->u)
   *    so if the "-" > 0, we know parentmap[v]=u
   * for #of Descendants:
   *    we care about the prefix_sums(v->p[v]) - prefix_sums(p[v]->v)
   *    which equals to abs("-") here
   */
  ComparePairRanks<PTREE,EP>(ptree, eulerpath, tailv, parentmap, 1); //callerid=1 for RootingTree

  parentmap[root] = root;
  stapl::rmi_fence();
}

template <class PTREE, class EP>
void PostorderNumbering(PTREE& ptree, map<VID,int>& postnomap, EP& eulerpath, VID root=-1) {
  /* 1. call RootingTree to get parentmap */
  map<VID,VID> parentmap;
  RootingTree<PTREE,EP>(ptree, parentmap, eulerpath, root);
  //typename PTREE::VI vi;
  //for (vi=ptree.local_begin(); vi!=ptree.local_end(); ++vi) {
  //  cout<<"parent["<<vi->vid<<"]="<<parentmap[vi->vid]<<endl;
  //}
  //stapl::rmi_fence();

  /* 2. set rank(p[v]->v) = 0 and rank(v->p[v]) = 1 */
  typename EP::PRange epr(&eulerpath);
  eulerpath.get_prange(epr);
  stapl::rmi_fence();
  defaultScheduler s1;
  __setranks<EP> setranks(2, parentmap);
  p_for_all<typename EP::PRange, __setranks<EP>, defaultScheduler>(epr, setranks, s1);
  stapl::rmi_fence();
    
  /* 3. compute prefix sums on the pList, using Recurrence */
  IncreaseRank increase_rank; 
  Recurrence1<typename EP::PRange, IncreaseRank> (epr, increase_rank);
  stapl::rmi_fence();

  /* 4. Tell tails to befores */
  vector<euleredge> tailv(eulerpath.get_num_parts() );
  PID rootowner = ptree.Lookup(root).locpid(); //the owner of end part
  TellTails(eulerpath, rootowner, tailv);
  stapl::rmi_fence();
  //for (PARTID partid=0; partid<eulerpath.get_num_parts(); partid++) {
  //  cout<<"#"<<get_thread_id()<<" tailv["<<partid<<"]="<<tailv[partid]<<endl;
  //} 
  //stapl::rmi_fence();

  /* 5. for each euleredge, if it is v->p[v], then postno of v is the euleredge's rank */
  euleredge ee;
  for (PARTID partid=0; partid<eulerpath.get_num_parts(); partid++) {
    typename EP::pContainer_Part_type::iterator partit = eulerpath.GetPart(partid)->begin(); 
    while (1) {
      if (partit == eulerpath.GetPart(partid)->end() )  {
	if (get_num_threads() > 1) 
  	  ee = tailv[partid];
	else 
	  break;
      } else {
        ee = partit->GetUserData(); 
      }
      if (parentmap[ee.GetFirstVid()] == ee.GetSecondVid() ) 
 	postnomap[ee.GetFirstVid()] = ee.GetRank(); 
      if (partit == eulerpath.GetPart(partid)->end() ) break;
      ++partit;
    } 
  }
  postnomap[root] = ptree.size();
}

template <class PTREE, class EP>
void VertexLeveling(PTREE& ptree, map<VID,int>& levelmap, EP& eulerpath, VID root=-1) {
  /* 1. call RootingTree to get parentmap */
  map<VID,VID> parentmap;
  RootingTree<PTREE,EP>(ptree, parentmap, eulerpath, root);
  stapl::rmi_fence();
  //cout<<"#"<<get_thread_id()<<" inside VertexLeveling and after rootingtree, num_parts="<<eulerpath.get_num_parts()<<endl;
  //stapl::rmi_fence();

  /* 2. set rank(p[v]->v) = 1 and rank(v->p[v]) = -1 */
  typename EP::PRange epr(&eulerpath);
  eulerpath.get_prange(epr);
  stapl::rmi_fence();
  defaultScheduler s1;
  __setranks<EP> setranks(3, parentmap);
  p_for_all<typename EP::PRange, __setranks<EP>, defaultScheduler>(epr, setranks, s1);
  stapl::rmi_fence();
    
  /* 3. compute prefix sums on the pList, using Recurrence */
  IncreaseRank increase_rank; 
  Recurrence1<typename EP::PRange, IncreaseRank> (epr, increase_rank);
  stapl::rmi_fence();

  /* 4. Tell tails to befores */
  vector<euleredge> tailv(eulerpath.get_num_parts() );
  PID rootowner = ptree.Lookup(root).locpid(); //the owner of end part
  TellTails(eulerpath, rootowner, tailv);
  stapl::rmi_fence();
  //for (PARTID partid=0; partid<eulerpath.get_num_parts(); partid++) {
   // cout<<"#"<<get_thread_id()<<" tailv["<<partid<<"]="<<tailv[partid]<<endl;
  //} 
  //stapl::rmi_fence();

  /* 5. for each euleredge, if it is v->p[v], then level of v is the euleredge's rank */
  euleredge ee;
  for (PARTID partid=0; partid<eulerpath.get_num_parts(); partid++) {
    typename EP::pContainer_Part_type::iterator partit = eulerpath.GetPart(partid)->begin(); 
    while (1) {
      if (partit == eulerpath.GetPart(partid)->end() )  {
	if (get_num_threads() > 1) 
  	  ee = tailv[partid];
	else 
	  break;
      } else {
        ee = partit->GetUserData(); 
      }
      if (parentmap[ee.GetSecondVid()] == ee.GetFirstVid() ) 
 	levelmap[ee.GetSecondVid()] = ee.GetRank(); 
      if (partit == eulerpath.GetPart(partid)->end() ) break;
      ++partit;
    } 
  }
  levelmap[root]=0;
}

template <class PTREE, class EP>
void NumberDesc(PTREE& ptree,  map<VID,int>& sizemap, EP& eulerpath, VID root=-1) {
  /* 1. call RootingTree to get parentmap */
  map<VID,VID> parentmap;
  RootingTree<PTREE,EP>(ptree, parentmap, eulerpath, root);
  stapl::rmi_fence();
  //for (typename PTREE::VI vi=ptree.local_begin(); vi!=ptree.local_end(); ++vi) {
  //  cout<<"parent["<<vi->vid<<"]="<<parentmap[vi->vid]<<endl;
  //}
  //stapl::rmi_fence();

  /* 2. set rank(p[v]->v) = 0 and rank(v->p[v]) = 1 */
  typename EP::PRange epr(&eulerpath);
  eulerpath.get_prange(epr);
  stapl::rmi_fence();
  defaultScheduler s1;
  __setranks<EP> setranks(4, parentmap);
  p_for_all<typename EP::PRange, __setranks<EP>, defaultScheduler>(epr, setranks, s1);
  stapl::rmi_fence();
    
  /* 3. compute prefix sums on the pList, using Recurrence */
  IncreaseRank increase_rank; 
  Recurrence1<typename EP::PRange, IncreaseRank> (epr, increase_rank);
  stapl::rmi_fence();

  /* 4. Tell to before */
  vector<euleredge> tailv(eulerpath.get_num_parts() );
  PID rootowner = ptree.Lookup(root).locpid(); //the owner of last part
  TellTails(eulerpath, rootowner, tailv);
  stapl::rmi_fence();
  //for (PARTID partid=0; partid<eulerpath.get_num_parts(); partid++) {
   // cout<<"#"<<get_thread_id()<<" tailv["<<partid<<"]="<<tailv[partid]<<endl;
  //} 
  //stapl::rmi_fence();

  /* 5. for each pair euleredges (u->v) and (v->u), compute prefix_sums(u->v) - prefix_sums(v->u) and put it in a hash map
   * pairhashmp which lets (u->v) and (v->u) mapped to the same position
   * and the value is set to prefix_sums(u->v) if meeting u->v first
   * and then set to prefix_sums(v->u)-prefix_sums(u->v)
   * for Rooting a Tree:
   *    we care about whether prefix_sums(u->v) < prefix_sums(v->u)
   *    so if the "-" > 0, we know parentmap[v]=u
   * for #of Descendants:
   *    we care about the prefix_sums(v->p[v]) - prefix_sums(p[v]->v)
   *    which equals to abs("-") here
   */
  ComparePairRanks<PTREE, EP>(ptree, eulerpath, tailv, sizemap, 4); //callerid=4 for NumberDesc

  sizemap[root] = ptree.size();

}

}
