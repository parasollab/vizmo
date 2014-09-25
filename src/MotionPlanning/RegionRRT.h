#ifndef REGIONRRT_H_
#define REGIONRRT_H_

#include "MPStrategies/MPStrategyMethod.h"
#include "MPStrategies/BasicRRTStrategy.h"

#include "Models/RegionModel.h"
#include "Models/RegionSphereModel.h"
#include "Models/RegionSphere2DModel.h"
#include "Models/Vizmo.h"

template<class MPTraits>
class RegionRRT : public BasicRRTStrategy<MPTraits> {
  public:
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPProblemType::VID VID;
    typedef typename MPProblemType::GraphType GraphType;
    typedef typename MPProblemType::LocalPlannerPointer LocalPlannerPointer;
    typedef typename MPProblemType::DistanceMetricPointer DistanceMetricPointer;
    typedef typename MPProblemType::NeighborhoodFinderPointer NeighborhoodFinderPointer;

    typedef EnvModel::RegionModelPtr RegionModelPtr;

    //Non-XML constructor w/ Query (by label)
    RegionRRT(const CfgType& _start = CfgType(),
        const CfgType& _goal = CfgType(), string _lp = "sl",
        string _dm = "euclidean", string _nf = "BFNF", string _vc = "PQP_SOLID",
        string _nc = "kClosest", string _gt = "UNDIRECTED_TREE",
        string _extenderLabel = "BERO",
        vector<string> _evaluators = vector<string>(), double _delta = 10.0,
        double _minDist = 0.001, double _growthFocus = 0.05,
        bool _evaluateGoal = true, size_t _numRoots = 1,
        size_t _numDirections = 1, size_t _maxTrial = 3,
        bool _growGoals = false);

    RegionRRT(MPProblemType* _problem, XMLNodeReader& _node);

    void Initialize();
    void Run();
    void Finalize();
    CfgType SelectDirection();

  protected:
    VID ExpandTree(CfgType& _dir);

  private:
    RegionModelPtr m_samplingRegion;
    CfgType m_qNew;
};

template<class MPTraits>
RegionRRT<MPTraits>::
RegionRRT(const CfgType& _start, const CfgType& _goal, string _lp, string _dm,
    string _nf, string _vc, string _nc, string _gt, string _extenderLabel,
    vector<string> _evaluators, double _delta, double _minDist,
    double _growthFocus, bool _evaluateGoal, size_t _numRoots,
    size_t _numDirections, size_t _maxTrial, bool _growGoals) :
    BasicRRTStrategy<MPTraits>(_lp, _dm, _nf, _vc, _nc, _gt, _extenderLabel,
        _evaluators, _delta, _minDist, _growthFocus, _evaluateGoal,
        _start, _goal, _numRoots, _numDirections, _maxTrial, _growGoals) {
  this->SetName("RegionRRT");
  this->m_delta = MAX_DBL;
}


template<class MPTraits>
RegionRRT<MPTraits>::
RegionRRT(MPProblemType* _problem, XMLNodeReader& _node) :
    BasicRRTStrategy<MPTraits>(_problem, _node) {
  this->SetName("RegionRRT");
  this->m_delta = MAX_DBL;
}

template<class MPTraits>
void
RegionRRT<MPTraits>::
Initialize() {
  BasicRRTStrategy<MPTraits>::Initialize();

  //Make non-user objects non-selectable while PathStrategy is running
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
  GetVizmo().GetRobot()->SetSelectable(false);
}

template<class MPTraits>
void
RegionRRT<MPTraits>::
Run() {
  //if(this->m_debug)
  cout << "\nRunning BasicRRTStrategy::" << endl;

  // Setup MP Variables
  StatClass* stats = this->GetMPProblem()->GetStatClass();

  GetVizmo().StartClock("RegionRRT");
  stats->StartClock("RRT Generation MP");

  CfgType dir;
  bool mapPassedEvaluation = false;
  while(!mapPassedEvaluation) {
    //find my growth direction. Default is to randomly select node or bias towards a goal
    double randomRatio = DRand();
    if(randomRatio < this->m_growthFocus) {
      dir = this->GoalBiasedDirection();
    }
    else {
      dir = this->SelectDirection();
    }

    // Randomize Current Tree
    this->m_currentTree = this->m_trees.begin() + LRand() % this->m_trees.size();

    VID recent = this->ExpandTree(dir);
    if(recent != INVALID_VID) {
      //connect various trees together
      this->ConnectTrees(recent);
      //see if tree is connected to goals
      if(this->m_evaluateGoal)
        this->EvaluateGoals(recent);

      //evaluate the roadmap
      bool evalMap = this->EvaluateMap(this->m_evaluators);
      if(!this->m_growGoals) {
        mapPassedEvaluation = this->m_trees.size() == 1 && evalMap &&
          ((this->m_evaluateGoal && this->m_goalsNotFound.size()==0) || !this->m_evaluateGoal);
        if(this->m_debug && this->m_goalsNotFound.size()==0)
          cout << "RRT FOUND ALL GOALS" << endl;
        if(this->m_trees.begin()->size() >= 15000)
          mapPassedEvaluation = true;
      }
      else
        mapPassedEvaluation = (evalMap && this->m_trees.size()==1);
    }
    else
      mapPassedEvaluation = false;

    /*if(m_samplingRegion != NULL) {
      //Delete region if q_new is in it
      shared_ptr<Boundary> boundary = m_samplingRegion->GetBoundary();
      if(this->GetMPProblem()->GetEnvironment()->InBounds(m_qNew, boundary)) {
        GetVizmo().GetSelectedModels().clear();
        GetMainWindow()->GetGLWidget()->SetCurrentRegion();
        GetVizmo().GetEnv()->DeleteRegion(m_samplingRegion);
      }
    }*/

    GetVizmo().GetMap()->RefreshMap();
    usleep(10000);
  }

  GetVizmo().StopClock("RegionRRT");
  stats->StopClock("RRT Generation MP");

  if(this->m_debug)
    cout<<"\nEnd Running RegionRRT::Run" << endl;
}

template<class MPTraits>
void
RegionRRT<MPTraits>::
Finalize() {
  //setup variables
  StatClass* stats = this->GetMPProblem()->GetStatClass();
  string basename = this->GetBaseFilename();

  //print info to console
  cout << "\nFinalizing BasicRRTStrategy::" << endl;
  GetVizmo().PrintClock("RegionRRT", cout);
  stats->PrintClock("RRT Generation MP", cout);

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //perform query if query was given as input
  if(this->m_query) {
    this->m_query->SetPathFile(basename + ".path");
    if(this->m_evaluateGoal) {
      if(this->m_query->PerformQuery(this->GetMPProblem()->GetRoadmap()) &&
          this->m_debug)
        cout << "Query successful!" << endl;
      else if(this->m_debug)
        cout << "Query unsuccessful." << endl;
    }
  }

  //output final map
  ofstream osMap((basename + ".map").c_str());
  this->GetMPProblem()->GetRoadmap()->Write(osMap,
      this->GetMPProblem()->GetEnvironment());
  osMap.close();

  //output stats
  ofstream osStat((basename + ".stat").c_str());
  osStat << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(osStat, this->GetMPProblem()->GetRoadmap());
  GetVizmo().PrintClock("RegionRRT", osStat);
  stats->PrintClock("RRT Generation MP", osStat);
  osStat.close();

  //show results pop-up
  ostringstream results;
  results << "Planning Complete!" << endl;
  GetVizmo().PrintClock("RegionRRT", results);
  stats->PrintClock("RRT Generation MP", results);
  GetMainWindow()->AlertUser(results.str());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
  GetVizmo().GetRobot()->SetSelectable(true);

  if(this->m_debug)
    cout << "\nEnd Finalizing BasicRRTStrategy" << endl;
}

template<class MPTraits>
typename RegionRRT<MPTraits>::CfgType
RegionRRT<MPTraits>::
SelectDirection() {
  shared_ptr<Boundary> samplingBoundary;
  const vector<RegionModelPtr>& regions = GetVizmo().GetEnv()->GetAttractRegions();
  Environment* env = this->GetMPProblem()->GetEnvironment();

  size_t _index = rand() % (regions.size() + 1);

  if(_index == regions.size()) {
    m_samplingRegion.reset();
    samplingBoundary = this->GetMPProblem()->GetEnvironment()->GetBoundary();
  }
  else {
    m_samplingRegion = regions[_index];
    samplingBoundary = m_samplingRegion->GetBoundary();
  }

  try {
    CfgType mySample;
    mySample.GetRandomCfg(env,samplingBoundary);
    if(m_samplingRegion != NULL)
      m_samplingRegion->SetColor(Color4(0, 1, 0, 0.5));
    return mySample;
  }
  //catch Boundary too small exception
  catch(PMPLException _e) {
    m_samplingRegion->SetColor(Color4(1, 0, 0, 1));
    CfgType mySample;
    mySample.GetRandomCfg(env);
    return mySample;
  }
  //catch all others and exit
  catch(exception _e) {
    cerr << _e.what() << endl;
    exit(1);
  }
}

template<class MPTraits>
typename RegionRRT<MPTraits>::VID
RegionRRT<MPTraits>::
ExpandTree(CfgType& _dir) {
  // Setup MP Variables
  DistanceMetricPointer dm = this->GetMPProblem()->GetDistanceMetric(this->m_dm);
  NeighborhoodFinderPointer nf = this->GetMPProblem()->GetNeighborhoodFinder(this->m_nf);
  LocalPlannerPointer lp = this->GetMPProblem()->GetLocalPlanner(this->m_lp);
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
  for(TRIT trit = this->m_trees.begin(); trit!=this->m_trees.end(); ++trit){
    treeSize += trit->size();
  }
  bool fixTree = false;
  if(treeSize > numRoadmapVertex)
    fixTree = true;
  else {
    vector<pair<size_t, VID> > ccs;
    stapl::sequential::vector_property_map<GraphType, size_t> cmap;
    get_cc_stats(*g, cmap, ccs);
    if(ccs.size() != this->m_trees.size())
      fixTree = true;
  }
  if(fixTree) { //node deleted by dynamic environment, fix all trees
    this->m_trees.clear();
    vector<pair<size_t, VID> > ccs;
    stapl::sequential::vector_property_map<GraphType, size_t> cmap;
    get_cc_stats(*g, cmap, ccs);
    vector<VID> ccVIDs;
    typename vector<pair<size_t, VID> >::const_iterator ccIt;
    for(ccIt = ccs.begin(); ccIt != ccs.end(); ccIt++) {
      cmap.reset();
      ccVIDs.clear();
      get_cc(*g, cmap, ccIt->second, ccVIDs);
      this->m_trees.push_back(ccVIDs);
    }
    this->m_currentTree = this->m_trees.begin();
  }

  StatClass* kcloseStatClass = this->GetMPProblem()->GetStatClass();
  string kcloseClockName = "kclosest time ";
  kcloseStatClass->StartClock(kcloseClockName);
  nf->FindNeighbors(this->GetMPProblem()->GetRoadmap(), this->m_currentTree->begin(), this->m_currentTree->end(), _dir, back_inserter(kClosest));
  kcloseStatClass->StopClock(kcloseClockName);

  CfgType nearest = this->GetMPProblem()->GetRoadmap()->GetGraph()->GetVertex(kClosest[0].first);
  CfgType newCfg;
  int weight = 0;

  StatClass* expandStatClass = this->GetMPProblem()->GetStatClass();
  string expandClockName = "RRTExpand time ";
  expandStatClass->StartClock(expandClockName);

  vector<CfgType> intermediateNodes;
  if(!this->GetMPProblem()->GetExtender(this->m_extenderLabel)->Extend(nearest, _dir, newCfg, intermediateNodes)) {
    if(this->m_debug) cout << "RRT could not expand!" << endl;
    return recentVID;
  }

  expandStatClass->StopClock(expandClockName);

  if(this->m_debug) cout << "RRT expanded to " << newCfg << endl;

  // If good to go, add to roadmap
  if(dm->Distance(newCfg, nearest) >= this->m_minDist ) {
    recentVID = this->GetMPProblem()->GetRoadmap()->GetGraph()->AddVertex(newCfg);

    // Store q_new to check if it is in region later
    m_qNew = newCfg;

    this->m_currentTree->push_back(recentVID);
    if(std::string::npos != this->m_gt.find("UNDIRECTED")){
      pair<WeightType, WeightType> weights = make_pair(WeightType("RRTExpand", weight), WeightType("RRTExpand", weight));
      this->GetMPProblem()->GetRoadmap()->GetGraph()->AddEdge(kClosest[0].first, recentVID, weights);
    }
    else
      this->GetMPProblem()->GetRoadmap()->GetGraph()->AddEdge(kClosest[0].first, recentVID, WeightType("RRTExpand", weight));
    this->GetMPProblem()->GetRoadmap()->GetGraph()->GetVertex(recentVID).SetStat("Parent", kClosest[0].first);


    if(std::string::npos != this->m_gt.find("GRAPH")){
      this->ConnectNeighbors(recentVID, kClosest[0].first);
    }

    for( size_t i = 2; i <= this->m_numDirections; i++){//expansion to other m-1 directions
      CfgType randdir = this->SelectDispersedDirection(kClosest[0].first);
      expandStatClass->StartClock(expandClockName);
      bool expandFlag = this->GetMPProblem()->GetExtender(this->m_extenderLabel)
        ->Extend(nearest, randdir, newCfg, intermediateNodes);

      expandStatClass->StopClock(expandClockName);
      StatClass* conStatClass = this->GetMPProblem()->GetStatClass();
      string conClockName = "Connection time ";
      conStatClass->StartClock(conClockName);

      if(!expandFlag) {
        if(this->m_debug) cout << "RRT could not expand to additional directions!" << endl;
      }
      else if(dm->Distance(newCfg, nearest) >= this->m_minDist ) {
        VID otherVID = this->GetMPProblem()->GetRoadmap()->GetGraph()->AddVertex(newCfg);
        this->m_currentTree->push_back(otherVID);
        if(std::string::npos != this->m_gt.find("UNDIRECTED")) {
          pair<WeightType, WeightType> weights = make_pair(WeightType("RRTExpand", weight), WeightType("RRTExpand", weight));
          this->GetMPProblem()->GetRoadmap()->GetGraph()->AddEdge(kClosest[0].first, otherVID, weights);
        }
        else
          this->GetMPProblem()->GetRoadmap()->GetGraph()->AddEdge(kClosest[0].first, otherVID, WeightType("RRTExpand", weight));
        this->GetMPProblem()->GetRoadmap()->GetGraph()->GetVertex(otherVID).SetStat("Parent", kClosest[0].first);

        if(std::string::npos != this->m_gt.find("GRAPH")){
          this->ConnectNeighbors( otherVID, kClosest[0].first);
        }
      }
      conStatClass->StopClock(conClockName);
    }
  }

  return recentVID;
}

#endif
