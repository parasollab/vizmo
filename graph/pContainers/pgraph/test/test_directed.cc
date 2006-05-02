/*!
        \file test_directed.cc
	\author Gabriel Tanase	
        \date Feb. 24, 04
        \ingroup stapl
        \brief Test suite for pGraph class. All methods from PDG(Methods related with
	\the fact that the graph is directed) class instantiated
	\and tested for correctness. These are mainly methods to get vertices/edges
*/
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

  for(vi = pg.local_begin(); vi != pg.local_end();++vi){
    vi->data.SetWeight(vi->vid);
  }

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


  vector<VID> succ;
  vector<VID> succ1;
  vector<VID> preds;
  vector<VID> preds1;

  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing SetPredecessors...");
  pg.SetPredecessors();
  for(vi = pg.local_begin(); vi != pg.local_end(); ++vi){
    for(ei = vi->predecessors.begin(); ei != vi->predecessors.end();++ei){
      if(!pg.IsEdge(ei->vertex2id,vi->vid)){
	cout<<"ERROR while testing set predecessors: edge added incorrect"<<endl;
      }
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetAdjacentVertices/DATA...");
  vector<Task> succD;
  vector<Task> succD1;

  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetAdjacentVertices(j,succ);
    if(tmp != NP*SZ){
      cout<<"ERROR while testing GetAdjacentVertices"<<endl;
    }
    tmp = pg.GetAdjacentVerticesDATA(j,succD);
    if(tmp != NP*SZ){
      cout<<"ERROR while testing GetAdjacentVerticesDATA(1)"<<endl;
    }
    for(i=0;i<tmp;i++){
      if(succD[i].GetWeight() != succ[i]) 
	cout<<"ERROR while testing GetAdjacentVerticesDATA(2)"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetPredVertices/DATA...");
  vector<Task> predsD;
  vector<Task> predsD1;
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetPredVertices(j,preds);
    if(tmp != NP*SZ){
      cout<<"ERROR while testing GetPredVertices"<<endl;
    }
    tmp = pg.GetPredVerticesDATA(j,predsD);
    if(tmp != NP*SZ){
      cout<<"ERROR while testing GetPredVerticesDATA(1)"<<endl;
    }
    for(i=0;i<tmp;i++){
      if(predsD[i].GetWeight() != preds[i]) 
	cout<<"ERROR while testing GetPredVerticesDATA(2)"<<endl;
    }
  }
  stapl_print("Passed\n");

  stapl_print("testing GetVertexOutDegree...");
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetVertexOutDegree(j);
    if(tmp != NP*SZ){
      cout<<"ERROR while testing GetVertexOutDegree"<<endl;
    }
  }
  stapl_print("Passed\n");

  //------------------------------------------------------

  stapl_print("testing GetSuccessors(VI,)...");
  tmp = pg.GetSuccessors(pg.local_begin(),succ);
  if(tmp != NP*SZ){
    cout<<"ERROR while testing GetSuccessors(VI"<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetSuccessors(VID, GetSuccessors(VERTEX...");
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetSuccessors(j,succ);
    if(tmp != NP*SZ){
      cout<<"ERROR while testing GetSuccessors(VID"<<endl;
    }
    data.SetWeight(j);
    tmp = pg.GetSuccessors(data,succ1);
    if(tmp != NP*SZ){
      cout<<"ERROR while testing GetSuccesors(VERTEX"<<endl;
    }
    for(i=0;i<tmp;i++){
      if(succ[i] != succ1[i]) cout<<"ERROR while testing GetSuccesors(VERTEX(2)"<<endl; 
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetSuccessorsDATA(VID, GetSuccessorsDATA(VERTEX...");
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetSuccessorsDATA(j,succD);
    if(tmp != NP*SZ){
      cout<<"ERROR while testing GetSuccesorsDATA(VID"<<endl;
    }
    data.SetWeight(j);
    tmp = pg.GetSuccessorsDATA(data,succD1);
    if(tmp != NP*SZ){
      cout<<"ERROR while testing GetSuccesorsDATA(VERTEX"<<endl;
    }
    for(i=0;i<tmp;i++){
      if(!(succD[i] == succD1[i])) cout<<"ERROR while testing GetSuccesorsDATA(VERTEX(2)"<<endl; 
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  //------------------------------------------------------
  stapl_print("testing GetPredecessors(VI,)...");
  tmp = pg.GetPredecessors(pg.local_begin(),preds);
  if(tmp != NP*SZ){
    cout<<"ERROR while testing GetPredecessors(VI"<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetPredecessors(VID, GetPredecessors(VERTEX...");
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetPredecessors(j,preds);
    if(tmp != NP*SZ){
      cout<<"ERROR while testing GetPredecessors(VID"<<endl;
    }
    data.SetWeight(j);
    tmp = pg.GetPredecessors(data,preds1);
    if(tmp != NP*SZ){
      cout<<"ERROR while testing GetPredecessors(VERTEX"<<endl;
    }
    for(i=0;i<tmp;i++){
      if(preds[i] != preds1[i]) cout<<"ERROR while testing GetPredecessors(VERTEX(2)"<<endl; 
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetPredecessorsDATA(VID, GetPredecessorsDATA(VERTEX...");
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetPredecessorsDATA(j,predsD);
    if(tmp != NP*SZ){
      cout<<"ERROR while testing GetPredecessorsDATA(VID"<<endl;
    }
    data.SetWeight(j);
    tmp = pg.GetPredecessorsDATA(data,predsD1);
    if(tmp != NP*SZ){
      cout<<"ERROR while testing GetPredecessorsDATA(VERTEX"<<endl;
    }

    for(i=0;i<tmp;i++){
      if(!(predsD[i] == predsD1[i])) cout<<"ERROR while testing GetPredecessorsData(VERTEX(2)"<<endl; 
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  //------------------------------------------------------ edges

  stapl_print("testing GetOutgoingEdges...");
  vector< pair<VID,VID> > iedges;
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetOutgoingEdges(j,iedges);
    for (i=0;i<tmp;i++){
      //cout<<iedges[i].first<<":"<<iedges[i].second<<":"<<tmp - i - 1<<":"<<j<<endl;
      if(iedges[i].first != j) cout<<"ERROR while testing GetOutgoingEdges"<<endl;
      if(iedges[i].second != (tmp - i-1)) cout<<"ERROR while testing GetOutgoingEdges(2)"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetOutgoingEdgesWeight...");
  vector< pair<pair<VID,VID>,Weight2> > iedgeswt;
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetOutgoingEdges(j,iedgeswt);
    for (i=0;i<tmp;i++){
      //cout<<iedges[i].first<<":"<<iedges[i].second<<":"<<tmp - i - 1<<":"<<j<<endl;
      if(iedgeswt[i].first.first != j) cout<<"ERROR while testing GetOutgoingEdgesWt"<<endl;
      if(iedgeswt[i].first.second != (tmp - i-1)) cout<<"ERROR while testing GetOutgoingEdgesWt(2)"<<endl;
      if(iedgeswt[i].second.GetWeight() != iedgeswt[i].first.first*iedgeswt[i].first.second) cout<<"ERROR while testing GetOutgoingEdgesWt(3)"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetOutgoingEdgesVData...");
  vector< pair<Task,Task> > iedgesVD;
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetOutgoingEdgesVData(j,iedgesVD);
    data = pg.GetData(j);
    for (i=0;i<tmp;i++){
      if(!(iedgesVD[i].first == data)) cout<<"ERROR while testing GetOutgoingEdgesVData"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetOutgoingEdgesWeightVData...");
  vector< pair<pair<Task,Task>,Weight2> > iedgesVDw;
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetOutgoingEdgesVData(j,iedgesVDw);
    data = pg.GetData(j);
    for (i=0;i<tmp;i++){
      //cout<<iedges[i].first<<":"<<iedges[i].second<<":"<<tmp - i - 1<<":"<<j<<endl;
      if(!(iedgesVDw[i].first.first == data)) cout<<"ERROR while testing GetOutgoingEdgesWeightVData"<<endl;
      if(iedgesVDw[i].second.GetWeight() != iedgesVDw[i].first.first.GetWeight()*iedgesVDw[i].first.second.GetWeight()) cout<<"ERROR while testing GetOutgoingEdgesWeightVData(2)"<<endl;

    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");
  return;
}
