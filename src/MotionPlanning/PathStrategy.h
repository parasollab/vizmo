#ifndef PATH_STRATEGY_H_
#define PATH_STRATEGY_H_

#include "MPStrategies/MPStrategyMethod.h"

#include "Models/EnvModel.h"
#include "Models/UserPathModel.h"
#include "Models/Vizmo.h"

#include "ValidityCheckers/CollisionDetectionValidity.h"

#include "Utilities/GLUtils.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief   PathStrategy generates configurations near each user-generated path.
/// \details PathStrategy collects a set of workspace points along the
///          user-generated paths and generates randomly oriented configurations
///          at those points. Valid points are added to the map and connected in
///          path order. Invalid points are pushed to the medial axis. If a
///          connection between two path configurations \c c1 and \c c2 fails, an
///          intermediate configuration cI is generated at their midpoint. If cI
///          is invalid, it is also pushed to the medial axis. The path
///          connection then recurses on the set (c1, cI, c2).
///
/// \bug     Medial-axis pushing is currently disabled due to erroneous trunk
///          code. Once the fixes are checked in (from the MA planning journal
///          branch), we can repair this functionality.
////////////////////////////////////////////////////////////////////////////////
template<class MPTraits>
class PathStrategy : public MPStrategyMethod<MPTraits> {

  public:

    // Local types.
    typedef typename MPTraits::MPProblemType     MPProblemType;
    typedef typename MPTraits::CfgType           CfgType;
    typedef typename MPTraits::WeightType        WeightType;
    typedef typename MPProblemType::RoadmapType  RoadmapType;
    typedef typename RoadmapType::VID            VID;
    typedef typename RoadmapType::GraphType      GraphType;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The default constructor uses pqp_solid collision checking,
    ///        uniform random sampling, straight-line local planning, k-closest
    ///        connector, and max nodes evaluation.
    PathStrategy();
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The XML constructor uses defaults if values are not provided.
    /// \param[in] _problem The current MPProblem.
    /// \param[in] _node The XML node reader.
    PathStrategy(MPProblemType* _problem, XMLNode& _node);

    void Initialize(); ///< Fetch data and start clocks.
    void Run();        ///< Execute path planning.
    void Finalize();   ///< Stop clocks and generate output files.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Print name, label, and component information.
    /// \param[in] _os The ostream to print to.
    void Print(ostream& _os) const;

  protected:

    //helper functions
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Check the configuration's validity. If invalid, push it to the
    /// medial axis.
    /// \param[in/out] _cfg The configuration to validate.
    /// \return A \c bool indicating whether the configuration was successfully
    /// validated.
    bool ValidateCfg(CfgType& _cfg);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Validate and add configurations to the roadmap. Store their VID's
    /// for connection.
    /// \param[in] _samples Configurations to add.
    /// \param[out] _vids   The corresponding VID's in the roadmap.
    void AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Try to connect new nodes to the roadmap in the path sequence. If
    /// a connection attempt fails, attempt to create intermediates.
    /// \param[in] _vids The VID's of the path nodes, in path order.
    void ConnectPath(const vector<VID>& _vids);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Try to connect new nodes to the roadmap using standard connector.
    /// \param[in] _vids The VID's of the new nodes.
    void Connect(vector<VID>& _vids);

  private:

    //data objects
    vector<UserPathModel*> m_userPaths; ///< The set of all existing user paths.
    vector<VID>            m_endPoints; ///< The set of all path heads/tails.
    Query<MPTraits>*       m_query;     ///< The PMPL Query.

    //pmpl function object labels
    string m_vcLabel;                   ///< The ValidityChecker label.
    string m_dmLabel;                   ///< The DistanceMetric label.
    string m_lpLabel;                   ///< The LocalPlanner label.
    string m_connectorLabel;            ///< The Connector label.

    //pmpl function objects
    MedialAxisUtility<MPTraits> m_medialAxisUtility; ///< For pushing to the MA.
};


template<class MPTraits>
PathStrategy<MPTraits>::
PathStrategy() :
    m_vcLabel("PQP_SOLID"), m_dmLabel("euclidean"), m_lpLabel("sl"),
    m_connectorLabel("kClosest") {
  this->SetName("PathStrategy");
}


template<class MPTraits>
PathStrategy<MPTraits>::
PathStrategy (MPProblemType* _problem, XMLNode& _node) :
    MPStrategyMethod<MPTraits>(_problem, _node) {
  this->SetName("PathStrategy");
  m_vcLabel = _node.Read("vcLabel", false, "PQP_SOLID", "Validity Checker");
  m_dmLabel = _node.Read("dmLabel", false, "euclidean", "Distance Metric");
  m_lpLabel = _node.Read("lpLabel", false, "sl", "Local Planner");
  m_connectorLabel = _node.Read("connectionLabel", false, "kClosest",
      "Connector");
}


template<class MPTraits>
void
PathStrategy<MPTraits>::
Print(ostream& _os) const {
  _os << this->GetNameAndLabel()
      << "\n\tValidity Checker: " << m_vcLabel
      << "\n\tDistance Metric:  " << m_dmLabel
      << "\n\tLocal Planner:    " << m_lpLabel
      << "\n\tConnector:        " << m_connectorLabel
      << endl << flush;
}


template<class MPTraits>
void
PathStrategy<MPTraits>::
Initialize() {
  /// This function is a no-op if no paths exist.
  m_userPaths = GetVizmo().GetEnv()->GetUserPaths();
  if(m_userPaths.empty())
    return;

  cout << "Initializing Path Strategy." << endl;

  //get function objects
  m_query     = NULL;
  m_medialAxisUtility = MedialAxisUtility<MPTraits>(this->GetMPProblem(),
      m_vcLabel, m_dmLabel, false, false);

  //if a vizmo query is loaded, add the query cfgs to the map and set the map
  //evaluator to bounded query
  if(GetVizmo().IsQueryLoaded()) {
    m_query = &*(static_pointer_cast<Query<MPTraits> >(
        this->GetMapEvaluator("Query")));
    AddToRoadmap(m_query->GetQuery(), m_endPoints); //store vids with path ends
  }

  //Make non-user objects non-selectable while PathStrategy is running
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
  GetVizmo().GetRobot()->SetSelectable(false);

  //start clocks
  GetVizmo().StartClock("PathStrategy");
  this->GetStatClass()->StartClock("PathStrategyMP");
}


template<class MPTraits>
void
PathStrategy<MPTraits>::
Run() {
  /// This function is a no-op if no paths exist.
  if(m_userPaths.empty())
    return;

  cout << "Running Path Strategy." << endl;

  //iterate through all paths to build and connect their c-space analogs
  typedef typename vector<UserPathModel*>::const_iterator PIT;
  for(PIT pit = m_userPaths.begin(); pit != m_userPaths.end(); ++pit) {
    //get cfgs along this path
    shared_ptr<vector<CfgType> > cfgs = (*pit)->GetCfgs();

    //add cfgs to roadmap
    vector<VID> vids;
    AddToRoadmap(*cfgs, vids);

    //store head and tail
    m_endPoints.push_back(vids.front());
    m_endPoints.push_back(vids.back());

    //connect cfgs with path connector
    ConnectPath(vids);
  }

  //try to connect all endpoints
  auto connector = this->GetConnector(m_connectorLabel);
  auto map = this->GetRoadmap();
  connector->Connect(map, m_endPoints.begin(), m_endPoints.end(),
      m_endPoints.begin(), m_endPoints.end());
}


template<class MPTraits>
void
PathStrategy<MPTraits>::
Finalize() {
  /// If no paths exist, alert user that planner did not run.
  if(m_userPaths.empty()) {
    GetMainWindow()->AlertUser("Error: no user-path exists!");
    return;
  }

  cout << "Finalizing Path Strategy." << endl;
  GetVizmo().StopClock("PathStrategy");
  auto stats = this->GetStatClass();
  stats->StopClock("PathStrategyMP");

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //print clocks to terminal
  GetVizmo().PrintClock("PathStrategy", cout);
  stats->PrintClock("PathStrategyMP", cout);

  //base filename
  string basename = this->GetBaseFilename();

  //output stats
  ofstream ostats((basename + ".stat").c_str());
  ostats << "Sampling and Connection Stats:" << endl;
  auto map = this->GetRoadmap();
  stats->PrintAllStats(ostats, map);
  GetVizmo().PrintClock("PathStrategy", ostats);
  stats->PrintClock("PathStrategyMP", ostats);

  //output roadmap
  Environment* env = this->GetEnvironment();
  map->Write(basename + ".map", env);

  //output a path file if a query was loaded
  ostringstream results;
  if(m_query != NULL) {
    if(!m_query->PerformQuery(map))
      results << "Planning Failed!" << endl;
    else
      results << "Planning Complete!" << endl;
  }
  else
    results << "Planning Complete!" << endl;

  //show results pop-up
  GetVizmo().PrintClock("PathStrategy", results);
  GetMainWindow()->AlertUser(results.str());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
  GetVizmo().GetRobot()->SetSelectable(true);
}


template<class MPTraits>
bool
PathStrategy<MPTraits>::
ValidateCfg(CfgType& _cfg) {
  /// If the configuration is invalid, push it to the medial axis.
  /// \bug The current trunk version of MedialAxisUtility is not correct.
  ///      Medial-axis validation will be disabled until the medial-axis branch
  ///      gets merged into to the trunk.
  auto env = this->GetEnvironment();
  if(!env->InBounds(_cfg))
    return false;
  auto vc = this->GetValidityChecker(m_vcLabel);
  if(!vc->IsValid(_cfg, this->GetNameAndLabel())) {
//assumes approximate MA push
//    CfgType temp;
//    bool fixed = false;
//    while(!fixed) {
//      temp = _cfg;
//      fixed = m_medialAxisUtility.PushToMedialAxis(temp, env->GetBoundary());
//    }
//    _cfg = temp;
    return false;
  }
  return true;
}


template<class MPTraits>
void
PathStrategy<MPTraits>::
AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids) {
  _vids.clear();
  auto g = this->GetRoadmap()->GetGraph();
  for(auto& cfg : _samples) {
    if(ValidateCfg(cfg)) {
      VID addedNode = g->AddVertex(cfg);
      _vids.push_back(addedNode);
    }
  }
}


template<class MPTraits>
void
PathStrategy<MPTraits>::
ConnectPath(const vector<VID>& _vids) {
  /// If a connection fails, an intermediate configuration is created and pushed
  /// to the medial axis. Assumes that the VID's are in path order.
  LPOutput<MPTraits> lpOutput;
  GraphType* g = this->GetRoadmap()->GetGraph();

  //iterate through the path configurations and try to connect them
  for(typename vector<VID>::const_iterator vit = _vids.begin();
      vit != _vids.end() - 1; ++vit) {
    const VID& v1 = *vit;
    const VID v2 = *(vit + 1);
    const CfgType& c1 = g->GetVertex(v1);
    const CfgType& c2 = g->GetVertex(v2);

    //if cfgs are connectable, add edge to map
    auto lp = this->GetLocalPlanner(m_lpLabel);
    auto env = this->GetEnvironment();
    if(lp->IsConnected(c1, c2, &lpOutput, env->GetPositionRes(),
        env->GetOrientationRes()))
      g->AddEdge(v1, v2, lpOutput.m_edge);
    //if connection fails, generate midpoint, validate it, and recurse
    else {
      CfgType cI = (c1 + c2) / 2.;

      //if cI can be validated, add it to the path and recurse on (c1, cI, c2)
      if(ValidateCfg(cI)) {
        vector<VID> recurseList(3);
        recurseList[0] = v1;
        recurseList[1] = g->AddVertex(cI);
        recurseList[2] = v2;
        ConnectPath(recurseList);
      }
    }
  }
}

#endif
