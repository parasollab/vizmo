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
#include "../pGraphTraversal.h"

#include "../../pvector/pvector.h"

#include "../pSCC.h"

#include "../pSCCsequential.h"
#include "../GraphPartitioner.h"

#include <set>

//#include <sys/time.h>

using namespace stapl;

int SZ,NP;

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

  cout<<"main started\n";

  SZ = atoi(argv[1]);
  printf("SIZE %d\n",SZ);
  printf("Reading graph from file %s\n",argv[2]);

  stapl::timer tm;
  double elapsed;

  PGRAPH ppg;
  stapl::rmi_fence();

  stapl::GraphPartitioner<UGRAPH> GP;

  //UGRAPH _ug;
  //_ug.ReadExoGraph(argv[2]);
  map<VID,PID> _pp;
  map<PID,vector<VID> > _vpp;

  //               Different partitions can be used
  //GP.MetisPartition(_ug,_pp);
  //GP.ChacoPartition(SZ,"c22_0",_pp);
  GP.BlockPartition(SZ,_pp);
  //GP.ModuloPartition(SZ,_pp);

  //               READ different graph formats
  //ppg.ReadExoGraph(argv[2], _vpp);
  ppg.Split_Crude_Exo<GRAPH>(argv[2], _pp);
  //ppg.pDisplayGraph();

  stapl::rmi_fence();

  cout<<"pSCC sequential started"<<endl;
  /*
   *                                  PSCC setup 
  */

  vector<trimerSequential<PGRAPH>* > vtr;
  trimerSequential<PGRAPH> tr1(&ppg);
  trimerSequential<PGRAPH> tr2(&ppg);
    
  tr1.sccg.SetTraversalNumber(NP);
  stapl::rmi_fence();

  vtr.push_back(&tr1);
  vtr.push_back(&tr2);

  stapl::pSCCsequential<PGRAPH> pscc1;

  stapl::rmi_fence();
  vector<vector<pair<VID,int> > > SCC;

  tm = stapl::start_timer();

  pscc1.run(vtr, SCC ,10);

  elapsed = stapl::stop_timer(tm);
  cout<<"1 run  time :"<<elapsed<<endl;

  pscc1.collect(SCC,2,SZ);
  stapl::rmi_fence();
  pscc1.dump("res1",1);
  cout<<"SCC size"<<SCC[0].size()<<endl;  
  stapl::rmi_fence();
  return;
}
