#ifndef I_RRT_STRATEGY_H_
#define I_RRT_STRATEGY_H_

#include "MPStrategies/BasicRRTStrategy.h"

#include "GUI/GLWidget.h"
#include "GUI/MainWindow.h"
#include "GUI/ModelSelectionWidget.h"
#include "GUI/RobotAvatar.h"
#include "Models/Vizmo.h"

template<class MPTraits>
class IRRTStrategy : public BasicRRTStrategy<MPTraits> {
  public:
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPProblemType::RoadmapType RoadmapType;
    typedef typename MPProblemType::VID VID;
    typedef typename MPProblemType::ValidityCheckerPointer ValidityCheckerPointer;
    typedef typename MPProblemType::NeighborhoodFinderPointer NeighborhoodFinderPointer;

    //Non-XML constructor w/ Query (by label)
    IRRTStrategy(Query<MPTraits>* _q = NULL, string _lp = "sl",
        string _dm = "euclidean", string _nf = "BFNF", string _vc = "PQP_SOLID",
        string _nc = "kClosest", string _gt = "UNDIRECTED_TREE",
        string _extenderLabel = "BERO",
        vector<string> _evaluators = vector<string>(), double _delta = 10.0,
        double _minDist = 0.01, double _growthFocus = 0.05,
        bool _evaluateGoal = true, size_t _numRoots = 1,
        size_t _numDirections = 1, size_t _maxTrial = 3,
        bool _growGoals = false);

    //XML Constructor
    IRRTStrategy(MPProblemType* _problem, XMLNodeReader& _node);

    ~IRRTStrategy();

    virtual void Initialize();
    virtual void Run();
    virtual void Finalize();
    virtual void Print(ostream& _os) const;

  protected:
    // Helper functions
    virtual CfgType SelectDirection();
    virtual VID ExpandTree(CfgType& _dir);
    void ConnectTrees(VID _recentlyGrown);
    void EvaluateGoals();

    CfgType AvatarBiasedDirection();

    double m_alpha; //ratio of user force vs algo force on q_rand
    double m_sigma; //variance of Gaussian Distribution around result force
    double m_beta; //ratio of interactive sampling vs uniform sampling
    RobotAvatar* m_avatar;
};

template<class MPTraits>
IRRTStrategy<MPTraits>::
IRRTStrategy(Query<MPTraits>* _q, string _lp, string _dm,
    string _nf, string _vc, string _nc, string _gt, string _extenderLabel,
    vector<string> _evaluators, double _delta, double _minDist,
    double _growthFocus, bool _evaluateGoal, size_t _numRoots,
    size_t _numDirections, size_t _maxTrial, bool _growGoals) :
    BasicRRTStrategy<MPTraits>(_lp, _dm, _nf, _vc, _nc, _gt, _extenderLabel,
        _evaluators, _delta, _minDist, _growthFocus, _evaluateGoal,
        CfgType(), CfgType(),_numRoots, _numDirections, _maxTrial, _growGoals),
    m_alpha(0.5), m_sigma(0.5), m_beta(0.5), m_avatar(NULL) {
  this->SetName("IRRTStrategy");

  this->m_query = shared_ptr<Query<MPTraits> >(_q);

//#ifdef USE_HAPTICS
//  m_avatar = new RobotAvatar(&m_query->GetQuery()[0], RobotAvatar::Haptic);
//#else
//  m_avatar = new RobotAvatar(&m_query->GetQuery()[0], RobotAvatar::Mouse);
//#endif
}

template<class MPTraits>
IRRTStrategy<MPTraits>::
IRRTStrategy(MPProblemType* _problem, XMLNodeReader& _node) :
  BasicRRTStrategy<MPTraits>(_problem, _node, false, true), m_avatar(NULL) {
  this->SetName("IRRTStrategy");
  m_alpha = _node.numberXMLParameter("alpha", false, 0.5, 0.0, 1.0, "Alpha");
  m_sigma = _node.numberXMLParameter("sigma", false, 0.5, 0.0, 1.0, "Sigma");
  m_beta = _node.numberXMLParameter("beta", false, 0.5, 0.0, 1.0, "Beta");
  _node.warnUnrequestedAttributes();

//#ifdef USE_HAPTICS
//  m_avatar = new RobotAvatar(&m_query->GetQuery()[0], RobotAvatar::Haptic);
//#else
//  m_avatar = new RobotAvatar(&m_query->GetQuery()[0], RobotAvatar::Mouse);
//#endif
}

template<class MPTraits>
IRRTStrategy<MPTraits>::~IRRTStrategy() {
  delete m_avatar;
  m_avatar = NULL;
}

template<class MPTraits>
void
IRRTStrategy<MPTraits>::
Print(ostream& _os) const {
  BasicRRTStrategy<MPTraits>::Print(_os);
  _os << "\talpha: " << m_alpha << endl;
}

//////////////////////
//Initialization Phase
/////////////////////
template<class MPTraits>
void
IRRTStrategy<MPTraits>::
Initialize() {
  BasicRRTStrategy<MPTraits>::Initialize();

  //m_avatar->Connect();
  //m_avatar->SummonMouse();
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
    if(randomRatio < this->m_growthFocus)
      dir = this->GoalBiasedDirection();
    else
      dir = this->SelectDirection();

    // Randomize Current Tree
    this->m_currentTree = this->m_trees.begin() + LRand() % this->m_trees.size();

    VID recent = this->ExpandTree(dir);
    if(recent != INVALID_VID) {
      //connect various trees together
      ConnectTrees(recent);
      //see if tree is connected to goals
      if(this->m_evaluateGoal)
        EvaluateGoals();

      //evaluate the roadmap
      bool evalMap = this->EvaluateMap(this->m_evaluators);
      if(!this->m_growGoals) {
        mapPassedEvaluation = this->m_trees.size() == 1 && evalMap &&
            ((this->m_evaluateGoal && this->m_goalsNotFound.size() == 0) || !this->m_evaluateGoal);
        if(this->m_debug && this->m_goalsNotFound.size() == 0)
          cout << "RRT FOUND ALL GOALS" << endl;
      }
      else
        mapPassedEvaluation = evalMap && this->m_trees.size()==1;
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

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();

  BasicRRTStrategy<MPTraits>::Finalize();
}

template<class MPTraits>
typename IRRTStrategy<MPTraits>::CfgType
IRRTStrategy<MPTraits>::
SelectDirection() {
  double r = DRand();
  if(r <= m_beta)
    return AvatarBiasedDirection();
  else
    return BasicRRTStrategy<MPTraits>::SelectDirection();
}

template<class MPTraits>
typename IRRTStrategy<MPTraits>::VID
IRRTStrategy<MPTraits>::
ExpandTree(CfgType& _dir) {
  QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
  return BasicRRTStrategy<MPTraits>::ExpandTree(_dir);
}

template<class MPTraits>
void
IRRTStrategy<MPTraits>::
ConnectTrees(VID _recentlyGrown) {
  QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
  return BasicRRTStrategy<MPTraits>::ConnectTrees(_recentlyGrown);
}

template<class MPTraits>
void
IRRTStrategy<MPTraits>::
EvaluateGoals() {
  QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
  BasicRRTStrategy<MPTraits>::EvaluateGoals();
}

template<class MPTraits>
typename MPTraits::CfgType
IRRTStrategy<MPTraits>::
AvatarBiasedDirection() {
  Point3d mouseW = GetMainWindow()->GetGLWidget()->GetMouseW();
  static CfgType oldPos;
  CfgType newPos;
  newPos[0] = mouseW[0]; newPos[1] = mouseW[1];
  //get avatar data
  /*CfgType newPos = m_avatar->GetCurrCfg();
  CfgType oldPos = m_avatar->GetPrevCfg();
  */

  RoadmapType* rdmp = this->GetMPProblem()->GetRoadmap();

  //find the nearest k neighbors.
  NeighborhoodFinderPointer nf = this->GetMPProblem()->GetNeighborhoodFinder("BFNF");
  vector<pair<VID, double> > kClosest;

  nf->FindNeighbors(rdmp, newPos, back_inserter(kClosest));

  //now get the average of the nearest k neighbors
  CfgType barycenter;
  {
    QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
    for(typename vector<pair<VID, double> >::iterator it = kClosest.begin();
        it != kClosest.end(); ++it)
      barycenter += rdmp->GetGraph()->GetVertex(it->first);
  }
  barycenter /= kClosest.size();
  //now find the pseudoforce vector from the avatar to the average
  CfgType forceAlgo = barycenter - newPos;

  //get the device pseudoforce
  CfgType forceDev = newPos - oldPos;
  oldPos = newPos;

  //get the user pseudoforce
  ValidityCheckerPointer vc = this->GetMPProblem()->GetValidityChecker(this->m_vc);
  bool uVal = vc->IsValid(newPos, this->GetNameAndLabel() + "::AvatarBiasedDirection()");
  CfgType forceUser = uVal ? forceDev : CfgType();

  //m_avatar->UpdatePos();

  //scale dirs by alpha
  forceUser *= m_alpha;
  forceAlgo *= (1. - m_alpha);
  CfgType forceResult = forceUser + forceAlgo;

  double stdDev = 0.5;
  for(size_t i = 0; i < CfgType::DOF(); ++i)
    forceResult[i] = GaussianDistribution(forceResult[i], stdDev);

  return newPos + forceResult;
}

#endif
