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
template <typename MPTraits>
class IRRTStrategy : public BasicRRTStrategy<MPTraits> {

  public:

    ///\name Motion Planning Types
    ///@{

    typedef typename MPTraits::CfgType          CfgType;
    typedef typename MPTraits::WeightType       WeightType;
    typedef typename MPTraits::MPProblemType    MPProblemType;
    typedef typename MPProblemType::RoadmapType RoadmapType;
    typedef typename MPProblemType::VID         VID;

    ///@}
    ///\name Construction
    ///@{

    IRRTStrategy(string _dm = "euclidean", string _nf = "BFNF",
        string _vc = "PQP_SOLID", string _nc = "kClosest",
        string _ex = "BERO", vector<string> _evaluators = vector<string>(),
        string _gt = "UNDIRECTED_TREE", bool _growGoals = false,
        double _growthFocus = 0.05, size_t _numRoots = 1,
        size_t _numDirections = 1, size_t _maxTrial = 3);

    IRRTStrategy(MPProblemType* _problem, XMLNode& _node);

    ///@}
    ///\name MPBaseObject Overrides
    ///@{

    virtual void Print(ostream& _os) const override;

    ///@}
    ///\name MPStrategyMethod Overrides
    ///@{

    virtual void Initialize() override;
    virtual void Iterate() override;
    virtual void Finalize() override;

    ///@}

  protected:

    ///\name RRT Overrides
    ///@{

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Chooses between user bias and standard RRT bias with a ratio of
    ///         \ref m_beta.
    /// \return The selected biasing direction.
    virtual CfgType SelectDirection() override;

    ///@}
    ///\name IRRT Functions
    ///@{

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Computes the user-influenced biasing direction.
    /// \return The user-influenced biasing direction.
    CfgType AvatarBiasedDirection();

    ///@}
    ///\name Internal State
    ///@{

    double m_alpha; ///< Ratio of user force vs algo force on q_rand.
    double m_sigma; ///< Variance of Gaussian Distribution around result force.
    double m_beta;  ///< Ratio of interactive sampling vs uniform sampling.

    ///@}
};

/*------------------------------ Construction --------------------------------*/

template <typename MPTraits>
IRRTStrategy<MPTraits>::
IRRTStrategy(string _dm, string _nf, string _vc, string _nc, string _ex,
    vector<string> _evaluators, string _gt, bool _growGoals, double _growthFocus,
    size_t _numRoots, size_t _numDirections, size_t _maxTrial) :
    BasicRRTStrategy<MPTraits>(_dm, _nf, _vc, _nc, _ex, _evaluators, _gt,
        _growGoals, _growthFocus, _numRoots, _numDirections, _maxTrial),
    m_alpha(0.5), m_sigma(0.5), m_beta(0.5) {
  this->SetName("IRRTStrategy");
}


template <typename MPTraits>
IRRTStrategy<MPTraits>::
IRRTStrategy(MPProblemType* _problem, XMLNode& _node) :
    BasicRRTStrategy<MPTraits>(_problem, _node) {
  this->SetName("IRRTStrategy");
  m_alpha = _node.Read("alpha", false, 0.5, 0.0, 1.0, "Alpha");
  m_sigma = _node.Read("sigma", false, 0.5, 0.0, 1.0, "Sigma");
  m_beta = _node.Read("beta", false, 0.5, 0.0, 1.0, "Beta");
}

/*------------------------- MPBaseObject Overrides ---------------------------*/

template <typename MPTraits>
void
IRRTStrategy<MPTraits>::
Print(ostream& _os) const {
  BasicRRTStrategy<MPTraits>::Print(_os);
  _os << "\talpha: " << m_alpha << endl
      << "\tsigma: " << m_sigma << endl
      << "\tbeta:  " << m_beta  << endl;
}

/*----------------------- MPStrategyMethod Overrides -------------------------*/

template <typename MPTraits>
void
IRRTStrategy<MPTraits>::
Initialize() {
  BasicRRTStrategy<MPTraits>::Initialize();

  // Configure the avatar.
  vector<double> data = this->m_query->GetQuery()[0].GetPosition();
  Point3d pos;
  copy(data.begin(), data.end(), pos.begin());
  AvatarModel* avatar = GetVizmo().GetEnv()->GetAvatar();
  avatar->UpdatePosition(pos);
  avatar->SetInputType(AvatarModel::Mouse);
  avatar->Enable();
  GetMainWindow()->GetGLWidget()->SetMousePos(pos);

  // Make non-user objects non-selectable while PathStrategy is running
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
  GetVizmo().StartClock("IRRTStrategy");
}


template <typename MPTraits>
void
IRRTStrategy<MPTraits>::
Iterate() {
  BasicRRTStrategy<MPTraits>::Iterate();
}


template <typename MPTraits>
void
IRRTStrategy<MPTraits>::
Finalize() {
  GetVizmo().StopClock("IRRTStrategy");
  GetVizmo().PrintClock("IRRTStrategy", cout);

  BasicRRTStrategy<MPTraits>::Finalize();

  // Append vizmo clock to stat file.
  ofstream statFile(this->GetBaseFilename() + ".stat", ios_base::app);
  GetVizmo().PrintClock("IRRTStrategy", statFile);
  statFile.close();

  // Disable avatar.
  GetVizmo().GetEnv()->GetAvatar()->Disable();

  // Redraw finished map.
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  // Show results pop-up.
  ostringstream results;
  results << "Planning Complete!" << endl;
  GetVizmo().PrintClock("IRRTStrategy", results);
  this->GetStatClass()->PrintClock(this->GetNameAndLabel() + "::Run()", results);
  GetMainWindow()->AlertUser(results.str());

  // Make things selectable again.
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
}

/*----------------------------- RRT Overrides --------------------------------*/

template <typename MPTraits>
typename IRRTStrategy<MPTraits>::CfgType
IRRTStrategy<MPTraits>::
SelectDirection() {
  if(DRand() <= m_beta) {
    CfgType dir = AvatarBiasedDirection();
    if(dir != this->GetRoadmap()->GetGraph()->GetVertex(
          *this->m_currentTree->begin()))
      return AvatarBiasedDirection();
  }
  return BasicRRTStrategy<MPTraits>::SelectDirection();
}

/*---------------------------- IRRT Functions --------------------------------*/

template <typename MPTraits>
typename MPTraits::CfgType
IRRTStrategy<MPTraits>::
AvatarBiasedDirection() {
  static CfgType oldPos;
  CfgType newPos = *GetVizmo().GetEnv()->GetAvatar();

  RoadmapType* rdmp = this->GetRoadmap();

  //find the nearest k neighbors.
  auto nf = this->GetNeighborhoodFinder("BFNF");
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
  auto vc = this->GetValidityChecker(this->m_vcLabel);
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

/*----------------------------------------------------------------------------*/

#endif
