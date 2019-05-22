#ifndef PATH_STRATEGY_H_
#define PATH_STRATEGY_H_

#include "MPStrategies/MPStrategyMethod.h"

#include "Models/EnvModel.h"
#include "Models/UserPathModel.h"
#include "Models/Vizmo.h"

#include "Utilities/MedialAxisUtilities.h"
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
template <typename MPTraits>
class PathStrategy : public MPStrategyMethod<MPTraits> {

  public:

    ///\name Motion Planning Types
    ///@{

    typedef typename MPTraits::MPProblemType     MPProblemType;
    typedef typename MPTraits::CfgType           CfgType;
    typedef typename MPTraits::WeightType        WeightType;
    typedef typename MPProblemType::RoadmapType  RoadmapType;
    typedef typename RoadmapType::VID            VID;
    typedef typename RoadmapType::GraphType      GraphType;

    ///@}
    ///\name Construction
    ///@{

    PathStrategy();
    PathStrategy(MPProblemType* _problem, XMLNode& _node);

    ///@}
    ///\name MPBaseObject Overrides
    ///@{

    virtual void Print(ostream& _os) const override;

    ///@}
    ///\name MPStrategyMethod Overrides
    ///@{

    virtual void Initialize() override;
    virtual void Run() override;
    virtual void Finalize() override;

    ///@}

  protected:

    ///\name Helpers
    ///@{

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Check the configuration's validity. If invalid, push it to the
    ///        medial axis.
    /// \param[in/out] _cfg The configuration to validate.
    /// \return A bool indicating whether the configuration was successfully
    ///         validated.
    bool ValidateCfg(CfgType& _cfg);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Validate and add configurations to the roadmap. Store their VID's
    ///        for connection.
    /// \param[in] _samples Configurations to add.
    /// \param[out] _vids   The corresponding VID's in the roadmap.
    void AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Try to connect new nodes to the roadmap in the path sequence. If
    ///        a connection attempt fails, attempt to create intermediates.
    /// \param[in] _vids The VID's of the path nodes, in path order.
    void ConnectPath(const vector<VID>& _vids);

    ///@}

  private:

    ///\name PMPL Object Labels
    ///@{

    string m_vcLabel{"pqp_solid"};            ///< The ValidityChecker label.
    string m_dmLabel{"euclidean"};      ///< The DistanceMetric label.
    string m_lpLabel{"sl"};             ///< The LocalPlanner label.
    string m_ncLabel{"Closest"};        ///< The Connector label.

    ///@}
    ///\name Internal State
    ///@{

    vector<UserPathModel*> m_userPaths; ///< The set of all existing user paths.
    vector<VID>            m_endPoints; ///< The set of all path heads/tails.
    PRMQuery<MPTraits>*    m_query;     ///< The PMPL Query.
    MedialAxisUtility<MPTraits> m_medialAxisUtility; ///< For pushing to the MA.

    ///@}
};

/*------------------------------- Construction -------------------------------*/

template <typename MPTraits>
PathStrategy<MPTraits>::
PathStrategy() : MPStrategyMethod<MPTraits>() {
  this->SetName("PathStrategy");
}


template <typename MPTraits>
PathStrategy<MPTraits>::
PathStrategy(MPProblemType* _problem, XMLNode& _node) :
    MPStrategyMethod<MPTraits>(_problem, _node) {
  this->SetName("PathStrategy");
  m_vcLabel = _node.Read("vcLabel", false, m_vcLabel, "Validity Checker");
  m_dmLabel = _node.Read("dmLabel", false, m_dmLabel, "Distance Metric");
  m_lpLabel = _node.Read("lpLabel", false, m_lpLabel, "Local Planner");
  m_ncLabel = _node.Read("connectionLabel", false, "kClosest",
      "Connector");
}

/*--------------------------- MPBaseObject Overrides -------------------------*/

template <typename MPTraits>
void
PathStrategy<MPTraits>::
Print(ostream& _os) const {
  _os << this->GetNameAndLabel()
      << "\n\tValidity Checker: " << m_vcLabel
      << "\n\tDistance Metric:  " << m_dmLabel
      << "\n\tLocal Planner:    " << m_lpLabel
      << "\n\tConnector:        " << m_ncLabel
      << endl;
}

/*------------------------- MPStrategyMethod Overrides -----------------------*/

template <typename MPTraits>
void
PathStrategy<MPTraits>::
Initialize() {
  // This function is a no-op if no paths exist.
  m_userPaths = GetVizmo().GetEnv()->GetUserPaths();
  if(m_userPaths.empty())
    return;

  cout << "Initializing Path Strategy." << endl;

  // Create medial axis utility.
  m_medialAxisUtility = MedialAxisUtility<MPTraits>(this->GetMPProblem(),
      m_vcLabel, m_dmLabel, false, false);

  // If a query is requested, add the query cfgs to the map.
  m_query = nullptr;
  if(this->UsingQuery()) {
    m_query = &*(static_pointer_cast<PRMQuery<MPTraits> >(
        this->GetMapEvaluator("PRMQuery")));
    auto g = this->GetRoadmap()->GetGraph();
    for(const auto& q : m_query->GetQuery())
      m_endPoints.push_back(g->AddVertex(q));
  }

  // Make non-user objects non-selectable while PathStrategy is running.
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);

  //start clocks
  GetVizmo().StartClock("PathStrategy");
  this->GetStatClass()->StartClock("PathStrategyMP");
}


template <typename MPTraits>
void
PathStrategy<MPTraits>::
Run() {
  // This function is a no-op if no paths exist.
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
  auto connector = this->GetConnector(m_ncLabel);
  auto map = this->GetRoadmap();
  connector->Connect(map, m_endPoints.begin(), m_endPoints.end(),
      m_endPoints.begin(), m_endPoints.end(), false);
}


template <typename MPTraits>
void
PathStrategy<MPTraits>::
Finalize() {
  // If no paths exist, alert user that planner did not run.
  if(m_userPaths.empty()) {
    GetMainWindow()->AlertUser("Error: no user-path exists!");
    return;
  }

  cout << "Finalizing Path Strategy." << endl;

  auto stats = this->GetStatClass();
  GetVizmo().StopClock("PathStrategy");
  stats->StopClock("PathStrategyMP");

  // Redraw finished map.
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  // Print clocks to terminal.
  GetVizmo().PrintClock("PathStrategy", cout);
  stats->PrintClock("PathStrategyMP", cout);

  string basename = this->GetBaseFilename();

  // Output stats.
  ofstream ostats(basename + ".stat");
  ostats << "Sampling and Connection Stats:" << endl;
  auto map = this->GetRoadmap();
  stats->PrintAllStats(ostats, map);
  GetVizmo().PrintClock("PathStrategy", ostats);
  stats->PrintClock("PathStrategyMP", ostats);

  // Output roadmap.
  Environment* env = this->GetEnvironment();
  map->Write(basename + ".map", env);

  // Show results pop-up.
  ostringstream results;
  results << "Planning Complete!" << endl;
  GetVizmo().PrintClock("PathStrategy", results);
  GetMainWindow()->AlertUser(results.str());

  // Make things selectable again.
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
}

/*--------------------------------- Helpers ----------------------------------*/

template <typename MPTraits>
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


template <typename MPTraits>
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


template <typename MPTraits>
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

/*----------------------------------------------------------------------------*/

#endif
