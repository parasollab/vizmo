#include <runtime.h>
#include "Defines.h"
#include "Graph.h"
#include "pGraph.h"

#include "task.h"

using namespace stapl;

int SZ,NP;

void stapl_print(const char* s){
  if(stapl::get_thread_id() == 0)
    cout << s << flush;
}


//class used to test the const version of Get methods
template <class PG>
class test_svf{
  PG* pg;
public:
  test_svf(PG& p){
    pg = &p;
  }

  void test(int myid, int SZ) const {
    int i,j,tmp;
    double dd;
    for (i = (myid+1) * SZ;i < (myid+2) * SZ;i++){
      j =  i % (NP*SZ);
      //cout<<"asking "<<j<<endl;
      dd = pg->pGetVertexField(j,&Task::GetWeightConst1);
      if(dd - 13 != j)
	cout<<"ERROR while testing pGetVertexField() const for "<<j<<endl;
    }
    stapl::rmi_fence();

    for (i = (myid+1) * SZ;i < (myid+2) * SZ;i++){
      j =  i % (NP*SZ);
      //cout<<"asking "<<j<<endl;
      dd = pg->pGetVertexField(j,&Task::GetWeightConst2,tmp);
      if(dd - 13 != j)
	cout<<"ERROR while testing pGetVertexField() const for "<<j<<endl;
    }
    stapl::rmi_fence();
  }

  void test_edges(int myid, int SZ,vector<pair<VID,VID> >& edges) const {
    int i,tmp;
    double dd;
    for(i=0;i<edges.size();i++){
      dd = pg->pGetWeightField(edges[i].first,edges[i].second,&Weight2::GetWeight1);
      if (dd != edges[i].first * edges[i].second)
	cout<<"ERROR while testing pSet/pGetWeightfield"<<endl;
    }
    stapl::rmi_fence();

    for(i=0;i<edges.size();i++){
      dd = pg->pGetWeightField(edges[i].first,edges[i].second,&Weight2::GetWeight1);
      if (dd != edges[i].first * edges[i].second)
	cout<<"ERROR while testing pSet/pGetWeightfield(2)"<<endl;
    }
    stapl::rmi_fence();

    for(i=0;i<edges.size();i++){
      dd = pg->pGetWeightField(edges[i].first,edges[i].second,&Weight2::GetWeight2,tmp);
      if (dd != edges[i].first * edges[i].second)
	cout<<"ERROR while testing pSet/pGetWeightfield(3)"<<endl;
    }
    stapl::rmi_fence();
  }
  void test_edges_edgeid(int myid, int SZ,vector<pair<VID,VID> >& edges) const {
    int i,tmp;
    double dd;
    for(i=0;i<edges.size();i++){
      dd = pg->pGetWeightFieldEdgeId(edges[i].first,edges[i].second,&Weight2::GetWeight1);
      if (dd != edges[i].first * edges[i].second)
	cout<<"ERROR while testing pSet/pGetWeightfield"<<endl;
    }
    stapl::rmi_fence();

    for(i=0;i<edges.size();i++){
      dd = pg->pGetWeightFieldEdgeId(edges[i].first,edges[i].second,&Weight2::GetWeight1);
      if (dd != edges[i].first * edges[i].second)
	cout<<"ERROR while testing pSet/pGetWeightfield(2)"<<endl;
    }
    stapl::rmi_fence();

    for(i=0;i<edges.size();i++){
      dd = pg->pGetWeightFieldEdgeId(edges[i].first,edges[i].second,&Weight2::GetWeight2,tmp);
      if (dd != edges[i].first * edges[i].second)
	cout<<"ERROR while testing pSet/pGetWeightfield(3)"<<endl;
    }
    stapl::rmi_fence();
  }
};
 
void stapl_main(int argc,char** argv){

  typedef Graph<DG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;
  typedef pGraph<PDG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PGRAPH;
  typedef Graph<UG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> UGRAPH;

  int myid=stapl::get_thread_id();
  NP=stapl::get_num_threads();
  printf("main started\n");

  PGRAPH pg;
  PGRAPH::VI vi1;
  PGRAPH::EI ei1;
  stapl::rmi_fence();
  vector<int> vertices;
  Task data;
  Weight2 wt;
  double dd;
  long ll;
  int i,j,tmp,localedges;
  test_svf<PGRAPH> svf(pg);

  SZ = atoi(argv[1]);
  printf("SIZE %d\n",SZ);


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
	//wt.SetWeight(vertices[i] * j);
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
  
  stapl_print("testing pSetVertexField(T1), pGetVertexField()...");
  for (i = (myid+1) * SZ;i < (myid+2) * SZ;i++){
    j =  i % (NP*SZ);
    //cout<<"setting "<<j<<endl;
    tmp = j + 11;
    pg.pSetVertexField(j,&Task::SetWeight1,tmp);
  }
  stapl::rmi_fence();

  for (i = (myid+1) * SZ;i < (myid+2) * SZ;i++){
    j =  i % (NP*SZ);
    //cout<<"asking "<<j<<endl;
    dd = pg.pGetVertexField(j,&Task::GetWeight1);
    if(dd - 11 != j)
      cout<<"ERROR while testing pSetVertexField(T) for "<<j<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");


  stapl_print("testing pSetVertexField(T1,T2), pGetVertexField(T1)...");
  for (i = (myid+1) * SZ;i < (myid+2) * SZ;i++){
    j =  i % (NP*SZ);
    //cout<<"setting "<<j<<endl;
    tmp = j + 12;
    pg.pSetVertexField(j,&Task::SetWeight2,tmp,dd);
  }
  stapl::rmi_fence();

  for (i = (myid+1) * SZ;i < (myid+2) * SZ;i++){
    j =  i % (NP*SZ);
    //cout<<"asking "<<j<<endl;
    dd = pg.pGetVertexField(j,&Task::GetWeight2,tmp);
    if(dd - 12 != j)
      cout<<"ERROR while testing pSetVertexField(T1,T2) for "<<j<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing pSetVertexField(T1,T2,T3)...");
  for (i = (myid+1) * SZ;i < (myid+2) * SZ;i++){
    j =  i % (NP*SZ);
    //cout<<"setting "<<j<<endl;
    tmp = j + 13;
    pg.pSetVertexField(j,&Task::SetWeight3,tmp,dd,ll);
  }
  stapl::rmi_fence();

  for (i = (myid+1) * SZ;i < (myid+2) * SZ;i++){
    j =  i % (NP*SZ);
    //cout<<"asking "<<j<<endl;
    dd = pg.pGetVertexField(j,&Task::GetWeight1);
    if(dd - 13 != j)
      cout<<"ERROR while testing pSetVertexField(T1,T2,T3) for "<<j<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");


  stapl_print("testing pGetVertexField const...");

  svf.test(myid,SZ);

  stapl_print("Passed\n");

  //---------------------------------------------------------
  //  pSet/GetWeightField(v1,v2)
  //---------------------------------------------------------

  //testing Set/Get Edge Weight Field
  stapl_print("testing set predecessors...");
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

  stapl_print("testing pSetWeightField ...");
  vector< pair< VID, VID > > edges;
  for(vi1 = pg.local_begin(); vi1 != pg.local_end(); ++vi1){
    for(ei1 = vi1->predecessors.begin(); ei1 != vi1->predecessors.end();ei1++){
      edges.push_back(pair<VID,VID>(ei1->vertex2id,vi1->vid));
    }
  }
  
  //set the edge weight; some of the edges will be local some remote
  for(i=0;i<edges.size();i++){
    tmp = edges[i].first * edges[i].second;
    pg.pSetWeightField(edges[i].first,edges[i].second,&Weight2::SetWeight1,tmp);
  }
  stapl::rmi_fence();

  for(i=0;i<edges.size();i++){
    dd = pg.pGetWeightField(edges[i].first,edges[i].second,&Weight2::GetWeight1);
    if (dd != edges[i].first * edges[i].second)
      cout<<"ERROR while testing pSet/pGetWeightfield"<<endl;
  }
  stapl::rmi_fence();


  //set weight 2 arguments
  for(i=0;i<edges.size();i++){
    tmp = edges[i].first * edges[i].second + 3;
    pg.pSetWeightField(edges[i].first,edges[i].second,&Weight2::SetWeight2,tmp,dd);
  }
  stapl::rmi_fence();

  for(i=0;i<edges.size();i++){
    dd = pg.pGetWeightField(edges[i].first,edges[i].second,&Weight2::GetWeight1);
    if (dd != edges[i].first * edges[i].second + 3)
      cout<<"ERROR while testing pSet/pGetWeightfield(2)"<<endl;
  }
  stapl::rmi_fence();

  //set weight 3 arguments
  for(i=0;i<edges.size();i++){
    tmp = edges[i].first * edges[i].second;
    pg.pSetWeightField(edges[i].first,edges[i].second,&Weight2::SetWeight3,tmp,dd,ll);
  }
  stapl::rmi_fence();

  for(i=0;i<edges.size();i++){
    dd = pg.pGetWeightField(edges[i].first,edges[i].second,&Weight2::GetWeight2,tmp);
    if (dd != edges[i].first * edges[i].second)
      cout<<"ERROR while testing pSet/pGetWeightfield(3)"<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing pGetWeightField const...");
   svf.test_edges(myid,SZ,edges);
  stapl_print("Passed\n");

  //-------------------------------------------------
  // set/Get using edge id;
  //-------------------------------------------------
  stapl_print("testing pGetWeightFieldEdgeId...");
  edges.clear();
  for(vi1 = pg.local_begin(); vi1 != pg.local_end(); ++vi1){
    for(ei1 = vi1->edgelist.begin(); ei1 != vi1->edgelist.end();++ei1){
      edges.push_back(pair<VID,VID>(vi1->vid,ei1->edgeid));
    }
  }
  
  //set the edge weight; some of the edges will be local some remote
  for(i=0;i<edges.size();i++){
    tmp = edges[i].first * edges[i].second;
    pg.pSetWeightFieldEdgeId(edges[i].first,edges[i].second,&Weight2::SetWeight1,tmp);
  }
  stapl::rmi_fence();

  for(i=0;i<edges.size();i++){
    dd = pg.pGetWeightFieldEdgeId(edges[i].first,edges[i].second,&Weight2::GetWeight1);
    if (dd != edges[i].first * edges[i].second)
      cout<<"ERROR while testing pSet/pGetWeightFieldEdgeId"<<endl;
  }
  stapl::rmi_fence();


  //set weight 2 arguments
  for(i=0;i<edges.size();i++){
    tmp = edges[i].first * edges[i].second + 3;
    pg.pSetWeightFieldEdgeId(edges[i].first,edges[i].second,&Weight2::SetWeight2,tmp,dd);
  }
  stapl::rmi_fence();

  for(i=0;i<edges.size();i++){
    dd = pg.pGetWeightFieldEdgeId(edges[i].first,edges[i].second,&Weight2::GetWeight1);
    if (dd != edges[i].first * edges[i].second + 3)
      cout<<"ERROR while testing pSet/pGetWeightFieldEdgeId(2)"<<endl;
  }
  stapl::rmi_fence();

  //set weight 3 arguments
  for(i=0;i<edges.size();i++){
    tmp = edges[i].first * edges[i].second;
    pg.pSetWeightFieldEdgeId(edges[i].first,edges[i].second,&Weight2::SetWeight3,tmp,dd,ll);
  }
  stapl::rmi_fence();

  for(i=0;i<edges.size();i++){
    dd = pg.pGetWeightFieldEdgeId(edges[i].first,edges[i].second,&Weight2::GetWeight2,tmp);
    if (dd != edges[i].first * edges[i].second)
      cout<<"ERROR while testing pSet/pGetWeightFieldEdgeId(3)"<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing pGetWeightFieldEdgeId const...");
   svf.test_edges_edgeid(myid,SZ,edges);
  stapl_print("Passed\n");

  stapl_print("FINISHED\n");
  stapl::rmi_fence();
}



