/*!
	\file SchedulerGraphMethods.h
	\author Ping An	
	\date Dec. 14, 2000
	\brief Graph methods specific for Scheduler
*/

#ifndef SchedulerGraphMethods_h
#define SchedulerGraphMethods_h

#include "Graph.h"
#include "GraphAlgo.h"
#include "Comm.h"
#include "Task.h"
#include "Schedule.h"

namespace scheduler {

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO> class SchedulerType;

//===================================================================
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
class SchedulerGraphMethods
{
  
  friend class SchedulerType<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>;

  typedef Task<TASKWEIGHT,TASKINFO> Ttype;
  typedef Comm<COMMWEIGHT,COMMINFO> Wtype;

  typedef Graph<DG<Ttype,Wtype>, NMG<Ttype,Wtype>, 
    WG<Ttype,Wtype>,Ttype,Wtype > WtDiGraph;

  typedef WtVertexType<Ttype,Wtype> Vert;
  typedef WtEdgeType<Ttype,Wtype> WEdge;

typedef  typename vector< Vert >::iterator TI;
typedef  typename vector< Vert >::const_iterator CTI;
typedef  typename vector< WEdge >::iterator EGI;
typedef  typename vector< WEdge >::const_iterator CEGI;

    public:
    SchedulerGraphMethods() {};
    ~SchedulerGraphMethods() {};

    void GraphPreprocessing(WtDiGraph&);
    bool GraphTransform(WtDiGraph& oldg, WtDiGraph& newg, 
			map<VID,VID>& vidmap);

    void AssignTaskLevel(WtDiGraph&);

    vector<VID> SortByLevel(WtDiGraph&);

    private:
    vector<VID> SortTasks(vector<VID>&,WtDiGraph&);

    static bool NumSuccGreater(pair<pair<VID, int>,int >,
			       pair<pair<VID,int>,int>);
    static bool LevelLess(pair<VID, int>, pair<VID,int>);
    static bool LevelLess1(pair<pair<VID, int>,int >, pair<pair<VID,int>,int>);
  };

template <class T>
struct __s1 : public binary_function<T, T, bool> {
  bool operator()(T x, T y) { return x.first < y.first; }
};

  //===================================================================
  template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
  void 
  SchedulerGraphMethods<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
    GraphPreprocessing(WtDiGraph& sgraph) {
/*     sgraph.DisplayGraph(); */
    while(IsCycle(sgraph)) {
      cout<<"There are cycles in the graph. from scheduling."<<endl;
      vector<pair<VID,VID> > v31;
      GetBackedge(sgraph,v31);
      stable_sort(v31.begin(), v31.end(), __s1<pair<VID,VID> >());
      for(int j=0;j < v31.size();j++){
/* 	sgraph.DeleteEdge(v31[j].first, v31[j].second); */
	sgraph.DeleteEdge(v31[j].second, v31[j].first);
	cout<<"Edge ("<<v31[j].first<<" "<<v31[j].second
	    <<") removed. "<<endl;
      }
/*       sgraph.DisplayGraph(); */
    }

    sgraph.SetPredecessors();
    AssignTaskLevel(sgraph);
  };

  template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
  bool
  SchedulerGraphMethods<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
    GraphTransform(WtDiGraph& oldg, WtDiGraph& newg, map<VID,VID>& vidmap) {
      bool trans=false;
      for(int i=0; i<oldg.GetVertexCount(); ++i) {
	if(i != (oldg.begin()+i)->vid ) {
	  trans = true;
	  break;
	}
      }
      if(!trans) {
	newg = oldg;
	return false;
      } else {
	newg.EraseGraph();
	TI vi;
	vi = oldg.begin();
	for(int i=0; i<oldg.GetVertexCount(); ++i, ++vi) {
	  vi->data.SetTaskID(vi->vid);
	  newg.AddVertex(vi->data);
	  vidmap[vi->vid] = i;
	}
	vi = oldg.v.begin();
	for(int i=0; i<oldg.GetVertexCount(); ++i,++vi) {
	  EGI ei= (vi->edgelist).begin();
	  for(int j=0; j<(vi->edgelist).size(); ++j,++ei) {
	    VID v2id = ei->vertex2id;
	    VID nv2id = vidmap[v2id];
	    newg.AddEdge(i,nv2id,ei->weight);
	  }
	}
      }
      return true;
  }

  template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
  void
  SchedulerGraphMethods<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
  AssignTaskLevel(WtDiGraph& sgraph) {
/*     if(GetCCcount(sgraph) > 1) { */
/*       cout<<"There are more than 1 Connected Components in the graph! exit\n"; */
/*       exit(1); */
/*     } */
    deque<VID> q;
    TI v1, v2;
    vector<VID> svec;
    sgraph.GetSources(svec);
    for(IVID iv=svec.begin(); iv!=svec.end(); iv++) {
     q.push_back(*iv);
     sgraph.IsVertex(*iv, &v1);
     v1->data.SetLevel(0);
    }

    VID _v1id,_v2id;
    while(!q.empty()) {   
      _v1id = q.front(); 
      q.pop_front();
      sgraph.IsVertex(_v1id, &v1);
      for(EGI e =v1->edgelist.begin(); e != v1->edgelist.end(); e++) {
	_v2id = e->vertex2id;
	sgraph.IsVertex(_v2id, &v2); 
	int tlevel = v1->data.GetLevel() + 1;
	if(v2->data.GetLevel() < tlevel ) {
/* 	  cout<<"here122 tlevel"<<tlevel<<" for v2id "<<_v2id<<"\n";   */
	  v2->data.SetLevel(tlevel);
	  q.push_back(_v2id);
	}
      }
    }  
#if STAPL_DEBUG1
    cout<<"\nAfter assign task level:"<<endl;
    sgraph.DisplayGraph();
#endif    
  };

  template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
  vector<VID>
  SchedulerGraphMethods<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
  SortByLevel(WtDiGraph& sgraph) {
        vector<pair<VID, int> > vid_level_pairs;
        vector<pair<VID, int> >::iterator ti;

        vector<VID> tmpv;

        for(TI vi=sgraph.v.begin(); vi<sgraph.v.end(); vi++) {
                int _level=vi->data.GetLevel();
                pair<VID, int> newpair(vi->vid, _level);
                vid_level_pairs.push_back( newpair );
        }
        stable_sort(vid_level_pairs.begin(), vid_level_pairs.end(), 
                    ptr_fun(LevelLess) );
        
        for(ti=vid_level_pairs.begin(); ti< vid_level_pairs.end(); ti++) 
                tmpv.push_back(ti->first);

        return tmpv;
    };

  template<class TASKWEIGHT, class COMMWEIGHT, 
    class TASKINFO,class COMMINFO>
  vector<VID> 
  SchedulerGraphMethods<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
  SortTasks(vector<VID>& _lvid,WtDiGraph& sgraph) {
    IVID vi;
    VID _vid,_v2id;
    TI v1;
    EGI ei;
    vector<pair< pair<VID,int>, int> > vidsucc;
    typename vector<pair< pair<VID,int>, int> >::iterator ti;
    vector<VID> tmpv;

    for(vi = _lvid.begin(); vi < _lvid.end(); vi++) {
      _vid = *vi;
      sgraph.IsVertex(_vid,&v1);
      int _level=v1->data.GetLevel();
      pair<VID, int> pair1(v1->vid, _level); 
      int i=0;
      for(ei=v1->edgelist.begin();ei<v1->edgelist.end();ei++) {
	_v2id = ei->vertex2id;
	IVID ci=find(_lvid.begin(),_lvid.end(),_v2id);
	if(ci == _lvid.end()) i++;			
      }
      pair<pair<VID, int>,int> pair2(pair1,i);
      vidsucc.push_back(pair2);
    }

    stable_sort(vidsucc.begin(), vidsucc.end(), ptr_fun(NumSuccGreater) );
    stable_sort(vidsucc.begin(), vidsucc.end(), ptr_fun(LevelLess1) );
                                        
    for(ti=vidsucc.begin(); ti< vidsucc.end(); ti++)
      tmpv.push_back(ti->first.first);
                         
    return tmpv;
  };

  template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
  bool 
  SchedulerGraphMethods<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
  NumSuccGreater(pair<pair<VID, int>,int > _x, 
		      pair<pair<VID,int>,int > _y) {
    return (_x.second > _y.second );
  };

  template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
  bool 
  SchedulerGraphMethods<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
  LevelLess(pair<VID, int> _x, pair<VID,int> _y) {
    return (_x.second < _y.second );
  };

  template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
  bool 
  SchedulerGraphMethods<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
  LevelLess1(pair<pair<VID, int>,int > _x, 
		  pair<pair<VID,int>,int > _y) {
    return (_x.first.second < _y.first.second );
  };
 
} //end namespace
#endif
