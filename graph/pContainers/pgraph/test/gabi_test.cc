//#undef STAPL_DEBUG
#include <runtime.h>
#include "Defines.h"
#include "task.h"
#include <assert.h>
#include "Graph.h"
#include "../pGraph.h"
#include "../pBaseGraph.h"
//#include "../GraphPart.h"
#include "../pGraphAlgo.h"

#include "../../pvector/pvector.h"

//#include "../pSCCsequential.h"
//#include "../pSCCmulticc.h"
#include "../GraphPartitioner.h"

#include <set>

//#include <sys/time.h>

using namespace stapl;

int SZ,NP;

int getOwner(VID _v){
  //return (_v % NP);

  //for m91
  if(NP==2){
    //--- np = 2
    if(_v < SZ/2) return 0;
    else return 1;
  }
  else if(NP==4){
    //- np=4
    if(_v < SZ/4) return 0;
    else if (_v < 2*SZ/4) return 1;
    else if (_v < 3*SZ/4) return 2;
    else return 3;
  }
  else if(NP==6){
    //- np = 6
    if(_v < SZ/6) return 0;
    else if (_v < 2*SZ/6) return 1;
    else if (_v < 3*SZ/6) return 2;
    else if (_v < 4*SZ/6) return 3;
    else if (_v < 5*SZ/6) return 4;
    else return 5;
  }
  else if(NP==8){
    //- np = 8
    if(_v < SZ/8) return 0;
    else if (_v < 2*SZ/8) return 1;
    else if (_v < 3*SZ/8) return 2;
    else if (_v < 4*SZ/8) return 3;
    else if (_v < 5*SZ/8) return 4;
    else if (_v < 6*SZ/8) return 5;
    else if (_v < 7*SZ/8) return 6;
    else return 7;
  }
  else return 0;
}
/*
template <class G>
class visitor: public visitor_base<G>{
  //this class is used by DFS/BFS
  //G* _g;
public:  
  //visitor(){
    //_g = &g;
  //}
  inline int vertex (typename G::VI v) {
    timeval tv;
    gettimeofday(&tv,NULL);
    printf("@@ %d\n",v->vid);
    return 1;
  }

  void define_type(stapl::typer &t) {
    //cout<<"Ask Nathan if this is correct"<<endl;
    //t.local(_g);
  }

};

class A{
public:
  virtual void Afoo(){
    cout<<"virtual A foo"<<endl;
  }
};

class B: public A, public BasePObject{
public:
  
  B(){}
  virtual void Bfoo(){
    printf("foo B");
    //Afoo();
  }
  void Brmi(){
    async_rmi(1,this->getHandle(),&B::Bfoo);
  }
};

class D1:public virtual B{
public:
  D1(){}
  void D1foo(){
    printf("foo D1\n");
  }
  void D1rmi(){
    this->Brmi();
    async_rmi(1,this->getHandle(),&D1::D1foo);
  }
};


class D2:public virtual B{
public:
  D2(){}
  void D2foo(){
    printf("foo D2\n");
  }
};
 
//template <class PG> 
class F: public D1, public D2 {
public:
  F(){
    this->register_this(this);
  }

 void run(){
    typedef pGraph<PDG<taskSCC,dummyw>,PMG<taskSCC,dummyw>, PWG<taskSCC,dummyw>, taskSCC,dummyw> PGRAPH_SCC;
    PGRAPH_SCC npg;
    visitor<PGRAPH_SCC>  vs3;
    typedef visitor<PGRAPH_SCC> VISI;
    npg.BFS_traversal<VISI>(0,vs3,0,true);
  }
};

*/

void stapl_print(const char* s){
  if(stapl::get_thread_id() == 0)
    cout << s << flush;
}

void stapl_main(int argc,char** argv){

  char crude[100];
  char dummy[100];

  typedef Graph<DG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;
  typedef Graph<UG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> UGRAPH;

  typedef pGraph<PUG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PGRAPH;


  Task data;
  Weight2 wt;


  int i=2;   
  int myid=stapl::get_thread_id();
  NP=stapl::get_num_threads();

  printf("main started\n");

  SZ = atoi(argv[1]);
  printf("SIZE %d\n",SZ);
  printf("Reading graph from file %s\n",argv[2]);

  stapl::timer tm;
  double elapsed;
  PGRAPH ppg;
  stapl::rmi_fence();

  stapl::GraphPartitioner<UGRAPH> GP;

  UGRAPH _ug;
  _ug.ReadDotGraph(argv[2]);
  map<VID,PID> _pp;
  map<PID,vector<VID> > _vpp;
  //GP.MetisPartition(_ug,_pp);

  //GP.ChacoPartition(SZ,"c22_0",_pp);
  //GP.BlockPartition(SZ,_pp);
  GP.ModuloPartition(SZ,_pp);
  //ppg.pFileRead(argv[2], 10);

  ppg.Split_Crude_Dot<GRAPH>(argv[2], _pp);
  //ppg.pDisplayGraph();

  
  ppg.SetTraversalNumber(NP);
  ppg.ResetTraversals();
  stapl::rmi_fence();
  stapl_print("testing GetCCcount...");
  pGetCCcount<PGRAPH> pgccc;
  cout<<"thread id"<<myid<<":"<<pgccc.GetCCcount(ppg)<<endl;
  stapl::rmi_fence();
  stapl_print("OK\n");

  stapl_print("testing GetCCstats...");
  pvector<pair<int,VID> > stats;
  pGetCCStats<PGRAPH> pgs;
  cout<<"STATS id"<<myid<<":"<<pgs.GetCCStats(ppg,stats)<<endl;
  //printing the 
  pDisplayCCStats(ppg,pgs);
  //stapl::rmi_fence();
  stapl_print("OK\n");

  stapl_print("testing pDisplayCCstats...");
  pDisplayCCStats(ppg,pgs);
  stapl_print("OK\n");

  stapl_print("testing pDisplayCC...");
  pGetCC<PGRAPH> getCC;
  pDisplayCC(ppg,getCC,ppg.local_begin()->vid);
  stapl_print("OK\n");

  stapl::rmi_fence();
  cout<<"finished id"<<myid<<endl;
  return;

  //                  The End
  //--------------------------------------------

  //ppg.pSetPredecessors();
  
  //visitor<PGRAPH> vis1;

  vector<PGRAPH*> vpgr;
  vpgr.push_back(&ppg);

  //vpgr.push_back(ppg);

  //vector<visitor<PGRAPH>* > vvis;
  //vvis.push_back(&vis1);
  //vvis.push_back(vis1);

  vector<vector<int> > vids(1);
  vids[0].push_back(946);
  //vids.push_back(0);

  stapl::rmi_fence();
  //                     traversals

  GRAPH sg;
  tm = stapl::start_timer();


  elapsed = stapl::stop_timer(tm);
  cout<<"serial run  time :"<<elapsed<<endl;

  tm = stapl::start_timer();


  elapsed = stapl::stop_timer(tm);
  cout<<"p run  time :"<<elapsed<<endl;



  //pBFS<PGRAPH,visitor<PGRAPH> > (vpgr,vvis,vids);

  //pBFSEQReverse<PGRAPH,visitor<PGRAPH> > (vpgr,vvis,vids);

  //pTOPOReverse<PGRAPH,visitor<PGRAPH> > (vpgr,vvis);

  //pBFS<PGRAPH,visitor<PGRAPH> > (ppg,vis1,0);
  //pBFSReverse<PGRAPH,visitor<PGRAPH> > (ppg,vis1,946);

  //pBFSEQReverse<PGRAPH,visitor<PGRAPH> > (ppg,vis1,946);

  //p_BFS_pRange_EQ<PGRAPH,visitor<PGRAPH> > (vpgr,vvis,vids);
  //p_TOPOReverse_pRange<PGRAPH,visitor<PGRAPH> > (vpgr,vvis);

  stapl::rmi_fence();
  //return;
  //ppg.WarmCache();
  vector<int> start_verts;
  stapl::rmi_fence();
  ppg.SetTraversalNumber(NP);
  ppg.pSetPredecessors();
  ppg.SetDependencies(0);
  stapl::rmi_fence();

  PGRAPH::VI pit = ppg.local_begin();
  PGRAPH::EI pei = pit->edgelist.begin();
  //cout<<*pit;

  printf("testing the size aspects of the pGraph\n");
  
  printf("Size of Graph %d\n",ppg.size());


  stapl::rmi_fence();



  //the test code for sequential distributed version of pSCC
  cout<<"pSCC sequential started"<<endl;
  /*
   *                                  PSCC setup 
  */
  //traversal

  /*
  vector<trimer<PGRAPH>* > vtr;
  //vector<trimerSequential<PGRAPH>* > vtr;

  trimer<PGRAPH> tr1(&ppg);
  trimer<PGRAPH> tr2(&ppg);
  //trimerSequential<PGRAPH> tr1(&ppg);
  
  tr1.sccg.SetTraversalNumber(NP);
  tr2.sccg.SetTraversalNumber(NP);
  stapl::rmi_fence();

  vtr.push_back(&tr1);
  //vtr.push_back(&tr2);

  //pSCC<PGRAPH,trimer<PGRAPH> > pscc1;

  //pSCCExecutor<PGRAPH> pscc1;
  pSCCmulticc<PGRAPH> pscc1;
  //stapl::pSCCsequential<PGRAPH> pscc1;

  stapl::rmi_fence();
  vector<vector<pair<VID,int> > > SCC;

  tm = stapl::start_timer();
  pscc1.run(vtr, SCC ,10,false);
  elapsed = stapl::stop_timer(tm);
  cout<<"1 run  time :"<<elapsed<<endl;

  //pscc1.collect(SCC,1,SZ);
  //stapl::rmi_fence();
  //pscc1.dump("res",0);

  //compact the scc

  pscc1.compactSCC(vtr, SCC);
  */

  /*
   *prepare the trimer to be passed to the pSCC
   */

  /*
  vtr.clear();
  trimer<PGRAPH> tr2(&ppg);
  //trimerSequential<PGRAPH> tr1(&ppg);
  
  tr2.sccg.SetTraversalNumber(NP);
  stapl::rmi_fence();

  vtr.push_back(&tr2);
  stapl::rmi_fence();

  tm = stapl::start_timer();

  pscc1.run(vtr, SCC ,10,true);

  elapsed = stapl::stop_timer(tm);
  cout<<"2run  time :"<<elapsed<<endl;
  */

  /*
  pscc1.collect(SCC,1,SZ);
  stapl::rmi_fence();
  pscc1.dump("res1",0);
  cout<<"SCC size"<<SCC[0].size()<<endl;  
  stapl::rmi_fence();
  return;
  */
}
