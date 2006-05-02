/*!
        \file test_undirected.cc
	\author Gabriel Tanase	
        \date Feb. 24, 04
        \ingroup stapl
        \brief Test suite for pGraph class. All methods from PUG(Methods related with
	\the fact that the graph is undirected) class instantiated
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


  typedef pGraph<PUG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PGRAPH;


  Task data,data1;
  Weight2 wt;

  int i,j,tmp,localedges;   
  int id=stapl::get_thread_id();
  vector<int> vertices;
  NP=stapl::get_num_threads();
  int myid=stapl::get_thread_id();
  PGRAPH::VI vi,vi1;
  PGRAPH::EI ei,ei1;
  PGRAPH::CVI cvi;

  PGRAPH pg,pg_copy;
  stapl::rmi_fence();

  printf("main started\n");

  SZ = atoi(argv[1]);
  printf("SIZE %d\n",SZ);
  stapl_print("testing AddVertex...");
  for(i=0;i<SZ;i++){
    tmp = i+myid*SZ;
    data.SetWeight(tmp);
    j = pg_copy.AddVertex(data);
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
  for(vi = pg_copy.local_begin(); vi != pg_copy.local_end();++vi){
    vi->data.SetWeight(vi->vid);
  }

  localedges = 0;

  //                       the graph will be a ring
  stapl_print("testing AddEdge...");
  for (i=0;i<SZ;i++){
    tmp = vertices[i] * ((vertices[i] + 1)%(NP*SZ));
    wt.SetWeight(tmp);
    pg.AddEdge(vertices[i],(vertices[i]+1)%(NP*SZ),wt);
    localedges++;
  }
  stapl::rmi_fence();
  for (i=0;i<SZ;i++){
    if(!pg.IsEdge(vertices[i],(vertices[i]+1)%(NP*SZ)))
      cout<<"ERROR while testing AddEdge/IsEdge"<<endl;
    if(!pg.IsEdge( (vertices[i]+1)%(NP*SZ),vertices[i]) )
      cout<<"ERROR while testing AddEdge/IsEdge(2)"<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");
  //pg.pDisplayGraph();
  
  vector<VID> succ;
  vector<VID> succ1;

  stapl_print("testing GetAdjacentVertices/DATA(VID...");
  vector<Task> succD;
  vector<Task> succD1;

  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetAdjacentVertices(j,succ);
    if(tmp != 2){
      cout<<"ERROR while testing GetAdjacentVertices"<<endl;
    }
    tmp = pg.GetAdjacentVerticesDATA(j,succD);
    if(tmp != 2){
      cout<<"ERROR while testing GetAdjacentVerticesDATA(1)"<<endl;
    }
    for(i=0;i<tmp;i++){
      //cout<<succ[i]<<":"<<succD[i].GetWeight()<<endl;
      if(succD[i].GetWeight() != succ[i]) 
	cout<<"ERROR while testing GetAdjacentVerticesDATA(2)"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetAdjacentVertices/DATA(VERTEX...");
  for(j=0;j<NP*SZ;j++){
    data.SetWeight(j);
    tmp = pg.GetAdjacentVertices(data,succ);
    if(tmp != 2){
      cout<<"ERROR while testing GetAdjacentVertices(VERTEX"<<endl;
    }
    tmp = pg.GetAdjacentVerticesDATA(data,succD);
    if(tmp != 2){
      cout<<"ERROR while testing GetAdjacentVerticesDATA(VERTEX)(1)"<<endl;
    }
    for(i=0;i<tmp;i++){
      //cout<<succ[i]<<":"<<succD[i].GetWeight()<<endl;
      if(succD[i].GetWeight() != succ[i]) 
	cout<<"ERROR while testing GetAdjacentVerticesDATA(Vertex)(2)"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");


  stapl_print("testing GetVertexOutDegree...");
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetVertexOutDegree(j);
    if(tmp != 2) cout<<"ERROR while testing GetVertexOutDegree"<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  //------------------------------------------------------ edges

  stapl_print("testing GetIncidentEdges...");
  vector< pair<VID,VID> > iedges;
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetIncidentEdges(j,iedges);
    for (i=0;i<tmp;i++){
      //cout<<iedges[i].first<<":"<<iedges[i].second<<":"<<tmp - i - 1<<":"<<j<<endl;
      if(iedges[i].first != j) cout<<"ERROR while testing GetIncidentEdges"<<endl;
      if((iedges[i].second != (j+NP*SZ +1)%(NP*SZ)) &&(iedges[i].second != (j+NP*SZ -1)%(NP*SZ)) ) cout<<"ERROR while testing GetIncidentEdges(2)"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetIncidentEdgesWeight...");
  vector< pair<pair<VID,VID>,Weight2> > iedgeswt;
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetIncidentEdges(j,iedgeswt);
    for (i=0;i<tmp;i++){
      //cout<<iedges[i].first<<":"<<iedges[i].second<<":"<<tmp - i - 1<<":"<<j<<endl;
      if(iedgeswt[i].first.first != j) cout<<"ERROR while testing GetIncidentEdgesWt"<<endl;
      if((iedgeswt[i].first.second != (j+NP*SZ +1)%(NP*SZ)) &&(iedgeswt[i].first.second != (j+NP*SZ -1)%(NP*SZ)) ) cout<<"ERROR while testing GetIncidentEdgesWt(2)"<<endl;
      if(iedgeswt[i].second.GetWeight() != iedgeswt[i].first.first*iedgeswt[i].first.second) cout<<"ERROR while testing GetIncidentEdgesWt(3)"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetIncidentEdgesVData...");
  vector< pair<Task,Task> > iedgesVD;
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetIncidentEdgesVData(j,iedgesVD);
    data = pg.GetData(j);
    for (i=0;i<tmp;i++){
      if(!(iedgesVD[i].first == data)) cout<<"ERROR while testing GetIncidentEdgesVData"<<endl;
      if((iedgesVD[i].second.GetWeight() != (j+NP*SZ +1)%(NP*SZ)) &&(iedgesVD[i].second.GetWeight() != (j+NP*SZ -1)%(NP*SZ)) ) cout<<"ERROR while testing GetIncidentEdgesVData(2)"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetIncidentEdgesWeightVData...");
  vector< pair<pair<Task,Task>,Weight2> > iedgesVDw;
  for(j=0;j<NP*SZ;j++){
    tmp = pg.GetIncidentEdgesVData(j,iedgesVDw);
    data = pg.GetData(j);
    for (i=0;i<tmp;i++){
      //cout<<iedges[i].first<<":"<<iedges[i].second<<":"<<tmp - i - 1<<":"<<j<<endl;
      if(!(iedgesVDw[i].first.first == data)) cout<<"ERROR while testing GetIncidentEdgesWeightVData"<<endl;
      if(iedgesVDw[i].second.GetWeight() != iedgesVDw[i].first.first.GetWeight()*iedgesVDw[i].first.second.GetWeight()) cout<<"ERROR while testing GetIncidentEdgesWeightVData(2)"<<endl;

    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  //more testing for add/delete edge in undirected case
  //add edges in pg_copy using AddEdge(VERTEX,VERTEX)
  stapl_print("testing AddEdge(VERTEX,VERTEX)...");
  for (i=0;i<SZ;i++){
    tmp = vertices[i] * ((vertices[i] + 1)%(NP*SZ));
    wt.SetWeight(tmp);
    data.SetWeight(vertices[i]);
    tmp = (vertices[i]+1) % (NP*SZ);
    data1.SetWeight(tmp);
    pg_copy.AddEdge(data,data1,wt);
    localedges++;
  }
  stapl::rmi_fence();
  for (i=0;i<SZ;i++){
    if(!pg_copy.IsEdge(vertices[i],(vertices[i]+1)%(NP*SZ)))
      cout<<"ERROR while testing AddEdge/IsEdge"<<endl;
    if(!pg_copy.IsEdge( (vertices[i]+1)%(NP*SZ),vertices[i]) )
      cout<<"ERROR while testing AddEdge/IsEdge(2)"<<endl;
  }
  stapl::rmi_fence();
  //compare the two graphs
  vi1 = pg_copy.local_begin();
  for(vi = pg.local_begin(); vi != pg.local_end();++vi,++vi1){
    if(vi->vid != vi1->vid) cout<<"ERROR while adding vertices in pg_copy"<<endl;
    if(!(vi->data == vi1->vid)) cout<<"ERROR while adding vertices in pg_copy(1)"<<endl;
    ei1 = vi1->edgelist.begin();
    for(ei = vi->edgelist.begin(); ei != vi->edgelist.end();++ei,++ei1){
      if(!(ei->weight == ei1->weight)) cout<<"ERROR while testing add edge(VERTEX,VERTEX)"<<endl;
    }
  }
  stapl_print("Passed\n");

  stapl_print("testing DeleteEdge(VERTEX,VERTEX)...");
  for (i=0;i<SZ;i++){
    tmp = vertices[i] * ((vertices[i] + 1)%(NP*SZ));
    wt.SetWeight(tmp);
    data.SetWeight(vertices[i]);
    tmp = (vertices[i]+1) % (NP*SZ);
    data1.SetWeight(tmp);
    pg_copy.DeleteEdge(data,data1);
    pg.DeleteEdge(vertices[i],(vertices[i]+1) % (NP*SZ));
    stapl::rmi_fence();

    if(pg.IsEdge(vertices[i],(vertices[i]+1)%(NP*SZ)))
      cout<<"ERROR while testing DeleteEdge/IsEdge"<<endl;
    if(pg.IsEdge( (vertices[i]+1)%(NP*SZ),vertices[i]) )
      cout<<"ERROR while testing DeleteEdge/IsEdge(2)"<<endl;

    stapl::rmi_fence();
    //compare the two graphs
    vi1 = pg_copy.local_begin();
    for(vi = pg.local_begin(); vi != pg.local_end();++vi,++vi1){
      if(vi->vid != vi1->vid) cout<<"ERROR while deleting edges in pg_copy"<<endl;
      if(!(vi->data == vi1->vid)) cout<<"ERROR while deleting edges in pg_copy(1)"<<endl;
      ei1 = vi1->edgelist.begin();
      for(ei = vi->edgelist.begin(); ei != vi->edgelist.end();++ei,++ei1){
	if(!(ei->weight == ei1->weight)) cout<<"ERROR while testing delete edge(VERTEX,VERTEX)"<<endl;
      }
    }
    stapl::rmi_fence();
  }
  stapl_print("Passed\n");

  return;
}
