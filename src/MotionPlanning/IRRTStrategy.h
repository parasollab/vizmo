#ifndef I_RRT_STRATEGY_H_
#define I_RRT_STRATEGY_H_

#include "MPStrategies/BasicRRTStrategy.h"

#include "GUI/GLWidget.h"
#include "GUI/MainWindow.h"
#include "GUI/ModelSelectionWidget.h"

#include "Models/AvatarModel.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief   Implements the user-guided IRRT (interactive RRT).
/// \details This algorithm is described in Taix, Flavigne, and Ferre, "Human
///          Interaction with Motion Planning Algorithm", Journal of Intelligent
///          Robot Systems 2012, vol 67 pg 285-306. It is essentially a
///          user-guided RRT where the biasing direction is influenced by the
///          continuous input of a user.
////////////////////////////////////////////////////////////////////////////////
template<class MPTraits>
class IRRTStrategy : public BasicRRTStrategy<MPTraits> {

  public:

    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPProblemType::RoadmapType RoadmapType;
    typedef typename MPProblemType::VID VID;
    typedef typename MPProblemType::ValidityCheckerPointer ValidityCheckerPointer;
    typedef typename MPProblemType::NeighborhoodFinderPointer
        NeighborhoodFinderPointer;

    // Constructors
    IRRTStrategy(const CfgType& _start = CfgType(),
        const CfgType& _goal = CfgType(),
        string _dm = "euclidean", string _nf = "BFNF", string _vc = "PQP_SOLID",
        string _nc = "kClosest", string _gt = "UNDIRECTED_TREE",
        string _extenderLabel = "BERO",
        vector<string> _evaluators = vector<string>(),
        double _minDist = 0.01, double _growthFocus = 0.05,
        bool _evaluateGoal = true, size_t _numRoots = 1,
        size_t _numDirections = 1, size_t _maxTrial = 3,
        bool _growGoals = false);
    IRRTStrategy(MPProblemType* _problem, XMLNode& _node);

    // Inherited functions
    virtual void Initialize();
    virtual void Run();
    virtual void Finalize();
    virtual void Print(ostream& _os) const;

  protected:

    // Helper functions
    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Chooses between user bias and standard RRT bias with a ratio of
    ///         \ref m_beta.
    /// \return The selected biasing direction.
    CfgType SelectDirection();
    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Computes the user-influenced biasing direction.
    /// \return The user-influenced biasing direction.
    CfgType AvatarBiasedDirection();

    double m_alpha; ///< Ratio of user force vs algo force on q_rand.
    double m_sigma; ///< Variance of Gaussian Distribution around result force.
    double m_beta;  ///< Ratio of interactive sampling vs uniform sampling.
};


template<class MPTraits>
IRRTStrategy<MPTraits>::
IRRTStrategy(const CfgType& _start, const CfgType& _goal, string _dm,
    string _nf, string _vc, string _nc, string _gt, string _extenderLabel,
    vector<string> _evaluators, double _minDist,
    double _growthFocus, bool _evaluateGoal, size_t _numRoots,
    size_t _numDirections, size_t _maxTrial, bool _growGoals) :
    BasicRRTStrategy<MPTraits>(_dm, _nf, _vc, _nc, _gt, _extenderLabel,
        _evaluators, _minDist, _growthFocus, _evaluateGoal,
        _start, _goal, _numRoots, _numDirections, _maxTrial, _growGoals),
    m_alpha(0.5), m_sigma(0.5), m_beta(0.5) {
  this->SetName("IRRTStrategy");
}


template<class MPTraits>
IRRTStrategy<MPTraits>::
IRRTStrategy(MPProblemType* _problem, XMLNode& _node) :
  BasicRRTStrategy<MPTraits>(_problem, _node, true) {
  this->SetName("IRRTStrategy");
  m_alpha = _node.Read("alpha", false, 0.5, 0.0, 1.0, "Alpha");
  m_sigma = _node.Read("sigma", false, 0.5, 0.0, 1.0, "Sigma");
  m_beta = _node.Read("beta", false, 0.5, 0.0, 1.0, "Beta");
}


template<class MPTraits>
void
IRRTStrategy<MPTraits>::
Print(ostream& _os) const {
  BasicRRTStrategy<MPTraits>::Print(_os);
  _os << "\talpha: " << m_alpha << endl;
}


template<class MPTraits>
void
IRRTStrategy<MPTraits>::
Initialize() {
  BasicRRTStrategy<MPTraits>::Initialize();
  vector<double> data = this->m_query->GetQuery()[0].GetPosition();
  Point3d pos;
  copy(data.begin(), data.end(), pos.begin());
  AvatarModel* avatar = GetVizmo().GetEnv()->GetAvatar();
  avatar->UpdatePosition(pos);
  avatar->SetInputType(AvatarModel::Mouse);
  avatar->Enable();
  GetMainWindow()->GetGLWidget()->SetMousePos(pos);

  //Make non-user objects non-selectable while PathStrategy is running
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
}


template<class MPTraits>
void
IRRTStrategy<MPTraits>::
Run() {
  if(this->m_debug)
    cout << "\nRunning IRRTStrategy::" << endl;

  // Setup MP Variables
  StatClass* stats = this->GetStatClass();

  GetVizmo().StartClock("IRRT Strategy");
  stats->StartClock("RRT Generation MP");

  CfgType dir;
  bool mapPassedEvaluation = false;
  size_t iter = 0;
  while(!mapPassedEvaluation) {
    //find my growth direction. Default is to randomly select
    //node or bias towards a goal
    double randomRatio = DRand();
    if(randomRatio < this->m_growthFocus)
      dir = this->GoalBiasedDirection();
    else
      dir = this->SelectDirection();

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
      bool evalMap = this->EvaluateMap();
      if(!this->m_growGoals) {
        mapPassedEvaluation = this->m_trees.size() == 1 && evalMap &&
            ((this->m_evaluateGoal && this->m_goalsNotFound.size() == 0) ||
             !this->m_evaluateGoal);
        if(this->m_debug && this->m_goalsNotFound.size() == 0)
          cout << "RRT FOUND ALL GOALS" << endl;
      }
      else
        mapPassedEvaluation = evalMap && this->m_trees.size()==1;
    }
    else
      mapPassedEvaluation = false;

    if(++iter % 20 == 0)
      GetVizmo().GetMap()->RefreshMap();
    usleep(10000);
  }

  GetVizmo().StopClock("IRRT Strategy");
  stats->StopClock("RRT Generation MP");
  if(this->m_debug)
    cout << "\nEnd Running IRRTStrategy::" << endl;
}


template<class MPTraits>
void
IRRTStrategy<MPTraits>::
Finalize() {
  //setup variables
  StatClass* stats = this->GetStatClass();
  string basename = this->GetBaseFilename();

  //print info to console
  cout << "\nFinalizing BasicRRTStrategy::" << endl;
  GetVizmo().PrintClock("IRRT Strategy", cout);
  stats->PrintClock("RRT Generation MP", cout);

  //disable avatar
  GetVizmo().GetEnv()->GetAvatar()->Disable();

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
  GetVizmo().PrintClock("IRRT Strategy", osStat);
  stats->PrintClock("RRT Generation MP", osStat);
  osStat.close();

  //show results pop-up
  ostringstream results;
  results << "Planning Complete!" << endl;
  GetVizmo().PrintClock("IRRT Strategy", results);
  stats->PrintClock("RRT Generation MP", results);
  GetMainWindow()->AlertUser(results.str());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);

  if(this->m_debug)
    cout << "\nEnd Finalizing BasicRRTStrategy" << endl;
}


template<class MPTraits>
typename IRRTStrategy<MPTraits>::CfgType
IRRTStrategy<MPTraits>::
SelectDirection() {
  double r = DRand();
  if(r <= m_beta) {
    CfgType dir = AvatarBiasedDirection();
    if(dir != this->GetRoadmap()->GetGraph()->
        GetVertex(*this->m_currentTree->begin()))
      return AvatarBiasedDirection();
  }
  return BasicRRTStrategy<MPTraits>::SelectDirection();
}


template<class MPTraits>
typename MPTraits::CfgType
IRRTStrategy<MPTraits>::
AvatarBiasedDirection() {
  static CfgType oldPos;
  CfgType newPos = *GetVizmo().GetEnv()->GetAvatar();

  RoadmapType* rdmp = this->GetRoadmap();

  //find the nearest k neighbors.
  NeighborhoodFinderPointer nf = this->GetNeighborhoodFinder("BFNF");
  vector<pair<VID, double> > kClosest;
  nf->FindNeighbors(rdmp, newPos, back_inserter(kClosest));

  //now get the average of the nearest k neighbors
  CfgType barycenter;
  for(auto& neighbor : kClosest)
    barycenter += rdmp->GetGraph()->GetVertex(neighbor.first);
  barycenter /= kClosest.size();

  //now find the pseudoforce vector from the avatar to the average
  CfgType forceAlgo = barycenter - newPos;

  //get the device pseudoforce
  CfgType forceDev = newPos - oldPos;
  oldPos = newPos;

  //get the user pseudoforce
  ValidityCheckerPointer vc = this->GetValidityChecker(this->m_vc);
  bool uVal = vc->IsValid(newPos,
      this->GetNameAndLabel() + "::AvatarBiasedDirection()");
  CfgType forceUser = uVal ? forceDev : CfgType();

  //m_avatar->UpdatePos();

  //scale dirs by alpha
  forceUser *= m_alpha;
  forceAlgo *= (1. - m_alpha);
  CfgType forceResult = forceUser + forceAlgo;

  double stdDev = 0.5;
  for(size_t i = 0; i < CfgType().DOF(); ++i)
    forceResult[i] = GaussianDistribution(forceResult[i], stdDev);

  return newPos + forceResult;
}

#endif
