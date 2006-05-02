//#undef STAPL_DEBUG
#include <runtime.h>
#include "Defines.h"
#include "task.h"
#include <assert.h>
#include "Graph.h"
#include "pGraph.h"

using namespace stapl;

int NP,SZ;

void stapl_print(const char* s){
  if(stapl::get_thread_id() == 0)
    cout << s << flush;
}

void stapl_main(int argc,char** argv){

  char crude[100];
  char dummy[100];

  typedef Graph<DG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;
  typedef Graph<UG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> UGRAPH;

  typedef pGraph<PDG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PGRAPH;


  Task data,data2;
  Weight2 wt;

  int i,j,tmp,localedges;   
  int id=stapl::get_thread_id();
  vector<int> vertices;
  NP=stapl::get_num_threads();
  int myid=stapl::get_thread_id();
  PGRAPH::VI vi;
  PGRAPH::EI ei;
  PGRAPH::CVI cvi;

  PGRAPH pg;
  stapl::rmi_fence();

  printf("main started\n");

  SZ = atoi(argv[1]);
  printf("SIZE %d\n",SZ);
  stapl_print("testing AddVertex...");
  for(i=0;i<SZ;i++){
    tmp = i+myid*SZ;
    data.SetWeight(tmp);
    j = pg.AddVertex(data);
    vertices.push_back(j);
  }
  stapl::rmi_fence();
  //here we should have NP*SZ vertices
  for (i=0;i<SZ*NP;i++){
    if(!pg.IsVertex(i)){
      cout<<"ERROR while testing AddVertex/IsVertex"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  localedges = 0;
  stapl_print("testing AddEdge...");
  for (i=0;i<SZ;i++){
      for (j=SZ*NP-1;j>=0;j--){
	tmp = vertices[i] * j;
	wt.SetWeight(tmp);
	pg.AddEdge(vertices[i],j,wt);
	localedges++;
      }
  }
  stapl::rmi_fence();
  for (i=0;i<SZ;i++){
      for (j=SZ*NP-1;j>=0;j--){
	if(!pg.IsEdge(vertices[i],j))
	  cout<<"ERROR while testing AddEdge/IsEdge"<<endl;
      }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");
  //pg.pDisplayGraph();

  stapl_print("start testing GetVID...");
  for (i=0;i<SZ*NP;i++){
    data.SetWeight(i);
    tmp = pg.GetVID(data);
    data = pg.GetData(tmp);
    if(data.GetWeight() != i){
      cout<<"ERROR while testing GETVID()"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("start testing IsVertex(VERTEX,VI)...");
  for(i=0;i<SZ;i++){
    tmp = i+myid*SZ;
    data.SetWeight(tmp);
    if(!pg.IsVertex(data,&vi)){
      cout<<"ERROR while testing IsVertex()"<<endl;
    }
    else{
      if(vi->data.GetWeight() != tmp ){
	cout<<"ERROR while testing IsVertex()(2)"<<endl;
      }
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("start testing IsVertex(VERTEX,CVI)...");
  for(i=0;i<SZ;i++){
    tmp = i+myid*SZ;
    data.SetWeight(tmp);
    if(!pg.IsVertex(data,&cvi)){
      cout<<"ERROR while testing IsVertex(cvi)"<<endl;
    }
    else{
      if(cvi->data.GetWeight() != tmp ){
	cout<<"ERROR while testing IsVertex(cvi)(2)"<<endl;
      }
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("start testing IsEdge(VERTEX,VERTEX)...");
  for (i=0;i<SZ;i++){
    data.SetWeight(i);
    for (j=SZ*NP-1;j>=0;j--){
      data2.SetWeight(j);
      if(!pg.IsEdge(data,data2))
	cout<<"ERROR while testing IsEdge"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");
    
  stapl_print("start testing IsEdge(vid,vid,WEIGHT)...");
  for (i=0;i<SZ*NP-1;i++){
    for (j=SZ*NP-1;j>=0;j--){
      tmp = i*j;
      wt.SetWeight(tmp);
      if(!pg.IsEdge(i,j,wt))
	cout<<"ERROR while testing IsEdge(vid,vid,weight)"<<endl;
    }
  }
  stapl::rmi_fence();
  for (i=0;i<SZ*NP-1;i++){
    for (j=SZ*NP-1;j>=0;j--){
      tmp = -1;
      wt.SetWeight(tmp);
      if(pg.IsEdge(i,j,wt))
	cout<<"ERROR while testing IsEdge(vid,vid,weight)(2)"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");
  
  stapl_print("start testing IsEdge(VERTEX,VERTEX,WEIGHT)...");
  int k=0;
  for (i=0;i < NP;i++){
    tmp=k;
    data.SetWeight(tmp);
    for (j=SZ*NP-1;j>=0;j--){
      data2.SetWeight(j);
      tmp = k*j;
      wt.SetWeight(tmp);
      if(!pg.IsEdge(data,data2,wt))
	cout<<"ERROR while testing IsEdge(VERTEX,VERTEX,WEIGHT)"<<endl;
    }
  }
  stapl::rmi_fence();
  for (i=0;i<SZ*NP-1;i++){
    data.SetWeight(i);
    for (j=SZ*NP-1;j>=0;j--){
      data2.SetWeight(j);
      tmp = -1;
      wt.SetWeight(tmp);
      if(pg.IsEdge(data,data2,wt))
	cout<<"ERROR while testing IsEdge(VERTEX,VERTEX,WEIGHT)(2)"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("start testing IsEdge(VERTEX,VERTEX,WEIGHT,VI,EI)...");
  stapl::rmi_fence();
  for (i=0;i<SZ;i++){
    tmp = i+myid*SZ;
    data.SetWeight(tmp);
    for (j=0;j<SZ;j++){
      tmp = j+myid*SZ;
      data2.SetWeight(tmp);
      tmp = vertices[i]*vertices[j];
      wt.SetWeight(tmp);
      if(!pg.IsEdge(data,data2,wt,&vi,&ei)){
	cout<<"ERROR while testing IsEdge(VERTEX,VERTEX,WEIGHT,VI,EI)"<<endl;
	cout<<vertices[i]<<":"<<j<<":"<<tmp<<":"<<vi->vid<<":"<<ei->weight.GetWeight()<<endl;
      }
      else{
	if(vi->vid != vertices[i])
	  cout<<"ERROR while testing IsEdge(VERTEX,VERTEX,WEIGHT,VI,EI)(2)"<<endl;
	if(ei->weight.GetWeight() != tmp)
	  cout<<"ERROR while testing IsEdge(VERTEX,VERTEX,WEIGHT,VI,EI)(3)"<<endl;
      }
   }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");


  stapl_print("start testing GetEdgeWeight(VID,VID)...");
  for (i=0;i<SZ*NP-1;i++){
    for (j=SZ*NP-1;j>=0;j--){
      tmp = i*j;
      wt = pg.GetEdgeWeight(i,j);
      if( wt.GetWeight() != tmp)
	cout<<"ERROR while testing GetEdgeWeight"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("start testing GetEdgeWeight(VERTEX,VERTEX)...");
  for (i=0;i<SZ*NP;i++){
    data = pg.GetData(i);
    for (j=0;j<SZ*NP;j++){
      data2 = pg.GetData(j);
      tmp = i*j;
      wt = pg.GetEdgeWeight(data,data2);
      if( wt.GetWeight() != tmp){
	cout<<"ERROR while testing GetEdgeWeight(VERTEX,VERTEX)"<<endl;
	cout<<i<<":"<<j<<":"<<wt.GetWeight()<<endl;
      }
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");
  return;
}
