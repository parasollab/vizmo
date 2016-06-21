#ifndef KINODYNAMIC_REGION_RRT_H_
#define KINODYNAMIC_REGION_RRT_H_

#include "MPStrategies/KinodynamicRRTStrategy.h"

#include "GUI/GLWidget.h"
#include "GUI/ModelSelectionWidget.h"
#include "Models/RegionModel.h"
#include "Models/RegionSphereModel.h"
#include "Models/RegionSphere2DModel.h"
#include "Models/Vizmo.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief  KinodynamicRegionRRT is a user-guided RRT that draws some of its samples q_rand
///         from the user-created attract regions.
/// \bug    Avoid regions are not yet supported.
////////////////////////////////////////////////////////////////////////////////
template<class MPTraits>
class KinodynamicRegionRRT : public KinodynamicRRTStrategy<MPTraits> {

  public:

    // Local Types
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType StateType;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPProblemType::VID VID;
    typedef typename MPProblemType::GraphType GraphType;
    typedef typename MPProblemType::LocalPlannerPointer LocalPlannerPointer;
    typedef typename MPProblemType::DistanceMetricPointer DistanceMetricPointer;
    typedef typename MPProblemType::NeighborhoodFinderPointer
        NeighborhoodFinderPointer;
    typedef EnvModel::RegionModelPtr RegionModelPtr;

    // Construction
    KinodynamicRegionRRT(string _dm = "", string _nf = "", string _vc= "",
        string _extenderLabel = "",
        vector<string> _evaluators = vector<string>(),
        double _goalDist = 10.0, double _minDist = 0.01,
        double _growthFocus = 0.05, bool _evaluateGoal = true,
        const StateType& _start=StateType(), const StateType& _goal=StateType());

    KinodynamicRegionRRT(MPProblemType* _problem, XMLNode& _node);

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
    StateType SelectDirection();

    RegionModelPtr m_samplingRegion;  ///< Points to the current sampling region.
    CfgType m_qNew;                   ///< Storage for checking avoid regions.
};


template<class MPTraits>
KinodynamicRegionRRT<MPTraits>::
KinodynamicRegionRRT(string _dm, string _nf, string _vc, string _extenderLabel,
        vector<string> _evaluators, double _goalDist, double _minDist,
        double _growthFocus, bool _evaluateGoal, const StateType& _start,
        const StateType& _goal) :
  KinodynamicRRTStrategy<MPTraits>(_dm, _nf, _vc, _extenderLabel, _evaluators, _goalDist,
      _minDist, _growthFocus, _evaluateGoal, _start, _goal) {
    this->SetName("KinodynamicRegionRRT");
  }


template<class MPTraits>
KinodynamicRegionRRT<MPTraits>::
KinodynamicRegionRRT(MPProblemType* _problem, XMLNode& _node) :
    KinodynamicRRTStrategy<MPTraits>(_problem, _node) {
  this->SetName("KinodynamicRegionRRT");
}


template<class MPTraits>
void
KinodynamicRegionRRT<MPTraits>::
Initialize() {
  KinodynamicRRTStrategy<MPTraits>::Initialize();

  //Make non-user objects non-selectable while PathStrategy is running
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);

}


template<class MPTraits>
void
KinodynamicRegionRRT<MPTraits>::
Run() {
  // Setup MP Variables
  StatClass* stats = this->GetStatClass();

  GetVizmo().StartClock("KinodynamicRegionRRT");
  stats->StartClock("RRT Generation");

  bool mapPassedEvaluation = false;
  while(!mapPassedEvaluation) {
    //find my growth direction. Default is too randomly select node or bias
    //towards a goal
    StateType dir;
    double randomRatio = DRand();
    if(randomRatio < this->m_growthFocus)
      dir = this->GoalBiasedDirection();
    else
      dir = this->SelectDirection();

    VID recent = this->ExpandTree(dir);
    if(recent != INVALID_VID) {
      m_qNew = this->GetRoadmap()->GetGraph()->GetVertex(recent);

      //see if tree is connected to goals
      if(this->m_evaluateGoal)
        this->EvaluateGoals(recent);

      //evaluate the roadmap
      bool evalMap = this->EvaluateMap();
      mapPassedEvaluation = evalMap &&
        ((this->m_evaluateGoal && this->m_goalsNotFound.empty()) || !this->m_evaluateGoal);
      if(this->m_debug && this->m_goalsNotFound.empty())
        cout << "RRT FOUND ALL GOALS" << endl;

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
    }

    usleep(10000);
  }

  GetVizmo().StopClock("KinodynamicRegionRRT");
  stats->StopClock("RRT Generation");
}


template<class MPTraits>
void
KinodynamicRegionRRT<MPTraits>::
Finalize() {
  //setup variables
  StatClass* stats = this->GetStatClass();
  string basename = this->GetBaseFilename();

  //print info to console
  GetVizmo().PrintClock("KinodynamicRegionRRT", cout);
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
  GetVizmo().PrintClock("KinodynamicRegionRRT", osStat);
  stats->PrintClock("RRT Generation MP", osStat);
  osStat.close();

  //show results pop-up
  ostringstream results;
  results << "Planning Complete!" << endl;
  GetVizmo().PrintClock("KinodynamicRegionRRT", results);
  stats->PrintClock("RRT Generation MP", results);
  GetMainWindow()->AlertUser(results.str());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
}

template<class MPTraits>
typename KinodynamicRegionRRT<MPTraits>::StateType
KinodynamicRegionRRT<MPTraits>::
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
    StateType mySample;
    mySample.GetRandomCfg(env, samplingBoundary);
    if(m_samplingRegion != NULL)
      m_samplingRegion->SetColor(Color4(0, 1, 0, 0.5));
    return mySample;
  }
  //catch Boundary too small exception
  catch(PMPLException _e) {
    m_samplingRegion->SetColor(Color4(1, 0, 0, 1));
    StateType mySample;
    mySample.GetRandomCfg(env);
    return mySample;
  }
  //catch all others and exit
  catch(exception _e) {
    cerr << _e.what() << endl;
    exit(1);
  }
}

#endif
