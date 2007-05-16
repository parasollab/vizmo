#include <stdlib.h>
#include "timing.h"
#include "original.h"
#include "task.h"

typedef WeightedMultiDiGraph<Task,Weight2> WG;
typedef WeightedMultiDiGraph<Task,Weight2> DIRG;
typedef WeightedMultiDiGraph<Task,Weight2> WDG;
//typedef Graph<DG<Task>, MG<Task>, NWG<Task>, Task> GRAPH;

#define VERTEX Task
#define WEIGHT Weight2

int main(void){
  WtEdgeType<int,int> w;
  int i;
  WG c3,c2;
  WDG::VI vi;
  WDG::EI ei;

  int k=5;
  int  j;
  c3.ReadDotGraph("in");
  cout<<"Start"<<endl;
  stapl::start_timer();
  //test the graph's methods
  Task d1(1),data(1);
  for(j=0;j < 4;j++){
    Weight2 w1(1),w2(2);
    vector<Task> v1;
    v1.push_back(data);
    v1.push_back(d1);
    c2.AddVertex(data);
    c2.AddVertex(data);
    c2.AddVertex(data);
    c2.AddVertex(data);
    c2.AddVertex(v1);
    cout<<c2.v.size()<<endl;
    
    cout<<"First "<< c2.AddEdge(0,1,w1)<<endl;
    cout<<"Second "<< c2.AddEdge(0,2,w1)<<endl;
    c2.AddEdge(c2.v.begin()->data,(c2.v.begin() + 3)->data,w1);
    pair<Weight2,Weight2> p;
    p.first = w1;
    p.second = w2;
    c2.AddEdge(1,2,p);
    c2.AddEdge(c2.v.begin()->data,(c2.v.begin() + 4)->data,p);
    
    int ne;
    c2.DisplayGraph();
    c2.DisplayVertexAndEdgelist(15);
    c2.DeleteVertex(15);
    c2.DisplayGraph();
    cout<<"Run GetEdges"<<endl;
    
    // the next two calls are not available in the old graph
    
    //vector<pair<VID,VID> > edges;
    // - ne = c2.GetEdges(edges);
    //cout<<"number of edges "<<ne<<endl;
    //for(i=0;i<ne;i++){
    //  cout<<edges[i].first<<" -> "<<edges[i].second<<endl;
    //}
    //i=c2.getVertIDs();
    //cout<<i;
    //c2.setVertIDs(i);
    //vector< pair< VERTEX, VERTEX > > edges1;
    // - ne = c2.GetEdgesVData(edges1);
    //cout<<"number of edges1 "<<ne<<endl;
    //for(i=0;i<ne;i++){
    //  cout<<edges1[i].first<<" -> "<<edges1[i].second<<endl;
    //} 
    vector< pair< pair< VID, VID >, WEIGHT > > edges2;
    edges2 = c2.GetEdges();
    ne = edges2.size();
    cout<<"number of edges2 "<<ne<<endl;
    for(i=0;i<ne;i++){
      cout<<edges2[i].first.first<<" -> "<<edges2[i].first.second<<" "<<edges2[i].second<<endl;
    }
    vector< pair< pair< VERTEX, VERTEX >, WEIGHT > > edges3;
    edges3 = c2.GetEdgesVData();
    ne = edges3.size();
    cout<<"number of edges3 "<<ne<<endl;
    for(i=0;i<ne;i++){
      cout<<edges3[i].first.first<<" -> "<<edges3[i].first.second<<" "<<edges3[i].second<<endl;
    }
    vector<VID> vd1;
    vd1.push_back(100);
    vd1.push_back(101);
    vd1.push_back(102);
    c2.AddPath(vd1, w1);
    
    //c2.ChangeEdgeWeight(0,1,w2);
    //c2.ChangeEdgeWeight(c2.v.begin()->data,(c2.v.begin() + 1)->data,w2);
    c2.DeleteVertex(i);
    c2.DisplayGraph();
    c2.DisplayVertices();
    c2.DisplayVertex(i);
    // not in WMDIG- c2.DisplayVertexandEdgelist(i);
    //!!!c2.DeleteVertex(v1);
    for(vi = c3.v.begin(); vi != c3.v.end(); vi++) {	 
      for(ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++) {
      }
    }
  }
 
  for(j=0;j < 4;j++){
    Weight2 w1(1),w2(2);
    vector<Task> v1;
    v1.push_back(data);
    v1.push_back(d1);
    c3.AddVertex(data);
    c3.AddVertex(data);
    c3.AddVertex(data);
    c3.AddVertex(data);
    c3.AddVertex(v1);
    cout<<c3.v.size()<<endl;
    
    cout<<"First "<< c3.AddEdge(0,1,w1)<<endl;
    cout<<"Second "<< c3.AddEdge(0,2,w1)<<endl;
    c3.AddEdge(c3.v.begin()->data,(c3.v.begin() + 3)->data,w1);
    pair<Weight2,Weight2> p;
    p.first = w1;
    p.second = w2;
    c3.AddEdge(1,2,p);
    c3.AddEdge(c3.v.begin()->data,(c3.v.begin() + 4)->data,p);
    
    int ne;
    c3.DisplayGraph();
    c3.DisplayVertexAndEdgelist(15);
    c3.DeleteVertex(15);
    c3.DisplayGraph();
    //is edge

    cout<<c3.IsEdge(0,j)<<endl;
    cout<<c3.IsEdge(c3.v.begin()->data,(c3.v.begin() + 3)->data);
    cout<<c3.IsEdge(0,j,w1);
    cout<<c3.IsEdge(c3.v.begin()->data,(c3.v.begin() + 3)->data,w1);
    /*
    vector< pair<pair<VID,VID>,Weight2> > iedges;
    iedges = c3.GetIncidentEdges(j);
    ne = iedges.size();
    for(i=0;i<ne;i++){
      cout<<iedges[i].first.first<<" -> "<<iedges[i].first.second<<" "<<iedges[i].second<<endl;
    }
    vector< pair<pair<Task,Task>,Weight2> > iedgesw;
    iedgesw=c3.GetIncidentEdgesVData(j);
    ne = iedgesw.size();
    for(i=0;i<ne;i++){
      cout<<iedges[i].first.first<<" -> "<<iedges[i].first.second<<" "<<iedges[i].second<<endl;
    }
    */
    //weighted methods;
    cout<<"Get EdgeWeight"<<endl;
    cout<<c3.GetEdgeWeight(0,j)<<endl;
    cout<<c3.GetEdgeWeight(c3.v.begin()->data , (c3.v.begin() + 3)->data)<<endl;

    c3.DeleteWtEdge(0, j , w1);
    c3.DeleteWtEdge(c3.v.begin()->data , (c3.v.begin() + 3)->data, w1);
    c3.DisplayGraph();


    cout<<"Run GetEdges"<<endl;
    
    // the next two calls are not available in the old graph
    
    //vector<pair<VID,VID> > edges;
    // - ne = c3.GetEdges(edges);
    //cout<<"number of edges "<<ne<<endl;
    //for(i=0;i<ne;i++){
    //  cout<<edges[i].first<<" -> "<<edges[i].second<<endl;
    //}
    //i=c3.getVertIDs();
    //cout<<i;
    //c3.setVertIDs(i);
    //vector< pair< VERTEX, VERTEX > > edges1;
    // - ne = c3.GetEdgesVData(edges1);
    //cout<<"number of edges1 "<<ne<<endl;
    //for(i=0;i<ne;i++){
    //  cout<<edges1[i].first<<" -> "<<edges1[i].second<<endl;
    //} 
    vector< pair< pair< VID, VID >, WEIGHT > > edges2;
    edges2 = c3.GetEdges();
    ne = edges2.size();
    cout<<"number of edges2 "<<ne<<endl;
    for(i=0;i<ne;i++){
      cout<<edges2[i].first.first<<" -> "<<edges2[i].first.second<<" "<<edges2[i].second<<endl;
    }
    vector< pair< pair< VERTEX, VERTEX >, WEIGHT > > edges3;
    edges3 = c3.GetEdgesVData();
    ne = edges3.size();
    cout<<"number of edges3 "<<ne<<endl;
    for(i=0;i<ne;i++){
      cout<<edges3[i].first.first<<" -> "<<edges3[i].first.second<<" "<<edges3[i].second<<endl;
    }
    vector<VID> vd1;
    vd1.push_back(100);
    vd1.push_back(101);
    vd1.push_back(102);
    c3.AddPath(vd1, w1);
    //c3.ChangeEdgeWeight(0,1,w2);
    //c3.ChangeEdgeWeight(c3.v.begin()->data,(c3.v.begin() + 1)->data,w2);
    c3.DeleteVertex(j);
    c3.DisplayGraph();
    c3.DisplayVertices();
    c3.DisplayVertex(j);
    // not in WMDIG- c3.DisplayVertexandEdgelist(i);
    //!!!c3.DeleteVertex(v1);
    for(vi = c3.v.begin(); vi != c3.v.end(); vi++) {	 
      for(ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++) {
      }
    }
  }
  float tt = stapl::stop_timer();
  cout<<endl<<"time_new :"<<tt<<endl;
  c3.WriteGraph("titi2");
  return 1;
}
