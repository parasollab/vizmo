#include <stdlib.h>
//#include "timing.h"
#include "original.h"
#include "task.h"

typedef WeightedMultiDiGraph<Task,Weight2> WG;
typedef WeightedGraph<Task,Weight2> WG2;
typedef WeightedMultiDiGraph<Task,Weight2> DIRG;
typedef WeightedMultiDiGraph<Task,Weight2> WDG;
//typedef Graph<DG<Task>, MG<Task>, NWG<Task>, Task> GRAPH;


int main(void){
	WG g;
	WG2 g2;

	g.ReadDotGraph("in");
	g.WriteGraph("old1");
	
	
	g2.ReadDotGraph("in");
	g2.WriteGraph("old2");

bool alltest=true;
bool graphtest=true;
if((alltest) || (graphtest))
{
	cout<<"*********START : Printig the GRAPH1*********"<<endl;
	g.DisplayGraph();
	cout<<"*********END   : Printig the GRAPH1*********"<<endl;


	cout<<"*********START : Printig the GRAPH2*********"<<endl;
	//g2.DisplayGraph();
	cout<<"*********END   : Printig the GRAPH2*********"<<endl;
}




vector<Task> v_tasks1;
v_tasks1=g.GetVerticesData();

vector<Task> v_tasks2;
v_tasks2=g2.GetVerticesData();


//TESTING DIRECTED GRAPH

for(int i=0; i<g.GetVertexCount(); ++i)
{
	if(graphtest)
	cout<<g.GetVertexOutDegree(i)<<endl;
	
	if(graphtest)	
	{
		vector<VID> result;
		result=g.GetSuccessors(i);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" Succ= "<<result[j]<<endl;
		}
	}
	
	if(graphtest)	
	{
	
		vector<Task> result;
		result=g.GetSuccessorsDATA(i);
		
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" SuccData= "<<result[j].GetTaskWeight()<<endl;
		}
	}
	
	if(graphtest)	
		g.SetPredecessors();

	if(graphtest)	
	{
		vector<VID> result;
		result=g.GetPredecessors(i);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" Succ= "<<result[j]<<endl;
		}
	}
	
	if(graphtest)	
	{
	
		vector<Task> result;
		result=g.GetPredecessorsDATA(i);
		
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" SuccData= "<<result[j].GetTaskWeight()<<endl;
		}
	}
}

	if(graphtest)		
	{
		vector<VID> result;
		result=g.GetSources();
		for(int j=0; j<result.size(); ++j)
		{
			cout<<" j: "<<j<<" Source = "<<result[j]<<endl;
		}
	}
	if(graphtest)		
	{
		vector<VID> result;
		result=g.GetSinks();
		for(int j=0; j<result.size(); ++j)
		{
			cout<<" j: "<<j<<" Sink = "<<result[j]<<endl;
		}
	}

//TESTING DIRECTED GRAPH OVER

//TESTING UNDIRECTED GRAPH




for(int i=0; i<g.GetVertexCount(); ++i)
{
	if(graphtest)
	cout<<g2.GetVertexOutDegree(i)<<endl;
	
	if(graphtest)	
	{
		vector<VID> result;
		result=g2.GetAdjacentVertices(i);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" Succ= "<<result[j]<<endl;
		}
	}
	
	if(graphtest)	
	{
		vector< pair<pair<VID,VID>,Weight2> > result;	
		result=g2.GetIncidentEdges(i);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" IncEdge.first= "<<result[j].first.first<<"  Second: "<<result[j].first.second<<endl;
		}
	}

	if(graphtest)	
	{
		vector< pair<pair<VID,VID>,Weight2> > result;	
		result=g2.GetIncidentEdges(i);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" IncEdge.first= "<<result[j].first.first<<"  Second: "<<result[j].first.second<<" Data:="<<result[j].second.Weight()<<endl;
		}
	}
	
	if(graphtest)	
	{
		vector< pair<pair<Task,Task>,Weight2> > result;		
		result=g2.GetIncidentEdgesVData(i);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" IncEdge.first= "<<result[j].first.first.GetTaskWeight()<<"  Second: "<<result[j].first.second.GetTaskWeight()<<endl;
		}
	}
	
	if(graphtest)	
	{
		vector< pair<pair<Task,Task>,Weight2> > result;	
		result=g2.GetIncidentEdgesVData(i);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" IncEdge.first= "<<result[j].first.first.GetTaskWeight()<<"  Second: "<<result[j].first.second.GetTaskWeight()<<" Data:="<<result[j].second.Weight()<<endl;
		}
	}
	
	
}


for(int counter=0; counter<1; counter++)	
{

	if(graphtest)
	{
		g2.AddEdge(counter+(random()%10)*counter,counter+(random()%10)*counter,Weight2(counter*1000+(counter*(random()%100)))); 		
		g2.AddEdge(v_tasks2[counter+(random()%10)*counter],v_tasks2[counter+(random()%10)*counter],Weight2(counter*1000+(counter*(random()%100)))); 

		pair<Weight2,Weight2> tmp1;
		tmp1.first=Weight2(counter*1000+(counter*(random()%100)));
		tmp1.second=Weight2(counter*1000+(counter*(random()%100)));
		pair<Weight2,Weight2> tmp2;
		tmp2.first=Weight2(counter*1000+(counter*(random()%100)));
		tmp2.second=Weight2(counter*1000+(counter*(random()%100)));

		g2.AddEdge(counter+(random()%10)*counter,counter+(random()%10)*counter,tmp1); 
		g2.AddEdge(v_tasks2[counter+(random()%10)*counter],v_tasks2[counter+(random()%10)*counter],tmp2); 
	
		g2.DeleteEdge(counter+(random()%10)*counter,counter+(random()%10)*counter);
		int e1=counter+(random()%10)*counter;
		int e2=counter+(random()%10)*counter;
		Weight2 tw1=g2.GetEdgeWeight(e1,e2);
		g2.DeleteWtEdge(e1,e2,tw1);
	
	
		g2.DeleteEdge(v_tasks2[counter+(random()%10)*counter],v_tasks2[counter+(random()%10)*counter]);
		int e3=counter+(random()%10)*counter;
		int e4=counter+(random()%10)*counter;
		Weight2 tw2=g2.GetEdgeWeight(e1,e2);
		g2.DeleteWtEdge(v_tasks2[e1],v_tasks2[e2],tw2);
		
		g2.DisplayGraph();
	
	
	}
	if(alltest)
	{
		cout<<" TESTIGN BFS: START "<<endl;
		WG result;
		result=g.BFS(counter);
		result.DisplayGraph(); //To compare with true_BFS
		result.DisplayGraph(); //To compare with BFS_VIS
		cout<<" TESTIGN BFS: END "<<endl;

	}
	
	if(alltest)	
	{
	
		cout<<"TESTING BFS VID"<<endl;
		vector<VID> result;
		result=g.BFSVID(counter);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<result[i]<<endl;
		}	
		cout<<"TESTING BFS VID OVER"<<endl;
	}
	
	if(alltest)
	{
	
		cout<<"TESTING BFS VID2"<<endl;
		vector<VID> result;
		result=g.BFSVID(counter);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<result[i]<<endl;
		}	
		cout<<"TESTING BFS VID OVER2"<<endl;
	}
	
	if(alltest)
	{
		cout<<"FIND PATH BFS TESTING"<<endl;		
		vector< pair<Task,Weight2> > result;
		result=g.FindPathBFS(counter,2*counter);
		for(int i=0; i<result.size(); ++i)
		{

			if(i!=result.size()-1)
				cout<<result[i].first.GetTaskWeight()<<"  "<<result[i].second.Weight()<<endl;
			else
				cout<<result[i].first.GetTaskWeight()<<"  "<<endl;
		}
		cout<<"FIND PATH BFS OVER"<<endl;

	}
	
	if(alltest)
	{
		cout<<"FIND PATH BFS TESTING2"<<endl;		
		vector< pair<Task,Weight2> > result;
		result=g.FindPathBFS(v_tasks1[counter],v_tasks1[2*counter]);
		for(int i=0; i<result.size(); ++i)
		{

			if(i!=result.size()-1)
				cout<<result[i].first.GetTaskWeight()<<"  "<<result[i].second.Weight()<<endl;
			else
				cout<<result[i].first.GetTaskWeight()<<"  "<<endl;
		}
		cout<<"FIND PATH BFS OVER2"<<endl;

	}
	
	if(alltest)
	{
		cout<<"FIND VID PATH BFS TESTING"<<endl;
		vector<VID> result;	
		result=g.FindVIDPathBFS(counter,counter*2);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<result[i]<<endl;
		}
		cout<<"FIND VID PATH BFS OVER"<<endl;
	}
	
	if(alltest)
	{
	
		cout<<"IsSameCC Test1 : START"<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*2<<"? "<<g2.IsSameCC(counter,counter*2)<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*3<<"? "<<g2.IsSameCC(counter,counter*3)<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*4<<"? "<<g2.IsSameCC(counter,counter*4)<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*5<<"? "<<g2.IsSameCC(counter,counter*5)<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*6<<"? "<<g2.IsSameCC(counter,counter*6)<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*7<<"? "<<g2.IsSameCC(counter,counter*7)<<endl;
		cout<<"IsSameCC Test1 : END"<<endl;
	
	}

	if(alltest)
	{
		cout<<"IsSameCC Test2 : START"<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*2<<"? "<<g2.IsSameCC(v_tasks2[counter],v_tasks2[counter*2])<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*3<<"? "<<g2.IsSameCC(v_tasks2[counter],v_tasks2[counter*3])<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*4<<"? "<<g2.IsSameCC(v_tasks2[counter],v_tasks2[counter*4])<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*5<<"? "<<g2.IsSameCC(v_tasks2[counter],v_tasks2[counter*5])<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*6<<"? "<<g2.IsSameCC(v_tasks2[counter],v_tasks2[counter*6])<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*7<<"? "<<g2.IsSameCC(v_tasks2[counter],v_tasks2[counter*7])<<endl;
		cout<<"IsSameCC Test2 : END"<<endl;
	}
	
	if(alltest)
	{
		cout<<"Testing GetCC1"<<endl;
		vector<VID> result;
		result=g2.GetCC(counter);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<result[i]<<endl;
		}
		cout<<"Testing GetCC1 end"<<endl;
	}


	if(alltest)
	{
		cout<<"Testing GetCC2"<<endl;
		vector<Task> result;
		result=g2.GetCC(v_tasks2[counter]);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<result[i].GetTaskWeight()<<endl;
		}
		cout<<"Testing GetCC1 end"<<endl;
	}

	if(alltest)	
	{
		cout<<"Testing GetCCEdges1"<<endl;
		vector< pair<pair<VID,VID>, Weight2> > result ;
		result=g2.GetCCEdges(counter);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<"i: "<<i<<" F: "<<result[i].first.first<<" S: "<<result[i].first.second<<endl;
		}
	
		cout<<"Testing GetCCEdges1 OVER"<<endl;
	
	}
	
	if(alltest)
	{
	
		cout<<"Testing GetCCEdges2"<<endl;
		vector< pair<pair<VID,VID>, Weight2> > result ;
		result=g2.GetCCEdges(counter);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<"i: "<<i<<" F: "<<result[i].first.first<<" S: "<<result[i].first.second<<"  W: "<<result[i].second.Weight()<<endl;
		}
	
		cout<<"Testing GetCCEdges2 OVER"<<endl;
	
	
	}
	
	if(alltest)	
	{
		cout<<"Testing GetCCEdgesVData1"<<endl;
		vector< pair< pair<Task,Task>,Weight2 > > result;
		result=g2.GetCCEdgesVData(counter);	
	
		for(int i=0; i<result.size(); ++i)
		{

			cout<<"i: "<<i<<" F: "<<result[i].first.first.taskwt<<" S: "<<result[i].first.second.taskwt<<"  W: "<<result[i].second.Weight()<<endl;
		}
	
		cout<<"Testing GetCCEdgesVData Over1"<<endl;
	
	}
	
	if(alltest)
	{
		cout<<"TESTING GetEdgesByCCVDataOnly1"<<endl;
		vector< vector<  pair<Task,Task> > > result;
		result=g2.GetEdgesByCCVDataOnly();	

	
		for(int i=0; i<result.size(); ++i)
		{
		
			for(int j=0; j<result[i].size(); ++j)
			{
			
				cout<<"i: "<<i<<" j:"<<j<<" F: "<<result[i][j].first.taskwt<<" S: "<<result[i][j].second.taskwt<<endl;
			}
		}
		cout<<"TESTING GetEdgesByCCVDataOnly OVER1"<<endl;
	}

	if(alltest)
	{

		cout<<"Testing GetCCStats1"<<endl;
		vector< pair<int,VID> > result;
		result=g2.GetCCStats();
		for(int i=0; i<result.size(); ++i)
		{
			cout<<"i: "<<i<<" F: "<<result[i].first<<" S: "<<result[i].second<<endl;
		}
		cout<<"Testing GetCCStats OVER"<<endl;
	}
	
	

	if(alltest)
	{
		cout<<"Testing GetCC Count"<<endl;
		cout<<g2.GetCCcount()<<endl;
		cout<<"Testing GetCC Count : OVER"<<endl;
	}
	

	if(alltest)
	{
	
	  //WG result=g.DijkstraSSSP(counter);
		//result.DisplayGraph();
	
	}
	
	
	if(alltest)
	{
	
	  //WG result=g2.DijkstraSSSP(counter);
		//result.DisplayGraph();
	
	}
	
	if(alltest)
	{
		
		vector< pair<Task,Weight2> > result;
		//result=g.FindPathDijkstra(counter,counter*2);
		double totaldist=0;
		for(int i=0; i<result.size(); ++i)
		{
			
			//cout<<"Second : "<<result[i].second.Weight()<<endl;
			totaldist+=result[i].second.Weight();
		
		}
		
	}
	
	if(alltest)
	{
		
		vector< pair<Task,Weight2> > result;
		//result=g.FindPathDijkstra(counter,counter*2);
		double totaldist=0;
		for(int i=0; i<result.size(); ++i)
		{
			
			//cout<<"Second : "<<result[i].second.Weight()<<endl;
			totaldist+=result[i].second.Weight();
		
		}
		
	}
	
	if(alltest)
	{
		
		vector< pair<Task,Weight2> > result;
		//result=g.FindPathDijkstra(counter,counter*2);
		double totaldist=0;
		for(int i=0; i<result.size(); ++i)
		{
			
			//cout<<"Second : "<<result[i].second.Weight()<<endl;
			totaldist+=result[i].second.Weight();
		
		}
		
	}

	if(alltest)
	{
		
		vector< pair<Task,Weight2> > result;
		//result=g.FindPathDijkstra(counter,counter*2);
		double totaldist=0;
		for(int i=0; i<result.size(); ++i)
		{
			
			//cout<<"Second : "<<result[i].second.Weight()<<endl;
			totaldist+=result[i].second.Weight();
		
		}
		
	}


}




  return 1;
}
