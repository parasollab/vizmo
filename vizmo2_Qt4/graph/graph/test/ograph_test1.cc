#include <stdlib.h>
#include "timing.h"
#include "original.h"
#include "task.h"

typedef WeightedMultiDiGraph<Task,Weight2> WG;
typedef WeightedMultiDiGraph<Task,Weight2> DIRG;
typedef WeightedMultiDiGraph<Task,Weight2> WDG;
//typedef Graph<DG<Task>, MG<Task>, NWG<Task>, Task> GRAPH;


int main(void){
  WtEdgeType<int,int> w;
  int i;
  WG c3;
  WDG::VI vi;
  WDG::EI ei;

  int k=5;
  int  j;
  c3.ReadDotGraph("in");
  cout<<"Start"<<endl;
  stapl::start_timer();

  for(i=0;i < k;i++){

    cout<<"new DFS"<<endl;
    DIRG g7;
    g7 = c3.DFS(i);
    g7.DisplayGraph();

    cout<<"VERTEX"<<endl;
    g7.EraseGraph();
    g7 = c3.DFS((c3.v.begin()+1)->data);
    g7.DisplayGraph();
    
    cout<<"DFS VID"<<endl;
    vector<VID> v;
    v = c3.DFSVID(i);
    for(j=0;j < v.size();j++){
      cout<<v[j]<<endl;
    }
    cout<<"VERTEX"<<endl;
    v=c3.DFSVID((c3.v.begin()+1)->data);
    for(j=0;j < v.size();j++){
      cout<<v[j]<<endl;
    }
    cout<<"DFS returns the forest"<<endl;
    vector<DIRG> v41;
    v41 = c3.DFS();
    for(j=0;j < v41.size();j++){
      v41[j].DisplayGraph();
    }
    cout<<"IsCycle "<<c3.IsCycle()<<endl;
    
    cout<<"GetBackEdges"<<endl;
    vector<pair<VID,VID> > v31;
    v31 = c3.GetBackedge();
    //for(j=0;j < v31.size();j++){
    //cout<<v31[j].first<<" "<<v31[j].second<<endl;
    //}
    
    cout<<"new BFS"<<endl;
    DIRG g6;
    g6 = c3.BFS(i);
    g6.DisplayGraph();
    cout<<"VERTEX"<<endl;
    g6 = c3.BFS((c3.v.begin()+1)->data);
    g6.DisplayGraph();
    cout<<"BFSVID"<<endl;
    vector<VID> v4;
    v4 = c3.BFSVID(i);
    for(j=0;j < v4.size();j++){
      cout<<v4[j]<<endl;
    }
    cout<<"VERTEX"<<endl;
    v4 = c3.BFSVID((c3.v.begin()+1)->data);
    for(j=0;j < v4.size();j++){
      cout<<v4[j]<<endl;
    }
  }
  float tt = stapl::stop_timer();
  //cout<<"time_new :"<<tt<<endl;
  c3.WriteGraph("titi2");
  return 1;
}
