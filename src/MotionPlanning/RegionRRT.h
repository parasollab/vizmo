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
    typedef typename MPProblemType::VID VID;
    typedef typename MPProblemType::GraphType GraphType;
    typedef typename MPProblemType::LocalPlannerPointer LocalPlannerPointer;
    typedef typename MPProblemType::DistanceMetricPointer DistanceMetricPointer;

    RegionRRT();
    RegionRRT(MPProblemType* _problem, XMLNodeReader& _node);

    void Run();
    CfgType SelectDirection();

  protected:
    string m_connectionLabel;

  private:
    RegionModel* m_samplingRegion;
    string m_dmLabel;
    string m_lpLabel;
};

template<class MPTraits>
RegionRRT<MPTraits>::
RegionRRT() {
  this->SetName("RegionRRT");
}

template<class MPTraits>
RegionRRT<MPTraits>::
RegionRRT(MPProblemType* _problem, XMLNodeReader& _node) : BasicRRTStrategy<MPTraits>(_problem, _node) {
  this->SetName("RegionRRT");
  m_connectionLabel = _node.stringXMLParameter("connectionLabel", true, "Closest", "Connection Strategy");
  m_dmLabel = _node.stringXMLParameter("dmLabel", true, "", "Distance Metric");
  m_lpLabel = _node.stringXMLParameter("lpLabel", true, "", "Local Planner Ponter");
}

template<class MPTraits>
void
RegionRRT<MPTraits>::
Run() {
  if(this->m_debug) cout << "\nRunning BasicRRTStrategy::" << endl;

  // Setup MP Variables
  StatClass* stats = this->GetMPProblem()->GetStatClass();

  stats->StartClock("RRT Generation");

  CfgType dir;
  bool mapPassedEvaluation = false;
  while(!mapPassedEvaluation){
    //find my growth direction. Default is too randomly select node or bias towards a goal
    double randomRatio = DRand();
    if(randomRatio<this -> m_growthFocus){
      dir = this -> GoalBiasedDirection();
    }
    else{
      dir = this->SelectDirection();
    }

    // Randomize Current Tree
    this -> m_currentTree = this -> m_trees.begin() + LRand()% this -> m_trees.size();
    //cout << "m_trees.size() = " << m_trees.size() << ", currentTree = " << m_currentTree - m_trees.begin() << endl;

    VID recent = this->ExpandTree(dir);
    if(recent != INVALID_VID){
      //connect various trees together
      this -> ConnectTrees(recent);
      //see if tree is connected to goals
      if(this -> m_evaluateGoal)
        this -> EvaluateGoals();

      //evaluate the roadmap
      bool evalMap = this->EvaluateMap(this -> m_evaluators);
      if(!this -> m_growGoals){
        mapPassedEvaluation = this -> m_trees.size()==1 && evalMap && ((this->m_evaluateGoal && this -> m_goalsNotFound.size()==0) || !this -> m_evaluateGoal);
        if(this->m_debug && this -> m_goalsNotFound.size()==0)
          cout << "RRT FOUND ALL GOALS" << endl;
      }
      else
        mapPassedEvaluation = evalMap && this -> m_trees.size()==1;
    }
    else
      mapPassedEvaluation = false;
    GetVizmo().GetMap()->RefreshMap();
    usleep(10000);
  }

  stats->StopClock("RRT Generation");
  if(this->m_debug) {
    stats->PrintClock("RRT Generation", cout);
    cout<<"\nEnd Running BasicRRTStrategy::" << endl;
  }
}

template<class MPTraits>
typename MPTraits::CfgType
RegionRRT<MPTraits>::
SelectDirection() {

  shared_ptr<Boundary> samplingBoundary;
  const vector<RegionModel*>& regions = GetVizmo().GetEnv()->GetAttractRegions();
  Environment* env = this -> GetMPProblem()->GetEnvironment();

  size_t _index = rand() % (regions.size() + 1);
  //size_t _index = rand() % (regions.size());
  vector<CfgType> col;

  if(_index == regions.size()) {
    m_samplingRegion = NULL;
    samplingBoundary = this->GetMPProblem()->GetEnvironment()->GetBoundary();
  }
  else {
    m_samplingRegion = regions[_index];
    samplingBoundary = m_samplingRegion->GetBoundary();
  }

  try {
    //vector<CfgType> mySample;
    CfgType mySample;
    mySample.GetRandomCfg(env,samplingBoundary);  
    //if this region is not the environment boundary, update failure count
    if(m_samplingRegion != NULL) {
      m_samplingRegion->IncreaseFACount(col.size());
    }
    return mySample;
  }
  //catch Boundary too small exception
  catch(PMPLException _e) {
    CfgType mySample;
    mySample.GetRandomCfg(env,samplingBoundary);  
    return mySample;
  }
  //catch all others and exit
  catch(exception _e) {
    cerr << _e.what() << endl;
    exit(1);
  }
}
#endif
