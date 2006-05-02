#include <assert.h>
#include <runtime.h>
#include "Defines.h"
#include "Graph.h"
#include "pGraph.h"

#include "task.h"

#include "../GraphPartitioner.h"

using namespace stapl;

int SZ,NP;
 
int getOwner(VID _v){
  return (_v % NP);
}


void stapl_main(int argc,char** argv){

  char crude[10];
  char dummy[10];
  typedef Graph<DG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;
  typedef pGraph<PDG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PGRAPH;
  typedef Graph<UG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> UGRAPH;

  int id=stapl::get_thread_id();
  NP=stapl::get_num_threads();
  sprintf(crude,"crude%d",id); 
  sprintf(dummy,"dummy%d",id);
  printf("main started\n");

  PGRAPH g;
  stapl::rmi_fence();
  Task data;
  int j; 


  SZ = atoi(argv[1]);
  printf("SIZE %d\n",SZ);
  printf("Reading graph from file %s\n",argv[2]);

  stapl::GraphPartitioner<UGRAPH> GP;
  UGRAPH _ug;
  _ug.ReadDotGraph(argv[2]);
  map<VID,PID> _pp;
  GP.BlockPartition(SZ,_pp);

  g.Split_Crude_Dot<GRAPH>(argv[2], _pp ,0);


  stapl::rmi_fence();

  //iterate trough vertices of the graph;
  PGRAPH::VI pvi;
  PGRAPH::CVI cpvi;
  PGRAPH::EI pei;
  PGRAPH::CEI cpei;

  //testing IsVertex(gid);
  assert(g.IsVertex(0));
  assert(g.IsVertex(1));
  assert(g.IsVertex(2));
  assert(g.IsVertex(3)); 

  assert(g.IsVertex(4));
  assert(g.IsVertex(5));
  assert(g.IsVertex(6));
  assert(g.IsVertex(7));

  assert(g.IsVertex(8));
  assert(g.IsVertex(9));
  assert(g.IsVertex(10));
  assert(g.IsVertex(11));

  assert(g.IsVertex(12));
  assert(g.IsVertex(13));
  assert(g.IsVertex(14));
  assert(g.IsVertex(15));

  //testing IsVertex(gid,vi*)  
  if(g.IsVertex(0,&pvi)){
    assert(pvi->vid == 0);
  }
  if(g.IsVertex(15,&pvi)){
    assert(pvi->vid == 15);
  }
  if(g.IsVertex(1,&pvi)){
    assert(pvi->vid == 1);
  }
  if(g.IsVertex(14,&pvi)){
    assert(pvi->vid == 14);
  }
  if(g.IsVertex(6,&pvi)){
    assert(pvi->vid == 6);
  }
/*
  //testing IsVertex(gid,cvi*)  
  if(g.IsVertex(1,&cpvi)){
    assert(cpvi->vid == 1);
  }

  if(g.IsVertex(13,&cpvi)){
    assert(cpvi->vid == 13);
  }
  if(g.IsVertex(2,&cpvi)){
    assert(cpvi->vid == 2);
  }
  if(g.IsVertex(12,&cpvi)){
    assert(cpvi->vid == 12);
  }
  if(g.IsVertex(5,&cpvi)){
    assert(cpvi->vid == 5);
  }
*/
  //testing IsEdge(vid1, vid2);
  assert(g.IsEdge(0,1));
  assert(g.IsEdge(0,2));
  assert(g.IsEdge(14,15));
  
  if(g.IsEdge(0,1,&pvi,&pei)){
    assert(pvi->vid == 0 && pei->vertex2id == 1);
  }

  if(g.IsEdge(14,15,&pvi,&pei)){
    assert(pvi->vid == 14 && pei->vertex2id == 15);
  }


  //testing IsEdgeId
  if(g.IsEdgeId(0,0,&pvi,&pei)){
    assert(pvi->vid == 0 && pei->vertex2id == 1);
  }

  if(g.IsEdgeId(14,0,&pvi,&pei)){
    assert(pvi->vid == 14 && pei->vertex2id == 15);
  }
  stapl::rmi_fence();
  //testing DeleteEdgeId
  int i=14;
  g.DeleteEdgeId(i,0);
  g.DeleteEdgeId(15,0);
  g.DeleteEdgeId(15,2);
  stapl::rmi_fence();

  assert (! g.IsEdgeId(i,0,&pvi,&pei));

  //g.pDisplayGraph();
  stapl::rmi_fence();

  double dd = g.pGetVertexField(0,&Task::GetWeight);
  dd = g.pGetWeightFieldEdgeId(0,
			 1,
			 &Weight2::GetWeight1);

  int nn=99;
  double du=1.0;

  g.pSetWeightFieldEdgeId(0,
			  1,
			  (int (Weight2::*)(int&))&Weight2::SetWeight,
			  nn);

  g.pSetWeightFieldEdgeId(0,
		    0,
		    &Weight2::SetWeight,
		    nn,du);

  g.pSetWeightFieldEdgeId(1,
		    0,
		    &Weight2::SetWeight,
		    nn,du,(long)nn);

  stapl::rmi_fence();

  dd = g.pGetWeightFieldEdgeId(0,
			       1,
			       &Weight2::GetWeight2,nn);

  cout<<"REad0 value"<<id<<":"<<dd<<endl;
  dd = g.pGetWeightFieldEdgeId(0,
			       1,
			       &Weight2::GetWeight2,nn);
  cout<<"REad1 value"<<id<<":"<<dd<<endl;
  dd = g.pGetWeightFieldEdgeId(1,
			       0,
			       &Weight2::GetWeight1);
  cout<<"REad2 value"<<id<<":"<<dd<<endl;

  dd = g.pGetWeightField(1,
			 1,
                         &Weight2::GetWeight3,nn,(long)nn);
  cout<<"REad3 value"<<id<<":"<<dd<<endl;
 
  stapl::rmi_fence();
  //stapl::rmi_fence();

  //the next code tests two different constructors for pIterator
  //and =, !=,  ++ and -> operators  
  
  for(pvi = g.begin();pvi !=g.end();pvi++){
    cout << pvi->data <<" :";
    for(pei = pvi->edgelist.begin();pei != pvi->edgelist.end();pei++){
      cout<<"["<<pei->vertex2id<<","<<pei->weight<<"] ";
    }
    cout<<endl;
  }

  //g.WriteGraph(crude);  
}



