/*!
  \file MachineInfo.h
  \author Ping An
  \date Dec. 20, 01
  \ingroup Staplscheduler
  \brief A recursive data structure to represent clustered 
         hierarchical machine topology and attributes. 

  Classe 
	SysNode<SYSTEMNODE>

*/
#ifndef MachineInfo_h
#define MachineInfo_h

#include "Defines.h"
#include "Graph.h"

//===================================================================
//===================================================================
template<class SYSTEMNODE>
class SysNode {

  typedef Graph<UG<SYSTEMNODE,int>, NMG<SYSTEMNODE,int>, 
    WG<SYSTEMNODE,int>,SYSTEMNODE,int > CommGraph;

 public:
  int nprocs;                           //total number of processors
  PID systemid;
  int nodesize;				//number of processors in cluster
  double speed;				//computation speed
  double startup;			//communication overhead
  double gh;				//communication bandwidth (rate)
  double L;				//communication cost (will remove) 
  double w;				//computation cost (will remove)
  int level;                            //levels of SysNode
  PID leadpid;
  CommGraph commgraph;

  //===================================================================
  // Constructors & Destructor
  //===================================================================
  SysNode() {};
  SysNode(int _inv) {}

  SysNode(PID _p, CommGraph _st) {
    systemid=_p; 
    nodesize = _st.GetVertNum();
    commgraph = _st;
  };

/*   SysNode(SysNode ccost) { */
/*     systemid=ccost.systemid;  */
/*     leadpid = ccost.leadpid; */
/*     nodesize = ccost.nodesize; */
/*     speed = ccost.speed; */
/*     startup = ccost.startup; */
/*     gh = ccost.gh; */
/*     L = ccost.L; */
/*     w = ccost.w; */
/*     level = ccost.level; */
/*     commgraph = ccost.commgraph; */
/*   }; */
  ~SysNode() {};

  //===================================================================
  // Operators
  //===================================================================

  bool operator== ( const SysNode& ccost) const {
    return  (systemid == ccost.systemid);
  };

  SysNode& operator= (const SysNode& ccost) {
    nprocs = ccost.nprocs;
    systemid = ccost.systemid;
    leadpid = ccost.leadpid;
    nodesize = ccost.nodesize;
    speed = ccost.speed;
    startup = ccost.startup;
    gh = ccost.gh;
    L = ccost.L;
    w = ccost.w;
    level = ccost.level;
    commgraph = ccost.commgraph;
    leadpid = ccost.leadpid;
    return *this;
  };

  //===================================================================
  //Set & Get methods
  //===================================================================
  PID GetSystemId() const { return systemid; };
  double GetProcSpeed() const { return speed; };
  double GetStartUp() const { return startup; };
  PID GetLeadPid() const { return leadpid; };
  int GetNodeSize() const { return nodesize; };

  void SetSystemId(const PID& sid) { systemid=sid; };
  void SetSpeed(const double& spd) { speed=spd; };

  //===================================================================
  // Other methods
  //===================================================================
  int GetNodeSize(int level, PID systemid) {
  //should be able to use level etc.
    return GetNodeSize();  
  };
  double GetProcSpeed(int level, PID systemid, PID pid) {
  //should be able to use level etc.
    return commgraph.v[pid].data.GetProcSpeed();  
  };
  
  void GetCommPara(int level, PID systemid, PID p1id, PID p2id, 
		   double& _startup,double& _gh) {
  //should be able to use level etc.
    _startup = commgraph.v[p1id].data.GetStartUp();
    _gh = commgraph.v[p1id].GetEdgeWeight(p2id);
  };

/*   static SysNode InvalidData() { */
/*     SysNode c; */
/*     return c; */
/*   }; */
};


inline ostream& operator << (ostream& s, const pair<double,double>& tw) {
  return s<<setw(2)<<"startup: "<<tw.first
	  <<" with throughput: "<<tw.second<<endl;
};

template<class SYSTYPE> 
inline void sysoutput(ostream& s, const SYSTYPE* tw, int indent) {
  s<<setw(indent)<<" System: "<<tw->systemid;
  s<<setw(indent)<<" with level: "<<tw->level<< " leadpid: "<<
    tw->leadpid<<" NodeSize:"<<tw->nodesize<<endl;
  tw->commgraph.WriteGraph(s);
};

typedef SysNode<PID> Sys0;
inline ostream& operator << (ostream& s, const Sys0& tw) {
  int indent=9;
  s<<setw(indent)<<" Proc: "<<tw.systemid<<" :"<<endl;
    s<<setw(indent)<<"  with speed "<<tw.speed<<endl;
    return s;
};

typedef SysNode<SysNode<PID > > Sys1;
inline ostream& operator << (ostream& s, const Sys1& tw) {
  sysoutput<Sys1>(s,&tw,6);
  return s;
};

typedef SysNode<SysNode<SysNode<PID > > > Sys2;
inline ostream& operator << (ostream& s, const Sys2& tw) {
  sysoutput<Sys2>(s,&tw,0);
  return s;
};

typedef Sys1 MachineInfo;


#endif
