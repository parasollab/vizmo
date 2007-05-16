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



template <class G>
class visitor: public visitor_base<G>{
  //this class is used by DFS/BFS
  G* _g;
public:  
  visitor(G& g){
      _g = &g;
  };
  inline int vertex (typename G::VI v) {
    printf("--------> %d %d\n",v->vid,_g->myid);
    return 1;
  }
};


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

  //GP.MetisPartition(_ug,_pp);
  GP.BlockPartition(SZ,_pp);

  g.Split_Crude_Dot<GRAPH>(argv[2], _pp ,0);


  stapl::rmi_fence();
  //iterate trough vertices of the graph;
  PGRAPH::VI pvi;
  PGRAPH::EI pei;

  double dd = g.pGetVertexField(0,&Task::GetWeight);

  dd = g.pGetWeightField(0,
			 1,
			 &Weight2::GetWeight1);


  g.pSetWeightField(0,
		    1,
		    &Weight2::SetWeight,
		    99);

  stapl::rmi_fence();

  dd = g.pGetWeightField(0,
                         1,
                         &Weight2::GetWeight2,1);

  cout<<"REad value"<<id<<":"<<dd<<endl;
  dd = g.pGetWeightField(0,
			 2,
                         &Weight2::GetWeight2,1);
  cout<<"REad value"<<id<<":"<<dd<<endl;
  dd = g.pGetWeightField(1,
			 3,
                         &Weight2::GetWeight1);
  cout<<"REad value"<<id<<":"<<dd<<endl;

 
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



