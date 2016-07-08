#ifndef AUTO_REGION_RRT_H_
#define AUTO_REGION_RRT_H_

#include <unordered_map>

#include "MPStrategies/MPStrategyMethod.h"
#include "MPStrategies/BasicRRTStrategy.h"

#include "GUI/ModelSelectionWidget.h"
#include "Models/RegionModel.h"
#include "Models/RegionSphereModel.h"
#include "Models/RegionSphere2DModel.h"
#include "Models/Vizmo.h"
#include "Utilities/ReebGraphConstruction.h"
#include "Utilities/TetGenDecomposition.h"

extern TetGenDecomposition* t;

////////////////////////////////////////////////////////////////////////////////
/// \brief  AutoRegionRRT
////////////////////////////////////////////////////////////////////////////////
template<class MPTraits>
class AutoRegionRRT : public BasicRRTStrategy<MPTraits> {

  public:

    // Local Types
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::CfgRef CfgRef;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPProblemType::VID VID;
    typedef typename MPProblemType::GraphType GraphType;
    typedef typename MPProblemType::LocalPlannerPointer LocalPlannerPointer;
    typedef typename MPProblemType::DistanceMetricPointer DistanceMetricPointer;
    typedef typename MPProblemType::NeighborhoodFinderPointer
        NeighborhoodFinderPointer;
    typedef EnvModel::RegionModelPtr RegionModelPtr;

    // Construction
    AutoRegionRRT(const CfgType& _start = CfgType(),
        const CfgType& _goal = CfgType(),
        string _dm = "euclidean", string _nf = "BFNF", string _vc = "PQP_SOLID",
        string _nc = "kClosest", string _gt = "UNDIRECTED_TREE",
        string _extenderLabel = "BERO",
        vector<string> _evaluators = vector<string>(),
        double _minDist = 0.001, double _growthFocus = 0.05,
        bool _evaluateGoal = true, size_t _numRoots = 1,
        size_t _numDirections = 1, size_t _maxTrial = 3,
        bool _growGoals = false);
    AutoRegionRRT(MPProblemType* _problem, XMLNode& _node);

    // Inherited functions
    void Initialize();
    void Run();
    void Finalize();

  private:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Computes the growth direction for the RRT, choosing between the
    ///         entire environment and each attract region with uniform
    ///         probability to generate q_rand.
    /// \return The resulting growth direction.
    CfgType SelectDirection();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Grows the tree in the input direction.
    /// \param[in] _dir The growth direction.
    /// \return         The VID of the new node.
    VID ExpandTree(CfgType& _dir);

    RegionModelPtr m_samplingRegion;  ///< Points to the current sampling region.
    CfgType m_qNew;                   ///< Storage for checking avoid regions.

    TetGenDecomposition* m_tetrahedralization; ///< TetGen decomposition
    string m_switches;             ///< Input switches to TetGen model
    bool m_writeFreeModel;         ///< Output TetGen model of freespace
    bool m_writeDecompModel;       ///< Output TetGen model tetrahedrons

    ReebGraphConstruction* m_reebGraphConstruction; ///< Embedded reeb graph
};


template<class MPTraits>
AutoRegionRRT<MPTraits>::
AutoRegionRRT(const CfgType& _start, const CfgType& _goal, string _dm,
    string _nf, string _vc, string _nc, string _gt, string _extenderLabel,
    vector<string> _evaluators, double _minDist,
    double _growthFocus, bool _evaluateGoal, size_t _numRoots,
    size_t _numDirections, size_t _maxTrial, bool _growGoals) :
  BasicRRTStrategy<MPTraits>(_dm, _nf, _vc, _nc, _gt, _extenderLabel,
      _evaluators, _minDist, _growthFocus, _evaluateGoal,
      _start, _goal, _numRoots, _numDirections, _maxTrial, _growGoals) {
    this->SetName("AutoRegionRRT");
    m_switches = "pqnQ";
    m_writeFreeModel = false;
    m_writeDecompModel = false;
  }

template<class MPTraits>
AutoRegionRRT<MPTraits>::
AutoRegionRRT(MPProblemType* _problem, XMLNode& _node) :
  BasicRRTStrategy<MPTraits>(_problem, _node) {
    this->SetName("AutoRegionRRT");
    m_switches = "pqnQ";
    m_writeFreeModel = false;
    m_writeDecompModel = false;
  }

template<class MPTraits>
void
AutoRegionRRT<MPTraits>::
Initialize() {
  BasicRRTStrategy<MPTraits>::Initialize();

  //Tetrahedralize environment
  m_tetrahedralization = new TetGenDecomposition(m_switches,
      m_writeFreeModel, m_writeDecompModel);
  m_tetrahedralization->Decompose(this->GetEnvironment());
  GetVizmo().GetEnv()->AddTetGenDecompositionModel(m_tetrahedralization);
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //Embed ReebGraph
  m_reebGraphConstruction = new ReebGraphConstruction();
  m_reebGraphConstruction->Construct(this->GetEnvironment(),
      this->GetBaseFilename());
  GetVizmo().GetEnv()->AddReebGraphModel(m_reebGraphConstruction);
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //Make non-user objects non-selectable while PathStrategy is running
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
}


template<class MPTraits>
void
AutoRegionRRT<MPTraits>::
Run() {
  //if(this->m_debug)
  cout << "\nRunning BasicRRTStrategy::" << endl;

  // Setup MP Variables
  StatClass* stats = this->GetMPProblem()->GetStatClass();

  GetVizmo().StartClock("AutoRegionRRT");
  stats->StartClock("RRT Generation MP");

  Vector3d start = this->m_query->GetQuery()[0].GetPoint();

  //Get directed flow network
  typedef ReebGraphConstruction::FlowGraph FlowGraph;
  typedef FlowGraph::vertex_descriptor FVD;
  typedef FlowGraph::edge_descriptor FED;
  pair<FlowGraph, FVD> flow = m_reebGraphConstruction->
    GetFlowGraph(start, this->GetEnvironment()->GetPositionRes());
  //cout << "flow: " << flow.get_num_vertices() << " " << flow.get_num_edges() << endl;

  unordered_map<FVD, bool> visited;
  for(auto vit = flow.first.begin(); vit != flow.first.end(); ++vit)
    visited[vit->descriptor()] = false;

  double robotRadius = this->GetEnvironment()->GetRobot(0)->GetBoundingSphereRadius();
  unordered_map<RegionModelPtr, pair<FED, size_t>> regions;
  auto sit = flow.first.find_vertex(flow.second);
  for(auto eit = sit->begin(); eit != sit->end(); ++eit) {
    auto i = regions.emplace(
        RegionModelPtr(new RegionSphereModel(start, 3*robotRadius, false)),
        make_pair(eit->descriptor(), 0));
    GetVizmo().GetEnv()->AddAttractRegion(i.first->first);
  }
  visited[sit->descriptor()] = true;

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

      if(m_samplingRegion) {
        m_samplingRegion->ClearFACount();
        //Delete region if q_new is in it
        while(this->GetMPProblem()->GetEnvironment()->InBounds(m_qNew, m_samplingRegion->GetBoundary())) {
          //GetVizmo().GetSelectedModels().clear();
          //GetMainWindow()->GetGLWidget()->SetCurrentRegion();
          Vector3d cur = m_samplingRegion->GetCenter();

          auto& pr = regions[m_samplingRegion];
          FlowGraph::vertex_iterator vi;
          FlowGraph::adj_edge_iterator ei;
          flow.first.find_edge(pr.first, vi, ei);
          vector<Vector3d>& path = ei->property();
          size_t& i = pr.second;
          size_t j = i+1;
          if(j < path.size()) {
            Vector3d& next = path[j];
            m_samplingRegion->ApplyOffset(next-cur);
            i = j;
          }
          //else need to delete region
          else {
            GetVizmo().GetEnv()->DeleteRegion(m_samplingRegion);
            regions.erase(m_samplingRegion);
            break;
          }
        }
      }

      //Add new regions
      CfgRef newest = this->GetRoadmap()->GetGraph()->GetVertex(recent);
      Vector3d p = newest.GetPoint();

      for(auto vit = flow.first.begin(); vit != flow.first.end(); ++vit) {
        double dist = (vit->property() - p).norm();
        if(dist < 3*robotRadius && !visited[vit->descriptor()]) {
          for(auto eit = vit->begin(); eit != vit->end(); ++eit) {
            auto i = regions.emplace(
                RegionModelPtr(new RegionSphereModel(vit->property(), 3*robotRadius, false)),
                make_pair(eit->descriptor(), 0));
            GetVizmo().GetEnv()->AddAttractRegion(i.first->first);
          }
          visited[vit->descriptor()] = true;
        }
      }

      //connect various trees together
      this->ConnectTrees(recent);
      //see if tree is connected to goals
      if(this->m_evaluateGoal)
        this->EvaluateGoals(recent);

      //evaluate the roadmap
      bool evalMap = this->EvaluateMap();
      bool oneTree = this->m_trees.size() == 1;
      if(!this->m_growGoals) {
        bool useGoals = this->m_evaluateGoal;
        bool allGoals = useGoals && this->m_goalsNotFound.size() == 0;
        mapPassedEvaluation = evalMap && oneTree && (allGoals || !useGoals);
        if(this->m_debug && allGoals)
          cout << "RRT FOUND ALL GOALS" << endl;
        if(this->m_trees.begin()->size() >= 15000)
          mapPassedEvaluation = true;
      }
      else
        mapPassedEvaluation = evalMap && oneTree;
    }
    else {
      if(m_samplingRegion) {
        m_samplingRegion->IncreaseFACount(1);
        if(m_samplingRegion->GetFACount() > 100) {
          GetVizmo().GetEnv()->DeleteRegion(m_samplingRegion);
          regions.erase(m_samplingRegion);
        }
      }
      mapPassedEvaluation = false;
    }

    GetVizmo().GetMap()->RefreshMap();
    //usleep(10000);
  }

  GetVizmo().StopClock("AutoRegionRRT");
  stats->StopClock("RRT Generation MP");

  for(auto r : regions)
    GetVizmo().GetEnv()->DeleteRegion(r.first);
  regions.clear();

  if(this->m_debug)
    cout<<"\nEnd Running AutoRegionRRT::Run" << endl;
}


template<class MPTraits>
void
AutoRegionRRT<MPTraits>::
Finalize() {
  //setup variables
  StatClass* stats = this->GetStatClass();
  string basename = this->GetBaseFilename();

  //print info to console
  cout << "\nFinalizing BasicRRTStrategy::" << endl;
  GetVizmo().PrintClock("AutoRegionRRT", cout);
  stats->PrintClock("RRT Generation MP", cout);

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //perform query if query was given as input
  if(this->m_query) {
    if(this->m_evaluateGoal) {
      if(this->m_query->PerformQuery(this->GetRoadmap()) && this->m_debug)
        cout << "Query successful!" << endl;
      else if(this->m_debug)
        cout << "Query unsuccessful." << endl;
    }
  }

  //output final map
  this->GetRoadmap()->Write(basename + ".map", this->GetEnvironment());

  //output stats
  ofstream osStat((basename + ".stat").c_str());
  osStat << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(osStat, this->GetRoadmap());
  GetVizmo().PrintClock("AutoRegionRRT", osStat);
  stats->PrintClock("RRT Generation MP", osStat);
  osStat.close();

  //show results pop-up
  ostringstream results;
  results << "Planning Complete!" << endl;
  GetVizmo().PrintClock("AutoRegionRRT", results);
  stats->PrintClock("RRT Generation MP", results);
  GetMainWindow()->AlertUser(results.str());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);

  if(this->m_debug)
    cout << "\nEnd Finalizing BasicRRTStrategy" << endl;
}


template<class MPTraits>
typename AutoRegionRRT<MPTraits>::CfgType
AutoRegionRRT<MPTraits>::
SelectDirection() {
  shared_ptr<Boundary> samplingBoundary;
  const vector<RegionModelPtr>& regions = GetVizmo().GetEnv()->GetAttractRegions();
  Environment* env = this->GetEnvironment();

  size_t _index = rand() % (regions.size() + 1);

  if(_index == regions.size()) {
    m_samplingRegion.reset();
    samplingBoundary = this->GetEnvironment()->GetBoundary();
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
typename AutoRegionRRT<MPTraits>::VID
AutoRegionRRT<MPTraits>::
ExpandTree(CfgType& _dir) {
  // Setup MP Variables
  DistanceMetricPointer dm = this->GetDistanceMetric(this->m_dm);
  NeighborhoodFinderPointer nf = this->GetNeighborhoodFinder(this->m_nf);
  GraphType* g = this->GetRoadmap()->GetGraph();
  VID recentVID = INVALID_VID;
  CDInfo cdInfo;

  // Find closest Cfg in map
  vector<pair<VID, double>> kClosest;
  vector<CfgType> cfgs;

  int numRoadmapVertex = g->get_num_vertices();
  int treeSize = 0;
  for(auto& tree : this->m_trees)
    treeSize += tree.size();

  bool fixTree = false;
  if(treeSize > numRoadmapVertex)
    fixTree = true;
  else {
    vector<pair<size_t, VID>> ccs;
    stapl::sequential::vector_property_map<GraphType, size_t> cmap;
    get_cc_stats(*g, cmap, ccs);
    if(ccs.size() != this->m_trees.size())
      fixTree = true;
  }

  if(fixTree) {
    //node deleted by dynamic environment, fix all trees
    this->m_trees.clear();
    vector<pair<size_t, VID>> ccs;
    stapl::sequential::vector_property_map<GraphType, size_t> cmap;
    get_cc_stats(*g, cmap, ccs);
    vector<VID> ccVIDs;
    for(const auto& cc : ccs) {
      cmap.reset();
      ccVIDs.clear();
      get_cc(*g, cmap, cc.second, ccVIDs);
      this->m_trees.push_back(ccVIDs);
    }
    this->m_currentTree = this->m_trees.begin();
  }

  StatClass* kcloseStatClass = this->GetStatClass();
  string kcloseClockName = "kclosest time ";
  kcloseStatClass->StartClock(kcloseClockName);
  nf->FindNeighbors(this->GetRoadmap(), this->m_currentTree->begin(),
      this->m_currentTree->end(), true, _dir, back_inserter(kClosest));
  kcloseStatClass->StopClock(kcloseClockName);

  CfgType nearest = g->GetVertex(kClosest[0].first);
  CfgType newCfg;

  StatClass* expandStatClass = this->GetStatClass();
  string expandClockName = "RRTExpand time ";
  expandStatClass->StartClock(expandClockName);

  LPOutput<MPTraits> lpOut;
  auto extender = this->GetExtender(this->m_extenderLabel);
  if(!extender->Extend(nearest, _dir, newCfg, lpOut)) {
    if(this->m_debug)
      cout << "RRT could not expand!" << endl;
    return recentVID;
  }

  expandStatClass->StopClock(expandClockName);

  if(this->m_debug)
    cout << "RRT expanded to " << newCfg << endl;

  // If good to go, add to roadmap
  if(dm->Distance(newCfg, nearest) >= this->m_minDist ) {
    recentVID = g->AddVertex(newCfg);

    // Store q_new to check if it is in region later
    m_qNew = newCfg;

    this->m_currentTree->push_back(recentVID);
    if(std::string::npos != this->m_gt.find("UNDIRECTED")){
      g->AddEdge(kClosest[0].first, recentVID, lpOut.m_edge);
    }
    else
      g->AddEdge(kClosest[0].first, recentVID, lpOut.m_edge.first);
    g->GetVertex(recentVID).SetStat("Parent", kClosest[0].first);


    if(std::string::npos != this->m_gt.find("GRAPH"))
      this->ConnectNeighbors(recentVID);

    for(size_t i = 2; i <= this->m_numDirections; i++) {
      //expansion to other m-1 directions
      CfgType randdir = this->SelectDispersedDirection(kClosest[0].first);
      expandStatClass->StartClock(expandClockName);
      LPOutput<MPTraits> lpOut;
      bool expandFlag = extender->Extend(nearest, randdir, newCfg, lpOut);

      expandStatClass->StopClock(expandClockName);
      StatClass* conStatClass = this->GetStatClass();
      string conClockName = "Connection time ";
      conStatClass->StartClock(conClockName);

      if(!expandFlag) {
        if(this->m_debug)
          cout << "RRT could not expand to additional directions!" << endl;
      }
      else if(dm->Distance(newCfg, nearest) >= this->m_minDist ) {
        VID otherVID = g->AddVertex(newCfg);
        this->m_currentTree->push_back(otherVID);
        if(std::string::npos != this->m_gt.find("UNDIRECTED")) {
          g->AddEdge(kClosest[0].first, otherVID, lpOut.m_edge);
        }
        else
          g->AddEdge(kClosest[0].first, otherVID, lpOut.m_edge.first);
        g->GetVertex(otherVID).SetStat("Parent", kClosest[0].first);

        if(std::string::npos != this->m_gt.find("GRAPH")){
          this->ConnectNeighbors(otherVID);
        }
      }
      conStatClass->StopClock(conClockName);
    }
  }

  return recentVID;
}

#endif
