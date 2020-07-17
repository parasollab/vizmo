#ifndef REGION_RRT_H_
#define REGION_RRT_H_

#include "MPStrategies/MPStrategyMethod.h"
#include "MPStrategies/BasicRRTStrategy.h"

#include "Models/RegionModel.h"
#include "Models/RegionSphereModel.h"
#include "Models/RegionSphere2DModel.h"
#include "Models/Vizmo.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief  RegionRRT is a user-guided RRT that draws some of its samples q_rand
///         from the user-created attract regions.
/// \bug    Avoid regions are not yet supported.
////////////////////////////////////////////////////////////////////////////////
template <typename MPTraits>
class RegionRRT : public BasicRRTStrategy<MPTraits> {

  public:

    ///\name Motion Planning Types
    ///@{

    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType       CfgType;
    typedef EnvModel::RegionModelPtr         RegionModelPtr;

    ///@}
    ///\name Construction
    ///@{

    RegionRRT(string _dm = "euclidean", string _nf = "BFNF",
        string _vc = "PQP_SOLID", string _nc = "kClosest",
        string _ex = "RegionBERO", vector<string> _evaluators = vector<string>(),
        string _gt = "UNDIRECTED_TREE", bool _growGoals = false,
        double _growthFocus = 0.05, size_t _numRoots = 1,
        size_t _numDirections = 1, size_t _maxTrial = 3);

    RegionRRT(MPProblemType* _problem, XMLNode& _node);

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
    /// \brief  Computes the growth direction for the RRT, choosing between the
    ///         entire environment and each attract region with uniform
    ///         probability to generate q_rand.
    /// \return The resulting growth direction.
    virtual CfgType SelectDirection() override;

    ///@}
    ///\name Internal State
    ///@{

    RegionModelPtr m_samplingRegion;  ///< Points to the current sampling region.

    ///@}
};

/*------------------------------ Construction --------------------------------*/

template <typename MPTraits>
RegionRRT<MPTraits>::
RegionRRT(string _dm, string _nf, string _vc, string _nc, string _ex,
    vector<string> _evaluators, string _gt, bool _growGoals, double _growthFocus,
    size_t _numRoots, size_t _numDirections, size_t _maxTrial) :
    BasicRRTStrategy<MPTraits>(_dm, _nf, _vc, _nc, _ex, _evaluators, _gt,
        _growGoals, _growthFocus, _numRoots, _numDirections, _maxTrial) {
  this->SetName("RegionRRT");
}


template <typename MPTraits>
RegionRRT<MPTraits>::
RegionRRT(MPProblemType* _problem, XMLNode& _node) :
    BasicRRTStrategy<MPTraits>(_problem, _node) {
  this->SetName("RegionRRT");
}

/*------------------------------ Construction --------------------------------*/

template <typename MPTraits>
void
RegionRRT<MPTraits>::
Initialize() {
  BasicRRTStrategy<MPTraits>::Initialize();

  //Make non-user objects non-selectable while PathStrategy is running
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
  GetVizmo().StartClock("RegionRRT");
}


template <typename MPTraits>
void
RegionRRT<MPTraits>::
Iterate() {
  BasicRRTStrategy<MPTraits>::Iterate();
  GetVizmo().ProcessAvoidRegions();
  //usleep(10000);
}


template <typename MPTraits>
void
RegionRRT<MPTraits>::
Finalize() {
  GetVizmo().StopClock("RegionRRT");
  GetVizmo().PrintClock("RegionRRT", cout);

  BasicRRTStrategy<MPTraits>::Finalize();

  // Append vizmo clock to stat file.
  ofstream statFile(this->GetBaseFilename() + ".stat", ios_base::app);
  GetVizmo().PrintClock("RegionRRT", statFile);
  statFile.close();

  // Redraw finished map.
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  // Show results pop-up.
  ostringstream results;
  results << "Planning Complete!" << endl;
  GetVizmo().PrintClock("RegionRRT", results);
  this->GetStatClass()->PrintClock(this->GetNameAndLabel() + "::Run()", results);
  GetMainWindow()->AlertUser(results.str());

  // Make things selectable again.
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
}

/*------------------------------ RRT Overrides -------------------------------*/

template <typename MPTraits>
typename RegionRRT<MPTraits>::CfgType
RegionRRT<MPTraits>::
SelectDirection() {
  shared_ptr<Boundary> samplingBoundary;
  const vector<RegionModelPtr>& regions = GetVizmo().GetEnv()->
      GetAttractRegions();
  Environment* env = this->GetEnvironment();

  size_t index = rand() % (regions.size() + 1);

  if(index == regions.size()) {
    m_samplingRegion.reset();
    samplingBoundary = this->GetEnvironment()->GetBoundary();
  }
  else {
    m_samplingRegion = regions[index];
    samplingBoundary = m_samplingRegion->GetBoundary();
  }

  try {
    CfgType mySample;
    mySample.GetRandomCfg(env, samplingBoundary);
    if(m_samplingRegion != NULL)
      m_samplingRegion->SetColor(Color4(0, 1, 0, 0.5));
    return mySample;
  }
  //catch Boundary too small exception
  catch(const PMPLException& _e) {
    m_samplingRegion->SetColor(Color4(1, 0, 0, 1));
    CfgType mySample;
    mySample.GetRandomCfg(env);
    return mySample;
  }
  //catch all others and exit
  catch(const std::exception& _e) {
    cerr << _e.what() << endl;
    exit(1);
  }
}

/*----------------------------------------------------------------------------*/

#endif
