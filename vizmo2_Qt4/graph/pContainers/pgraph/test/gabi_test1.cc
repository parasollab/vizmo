//#undef STAPL_DEBUG
#include <runtime.h>
#include "Defines.h"
#include "task.h"
#include <assert.h>
#include "Graph.h"
#include "../pGraph.h"
#include "../pBaseGraph.h"
#include "../GraphPart.h"
#include "../pGraphAlgo.h"


#include "../../pvector/pvector.h"

//#include "../pSCCgeneric.h"
//#include "../pSCCsequential.h"

#include "../pSCCmulticc.h"
#include "../pSCCasci.h"

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
    printf("%d --> %d %d\n",tv.tv_usec,v->vid,stapl::get_thread_id());
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



void stapl_main(int argc,char** argv){

  char crude[100];
  char dummy[100];

  typedef Graph<DG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;
  typedef Graph<UG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> UGRAPH;

  typedef pGraph<PDG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PGRAPH;


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
  //ppg.AddVertex(data,myid);

  stapl::rmi_fence();

  stapl::GraphPartitioner<UGRAPH> GP;

  UGRAPH _ug;
  //_ug.ReadExoGraph(argv[2]);
  map<VID,PID> _pp;
  //GP.MetisPartition(_ug,_pp);
  //GP.ChacoPartition(SZ,"c22_0",_pp);
  GP.BlockPartition(SZ,_pp);
  //ppg.Split_Crude_Dot<GRAPH>(argv[2], _pp ,0);
  ppg.pFileRead(argv[2],10);

  stapl::rmi_fence();
  ppg.SetTraversalNumber(NP);
  //ppg.pSetPredecessors();
  ppg.SetDependencies(0);
  stapl::rmi_fence();

  /*
   *                                  PSCC setup 
  */

  vector<trimer<PGRAPH>* > vtr;
  //vector<trimerSequential<PGRAPH>* > vtr;

  trimer<PGRAPH> tr1(&ppg);
  //trimerSequential<PGRAPH> tr1(&ppg);
  
  tr1.sccg.SetTraversalNumber(NP);
  stapl::rmi_fence();

  vtr.push_back(&tr1);

  //pSCC<PGRAPH,trimer<PGRAPH> > pscc1;
  stapl::pSCCasci<PGRAPH> pscc1;
  //stapl::pSCCmulticc<PGRAPH> pscc1;
  //stapl::pSCCsequential<PGRAPH> pscc1;

  stapl::rmi_fence();
  //vector<vector<pair<VID,int> > > SCC;

  vector<map<VID,vector<VID> > >  SCC;
  vector<vector<pair<VID,VID> > > Edges;

  pSCCWorkFunction<PGRAPH> pf;

  tm = stapl::start_timer();
  pscc1.run(vtr, Edges, SCC, &pf,50, false);
  // pscc1.run(vtr,SCC,50,false);
  elapsed = stapl::stop_timer(tm);
  cout<<"1 run  time :"<<elapsed<<endl;

  pscc1.collect(SCC,1,SZ);
  stapl::rmi_fence();
  pscc1.dump("res",0);

  //compact the scc

  //pscc1.compactSCC(vtr, SCC);
  //pscc1.collect(SCC,1,SZ);
  stapl::rmi_fence();

  //pscc1.dump("res1",0);
  //cout<<"SCC size"<<SCC[0].size()<<endl;  
  stapl::rmi_fence();
  return;
}
