/*!
	\file TaskScheduler.h
	\author Ping An	
	\date Dec. 14, 2000
	\brief Generic Clustering Algorithms for uniform clique systems

	1	Wavefront, critical path, dominent sequence clustering for DAG
	2	Join Latest Predecessor	for In_Tree graph
	3	Function to get optimal makespan (critical path)

	Input:
	  a dependence graph (with format defined by Graph.h & Task.h);

	Output:
	  Schedule
*/

#ifndef TaskScheduler_h
#define TaskScheduler_h

#include "Graph.h"
#include "Comm.h"
#include "Task.h"
#include "Schedule.h"
#include "SchedulerGraphMethods.h"
#include "TaskSchedAux.h"

namespace scheduler {

//===================================================================
//===================================================================
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
class TaskScheduler
{
 public:
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

  typedef  SchedulerGraphMethods<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO> 
    SchedulerGraphMethods_type;

  typedef vector<TASKWEIGHT> TW_vector;
  typedef vector<VID> VID_vector;
  typedef vector<PID> PID_vector;
  typedef vector< ProcPair > PP_vector;
  typedef priority_queue<ProcPair, vector< ProcPair >, PP_GT> 
    PP_priority_queue;
  typedef multiset< ProcPair, PP_LT > PP_multiset;
  typedef multiset< TaskTriplet, TT_Cmp > TT_multiset;
  typedef vector< TT_multiset > TT_multiset_vector;
//===================================================================
//  Data
//===================================================================
 protected:

  WtDiGraph dependent_graph;
  Schedule schedule;
  vector<VID> sourcevids;
  vector<VID> vsortbylevel;

 public:
  TW_vector tlevel_list;        // tlevel of each vertex
 private:

  // common data structures
  Schedule newSchedule;
  TW_vector blevel_list;        // blevel of each vertex
  vector<int> indegree_list;    // indegree of each vertex
  TW_vector FT_list;            // finish time of each vertex
  PID_vector PROC_list;         // assigned proc of each vertex
  PID_vector clusterID_list;    // cluster owner for each task

  // FLB-only data structures
  TW_vector PRT_list;           // ready time of each proc
  TW_vector EMT_list;           // EMT of each task
  TW_vector LMT_list;           // LMT of each task
  PID_vector EP_list;           // EP of each task
  TT_multiset_vector EMT_EPtask_list; // list of each proc's EPtasks, s.by -EMT
  TT_multiset_vector LMT_EPtask_list; // list of each proc's EPtasks, s.by -LMT
  PP_multiset allProc_list;     // list of all procs, s. by -PRT
  PP_multiset activeProc_list;  // list of procs with EP tasks, s. by -min EST
  TT_multiset nonEPtask_list;   // list of nonEP tasks, sorted by -LMT

  // FCP data structures
  PP_priority_queue task_pq;

 public:
//===================================================================
//  Constructors and Destructor
//===================================================================
  TaskScheduler();
  TaskScheduler(WtDiGraph&);
  ~TaskScheduler();

  //===================================================================
  //  Other Methods
  //===================================================================

  /*!
    \brief Algorithms for task scheduling
  */
  Schedule CriticalPathClustering();
  Schedule DominantSequence();
  Schedule FastLoadBalance(int);
  Schedule FastCriticalPath(int);

  /*Schedule JoinLatestPredecessor(); */
  /*Schedule WaveFront_BFS(); */
  /*Schedule HeavyEdgeMerge();    //Need to decide on the Getmakespan */
	
  //Get optimal makespan
  vector<VID> GetCriticalPath(TASKWEIGHT&);

  //===================================================================
 private:	      
  //used in JoinLatestPredecessor
  void Find2LatestPredecessors(VID, VID&, VID&);

  //used in CriticalPathClustering
  vector< VID > CriticalPath(vector<int> &, vector<VID>);

  //used in HeavyEdgeMerge
  static bool WtGreater(pair< pair<VID,VID>, COMMWEIGHT  > ,
			pair< pair<VID,VID>, COMMWEIGHT > );

  //used in DSC algorithm
  VID DSC_FindLatestInQPred(PID, Schedule&, vector<TASKWEIGHT> &);
  void GetBottomBound( vector<TASKWEIGHT> & );
  VID my_max( vector< pair<VID, TASKWEIGHT> >& );
  void DeleteL(VID&, vector< pair<VID, TASKWEIGHT> >& );
  void MinimizeT(VID&, VID&, vector<TASKWEIGHT> &, bool&, vector<bool>&,
		 Schedule&);
  static bool PreWtGreater(pair<VID, TASKWEIGHT> ,
			   pair<VID, TASKWEIGHT> );
  static bool PreWtLess(pair<VID, TASKWEIGHT> ,
			pair<VID, TASKWEIGHT> );

  //used in FLB algorithm
  void FLB_Setup(int);
  void FLB_ScheduleTask(int &, int &);
  void FLB_UpdateTaskLists(int, int);
  void FLB_UpdateProcLists(int, int);
  void FLB_UpdateReadyTasks(int, int);

  //used in FCP algorithm
  void FCP_Setup(int);

  //miscellaneous routines
  void CalculateEP(int);
  void CalculateEPandLMT(int);
  void CalculateEMT(int, int);
  TASKWEIGHT GetEST(int, int);
  void PP_MS_remove(PP_multiset &, int);
  void TT_MS_remove(TT_multiset &, int);
  TASKWEIGHT my_max2 (vector<TASKWEIGHT>& fp);

 public:
  void AssignBlevels();
  void AssignTlevels();
  void AssignBlevels(WtDiGraph&);
  void AssignTlevels(WtDiGraph&);
  void AssignALAPLevel();

};

//================================================================
//  Constructors and Destructor
//================================================================
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
TaskScheduler() {
};

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
TaskScheduler(WtDiGraph& _g) {
	AssignBlevels(_g);
	_g.SetPredecessors();
        dependent_graph = _g;
	dependent_graph.GetSources(sourcevids);
	SchedulerGraphMethods_type sgm;
	vsortbylevel = sgm.SortByLevel(dependent_graph);
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
~TaskScheduler() {
};
                
//================================================================
//	Other Methods
//================================================================

//================================================
	//Algorithms for tasks scheduling  
//        Schedule CriticalPathClustering();
//        Schedule HeavyEdgeMerge();
//        Schedule DominantSequence ();
//================================================
  
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO> 
Schedule
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
CriticalPathClustering() {
             
  CTI v1,vv1;
  VID _sourcevid;
  Schedule s1;

  vector< VID > cpp;
  vector< VID > samelevelvid;
  int nv = dependent_graph.GetVertexCount();
  vector<int> mark(nv,0);

  bool allmark = false;
  while( !allmark ) {
    _sourcevid = -1;
    int _plevel = nv+1;
    for(v1 = dependent_graph.v.begin(); 
	v1 != dependent_graph.v.end(); v1++) {
      if(mark[v1->vid] == 0) {
	int _level = v1->data.GetLevel();
	if( _level < _plevel) {
	  _sourcevid = v1->vid;
	  _plevel = _level;
	  vv1 = v1;
	}  
      }  
    }

    if (_sourcevid == -1) allmark = true;
    else {
      for( ; vv1 != dependent_graph.v.end(); vv1++) {
	int _level = vv1->data.GetLevel();
	if(mark[vv1->vid] == 0 && _level == _plevel)
	  samelevelvid.push_back(vv1->vid);
      }
     
      cpp = CriticalPath(mark, samelevelvid);
      samelevelvid.erase(samelevelvid.begin(), samelevelvid.end());
      for(IVID iicp = cpp.begin(); iicp!=cpp.end(); iicp++) {
	mark[*iicp] = 1;
      }
      s1.AddTaskSet(cpp);
    }
  }
  s1.AlgName = "CPM";
  return s1;
};
             

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
Schedule
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
DominantSequence () {  
   
        VID _v1id,_v2id, fvid, pvid;
        TI v1, v2;
        int i=0;
	Schedule s1;

        bool mflag=true;
        TASKWEIGHT tmp=0;

	typedef pair<TASKWEIGHT, VID> TW_VID;
	typedef multiset< TW_VID, greater< TW_VID > > TW_VID_MS;
	TW_VID_MS pfl;
	priority_queue< TW_VID > fl;
        typedef typename vector < pair<VID, TASKWEIGHT> >::iterator PFL;
                        
	EGI ee;
        int nv=dependent_graph.GetVertexCount();   
        vector<TASKWEIGHT> t(nv,0); //top bounds 
        vector<TASKWEIGHT> b(nv,0); //bottom bounds
        vector<TASKWEIGHT> pri(nv,0); //priority = t + b
        vector<bool> mark(nv,0); //schedule flag
        vector<bool> addf(nv,0); //add to fl only once
        vector<bool> addpf(nv,0); //add to pfl only once
  
        GetBottomBound(b);   
        for(i=0; i<dependent_graph.GetVertexCount(); i++) {
                pri[i] = t[i]+b[i];
        }

	for(IVID iv=sourcevids.begin(); iv!=sourcevids.end(); iv++) {
	  TW_VID newfl(pri[*iv], *iv);
	  fl.push(newfl);
	}

 while( !fl.empty() ) {
    fvid = fl.top().second; //free task with maximum priority
    fl.pop();
    if (pfl.size() == 0) {
      pvid = 0;
    } else {
      pvid = pfl.begin()->second;
    }
        if(pri[fvid] >= pri[pvid] ) {
                mflag = false;
                MinimizeT(fvid, pvid, t, mflag, mark,s1);
        }
        else {
                mflag = true;
                MinimizeT(fvid, pvid, t, mflag, mark,s1);
        }
        mark[fvid] = true;
        
	//compute t[] of fvid's successors and updage fl & pfl
	dependent_graph.IsVertex(fvid,&v1);
        for (CEGI e = v1->edgelist.begin(); e!=v1->edgelist.end(); e++) {
            _v2id = e->vertex2id;
            tmp = t[fvid] + dependent_graph.v[fvid].data.GetTaskWeight() +
	          e->weight.GetMessageSize();
            if( tmp > t[_v2id] ) t[_v2id] = tmp;
            pri[_v2id] = t[_v2id] + b[_v2id];
            if ( !addf[_v2id] ) {
	      dependent_graph.IsVertex(_v2id,&v2);
                bool free = true;
                for (ee =v2->predecessors.begin();ee!=v2->predecessors.end(); ee++) {
                        _v1id = ee->vertex2id;
                        if(!mark[_v1id]) {
                          free = false;
                          if(!addpf[_v2id]) {
			    TW_VID newpfl(pri[_v2id], _v2id);
			    pfl.insert(newpfl);
			    addpf[_v2id] = true;
                          }
                          break;
                        }
                        
                }
                if(free) {  
		  TW_VID newfl(pri[_v2id], _v2id);
		  fl.push(newfl);
		  addf[_v2id] = true;
		  TW_VID newpfl(pri[_v2id], _v2id);
		  typename TW_VID_MS::iterator vid_loc = pfl.find(newpfl);
		  if (vid_loc != pfl.end())
		    pfl.erase(vid_loc);
                }
            }
        }
  }
  s1.AlgName = "DSC";
  return s1;            

};


//=============================================
   //Methods to get optimum makespan
//        vector<VID> GetCriticalPath(TASKWEIGHT&);
//=============================================

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
vector<VID>
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
GetCriticalPath(TASKWEIGHT& cwt ) {
	vector<VID> criticalpath;
	int nv=dependent_graph.GetVertexCount();
        vector<int> mark(nv,0);

 	criticalpath=CriticalPath(mark,sourcevids);

	cwt=0;
        for(IVID ii=criticalpath.begin(); ii!=criticalpath.end(); ii++)
              cwt+=dependent_graph.v[*ii].data.GetTaskWeight();

	return criticalpath;
};
           
//===================================================================
//===================================================================
	//Private Methods
//===================================================================

//===================================================================
	//used in JLP
//      void Find2LatestPredecessors(VID, VID, VID);

	//used in CPClustering
//      vector< VID > CriticalPath(vector<int> &, vector<VID>);   
//      static bool WtGreater(const pair< pair<VID, VID>, COMMWEIGHT>& ,
//      const pair< pair<VID, VID>, COMMWEIGHT>& )
//===================================================================
         
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
Find2LatestPredecessors(VID _vid, VID& _v1id, VID& _v2id) {
        
  TI v1, vtmp1, vtmp2;
  CEGI e12;
  COMMWEIGHT tmpwt1=0, tmpwt2=0, cost;
                     
  for (v1 = dependent_graph.v.begin(); v1 != dependent_graph.v.end(); v1++) {
        if( v1->IsEdge(_vid, &e12) ) {
                cost =v1->data.GetFinishTime()+v1->GetEdgeWeight(_vid).GetMessageSize();
                if(tmpwt1 <= cost ) {
                        tmpwt1 = cost;
                        vtmp1 = v1;
                }
                else if(tmpwt2 < cost) {
                        tmpwt2 = cost;
                        vtmp2 = v1;
                }
        }
   }
         _v1id = vtmp1->vid;
         _v2id = vtmp2->vid;
};  

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
vector<VID>
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
CriticalPath(vector<int> &mark, vector<VID> samelevelvid) {

        CTI v1;
        VID _v1id, _vid, _v2id;
        
        vector< VID > criticalpath;
        vector< VID > cp;
        
	int nt = dependent_graph.GetVertexCount();
        vector<TASKWEIGHT> d(nt,NINFINITE);
        vector<VID> a(nt,0);
        
        TASKWEIGHT tmp,cwt=0,cost;
          
	for(IVID ii = samelevelvid.begin(); ii!=samelevelvid.end(); ii++) 
	  d[*ii] = dependent_graph.v[*ii].data.GetTaskWeight();
        
//Relax
        _v1id = (VID) 0;
        d[_v1id] = 0;
	//check
        for(IVID vs=vsortbylevel.begin(); vs != vsortbylevel.end()-1; vs++) {
	  _vid=*vs;
	  dependent_graph.IsVertex(_vid,&v1);
	  tmp = 0.0;
	  if ( mark[_vid] == 0 ) {
	    for (CEGI e1=v1->edgelist.begin(); e1!=v1->edgelist.end();e1++) {
	      _v2id = e1->vertex2id;
	      if (mark[_v2id] == 0 ) {
		d[_v2id] = d[_vid] + 
		  dependent_graph.v[_vid].GetEdgeWeight(_v2id).GetMessageSize()
		  + dependent_graph.v[_v2id].data.GetTaskWeight();
		if( d[_v2id] > tmp ) {
		  tmp = d[_v2id];
		  a[_vid] = _v2id;
		}
	      }
	    }
	    if(d[_vid] > d[_v1id]) _v1id = _vid;
	  }
        }
        
	//record critical path  
	cost=NINFINITE;
	for(IVID ij = samelevelvid.begin(); ij!= samelevelvid.end(); ij++) {
	  VID _v3id = *ij;
	  cp.push_back(_v3id);
	  if( a[_v3id] !=0 ) {
	    _v1id = _v3id;
	    while( a[_v1id] != 0) {
	      cp.push_back(a[_v1id]);
	      _v1id = a[_v1id];
	    }
	  }
	  cwt=dependent_graph.v[*cp.begin()].data.GetTaskWeight();
	  for(IVID ii=cp.begin()+1; ii!=cp.end(); ii++) {
	    cwt+=dependent_graph.v[*ii].data.GetTaskWeight()+
	      dependent_graph.v[*(ii-1)].GetEdgeWeight(*ii).GetMessageSize();
	  }
	  if(cwt>cost) {
	    criticalpath = cp;
	    cost = cwt;
	  }
	  cp.erase(cp.begin(), cp.end());
	  cwt=0;
	}
	return criticalpath;
};

// used to sort alledges in Heavy Edge Merge
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
bool
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
WtGreater(pair< pair<VID, VID>, COMMWEIGHT> _x, 
	  pair< pair<VID,VID>, COMMWEIGHT> _y) {
    return (_x.second > _y.second );
};

//===================================================================
        // Methods used in DSC algorithm
//        VID DSC_FindLatestInQPred(PID, Schedule&, vector<TASKWEIGHT>&);
//        void GetBottomBound(vector<TASKWEIGHT> & );
//        VID my_max( vector< pair<VID, TASKWEIGHT> >& );
//        void MinimizeT(VID& , VID& , vector<TASKWEIGHT> &, bool&,
//                      vector<bool> & , VID&, Schedule&);
//        static bool PreWtGreater(const pair<VID, TASKWEIGHT>& ,const
//                                pair<VID, TASKWEIGHT>& );
//        static bool PreWtLess(const pair<VID, TASKWEIGHT>& ,const
//                                pair<VID, TASKWEIGHT>& );
//===================================================================
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>   
VID
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>:: 
DSC_FindLatestInQPred(PID _pid, Schedule& s1, vector<TASKWEIGHT> &t) {
  GII p1;
  COMMWEIGHT tmpwt1=NINFINITE;
  VID _v2id;
  bool found = false;
                                
  const TaskSet &ref_ts = s1.GetTaskSetbyPID(_pid);
  TASKWEIGHT tw;
  VID nodeid;
  CIVID ts_iter, ts_end;
  for (ts_iter = ref_ts.taskset.begin(), ts_end = ref_ts.taskset.end();
       ts_iter != ts_end; ts_iter++) {
    nodeid = *ts_iter;
    tw = t[*ts_iter] + dependent_graph.GetData(nodeid).GetTaskWeight();
    if (tmpwt1 < tw) {
      tmpwt1 = tw;
      _v2id = nodeid;
      found = true;
    }
  }
  if (found) return _v2id;
  return INVALID_VID;
};
  
  template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
GetBottomBound( vector<TASKWEIGHT> &b) {
  list<VID> q;
  CTI cv1,cv2;
  TI v1,v2;
  VID v1id, v2id;
  TASKWEIGHT tmp;
  EGI ee;

        vector<bool> mark; //each vid only pushed into q once
        mark.reserve(dependent_graph.GetVertexCount());
        for(int i=0; i<dependent_graph.GetVertexCount(); i++) mark[i] = false;

        vector<VID> sink;
	dependent_graph.GetSinks(sink);

        for (v1=dependent_graph.v.end()-1; v1 >= dependent_graph.v.begin(); v1--) {
		if(v1->edgelist.empty()) {
			b[v1->vid] = v1->data.GetTaskWeight();
		}
        }
                
 for(IVID s1 = sink.begin(); s1 != sink.end(); s1++) {
  VID _startVid = *s1;
  if ( dependent_graph.IsVertex(_startVid,&v1) ) {  
     q.push_back(_startVid);
     mark[_startVid] = true;
  } else {
     cout << "\nIn GraphBFS: sink vid=" << _startVid << " not in graph";
  }
  while ( !q.empty() ) {
     v1id = q.front();
     if ( dependent_graph.IsVertex(v1id,&v1) ) {
       for ( ee = v1->predecessors.begin(); ee != v1->predecessors.end(); ee++) {
         v2id = ee->vertex2id;
         if ( dependent_graph.IsVertex(v2id,&v2) ) {
            if(!mark[v2id]) {
                mark[v2id] = true;
                q.push_back(v2id);
            }
            tmp = b[v1id] + v2->data.GetTaskWeight() + ee->weight.GetMessageSize();
            if( tmp > b[v2id] ) b[v2id] = tmp;
         }
       }
     } else {
       cout << "\nIn Graph: vertex=" << v1id << " not in graph";
     }
     q.pop_front();
  }

 }
        
}; 
        
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
VID
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
my_max( vector< pair<VID, TASKWEIGHT> >& fp) {   
                        
        TASKWEIGHT tmp=0;
        VID mvid=0;
        typename vector < pair<VID, TASKWEIGHT> >::iterator _p;
        for(_p = fp.begin(); _p!=fp.end(); _p++) {
                if( _p->second >= tmp ) {
                        mvid = _p->first;
                        tmp = _p->second;
                }
        }
        return mvid;
};
            
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
MinimizeT(VID& fvid, VID& pvid, vector<TASKWEIGHT> &t, bool&
mflag, vector<bool> &mark, Schedule& s1) {
        TI v1, v2; 
        PID _pid, _p2id;
        VID _v1id, _v2id, _bvid;
        bool flag3 = true, flag=false;
        int i=0, j=0,jj;
        TASKWEIGHT tmp=0, twt, tmp1, tmp2,v1wt;
        
        vector< pair< VID, TASKWEIGHT> > tc;
        typedef typename vector< pair< VID, TASKWEIGHT> >::iterator TC;
        TC t1;
	EGI e;  
	GII p1i;
                
//============================================
//all fvid's predecessors. Get tc and sort it
//============================================
	dependent_graph.IsVertex(fvid, &v1);
for ( e = v1->predecessors.begin(); e!=v1->predecessors.end(); e++) {
     _v2id = e->vertex2id;
     TASKWEIGHT _tmp = t[_v2id] + dependent_graph.v[_v2id].data.GetTaskWeight()
       + e->weight.GetMessageSize();
     pair<VID, TASKWEIGHT> newtc(_v2id, _tmp);
     tc.push_back( newtc );
}

if( tc.empty() ) 
	{ s1.AddTaskSet(fvid); return; }

//Start only from level 2 tasks
     stable_sort (tc.begin(), tc.end(), ptr_fun(PreWtGreater) );

//============================================
//the first predecessor may have child other than fvid
//find the latest vid in that cluster
//============================================
_v1id = tc.begin()->first;
  _pid = s1.GetTaskSetPID(_v1id);
_v2id = DSC_FindLatestInQPred(_pid,s1,t);
if(_v2id != _v1id && _v2id != INVALID_VID )   {
        tc.begin()->first = _v2id;
	tc.begin()->second = t[_v2id] + 
	  dependent_graph.v[_v2id].data.GetTaskWeight();
}

//============================================
//other predecessors cannot have child other than fvid
//if they do, erase them from tc
//============================================
if(tc.size()>1) {
  vector< pair< VID, TASKWEIGHT> > t_erase;
  for(t1 = tc.begin()+1; t1 != tc.end(); t1++) {
        _v2id = t1->first;
	_p2id = s1.GetTaskSetPID(_v2id);
        if(_pid != _p2id) {
	  dependent_graph.IsVertex(_v2id,&v2);
            for(CEGI e1 = v2->edgelist.begin(); e1!=v2->edgelist.end(); e1++) {
                if(e1->vertex2id != fvid) t_erase.push_back(*t1);
            }
        }
   }

 for(TC t2=t_erase.begin(); t2!=t_erase.end(); t2++) {
  	_v1id = t2->first;
  	for(t1 = tc.begin()+1; t1 != tc.end(); t1++) {
        	_v2id = t1->first;
		if(_v1id == _v2id) { tc.erase(t1); break; }
  	}
 }

}
//==========================================        
//CheckDSRW: pvid's t[] can be reduced or not
//if can be reduced, flag3=false
//==========================================        
 if ( mflag ) {
   dependent_graph.IsVertex(pvid,&v1);
        for ( e = v1->predecessors.begin(); e !=v1->predecessors.end(); e++) {
            _v2id = e->vertex2id;
            if(mark[_v2id]) {
                tmp = t[_v2id] + dependent_graph.v[_v2id].data.GetTaskWeight();
                if(t[pvid] > tmp ) {
                        _bvid = _v2id;
                        flag3 =false; 
                        break;
                }
            }
        }
  }   
//==========================================        
//the cluster that pvid can be reduced will be touched or not
//if not touched, flag3=true
//==========================================        
    if (mflag  && !flag3) {
	_p2id = s1.GetTaskSetPID(_bvid);
	if (_p2id != _pid ) flag3 = true;
    }
         
//==========================================        
//find critical point _bvid in clustering fvid's predecessors
//i denotes the critical point _bvid
//==========================================        
if( (!mflag || flag3) ) {
    _v1id = tc.begin()->first;
    tmp1 = t[_v1id] + dependent_graph.v[_v1id].data.GetTaskWeight();
    if( tc.size() == 1 ) {
	i=2;
	_bvid = INVALID_VID;
    }
    else if( tc.size() == 2 ) {
	i=2;
	_bvid = (tc.begin()+1)->first;
    }
    else {
	i=1;
    	tmp = 0;
        for(t1 = tc.begin(); t1 != tc.end(); t1++) {
                i++;
                if( i > tc.size() ) {
                        _bvid = INVALID_VID;
                        break;   
                }
                _v1id = t1->first;
		v1wt= dependent_graph.v[_v1id].data.GetTaskWeight();
                tmp = tmp + v1wt;
                _v2id = (t1+1)->first;
                _bvid = _v2id;
                twt=dependent_graph.v[_v2id].GetEdgeWeight(fvid).GetMessageSize();
                if( twt < tmp ) {
                        _bvid = _v1id;
                        i--;
                        break;
                }
        }
	//=====================================
	//sort the clustered predecessors by t[] in increasing order (execution order)

        for(t1 = tc.begin(), jj=1; jj<i; t1++, jj++) 
                t1->second = t[t1->first];
        stable_sort (tc.begin(), tc.begin()+i-1, ptr_fun(PreWtLess) );

	//=====================================
	//update t[fvid]
	//t[fvid]=tmp2  if _bvid finishes later than those clustered

	_v1id = tc.begin()->first;
	tmp1=t[_v1id];
        for(t1 = tc.begin(), jj=1; jj<i; jj++, t1++) {
                _v1id = t1->first;
		v1wt=dependent_graph.v[_v1id].data.GetTaskWeight();
                tmp1 = tmp1 + v1wt;
        }
   
        tmp2 = 0;
        if (  _bvid != INVALID_VID ) {
                twt=dependent_graph.v[_bvid].GetEdgeWeight(fvid).GetMessageSize();
                tmp2 = t[_bvid] + 
		  dependent_graph.v[_bvid].data.GetTaskWeight() + twt;
	}
        tmp1 = max(tmp1, tmp2);

    }  //else

        if ( t[fvid] >= tmp1 ) {
                t[fvid] = tmp1;
                flag = true;
        } else flag = false;
}	//if( (!mflag || flag3) )

//=====================================
//assign clustered predecessors to the processor which is the first one in
//the first sorted tc list, w/ _pid
//=====================================
if( ( !mflag||flag3) && flag) {
	for( t1 = tc.begin(), j=1; j < i; t1++, j++) {
        	_v2id = t1->first;
		_p2id = s1.GetTaskSetPID(_v2id);
                if(_p2id != _pid) {
                	s1.DeleteTask( _v2id );
                        s1.AddTask(_pid, _v2id);
                }
        }
        s1.AddTask(_pid, fvid);
}
else    s1.AddTaskSet(fvid);
};
                
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
bool
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
PreWtGreater(pair<VID, TASKWEIGHT> _x, pair<VID, TASKWEIGHT>
_y ) {
                return (_x.second > _y.second );
}; 

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
bool
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
PreWtLess(pair<VID, TASKWEIGHT> _x, pair<VID, TASKWEIGHT>
_y ) {   
                return (_x.second < _y.second );
};

//======================================================================
//
// FLB algorithm
//
//======================================================================

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
Schedule
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
FastLoadBalance(int target_np)
{
  FLB_Setup(target_np);

  int rem_tasks = dependent_graph.GetVertexCount();
  int sch_pid, sch_tid;
  while (rem_tasks > 0) {
    FLB_ScheduleTask(sch_pid, sch_tid);
    FLB_UpdateTaskLists(sch_pid, sch_tid);
    FLB_UpdateProcLists(sch_pid, sch_tid);
    FLB_UpdateReadyTasks(sch_pid, sch_tid);
    rem_tasks--;
  }
  newSchedule.SetName("FLB");
  return newSchedule;
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
FLB_Setup(int np)
{
  int nvertices = dependent_graph.GetVertexCount();
  FT_list.reserve(nvertices);
  PROC_list.reserve(nvertices);
  PROC_list.assign(nvertices, -1);
  PRT_list.reserve(np);
  PRT_list.assign(np, 0.0);
  EMT_list.reserve(nvertices);
  LMT_list.reserve(nvertices);
  EP_list.reserve(nvertices);
  EP_list.assign(nvertices, -1);
  EMT_EPtask_list.reserve(np);
  LMT_EPtask_list.reserve(np);
  AssignBlevels();

  for (VID i = 0; i < nvertices; i++) {
    vector<VID> tmp;
    dependent_graph.GetPredecessors(i,tmp);
    int indegree = tmp.size();
    indegree_list.push_back(indegree);
    if (indegree == 0) {
      EMT_list[i] = 0.0;
      LMT_list[i] = 0.0;
      nonEPtask_list.insert(TaskTriplet(0.0, blevel_list[i], i));
    }
  }

  for (PID p = 0; p < np; p++) {
    TT_multiset tmp_vector, tmp_vector2;
    EMT_EPtask_list.push_back(tmp_vector);
    LMT_EPtask_list.push_back(tmp_vector2);
    allProc_list.insert(ProcPair(0.0, p));
    newSchedule.AddTaskSet();
  }
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
FLB_ScheduleTask(int &sch_pid, int &sch_tid)
{
  PID EP_pid, earliest_pid;
  VID EP_tid, nonEP_tid;
  if (! activeProc_list.empty()) {
    EP_pid = activeProc_list.begin()->id;
    EP_tid = EMT_EPtask_list[EP_pid].begin()->id;
  } else {
    EP_pid = -1;
    EP_tid = -1;
  }
  earliest_pid = allProc_list.begin()->id;
  if (! nonEPtask_list.empty()) {
    nonEP_tid = nonEPtask_list.begin()->id;
  } else {
    nonEP_tid = -1;
  }

  if (EP_pid == -1) {
    sch_pid = earliest_pid;
    sch_tid = nonEP_tid;
    newSchedule.AddTask(sch_pid, sch_tid);
    // Remove proc from all proc list
    allProc_list.erase(allProc_list.begin());
    // Remove task from nonEP task list
    nonEPtask_list.erase(nonEPtask_list.begin());
  } else if (nonEP_tid == -1) {
    sch_pid = EP_pid;
    sch_tid = EP_tid;
    newSchedule.AddTask(sch_pid, sch_tid);
    // Remove proc from active proc list
    PP_MS_remove(activeProc_list, sch_pid);
    // Remove task from proc's EMT EP task list
    EMT_EPtask_list[sch_pid].erase(EMT_EPtask_list[sch_pid].begin());
    // Remove task from proc's LMT EP task list
    TT_MS_remove(LMT_EPtask_list[sch_pid], sch_tid);
    // Remove proc from all proc list
    PP_MS_remove(allProc_list, sch_pid);
  } else if (GetEST(EP_pid, EP_tid) < GetEST(earliest_pid, nonEP_tid)) {
    sch_pid = EP_pid;
    sch_tid = EP_tid;
    newSchedule.AddTask(sch_pid, sch_tid);
    // Remove proc from active proc list
    PP_MS_remove(activeProc_list, sch_pid);
    // Remove task from proc's EMT EP task list
    EMT_EPtask_list[sch_pid].erase(EMT_EPtask_list[sch_pid].begin());
    // Remove task from proc's LMT EP task list
    TT_MS_remove(LMT_EPtask_list[sch_pid], sch_tid);
    // Remove proc from all proc list
    PP_MS_remove(allProc_list, sch_pid);
  } else {
    sch_pid = earliest_pid;
    sch_tid = nonEP_tid;
    newSchedule.AddTask(sch_pid, sch_tid);
    // Remove proc from all proc list
    allProc_list.erase(allProc_list.begin());
    // Remove task from nonEP task list
    nonEPtask_list.erase(nonEPtask_list.begin());
    // Remove proc from active proc list
    if (! EMT_EPtask_list[sch_pid].empty())
      PP_MS_remove(activeProc_list, sch_pid);
  }

  TASKWEIGHT st = GetEST(sch_pid, sch_tid);
  FT_list[sch_tid] = st + dependent_graph.GetData(sch_tid).GetTaskWeight();
  PROC_list[sch_tid] = sch_pid;
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
FLB_UpdateTaskLists(int sch_pid, int sch_tid)
{
  TT_multiset &ref_lmt_vec = LMT_EPtask_list[sch_pid];
  TT_multiset &ref_emt_vec = EMT_EPtask_list[sch_pid];
  TASKWEIGHT newPRT = GetEST(sch_pid, sch_tid)
    + dependent_graph.GetData(sch_tid).GetTaskWeight();
  // Remove EP tasks whose LMT > PRT of proc
  while (! ref_lmt_vec.empty()) {
    VID t = ref_lmt_vec.begin()->id;
    if (LMT_list[t] >= newPRT) break;
    // Remove EP task from LMT list
    ref_lmt_vec.erase(ref_lmt_vec.begin());
    // Remove EP task from EMT list
    TT_MS_remove(ref_emt_vec, t);
    // update EMT for nonEP task
    CalculateEMT(-1, t);
    nonEPtask_list.insert(TaskTriplet(LMT_list[t], blevel_list[t], t));
  }
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
FLB_UpdateProcLists(int sch_pid, int sch_tid)
{
  // update PRT of proc and place in all proc list
  PRT_list[sch_pid] = GetEST(sch_pid, sch_tid)
    + dependent_graph.GetData(sch_tid).GetTaskWeight();
  allProc_list.insert(ProcPair(PRT_list[sch_pid], sch_pid));
  // add proc back to active proc lists if it has EP tasks
  // proc should have been removed from active proc lists by now
  if (! EMT_EPtask_list[sch_pid].empty()) {
    activeProc_list.insert(ProcPair(GetEST(sch_pid, sch_tid), sch_pid));
  }
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
FLB_UpdateReadyTasks(int sch_pid, int sch_tid)
{
  VID_vector succ_list;
  dependent_graph.GetSuccessors(sch_tid,succ_list);
  CIVID succ_end = succ_list.end();
  PID EP_pid;
  
  for (CIVID i = succ_list.begin(); i != succ_end; i++) {
    VID tid = *i;
    indegree_list[tid]--;
    if (indegree_list[tid] == 0) {
      CalculateEPandLMT(tid);
      EP_pid = EP_list[tid];
      CalculateEMT(EP_pid, tid);
      if (LMT_list[tid] < PRT_list[EP_pid]) {
        nonEPtask_list.insert(TaskTriplet(LMT_list[tid], blevel_list[tid], tid));
      } else {
        if (EMT_EPtask_list[EP_pid].empty()) {
          activeProc_list.insert(ProcPair(GetEST(EP_pid, tid), EP_pid));
        } else {
          VID v = EMT_EPtask_list[EP_pid].begin()->id;
          if (EMT_list[tid] < EMT_list[v]) {
            // Remove proc from active proc list
            PP_MS_remove(activeProc_list, EP_pid);
            // Insert proc back into active proc list with new EST
            activeProc_list.insert(ProcPair(GetEST(EP_pid, tid), EP_pid));
          }
        }
        EMT_EPtask_list[EP_pid].insert(TaskTriplet(EMT_list[tid], 
                                                   blevel_list[tid], tid));
        LMT_EPtask_list[EP_pid].insert(TaskTriplet(LMT_list[tid],
                                                   blevel_list[tid], tid));
      }
    }
  }
}

//======================================================================
//
// Fast Critical Path
//
//======================================================================

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
Schedule
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
FastCriticalPath(int target_np)
{
  // Setup for FCP
  FCP_Setup(target_np);
  int epc=0, eic=0;

  int rem_tasks = dependent_graph.GetVertexCount();
  VID sch_tid, tid;
  PID sch_pid, ep_pid, ei_pid;
  while (rem_tasks > 0) {
    // Select ready task
    sch_tid = task_pq.top().id;
    task_pq.pop();
/*     cout<<"pop vid:"<<sch_tid<<endl; */

    // Select processor
    ep_pid = EP_list[sch_tid];
    ei_pid = allProc_list.begin()->id;
    if ( ep_pid == -1) {
      allProc_list.erase(allProc_list.begin());
      sch_pid = ei_pid; eic++;
    } else if (GetEST(ep_pid, sch_tid) < GetEST(ei_pid, sch_tid)) {
      PP_MS_remove(allProc_list, ep_pid);
 	epc++;
     sch_pid = ep_pid;
    } else {
      allProc_list.erase(allProc_list.begin());
      sch_pid = ei_pid; eic++;
    }    

    // Schedule task
    //fixed a bug in computing finishtime
    //LMT_list should be used instead of EMT_list
    //when the enabling P is not used.

/*     FT_list[sch_tid] = GetEST(sch_pid, sch_tid)   --bug */
/*       + dependent_graph.GetData(sch_tid).GetTaskWeight(); */
    TASKWEIGHT estart;
    if(sch_pid == ep_pid) 
      estart = EMT_list[sch_tid]>PRT_list[sch_pid]? EMT_list[sch_tid]:PRT_list[sch_pid]; 
    else
      estart = LMT_list[sch_tid]>PRT_list[sch_pid]? LMT_list[sch_tid]:PRT_list[sch_pid]; 
      
    FT_list[sch_tid] = estart+ dependent_graph.GetData(sch_tid).GetTaskWeight();
    PROC_list[sch_tid] = sch_pid;
    PRT_list[sch_pid] = FT_list[sch_tid];
    allProc_list.insert( ProcPair(PRT_list[sch_pid], sch_pid) );
/*     cout<<"sch_pid:"<<sch_pid<<" vid:"<<sch_tid<<endl; */
    newSchedule.AddTask(sch_pid, sch_tid);
/* 	cout<<"sched vid:"<<sch_tid<<endl; */

    // Add newly ready tasks
    VID_vector succ_list;
    dependent_graph.GetSuccessors(sch_tid,succ_list);
    CIVID succ_end = succ_list.end();
    for (CIVID i = succ_list.begin(); i != succ_end; i++) {
      tid = *i;
      indegree_list[tid]--;
      if (indegree_list[tid] == 0) {
        CalculateEP(tid);
        CalculateEMT(EP_list[tid], tid);
	task_pq.push(ProcPair(blevel_list[tid], tid));
/*       cout<<"push ... vid:"<<tid<<endl; */
      }
    }

    rem_tasks--;
  }

  int nv=dependent_graph.GetVertexCount();
  VID lvid;
  TASKWEIGHT tmp=-1.;
  for(int i=0; i<nv; ++i) {
    if(FT_list[i] > tmp) {
      lvid = i;
      tmp=FT_list[i];
    }
  }
  cout<<"last vid: "<<lvid<<" scheduling length: "<<tmp<<endl;
  cout<<"epc : "<<epc<<" eic:"<<eic<<endl;

  newSchedule.SetName("FCP");
  return newSchedule;
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
FCP_Setup(int target_np)
{
  int nvertices = dependent_graph.GetVertexCount();
  
  // Reserve storage
  PRT_list.reserve(target_np);
  PRT_list.assign(target_np, 0.0);
  EMT_list.reserve(nvertices);
  EMT_list.assign(nvertices, -1);
  LMT_list.reserve(nvertices);
  LMT_list.assign(nvertices, 0.0);
  EP_list.reserve(nvertices);
  EP_list.assign(nvertices, -1);
  FT_list.reserve(nvertices);
  FT_list.assign(nvertices, -1);
  PROC_list.reserve(nvertices);
  PROC_list.assign(nvertices, -1);

  // Set up proc related stuff
  for (PID p = 0; p < target_np; p++) {
    allProc_list.insert( ProcPair(0.0, p) );
    newSchedule.AddTaskSet();
  }

  // Add ready tasks
  for (VID i = 0; i < nvertices; i++) {
    vector<VID> tmp;
    dependent_graph.GetPredecessors(i,tmp);
    int indegree = tmp.size();
    indegree_list.push_back(indegree);
    if (indegree == 0) {
      EMT_list[i] = 0.0;
      task_pq.push( ProcPair(blevel_list[i], i) );
/*        cout<<"push ... ....vid:"<<i<<endl; */
   }
  }

}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void 
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
AssignALAPLevel() {
  // Get B-levels for all tasks
  AssignBlevels();

  // Get ALAPs for all tasks
//  TASKWEIGHT cp_wt = *max_element(blevel_list.begin(), blevel_list.end());
  TASKWEIGHT cp_wt = my_max2(blevel_list);
/*   cout<<"maxbl "<<cp_wt<<endl; */
  typename TW_vector::iterator blevel_iter;
  typename TW_vector::iterator vend = blevel_list.end();
  for (blevel_iter = blevel_list.begin(); blevel_iter != vend; blevel_iter++)
    *blevel_iter = cp_wt - *blevel_iter;

/*   for(int i=0; i<blevel_list.size(); ++i) */
/*     cout<<"bl["<<i<<"]="<<blevel_list[i]<<endl; */


}

//======================================================================
//
// Miscellaneous routines
//
//======================================================================

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
TASKWEIGHT
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
my_max2 (vector<TASKWEIGHT>& fp) {   
                        
        TASKWEIGHT tmp=-1.0;
	typename vector <TASKWEIGHT>::iterator _p;
        for(_p = fp.begin(); _p!=fp.end(); _p++) {
                if( *_p > tmp ) {
                        tmp = *_p;
                }
        }
        return tmp;
};

// Assign B-levels for each node
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
AssignBlevels()
{
  AssignBlevels(dependent_graph);
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
AssignTlevels()
{
  AssignTlevels(dependent_graph);
}

// Assign B-levels for each node
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
AssignBlevels(WtDiGraph& _gg)
{
  // Get topological sort
  VID_vector tsort;
  TopologicalSort<WtDiGraph>(_gg,tsort);

  // Initialize for computing b-levels
  int nvertices = _gg.GetVertexCount();
  blevel_list.reserve(nvertices);
  blevel_list.assign(nvertices, 0.0);

  // Assign b-levels (including edge weights)
  // VID_vector succs;
  TASKWEIGHT maxval, taskwt;
  COMMWEIGHT edge_wt;
  VID parentid, childid;
  VID_vector::reverse_iterator i, rend;
  CIVID j, send;
  for (i = tsort.rbegin(), rend = tsort.rend(); i != rend; i++) {
    parentid = *i;
    VID_vector succs;
    _gg.GetSuccessors(parentid,succs);
    maxval = 0;
    for (j = succs.begin(), send = succs.end(); j != send; j++) {
      childid = *j;
      edge_wt = _gg.GetEdgeWeight(parentid, childid).GetMessageSize();
      if (edge_wt + blevel_list[childid] > maxval)
        maxval = edge_wt + blevel_list[childid];
    }
    taskwt = _gg.GetData(parentid).GetTaskWeight();
    blevel_list[parentid] = taskwt + maxval;
  }
/*   for(int i=0; i<blevel_list.size(); ++i) */
/*     cout<<"bl["<<i<<"]="<<blevel_list[i]<<endl; */
}

// Assign T-levels for each node
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
AssignTlevels(WtDiGraph& _gg)
{
  // Get topological sort
  VID_vector tsort;
  TopologicalSort<WtDiGraph>(_gg,tsort);
  
#ifdef STAPL_DEBUG
  cout<<"In AssignTlevels:"<<endl;
  _gg.DisplayGraph();
#endif

  // Initialize for computing t-levels
  int nvertices = _gg.GetVertexCount();
  tlevel_list.reserve(nvertices);
  tlevel_list.assign(nvertices, 0.0);

  // Assign b-levels (including edge weights)
  // VID_vector preds;
  TASKWEIGHT maxval, taskwt, newwt;
  COMMWEIGHT edgewt;
  VID vertexid, parentid;
  CIVID i, iend, j, pend;
  for (i = tsort.begin(), iend = tsort.end(); i != iend; i++) {
    vertexid = *i;
    VID_vector preds;
    _gg.GetPredecessors(vertexid,preds);
    maxval = 0;
    for (j = preds.begin(), pend = preds.end(); j != pend; j++) {
      parentid = *j;
      newwt = _gg.GetEdgeWeight(parentid, vertexid).GetMessageSize()
        + _gg.GetData(parentid).GetTaskWeight()
        + tlevel_list[parentid];
      maxval = max(newwt, maxval);
    }
    tlevel_list[vertexid] = maxval;
/*     cout<<maxval<<endl; */
  }

}


template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
CalculateEPandLMT(int tid)
{
  COMMWEIGHT maxLMT = -1.0;
  PID LMTpid = -1;

  VID_vector pred_list;
  dependent_graph.GetPredecessors(tid,pred_list);
  CIVID pred_end = pred_list.end();
  for (CIVID i = pred_list.begin(); i != pred_end; i++) {
    TASKWEIGHT new_wt = dependent_graph.GetEdgeWeight(*i, tid).GetMessageSize()
      + FT_list[*i];
    if (maxLMT < new_wt) {
      maxLMT = new_wt;
      LMTpid = PROC_list[*i];
    }
  }
  LMT_list[tid] = maxLMT;
  EP_list[tid] = LMTpid;
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
CalculateEP(int tid)
{
  COMMWEIGHT maxLMT = -1.0;
  PID LMTpid = -1;

  VID_vector pred_list;
  dependent_graph.GetPredecessors(tid,pred_list);
  CIVID pred_end = pred_list.end();
  for (CIVID i = pred_list.begin(); i != pred_end; i++) {
    COMMWEIGHT new_wt = dependent_graph.GetEdgeWeight(*i, tid).GetMessageSize()
      + FT_list[*i];
    if (maxLMT < new_wt) {
      maxLMT = new_wt;
      LMTpid = PROC_list[*i];
    }
  }
  LMT_list[tid]=maxLMT;  
  EP_list[tid] = LMTpid;
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
CalculateEMT(int pid, int tid)
{
  COMMWEIGHT maxEMT = -1.0;
  VID_vector pred_list;
  dependent_graph.GetPredecessors(tid,pred_list);
  CIVID pred_end = pred_list.end();
  for (CIVID i = pred_list.begin(); i != pred_end; i++) {
    if (PROC_list[*i] != pid) {
      COMMWEIGHT new_wt = FT_list[*i]
        + dependent_graph.GetEdgeWeight(*i, tid).GetMessageSize();
      if (maxEMT < new_wt)
        maxEMT = new_wt;
    } 
  }
  EMT_list[tid] = maxEMT;
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
TASKWEIGHT
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
GetEST(int pid, int tid)
{
  return max(EMT_list[tid], PRT_list[pid]);
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
PP_MS_remove(PP_multiset &ppv, int id)
{
  PP_multiset::iterator pp_loc = ppv.begin();
  PP_multiset::iterator pp_end = ppv.end();

  while (pp_loc != pp_end) {
    if (pp_loc->id == id) break;
    pp_loc++;
  }
  if (pp_loc != pp_end)
    ppv.erase(pp_loc);
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
TT_MS_remove(TT_multiset &ttv, int id)
{
  TT_multiset::iterator tt_loc = ttv.begin();
  TT_multiset::iterator tt_end = ttv.end();
  
  while (tt_loc != tt_end) {
    if (tt_loc->id == id) break;
    tt_loc++;
  }
  if (tt_loc != tt_end)
    ttv.erase(tt_loc);
}

/*  Need to decide how to compute the GetMakespan part
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
Schedule
TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
HeavyEdgeMerge() {
  CTI v1;
  GII cp1;
  GII p1;
  VID _vid = 0, _v1id = 0, _v2id = 0;
  PID _pid, _p1id, _p2id;
  Schedule s1;
  vector< pair< pair<VID,VID>, COMMWEIGHT> > alledges;
  typename vector< pair< pair<VID,VID>, COMMWEIGHT> >::iterator a1;
  pair<VID, VID> tpair; //tmp pair
  vector< VID > vidque;  //list of merged vid
  vector< VID > tmpvct; //tmp vector to cancel merging of list of vids
  COMMWEIGHT mspan=0, cost=0;
  int i=0, mflag;
  bool c1=false, c2=false;
  GII p1i; IVID v1i;
                        
  alledges = dependent_graph.GetEdges();
          
//Sort by Weight
  stable_sort (alledges.begin(), alledges.end(), ptr_fun(WtGreater) );

        for(v1 = dependent_graph.v.begin(); v1 != dependent_graph.v.end(); v1++) {
                _pid = s1.AddTaskSet(v1->vid);
        }
        cost = GetMakespan(s1);

        for(a1 = alledges.begin(); a1 != alledges.end(); a1++) {
                _v1id = a1->first.first;
                _v2id = a1->first.second;
                _p1id = s1.GetTaskSetPID(_v1id);
		_p2id = s1.GetTaskSetPID(_v2id);
	IVID ii1=NULL; 
	IVID ii2=NULL; 
	ii1= MyFind(vidque, _v1id);
	ii2= MyFind(vidque, _v2id);
	if(ii1!=vidque.end()) c1=true; else c1=false;
	if(ii2!=vidque.end()) c2=true; else c2=false;
                if( !c1 ) {
                        vidque.push_back(_v1id);
                        if( !c2 ) {             //none merged
                                vidque.push_back(_v2id);
                                s1.DeleteTaskSet(_p2id);
                                s1.AddTask(_p1id, _v2id);
                                mflag = 0;
                        }
                        else {                                  //2nd merged
                                s1.DeleteTaskSet(_p1id);
                                s1.AddTask(_p2id, _v1id);
                                mflag = 1;
                        }
                }
                else {                                          //1st merged
                        if( !c2 ) {
                                vidque.push_back(_v2id);
                                s1.DeleteTaskSet(_p2id);
                                s1.AddTask(_p1id, _v2id);
                                mflag = 2;
                                mflag = 2;
                        }
                        else {                                  //both merged
                                if(_p1id != _p2id) {
                                 if( s1.IsTaskSet(_p2id, &p1) );
                                 tmpvct = p1->taskset;
                                 s1.DeleteTaskSet(_p2id);
                                 s1.AddTask(_p1id, tmpvct);
                                 mflag = 3;
                                }
                        }
                }
//checking makespan after merging
                mspan = GetMakespan(s1);
                if( mspan <= cost ) {
                        cost = mspan;
                }
                        //mspan not decrease, stop merging
                else {

//Recover from last merge
        switch(mflag) {
                case 0 : {
                        s1.DeleteTask(_v2id);
                        s1.AddTaskSet(_v2id);
                        break;
                        }
                case 1 : {
                        s1.DeleteTask(_v1id);
                        s1.AddTaskSet(_v1id);
                        break;
                        }
                case 2 : {
                        s1.DeleteTask(_v2id);
                        s1.AddTaskSet(_v2id);
                        break;
                        }
                case 3 : {
                        s1.DeleteTask(tmpvct);
                        s1.AddTaskSet(tmpvct);
                        break;
                        }
                default :
                        { break; }
                  }             //switch
            }           //else
   
        }   //for(alledges..)

	s1.AlgName = "HEM";
  return s1;
};
*/

// template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
// Schedule
// TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
// JoinLatestPredecessor() {
// 	TI v1;
//         VID _v1id, _vid, _v2id;
//         PID _pid;   
//         int i=0, j=0, k=0;
// 	Schedule s1;
// 	GII p1i; 
// 	IVID v1i;

// //assign all tasks
// 	for(IVID vs=vsortbylevel.begin(); vs != vsortbylevel.end(); vs++) {
//         	    _vid=*vs;
//                  dependent_graph.IsVertex(_vid, &v1);
//                     if( v1->data.level == 0) {
//                         _pid = s1.AddTaskSet(_vid);
//                         v1->data.SetStartTime(0);
//                         v1->data.SetFinishTime(v1->data.taskwt);
//                     }
//                     else {
//                         Find2LatestPredecessors(_vid,_v1id,_v2id);

//                          _pid = s1.GetTaskSetPID(_v1id);
//                         _pid = s1.AddTask(_pid, _vid);
// 			TASKWEIGHT _cwt = max( v1->data.GetFinishTime(), 
// 				 v1->data.GetFinishTime()+
// 				dependent_graph.v[_vid].GetEdgeWeight(_v2id).
// 					       GetMessageSize() );

// 			v1->data.SetStartTime(_cwt);
//                         v1->data.SetFinishTime(_cwt +
// 				 dependent_graph.v[_vid].data.GetTaskWeight());

//                     }
//         }
// 	s1.AlgName = "JLP";
//         return s1;
// };


// template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
// Schedule
// TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
// WaveFront_BFS() {
//         list<VID> q;
//         CTI cv1;
//         TI v1, v2;
//         VID _v0id,_v1id, v2id, _vid;
//         PID _p1id, _p2id;
//         int i=0,j=0, k=0, lev1, lev2=-1;
// 	Schedule s1;
  
//         vector<int> ind; //indegree
//         ind.reserve(dependent_graph.GetVertexCount());
//         for(v1 = dependent_graph.v.begin(); v1 != dependent_graph.v.end(); v1++) {
//              _vid = v1->vid;
//              ind[_vid]=v1->predecessors.size();
//         }
 
//         vector<bool> mark;
//         mark.reserve(dependent_graph.GetVertexCount());
//         for(i=0;i<dependent_graph.GetVertexCount();i++) mark[i]=false;
     
// 	i=0;
//        	for(IVID iv=sourcevids.begin(); iv!=sourcevids.end(); iv++) {
// 		q.push_back(*iv);
// 	cout<<"\nSource vids: "<<*iv<<endl;
//       		mark[*iv]=true;
// 		s1.AddTaskSet(*iv);
// 		i++;
// 	}
                                                
//   while ( !q.empty() ) {
//     _v1id = q.front();
//      k=0;
//      if ( dependent_graph.IsVertex(_v1id,&v1) ) {
//        lev1 = v1->data.GetLevel();
//        if(lev1 != lev2)  _p1id=0;
//        for (CEGI e = cv1->edgelist.begin(); e != cv1->edgelist.end(); e++) {
//          v2id = e->vertex2id;
//          if(!mark[v2id]) {
//                 q.push_back(v2id);
//                 mark[v2id] = true;
//          }
//          ind[v2id]--;
//          if(ind[v2id] == 0) {  
//            if( _p1id<i)  {
//                 s1.AddTask(_p1id, v2id);
//                 _p1id++;
//            } else {
//                 _p2id = s1.AddTaskSet(v2id);
//                 k++;
//            }
//          }
//        }
//        lev2 = lev1;
//      } else {
//        cout << "\nIn WaveFront_BFS: vertex=" <<_v1id << " not in graph";
//      }
//      i+=k;
//      q.pop_front();
//   }
//   s1.AlgName = "WFM" ;
//   return s1;
        
// };

}  //end namespace

#endif
