#ifndef I_RRT_STRATEGY_H_
#define I_RRT_STRATEGY_H_

#include "Extenders/BasicExtender.h"
#include "MPStrategies/MPStrategyMethod.h"

#include "GUI/MainWindow.h"
#include "GUI/ModelSelectionWidget.h"
#include "GUI/RobotAvatar.h"
#include "Models/Vizmo.h"

template<class MPTraits>
class IRRTStrategy : public MPStrategyMethod<MPTraits> {
  public:
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPProblemType::RoadmapType RoadmapType;
    typedef typename MPProblemType::GraphType GraphType;
    typedef typename MPProblemType::VID VID;
    typedef typename MPProblemType::DistanceMetricPointer DistanceMetricPointer;
    typedef typename MPProblemType::ValidityCheckerPointer
        ValidityCheckerPointer;
    typedef typename MPProblemType::NeighborhoodFinderPointer
        NeighborhoodFinderPointer;
    typedef typename MPProblemType::LocalPlannerPointer LocalPlannerPointer;
    typedef typename MPProblemType::ConnectorPointer ConnectorPointer;
    typedef typename MPProblemType::MapEvaluatorPointer MapEvaluatorPointer;

    //Non-XML constructor sets all private variables
    IRRTStrategy(const CfgType& _start = CfgType(),
        const CfgType& _goal = CfgType(), string _lp = "sl",
        string _dm = "euclidean", string _nf = "BFNF", string _vc = "PQP_SOLID",
        string _nc = "kClosest", string _gt = "UNDIRECTED_TREE",
        string _extenderLabel = "BERO",
        vector<string> _evaluators = vector<string>(), double _delta = 10.0,
        double _minDist = 0.01, double _growthFocus = 0.05,
        bool _evaluateGoal = true, size_t _numRoots = 1,
        size_t _numDirections = 1, size_t _maxTrial = 3,
        bool _growGoals = false);

    //Non-XML constructor w/ Query (by label)
    IRRTStrategy(Query<MPTraits>* _q, string _lp = "sl",
        string _dm = "euclidean", string _nf = "BFNF", string _vc = "PQP_SOLID",
        string _nc = "kClosest", string _gt = "UNDIRECTED_TREE",
        string _extenderLabel = "BERO",
        vector<string> _evaluators = vector<string>(), double _delta = 10.0,
        double _minDist = 0.01, double _growthFocus = 0.05,
        bool _evaluateGoal = true, size_t _numRoots = 1,
        size_t _numDirections = 1, size_t _maxTrial = 3,
        bool _growGoals = false);

    //XML Constructor
    IRRTStrategy(MPProblemType* _problem, XMLNodeReader& _node,
        bool _warnXML = true, bool _child = false);

    virtual ~IRRTStrategy();

    virtual void ParseXML(XMLNodeReader& _node, bool _child = false);

    virtual void Initialize();
    virtual void Run();
    virtual void Finalize();
    virtual void Print(ostream& _os) const;

    void SetMPProblem(MPProblemType* _problem);

    //return path computed if RRT is successfully run with a query
    vector<CfgType> GetPath() {return m_query->GetPath();}

  protected:
    // Helper functions
    CfgType AvatarBiasedDirection();
    CfgType GoalBiasedDirection();
    CfgType SelectDirection();
    CfgType SelectDispersedDirection(VID v1);
    vector<CfgType> SelectNeighbors(VID v1);
    virtual VID ExpandTree(CfgType& _dir);
    void ConnectTrees(VID _recentlyGrown);
    void ConnectNeighbors(VID _newVID, VID _nearVID);
    void EvaluateGoals();

    vector<string> m_evaluators;
    string m_lp;
    string m_dm;
    string m_nf;
    string m_vc;
    shared_ptr<Query<MPTraits> > m_query;
    string m_nc;
    string m_gt;
    string m_extenderLabel;
    double m_delta, m_minDist, m_growthFocus, m_alpha;
    bool m_evaluateGoal;
    size_t m_numRoots,m_numDirections, m_maxTrial;
    bool m_growGoals;
    vector< vector<VID> > m_trees;
    typename vector<vector<VID> >::iterator m_currentTree;
    vector<CfgType> m_goals;
    vector<size_t> m_goalsNotFound;

    RobotAvatar* m_avatar;
};

template<class MPTraits>
IRRTStrategy<MPTraits>::
IRRTStrategy(const CfgType& _start, const CfgType& _goal, string _lp,
    string _dm, string _nf, string _vc, string _nc, string _gt,
    string _extenderLabel, vector<string> _evaluators, double _delta,
    double _minDist, double _growthFocus, bool _evaluateGoal, size_t _numRoots,
    size_t _numDirections, size_t _maxTrial, bool _growGoals):
    m_evaluators(_evaluators), m_lp(_lp), m_dm(_dm), m_nf(_nf), m_vc(_vc),
    m_query(new Query<MPTraits>(_start, _goal)), m_nc(_nc), m_gt(_gt),
    m_extenderLabel(_extenderLabel), m_delta(_delta), m_minDist(_minDist),
    m_growthFocus(_growthFocus), m_evaluateGoal(_evaluateGoal),
    m_numRoots(_numRoots), m_numDirections(_numDirections),
    m_maxTrial(_maxTrial), m_growGoals(_growGoals), m_avatar(NULL) {
  this->SetName("IRRTStrategy");

//#ifdef USE_HAPTICS
//  m_avatar = new RobotAvatar(&m_query->GetQuery()[0], RobotAvatar::Haptic);
//#else
//  m_avatar = new RobotAvatar(&m_query->GetQuery()[0], RobotAvatar::Mouse);
//#endif
}

template<class MPTraits>
IRRTStrategy<MPTraits>::
IRRTStrategy(Query<MPTraits>* _q, string _lp, string _dm,
    string _nf, string _vc, string _nc, string _gt, string _extenderLabel,
    vector<string> _evaluators, double _delta, double _minDist,
    double _growthFocus, bool _evaluateGoal, size_t _numRoots,
    size_t _numDirections, size_t _maxTrial, bool _growGoals) :
    m_evaluators(_evaluators), m_lp(_lp), m_dm(_dm), m_nf(_nf), m_vc(_vc),
    m_nc(_nc), m_gt(_gt), m_extenderLabel(_extenderLabel), m_delta(_delta),
    m_minDist(_minDist), m_growthFocus(_growthFocus),
    m_evaluateGoal(_evaluateGoal), m_numRoots(_numRoots),
    m_numDirections(_numDirections), m_maxTrial(_maxTrial),
    m_growGoals(_growGoals), m_avatar(NULL) {
  this->SetName("IRRTStrategy");
  m_query = shared_ptr<Query<MPTraits> >(new Query<MPTraits>(*_q));
  m_query->SetMPProblem(this->GetMPProblem());
  m_query->SetDebug(this->m_debug);

//#ifdef USE_HAPTICS
//  m_avatar = new RobotAvatar(&m_query->GetQuery()[0], RobotAvatar::Haptic);
//#else
//  m_avatar = new RobotAvatar(&m_query->GetQuery()[0], RobotAvatar::Mouse);
//#endif
}

template<class MPTraits>
IRRTStrategy<MPTraits>::
IRRTStrategy(MPProblemType* _problem, XMLNodeReader& _node, bool _warnXML,
    bool _child) : MPStrategyMethod<MPTraits>(_problem, _node), m_avatar(NULL) {
  this->SetName("IRRTStrategy");
  ParseXML(_node, _child);
  if(_warnXML)
    _node.warnUnrequestedAttributes();

//#ifdef USE_HAPTICS
//  m_avatar = new RobotAvatar(&m_query->GetQuery()[0], RobotAvatar::Haptic);
//#else
//  m_avatar = new RobotAvatar(&m_query->GetQuery()[0], RobotAvatar::Mouse);
//#endif
}

template<class MPTraits>
IRRTStrategy<MPTraits>::
~IRRTStrategy() { }

template<class MPTraits>
void
IRRTStrategy<MPTraits>::
ParseXML(XMLNodeReader& _node, bool _child) {
  for(XMLNodeReader::childiterator citr = _node.children_begin();
      citr != _node.children_end(); ++citr) {
    if(citr->getName() == "Evaluator") {
      string evalMethod = citr->stringXMLParameter("label", true, "",
          "Evaluation Method");
      m_evaluators.push_back(evalMethod);
      citr->warnUnrequestedAttributes();
    }
    else
      citr->warnUnknownNode();
  }

  m_delta = _node.numberXMLParameter("delta", false, 1.0, 0.0, MAX_DBL,
      "Delta Distance");
  m_minDist = _node.numberXMLParameter("minDist", false, 0.0, 0.0, m_delta,
      "Minimum Distance");
  m_numRoots = _node.numberXMLParameter("numRoots", false, 1, 0, MAX_INT,
      "Number of Roots");
  m_growthFocus = _node.numberXMLParameter("growthFocus", false, 0.0, 0.0, 1.0,
      "#GeneratedTowardsGoal/#Generated");
  m_vc = _node.stringXMLParameter("vcLabel", true, "", "Validity Test Method");
  m_nf = _node.stringXMLParameter("nfLabel", true, "", "Neighborhood Finder");
  m_dm = _node.stringXMLParameter("dmLabel", true, "", "Distance Metric");
  m_lp = _node.stringXMLParameter("lpLabel", true, "", "Local Planning Method");
  m_numDirections = _node.numberXMLParameter("m", false, 1, 1, 1000,
      "Number of directions to extend");
  m_nc = _node.stringXMLParameter("connectorLabel", false, "",
      "Node Connection Method");
  m_gt = _node.stringXMLParameter("gtype", true, "",
      "Graph type dir/undirected tree/graph");
  if(!_child)
    m_extenderLabel = _node.stringXMLParameter("extenderLabel", true, "",
        "Extender label");
  m_evaluateGoal = _node.boolXMLParameter("evaluateGoal", false, false, "");
  m_growGoals = _node.boolXMLParameter("growGoals", false, false,
      "Determines whether or not we grow a tree from the goal");
  m_maxTrial = _node.numberXMLParameter("trial", false, 3, 1, 1000,
      "Number of trials to get a dispersed direction");

  //optionally read in a query and create a Query object.
  string query = _node.stringXMLParameter("query", false, "", "Query Filename");
  if(query != "") {
    m_query = shared_ptr<Query<MPTraits> >(new Query<MPTraits>(query));
    m_query->SetMPProblem(this->GetMPProblem());
    m_query->SetDebug(this->m_debug);
  }
}

template<class MPTraits>
void
IRRTStrategy<MPTraits>::
Print(ostream& _os) const {
  _os << "IRRTStrategy::Print" << endl;
  _os << "\tNeighborhood Finder:: " << m_nf << endl;
  _os << "\tDistance Metric:: " << m_dm << endl;
  _os << "\tValidity Checker:: " << m_vc << endl;
  _os << "\tLocal Planner:: " << m_lp << endl;
  _os << "\tConnection Method:: " << m_nc << endl;
  _os << "\tGraph Type:: " << m_gt << endl;
  _os << "\tExtender:: " << m_extenderLabel << endl;
  _os << "\tEvaluate Goal:: " << m_evaluateGoal << endl;
  _os << "\tEvaluators:: " << endl;
  _os << "\tGrow Goals:: " << m_growGoals << endl;
  typedef vector<string>::const_iterator SIT;
  for(SIT sit = m_evaluators.begin(); sit!=m_evaluators.end(); sit++)
    _os << "\t\t" << *sit << endl;
  _os << "\tdelta:: " << m_delta << endl;
  _os << "\tminimum distance:: " << m_minDist << endl;
  _os << "\tnumber of roots:: " << m_numRoots << endl;
  _os << "\tgrowth focus:: " << m_growthFocus << endl;
  _os << "\tnumber of expansion directions:: " << m_numDirections << endl;
}

//////////////////////
//Initialization Phase
/////////////////////
template<class MPTraits>
void
IRRTStrategy<MPTraits>::
Initialize() {
  if(this->m_debug)
    cout << "\nInitializing IRRTStrategy::" << endl;

  m_alpha = .5;

  // Setup MP variables
  Environment* env = this->GetMPProblem()->GetEnvironment();
  string callee = "IRRTStrategy::RRT";
  // Setup RRT Variables
  CfgType tmp;
  if(m_query) {
    vector<CfgType>& queryCfgs = m_query->GetQuery();
    typedef typename vector<CfgType>::iterator CIT;
    if(m_growGoals) {
      for(CIT cit1 = queryCfgs.begin(); cit1 != queryCfgs.end(); cit1++) {
        VID add = this->GetMPProblem()->GetRoadmap()->GetGraph()->
            AddVertex(*cit1);
        m_trees.push_back(vector<VID>(1, add));
      }
    }
    else {
      for(CIT cit1 = queryCfgs.begin(), cit2 = cit1 + 1; cit2 != queryCfgs.end();
          cit1++, cit2++) {
        VID add = this->GetMPProblem()->GetRoadmap()->GetGraph()->
            AddVertex(*cit1);
        m_trees.push_back(vector<VID>(1, add));
        m_goals.push_back(*cit2);
        m_goalsNotFound.push_back(m_goals.size() - 1);
      }
    }
  }
  else{
    // Add root vertex/vertices
    size_t i = 0;
    while(i < m_numRoots) {
      tmp.GetRandomCfg(env);
      if(env->InBounds(tmp) && this->GetMPProblem()->GetValidityChecker(m_vc)->
          IsValid(tmp, callee)) {
        VID add = this->GetMPProblem()->GetRoadmap()->GetGraph()->AddVertex(tmp);
        m_trees.push_back(vector<VID>(1, add));
      }
      i++;
    }
  }

  m_currentTree = m_trees.begin();//set initial tree to be grown

  //m_avatar->Connect();
  //m_avatar->SummonMouse();

  if(this->m_debug)
    cout << "\nEnding Initializing IRRTStrategy" << endl;
}

////////////////
//Run/Start Phase
////////////////
template<class MPTraits>
void
IRRTStrategy<MPTraits>::
Run() {
  if(this->m_debug)
    cout << "\nRunning IRRTStrategy::" << endl;

  // Setup MP Variables
  StatClass* stats = this->GetMPProblem()->GetStatClass();

  stats->StartClock("RRT Generation");

  CfgType dir;
  bool mapPassedEvaluation = false;
  size_t iter = 0;
  while(!mapPassedEvaluation) {
    //find my growth direction. Default is to randomly select
    //node or bias towards a goal
    double randomRatio = DRand();
    if(randomRatio < m_growthFocus) {
      dir = GoalBiasedDirection();
    }
    else {
      dir = this->SelectDirection();
    }

    // Randomize Current Tree
    m_currentTree = m_trees.begin() + LRand() % m_trees.size();

    VID recent = this->ExpandTree(dir);
    if(recent != INVALID_VID) {
      //connect various trees together
      ConnectTrees(recent);
      //see if tree is connected to goals
      if(m_evaluateGoal)
        EvaluateGoals();

      //evaluate the roadmap
      bool evalMap = this->EvaluateMap(m_evaluators);
      if(!m_growGoals) {
        mapPassedEvaluation = m_trees.size() == 1 && evalMap &&
            ((m_evaluateGoal && m_goalsNotFound.size() == 0) || !m_evaluateGoal);
        if(this->m_debug && m_goalsNotFound.size() == 0)
          cout << "RRT FOUND ALL GOALS" << endl;
      }
      else
        mapPassedEvaluation = evalMap && m_trees.size()==1;
    }
    else
      mapPassedEvaluation = false;

    if(++iter % 20 == 0) {
      GetVizmo().GetMap()->RefreshMap();
      GetMainWindow()->GetModelSelectionWidget()->CallResetLists();
    }
    usleep(10000);
  }

  stats->StopClock("RRT Generation");
  if(this->m_debug) {
    stats->PrintClock("RRT Generation", cout);
    cout << "\nEnd Running IRRTStrategy::" << endl;
  }
}

/////////////////////
//Finalization phase
////////////////////
template<class MPTraits>
void
IRRTStrategy<MPTraits>::
Finalize() {

  if(this->m_debug)
    cout << "\nFinalizing IRRTStrategy::" << endl;

  //clean up robot avatar
  //delete m_avatar;
  //m_avatar = NULL;

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();

  //setup variables
  StatClass* stats = this->GetMPProblem()->GetStatClass();
  string str;

  //perform query if query was given as input
  if(m_query) {
    str = this->GetBaseFilename() + ".path";
    m_query->SetPathFile(str);
    if(m_evaluateGoal) {
      if(m_query->PerformQuery(this->GetMPProblem()->GetRoadmap())) {
        if(this->m_debug)
          cout << "Query successful! Output written to " << str << "." << endl;
      }
      else {
        if(this->m_debug)
          cout << "Query unsuccessful." << endl;
      }
    }
  }

  //output final map
  str = this->GetBaseFilename() + ".map";
  ofstream osMap(str.c_str());
  this->GetMPProblem()->GetRoadmap()->
      Write(osMap, this->GetMPProblem()->GetEnvironment());
  osMap.close();

  //output stats
  str = this->GetBaseFilename() + ".stat";
  ofstream  osStat(str.c_str());
  osStat << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(osStat, this->GetMPProblem()->GetRoadmap());
  stats->PrintClock("RRT Generation", osStat);
  osStat.close();

  if(this->m_debug)
    cout << "\nEnd Finalizing IRRTStrategy" << endl;
}

template<class MPTraits>
typename MPTraits::CfgType
IRRTStrategy<MPTraits>::
AvatarBiasedDirection() {
  //get avatar data
  CfgType newPos = m_avatar->GetCurrCfg();
  CfgType oldPos = m_avatar->GetPrevCfg();

  //find the nearest k neighbors.
  NeighborhoodFinderPointer nf = this->GetMPProblem()->
      GetNeighborhoodFinder("BFNF");
  vector<pair<VID, double> > kClosest;

  StatClass* kcloseStatClass = this->GetMPProblem()->GetStatClass();
  string kcloseClockName = "kclosest time ";
  kcloseStatClass->StartClock(kcloseClockName);

  nf->FindNeighbors(this->GetMPProblem()->GetRoadmap(), newPos,
      back_inserter(kClosest));

  kcloseStatClass->StopClock(kcloseClockName);

  //now get the average of the nearest k neighbors
  CfgType knnDir = CfgType();
  {
    QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
    for(typename vector<pair<VID, double> >::iterator it = kClosest.begin();
        it != kClosest.end(); ++it)
      knnDir += this->GetMPProblem()->GetRoadmap()->GetGraph()->
          GetVertex(it->first);
  }
  knnDir /= kClosest.size();
  //now find the pseudoforce vector from the avatar to the average
  knnDir -= newPos;

  //get the user pseudoforce
  CfgType userDir = newPos - oldPos;
  m_avatar->UpdatePos();

  //scale dirs by alpha
  userDir *= m_alpha;
  knnDir *= (1. - m_alpha);

  double stdDev = .5;
  return (userDir + knnDir);
}

template<class MPTraits>
typename MPTraits::CfgType
IRRTStrategy<MPTraits>::
GoalBiasedDirection() {
  // Determine direction, make sure goal not found
  if (m_goalsNotFound.size() == 0) {
    return CfgType();
  }
  else {
    size_t goalNum = LRand() % m_goalsNotFound.size();
    return m_goals[m_goalsNotFound[goalNum]];
  }
}

template<class MPTraits>
typename MPTraits::CfgType
IRRTStrategy<MPTraits>::
SelectDirection() {
  Environment* env = this->GetMPProblem()->GetEnvironment();
  CfgType dir;
  dir.GetRandomCfg(env);
  return dir;
}

template<class MPTraits>
typename MPTraits::CfgType
IRRTStrategy<MPTraits>::
SelectDispersedDirection(VID vd1) {
  StatClass* disperseStatClass = this->GetMPProblem()->GetStatClass();
  string disperseClockName = "disperse sampling time ";
  disperseStatClass->StartClock(disperseClockName);

  CfgType bestCfg;

  typename GraphType::vertex_iterator vi = this->GetMPProblem()->GetRoadmap()->
      GetGraph()->find_vertex(vd1);
  CfgType c1 = (*vi).property();

  double maxAngle = -MAX_DBL;
  for(size_t i = 0; i < m_maxTrial; i++) {
    CfgType randdir = this->SelectDirection();

    //calculating angle between unit vectors
    CfgType difCfg = randdir - c1;
    difCfg = difCfg / difCfg.Magnitude();
    vector<double> v1 = difCfg.GetData();

    //do for all the expanded directions
    vector<CfgType> x = SelectNeighbors(vd1);
    double minAngle = MAX_DBL;
    for(typename vector<CfgType>::iterator vecIT = x.begin(); vecIT!=x.end();
        vecIT++) {
      CfgType difCfg2 = *vecIT - c1;
      difCfg2 = difCfg2 / difCfg2.Magnitude();
      vector<double> v2 = difCfg2.GetData();
      double res=0;
      for(size_t j = 0; j < v1.size(); j++) {
        res += (v1[j] * v2[j]);
      }

      double angle = acos(res) * 180 / M_PI;
      if(minAngle > angle)
        minAngle = angle;
    }
    if(maxAngle < minAngle) {
      maxAngle = minAngle;
      bestCfg = randdir;
    }
  }

  disperseStatClass->StopClock(disperseClockName);

  return bestCfg;
}

template<class MPTraits>
vector<typename MPTraits::CfgType>
IRRTStrategy<MPTraits>::
SelectNeighbors(VID v1) {
  typename GraphType::vertex_iterator vi = this->GetMPProblem()->GetRoadmap()->
      GetGraph()->find_vertex(v1);
  vector<CfgType> vec;
  for(typename GraphType::adj_edge_iterator ei =(*vi).begin(); ei!=(*vi).end();
      ei++){
    VID tgt = (*ei).target();
    CfgType target = this->GetMPProblem()->GetRoadmap()->GetGraph()->
        GetVertex(tgt);
    vec.push_back(target);
  }
  return vec;
}

template<class MPTraits>
void
IRRTStrategy<MPTraits>::
ConnectNeighbors(VID _newVID, VID _nearVID) {
  if (_newVID != INVALID_VID) {
    vector<VID> currentVID;
    currentVID.push_back(_newVID);

    ConnectorPointer pConnection;
    pConnection = this->GetMPProblem()->GetConnector(m_nc);
    stapl::sequential::
        vector_property_map<typename GraphType::GRAPH, size_t> cmap;

    StatClass* conStatClass = this->GetMPProblem()->GetStatClass();
    string conClockName = "Total Connection time ";
    conStatClass->StartClock(conClockName);

    pConnection->Connect(this->GetMPProblem()->GetRoadmap(),
          *(this->GetMPProblem()->GetStatClass()), cmap,
          currentVID.begin(), currentVID.end(),
          m_currentTree->begin(), m_currentTree->end());

    conStatClass->StopClock(conClockName);
  }

}

template<class MPTraits>
typename IRRTStrategy<MPTraits>::VID
IRRTStrategy<MPTraits>::
ExpandTree(CfgType& _dir) {
  QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
  // Setup MP Variables
  DistanceMetricPointer dm = this->GetMPProblem()->GetDistanceMetric(m_dm);
  NeighborhoodFinderPointer nf =
      this->GetMPProblem()->GetNeighborhoodFinder(m_nf);
  LocalPlannerPointer lp = this->GetMPProblem()->GetLocalPlanner(m_lp);
  LPOutput<MPTraits> lpOutput;
  VID recentVID = INVALID_VID;
  CDInfo  cdInfo;
  // Find closest Cfg in map
  vector<pair<VID, double> > kClosest;
  vector<CfgType> cfgs;

  GraphType* g = this->GetMPProblem()->GetRoadmap()->GetGraph();

  int numRoadmapVertex  = g->get_num_vertices();
  typedef typename vector<vector<VID> >::iterator TRIT;
  int treeSize = 0;
  for(TRIT trit = m_trees.begin(); trit != m_trees.end(); ++trit) {
    treeSize += trit->size();
  }
  bool fixTree = false;
  if(treeSize > numRoadmapVertex)
    fixTree = true;
  else {
    vector<pair<size_t, VID> > ccs;
    stapl::sequential::vector_property_map<GraphType, size_t> cmap;
    get_cc_stats(*g, cmap, ccs);
    if(ccs.size() != m_trees.size())
      fixTree = true;
  }
  if(fixTree) { //node deleted by dynamic environment, fix all trees
    m_trees.clear();
    vector<pair<size_t, VID> > ccs;
    stapl::sequential::vector_property_map<GraphType, size_t> cmap;
    get_cc_stats(*g, cmap, ccs);
    vector<VID> ccVIDs;
    typename vector<pair<size_t, VID> >::const_iterator ccIt;
    for(ccIt = ccs.begin(); ccIt != ccs.end(); ccIt++) {
      cmap.reset();
      ccVIDs.clear();
      get_cc(*g, cmap, ccIt->second, ccVIDs);
      m_trees.push_back(ccVIDs);
    }
    m_currentTree = m_trees.begin();
  }

  StatClass* kcloseStatClass = this->GetMPProblem()->GetStatClass();
  string kcloseClockName = "kclosest time ";
  kcloseStatClass->StartClock(kcloseClockName);
  nf->FindNeighbors(this->GetMPProblem()->GetRoadmap(), m_currentTree->begin(),
      m_currentTree->end(), _dir, back_inserter(kClosest));
  kcloseStatClass->StopClock(kcloseClockName);

  CfgType nearest = this->GetMPProblem()->GetRoadmap()->GetGraph()->
      GetVertex(kClosest[0].first);
  CfgType newCfg;
  int weight = 0;

  StatClass* expandStatClass = this->GetMPProblem()->GetStatClass();
  string expandClockName = "RRTExpand time ";
  expandStatClass->StartClock(expandClockName);

  vector<CfgType> intermediateNodes;
  if(!this->GetMPProblem()->GetExtender(m_extenderLabel)->
      Extend(nearest, _dir, newCfg, intermediateNodes)) {
    if(this->m_debug)
      cout << "RRT could not expand!" << endl;
    return recentVID;
  }

  expandStatClass->StopClock(expandClockName);

  if(this->m_debug)
    cout << "RRT expanded to " << newCfg << endl;

  // If good to go, add to roadmap
  if(dm->Distance(newCfg, nearest) >= m_minDist) {
    recentVID = this->GetMPProblem()->GetRoadmap()->GetGraph()->
        AddVertex(newCfg);
    m_currentTree->push_back(recentVID);
    if(std::string::npos != m_gt.find("UNDIRECTED")) {
      pair<WeightType, WeightType> weights = make_pair(WeightType("RRTExpand",
          weight), WeightType("RRTExpand", weight));
      this->GetMPProblem()->GetRoadmap()->GetGraph()->
          AddEdge(kClosest[0].first, recentVID, weights);
    }
    else
      this->GetMPProblem()->GetRoadmap()->GetGraph()->AddEdge(kClosest[0].first,
          recentVID, WeightType("RRTExpand", weight));
    this->GetMPProblem()->GetRoadmap()->GetGraph()->
        GetVertex(recentVID).SetStat("Parent", kClosest[0].first);


    if(std::string::npos != m_gt.find("GRAPH")) {
      ConnectNeighbors(recentVID, kClosest[0].first);
    }

    for(size_t i = 2; i <= m_numDirections; i++) {
      //expansion to other m-1 directions
      CfgType randdir = this->SelectDispersedDirection(kClosest[0].first);
      expandStatClass->StartClock(expandClockName);
      bool expandFlag = this->GetMPProblem()->GetExtender(m_extenderLabel)->
          Extend(nearest, randdir, newCfg, intermediateNodes);

      expandStatClass->StopClock(expandClockName);
      StatClass* conStatClass = this->GetMPProblem()->GetStatClass();
      string conClockName = "Connection time ";
      conStatClass->StartClock(conClockName);

      if(!expandFlag && this->m_debug)
        cout << "RRT could not expand to additional directions!" << endl;
      else if(dm->Distance(newCfg, nearest) >= m_minDist) {
        VID otherVID = this->GetMPProblem()->GetRoadmap()->GetGraph()->
            AddVertex(newCfg);
        m_currentTree->push_back(otherVID);
        if(std::string::npos != m_gt.find("UNDIRECTED")) {
          pair<WeightType, WeightType> weights =
              make_pair(WeightType("RRTExpand", weight), WeightType("RRTExpand",
              weight));
          this->GetMPProblem()->GetRoadmap()->GetGraph()->
              AddEdge(kClosest[0].first, otherVID, weights);
        }
        else
          this->GetMPProblem()->GetRoadmap()->GetGraph()->AddEdge(
              kClosest[0].first, otherVID, WeightType("RRTExpand", weight));
        this->GetMPProblem()->GetRoadmap()->GetGraph()->GetVertex(
            otherVID).SetStat("Parent", kClosest[0].first);

        if(std::string::npos != m_gt.find("GRAPH")) {
          ConnectNeighbors(otherVID, kClosest[0].first);
        }
      }
      conStatClass->StopClock(conClockName);
    }
  }

  return recentVID;
}

template<class MPTraits>
void
IRRTStrategy<MPTraits>::
ConnectTrees(VID _recentlyGrown) {
  QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
  //Setup MP variables
  RoadmapType* rdmp = this->GetMPProblem()->GetRoadmap();
  DistanceMetricPointer dm = this->GetMPProblem()->GetDistanceMetric(m_dm);
  NeighborhoodFinderPointer nf =
      this->GetMPProblem()->GetNeighborhoodFinder(m_nf);
  CDInfo  cdInfo;

  if(m_trees.size() == 1)
    return; //trees connected

  CfgType c1 = rdmp->GetGraph()->GetVertex(_recentlyGrown);

  VID closestNode = INVALID_VID;
  CfgType closestCfg;
  double minDis = MAX_DBL;
  typename vector<vector<VID> >::iterator treeClosest = m_currentTree;
  CfgType c2;
  typedef typename vector<vector<VID> >::iterator TRIT;

  for(TRIT trit = m_trees.begin(); trit!=m_trees.end(); ++trit) {
    if(trit != m_currentTree) {
      vector<pair<VID, double> > closest;
      nf->FindNeighbors(rdmp, trit->begin(), trit->end(), c1,
          back_inserter(closest));
      if(closest.size() != 0) {
        c2  = rdmp->GetGraph()->GetVertex(closest[0].first);
        double dist = dm->Distance(c1, c2);
        if(dist < minDis) {
          treeClosest = trit;
          minDis = dist;
          closestNode = closest[0].first;
          closestCfg = c2;
        }
      }
    }
  }

  CfgType newCfg;
  int weight = 0;
  vector<CfgType> intermediateNodes;
  if(!this->GetMPProblem()->GetExtender(m_extenderLabel)->
      Extend(closestCfg, c1, newCfg, intermediateNodes)) {
    if(this->m_debug)
      cout << "RRT could not expand in connection phase, trapped!" << endl;
    return;
  }
  else {//add node to roadmap provided it's faraway enough or connects two trees
    VID newVID;
    if(c1 == newCfg) {//we connect the trees
      if(this->m_debug)
        cout<<"We connected the trees"<<endl;
      if(distance(m_trees.begin(), m_currentTree) >
          distance(m_trees.begin(), treeClosest)) {
        swap(m_currentTree, treeClosest);
      }
      m_currentTree->insert(m_currentTree->end(),treeClosest->begin(),
          treeClosest->end());
      m_trees.erase(treeClosest);
      newVID = _recentlyGrown;
    }
    else if(dm->Distance(newCfg, c2) >= m_minDist ) {
      newVID = this->GetMPProblem()->GetRoadmap()->GetGraph()->AddVertex(newCfg);
      treeClosest->push_back(newVID);
      m_currentTree = treeClosest;
    }
    else  //too close to add, keep expanding current tree
      return;

    //add to roadmap
    if(std::string::npos != m_gt.find("UNDIRECTED")) {
      pair<WeightType, WeightType> weights = make_pair(WeightType("RRTExpand",
          weight), WeightType("RRTExpand", weight));
      this->GetMPProblem()->GetRoadmap()->GetGraph()->AddEdge(closestNode,
          newVID, weights);
    }
    else
      this->GetMPProblem()->GetRoadmap()->GetGraph()->AddEdge(closestNode,
          newVID, WeightType("RRTExpand", weight));
    this->GetMPProblem()->GetRoadmap()->GetGraph()->
        GetVertex(closestNode).SetStat("Parent", newVID);

    if(std::string::npos != m_gt.find("GRAPH")) {
      ConnectNeighbors(newVID, closestNode);
    }
  }
}

template<class MPTraits>
void
IRRTStrategy<MPTraits>::
EvaluateGoals() {
  QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
  // Setup MP Variables
  Environment* env = this->GetMPProblem()->GetEnvironment();
  DistanceMetricPointer dmp = this->GetMPProblem()->GetDistanceMetric(m_dm);
  LocalPlannerPointer lpp = this->GetMPProblem()->GetLocalPlanner(m_lp);
  NeighborhoodFinderPointer nfp =
      this->GetMPProblem()->GetNeighborhoodFinder(m_nf);
  RoadmapType* rdmp = this->GetMPProblem()->GetRoadmap();

  LPOutput<MPTraits> lpOutput;
  // Check if goals have been found
  vector<size_t>::iterator i = m_goalsNotFound.begin();
  while(i != m_goalsNotFound.end()) {
    vector<pair<VID, double> > closests;
    nfp->FindNeighbors(rdmp, m_goals[*i], back_inserter(closests));
    CfgType closest = rdmp->GetGraph()->GetVertex(closests[0].first);
    double dist = dmp->Distance(m_goals[*i], closest);
    if(this->m_debug)
      cout << "Distance to goal::" << dist << endl;
    CfgType col;
    if(dist < m_delta && lpp->IsConnected(closest, m_goals[*i], col, &lpOutput,
        env->GetPositionRes(), env->GetOrientationRes(), true, false, false)) {
      if(this->m_debug)
        cout << "Goal found::" << m_goals[*i] << endl;
      VID goalVID;
      if(!(rdmp->GetGraph()->IsVertex( m_goals[*i])))
        goalVID = rdmp->GetGraph()->AddVertex(m_goals[*i]);
      else
        goalVID = rdmp->GetGraph()->GetVID(m_goals[*i]);
      rdmp->GetGraph()->AddEdge(closests[0].first, goalVID, lpOutput.m_edge);
      i = m_goalsNotFound.erase(i);
    }
    else
      ++i;
  }
}

template<class MPTraits>
void
IRRTStrategy<MPTraits>::
SetMPProblem(MPProblemType* _problem) {
  MPBaseObject<MPTraits>::SetMPProblem(_problem);
  if(m_query)
    m_query->SetMPProblem(_problem);
}

#endif
