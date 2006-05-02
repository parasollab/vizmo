/*!
        \file test_pbasegraph.cc
	\author Gabriel Tanase	
        \date Feb. 16, 04
        \ingroup stapl
        \brief Test suite for pBaseGraph class. All methods from pGraph class instantiated
	\and tested for correctness. These are mainly methods to add/get vertices, edges
*/
#include <runtime.h>
#include "Defines.h"
#include "task.h"
#include <assert.h>
#include "Graph.h"
#include "pGraph.h"
#include "set"

#include <sys/time.h>

using namespace stapl;

int SZ,NP;

void stapl_print(const char* s){
  if(stapl::get_thread_id() == 0)
    cout << s << flush;
}


void stapl_main(int argc,char** argv){

  typedef Graph<DG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;
  typedef Graph<UG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> UGRAPH;
  typedef pGraph<PDG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PGRAPH;


  Task data;
  Weight2 wt;
  PGRAPH::VI vi1,vi2,vi3;
  PGRAPH::EI ei1,ei2;
  PGRAPH::CVI cvi1;
  int i,j,localedges;   
  int id=stapl::get_thread_id();
  vector<int> vertices;
  NP=stapl::get_num_threads();

  stapl_print("main started\n");

  SZ = atoi(argv[1]);

  stapl_print("testing default pGraph  constructor...");
  PGRAPH pg,pg2;
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing pGraph properties...");
  stapl_assert(pg.IsDirected(),"is directed failed");
  stapl_assert(pg.IsMulti(),"is multi failed");
  stapl_assert(pg.IsWeighted(),"is weighted failed");
  stapl_print("Passed\n");

  stapl_print("testing AddVertex...");
  for(i=0;i<SZ;i++){
    data.SetWeight(i);
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
  //pg.pDisplayGraph();
  //stapl::rmi_fence();
  //return;

  localedges = 0;
  stapl_print("testing AddEdge...");
  for (i=0;i<SZ;i++){
      for (j=SZ*NP-1;j>=0;j--){
	wt.SetWeight(vertices[i] * j);
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

  stapl_print("testing empty,local_empty ...");
  if(pg.empty()) cout<<"ERROR while testing empty"<<endl;
  if(pg.local_empty()) cout<<"ERROR while testing local empty(1)"<<endl;
  if(!pg2.empty()) cout<<"ERROR while testing empty(2)"<<endl;
  if(!pg2.local_empty()) cout<<"ERROR while testing local empty(3)"<<endl;
  
  stapl_print("Passed\n");

  stapl_print("testing copy constructor...");
  PGRAPH pg1(pg);
  stapl::rmi_fence();
  //compare pg1 with pg;
  vi1 = pg.begin();
  vi2 = pg1.begin();
  for(;vi1 != pg.end();++vi1,++vi2){
    if(vi1->vid != vi2->vid) cout<<"ERROR while testing read from file(vid)"<<endl;
    if(vi1->data.GetWeight() != vi2->data.GetWeight()) 
      cout<<"ERROR while testing read from file(data)"<<endl;
    //testing edges
    ei1 = vi1->edgelist.begin();
    ei2 = vi2->edgelist.begin();
    for(;ei1 != vi1->edgelist.end();++ei1,++ei2){
      if(ei1->vertex2id != ei2->vertex2id)
	cout<<"ERROR while testing read from file(edges vids)"<<endl;
      if(ei1->weight.GetWeight() != ei2->weight.GetWeight())
	cout<<"ERROR while testing read from file(edges data)"<<endl;
    }
  }
  stapl_print("Passed\n");

  //cvi1 = pg.local_begin();
  stapl_print("testing iterators...");
  i=0;
  for(vi1 = pg.local_begin(); vi1 != pg.local_end(); ++vi1){
    if(vi1->vid != vertices[i]) cout<<"ERROR while testing iterator"<<endl;
    i++;
    for(ei1 = vi1->edgelist.begin(); ei1 != vi1->edgelist.end();ei1++){
      if(ei1->weight.GetWeight() != vi1->vid * ei1->vertex2id)
	cout<<"ERROR while testing edge iterator"<<endl;
    }
  }
  stapl_print("Passed\n");

  stapl_print("testing IsVertex...");
  for (i=0;i<SZ*NP;i++){
    if(!pg.IsVertex(i)){
      cout<<"ERROR while testing IsVertex"<<endl;
    }
  }
  stapl::rmi_fence();
  for(i=0;i<vertices.size();i++){
    if(!pg.IsVertex(vertices[i],&vi1)){
      cout<<"ERROR while testing IsVertex(gid,&it)"<<endl;
    }
    else{
      if(vi1->vid != vertices[i])
	cout<<"ERROR while testing IsVertex(gid,&it)(2) "<<endl;
    }
    
    if(!pg.IsVertex(vertices[i],&cvi1)){
      cout<<"ERROR while testing IsVertex(gid,&it)(3)"<<endl;
    }
    else{
      if(cvi1->vid != vertices[i])
	cout<<"ERROR while testing IsVertex(gid,&it)(4) "<<endl;
    }
  }
  stapl_print("Passed\n");

  stapl_print("testing isedge...");
  for(vi1 = pg.local_begin(); vi1 != pg.local_end(); ++vi1){
    for(ei1 = vi1->edgelist.begin(); ei1 != vi1->edgelist.end();ei1++){
      if(!pg.IsEdge(vi1->vid,ei1->vertex2id,&vi2,&ei2)){
	cout<<"ERROR while testing IsEdge(vi,ei)"<<endl;
      }
      if(vi2->vid != vi1->vid || ei2->vertex2id != ei1->vertex2id)
	cout<<"ERROR while testing IsEdge(vi,ei)"<<endl;
    }
  }  
  stapl_print("Passed\n");

  stapl_print("testing isedge edge id...");
  vi1 = pg.local_begin();
  vi2 = vi1;vi2++;
  wt.SetWeight(10);
  int id1 = pg.AddEdge(vi1->vid,vi2->vid,wt);
  wt.SetWeight(20);
  int id2 = pg.AddEdge(vi1->vid,vi2->vid,wt);
  //cout<<id1<<":-:"<<id2<<endl;
  if(!pg.IsEdgeId(vi1->vid,id1,&vi3,&ei1)){
    cout<<"ERROR while testing IsEdge with edge id:"<<id1<<endl;
  }
  else{
    if(vi3->vid != vi1->vid || ei1->weight.GetWeight() != 10)
      cout<<"ERROR while testing IsEdge with edge id(2):"<<endl;
  }
  if(!pg.IsEdgeId(vi1->vid,id2,&vi3,&ei1)){
    cout<<"ERROR while testing IsEdge with edge id(3):"<<id2<<endl;
  }
  else{
    if(vi3->vid != vi1->vid || ei1->weight.GetWeight() != 20)
      cout<<"ERROR while testing IsEdge with edge id(4)"<<endl;
  }
  stapl_print("Passed\n");

  stapl_print("testing GetData...");
  for(i=0;i<NP;i++){
    data = pg.GetData(i);
    stapl::rmi_fence();
    if(data.GetWeight() != 0) {
      cout<<"ERROR this is not the intended behavior; possible ERROR"<<endl;
    }
  }
  stapl_print("Passed\n");

  stapl_print("testing set predecessors ...");
  pg.pSetPredecessors();
  //testing the correctness
  for(vi1 = pg.local_begin(); vi1 != pg.local_end(); ++vi1){
    for(ei1 = vi1->predecessors.begin(); ei1 != vi1->predecessors.end();ei1++){
      if(!pg.IsEdge(ei1->vertex2id,vi1->vid)){
	cout<<"ERROR while testing set predecessors: edge added incorrect"<<endl;
      }
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");


  stapl_print("testing async set predecessors ...");
  pg.ClearPredecessors();
  pg.pAsyncSetPredecessors();
  //testing the correctness
  for(vi1 = pg.local_begin(); vi1 != pg.local_end(); ++vi1){
    for(ei1 = vi1->predecessors.begin(); ei1 != vi1->predecessors.end();ei1++){
      if(!pg.IsEdge(ei1->vertex2id,vi1->vid)){
	cout<<"ERROR while testing set predecessors: edge added incorrect"<<endl;
      }
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing get cut edges ...");
  set< pair< VID, VID > > cut;
  pg.pGetCutEdges(cut);
  for(set<pair<VID,VID> >::iterator sit = cut.begin();sit != cut.end();++sit){
    if(!pg.IsEdge(sit->first,sit->second)){
      cout<<"ERROR while testing get cut edges"<<endl;
    }
    if(pg.IsLocal(sit->second)){
      cout<<"ERROR while testing get cut edges(2)"<<endl;
    }
  }
  stapl_print("Passed\n");
  

  /*
  stapl_print("testing GetEdgeCount ...");
  i = pg.GetLocalEdgeCount();
  if(i != localedges) 
    cout<<"ERROR while testing GetLocalEdgeCount"<<i<<" should be "<<localedges<<endl; 

  i = pg.GetEdgeCount();
  if(i != NP * localedges) 
    cout<<"ERROR while testing GetEdgeCount"<<i<<" should be "<<NP * localedges<<endl; 
  
  stapl::rmi_fence();
  stapl_print("Passed\n");
  */

  //------------------------------------------------
  //   Deleting the edges vertices
  //------------------------------------------------
  stapl_print("testing delete edge ...");
  vi1 = pg.local_begin();
  vi2 = vi1;vi2++;
  pg.DeleteEdgeId(vi1->vid, id1);
  if(pg.IsEdgeId(vi1->vid,id1,&vi3,&ei1)){
    cout<<"ERROR while testing DeleteEdgeId with edge id:"<<id1<<endl;
  }
  pg.DeleteEdgeId(vi1->vid, id2);
  if(pg.IsEdgeId(vi1->vid,id1,&vi3,&ei1)){
    cout<<"ERROR while testing DeleteEdgeId with edge id(2):"<<id1<<endl;
  }
  //delete remote incoming edges;
  vector< pair< VID, VID > > edges;

  for(vi1 = pg.local_begin(); vi1 != pg.local_end(); ++vi1){
    for(ei1 = vi1->predecessors.begin(); ei1 != vi1->predecessors.end();ei1++){
      if(!pg.IsLocal(ei1->vertex2id)){
	edges.push_back(pair<VID,VID>(ei1->vertex2id,vi1->vid));
      }
    }
  }

  for(i=0;i<edges.size();i++){
    pg.DeleteEdge(edges[i].first,edges[i].second);
  }

  stapl::rmi_fence();

  for(i=0;i<edges.size();i++){
    if(pg.IsEdge(edges[i].first,edges[i].second)) 
      cout<<"ERROR while deleting edges that are remote"<<endl;
  }
  //delete all remaining edges
  edges.clear();
  pg.GetEdges(edges);
  for(i=0;i<edges.size();i++){
    pg.DeleteEdge(edges[i].first,edges[i].second);
  }

  for(i=0;i<edges.size();i++){
    if(pg.IsEdge(edges[i].first,edges[i].second)) 
      cout<<"ERROR while deleting edges"<<endl;
  }
  stapl_print("Passed\n");

/*
  stapl_print("testing DeleteVertex ...");
  vector<VID> verts;
  j = pg.GetVertices(verts);
  for(i=0;i<j;i++){
    pg.DeleteVertex(verts[i]);
    if(pg.IsVertex(verts[i])){
      cout<<"ERROR while deleting vertex:"<<verts[i]<<endl;
    }
  }
  stapl::rmi_fence();
  if(pg.local_size() != 0 ){
    cout<<"ERROR while testing delete vertex"<<endl;
  }
  stapl_print("Passed\n");

  stapl_print("testing ErasePGraph ...");
  pg1.ErasePGraph();
  if(!pg1.empty()) cout<<"ERROR while testing ErasePGraph"<<endl;
  if(pg1.IsVertex(0)) cout<<"ERROR while testing ErasePGraph(1)"<<endl;
  stapl_print("Passed\n");
*/

  stapl_print("FINISHED\n");
  return;
}



