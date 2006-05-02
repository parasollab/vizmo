//#undef STAPL_DEBUG
#include <runtime.h>
#include "Defines.h"
#include "task.h"
#include <assert.h>

#include "pGraph.h"
#include "GraphPartitioner.h"

#include <set>


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
  bool passed=true;

  int myid=stapl::get_thread_id();
  NP=stapl::get_num_threads();

  if(argc < 3){
    if(myid==0){
      cout<<"Invalid arguments:"<<endl;
      cout<<"./test_partition input_size input_file"<<endl;
    }
    return;
  }


  SZ = atoi(argv[1]);
  //printf("SIZE %d\n",SZ);
  //printf("Reading graph from file %s\n",argv[2]);


  stapl::timer tm;
  double elapsed;

  PGRAPH ppg,ppcrude;
  stapl::rmi_fence();

  stapl::GraphPartitioner<UGRAPH> GP;

  UGRAPH _ug;
  _ug.ReadDotGraph(argv[2]);
  map<VID,PID> _pp;
  GP.MetisPartition(_ug,_pp);

  map<PID,vector<VID> > partition;
  vector<int> where;
  for(int p=0;p<NP;p++){
    partition[p] = where;
  }

  GP.MetisPartition(_ug,partition);
  ppg.ReadDotGraph(argv[2],partition);
  ppcrude.Split_Crude_Dot<GRAPH>(argv[2],_pp,0);
  //compare if the tho pgraphs are the same;
  PGRAPH::VI it2 = ppcrude.local_begin();
  for(PGRAPH::VI it = ppg.local_begin();it != ppg.local_end();it++){
    assert(it->vid == it2->vid);
    PGRAPH::EI ei2 = it2->edgelist.begin();
    for(PGRAPH::EI ei = it->edgelist.begin();ei != it->edgelist.end();ei++){
      if(ei->vertex2id != ei2->vertex2id) passed = false;
      ei2++;
    }
    it2++;
  }

  if(passed)
    cout<<"Testing metis partitioner...Passed"<<endl;
  else
    cout<<"Testing metis partitioner...Failed"<<endl;
  //ppg.pDisplayGraph();
  //ppcrude.pDisplayGraph();
  stapl::rmi_fence();
  return;
}



