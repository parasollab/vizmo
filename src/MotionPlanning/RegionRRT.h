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
    virtual void SelectDirection(size_t _index, vector<CfgType>& _samples);

  protected:
    string m_samplerLabel;
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
  m_samplerLabel = _node.stringXMLParameter("samplerLabel", true, "UniformRandomFreePQP", "Sampler Strategy");
  m_connectionLabel = _node.stringXMLParameter("connectionLabel", true, "Closest", "Connection Strategy");
  m_dmLabel = _node.stringXMLParameter("dmLabel", true, "", "Distance Metric");
  m_lpLabel = _node.stringXMLParameter("lpLabel", true, "", "Local Planner Ponter");
}

template<class MPTraits>
void
RegionRRT<MPTraits>::
SelectDirection(size_t _index, vector<CfgType>& _samples) {

  shared_ptr<Boundary> samplingBoundary;
  const vector<RegionModel*>& regions = GetVizmo().GetEnv()->GetAttractRegions();
  typename MPProblemType::SamplerPointer sp = this->GetMPProblem()->GetSampler(m_samplerLabel);

  if(_index == regions.size()) {
    m_samplingRegion = NULL;
    samplingBoundary = this->GetMPProblem()->GetEnvironment()->GetBoundary();
  }
  else {
    m_samplingRegion = regions[_index];
    samplingBoundary = m_samplingRegion->GetBoundary();
  }

  //attempt to sample the region. track failures in col for density calculation.
  try {
    //track failures in col for density calculation
    vector<CfgType> col;
    sp->Sample(this->GetMPProblem()->GetEnvironment(), samplingBoundary,
        *this->GetMPProblem()->GetStatClass(), 1, 10,
        back_inserter(_samples), back_inserter(col));
cout << "_Samples.size(): " << _samples.size() << endl << flush;
    //if this region is not the environment boundary, update failure count
    if(m_samplingRegion != NULL)
      m_samplingRegion->IncreaseFACount(col.size());
      return;
  }
  //catch Boundary too small exception
  //catch(PMPLException _e) {
    //static size_t numE = 0;
    //cout << _e.what() << endl << numE++ << endl;
    //return;
  //}
  //catch all others and exit
  catch(exception _e) {
    cerr << _e.what() << endl;
    exit(1);
  }
}

template<class MPTraits>
void
RegionRRT<MPTraits>::
Run() {
  LocalPlannerPointer lp = this->GetMPProblem()->GetLocalPlanner(m_lpLabel);
  DistanceMetricPointer dm = this->GetMPProblem()->GetDistanceMetric(m_dmLabel);
  GraphType* g = this->GetMPProblem()->GetRoadmap()->GetGraph();
  Environment* env = this->GetMPProblem()->GetEnvironment();

  if(this->m_debug) cout << "\nRunning RegionRRT::" << endl;

  // Setup MP Variables
  StatClass* stats = this->GetMPProblem()->GetStatClass();

  stats->StartClock("RRT Generation");

  CfgType dir;
  bool mapPassedEvaluation = false;
  while(!mapPassedEvaluation) {

    vector<CfgType> samples;
    const vector<RegionModel*>& regions = GetVizmo().GetEnv()->GetAttractRegions();
    int index = rand() % (regions.size() + 1);
    SelectDirection(index, samples);
    cout << "Got 123" << endl << flush;
    // Next, Grow the RRT toward the generated CfgType in samples
    size_t treeIndex = rand() % (*this->m_currentTree).size();

    int maxDist = 5;

    CfgType startNode = g->GetVertex((*this->m_currentTree)[treeIndex]); //TODO

    CfgType regionCfg;
    if(samples.size() > 0) {
      cout << "It took the first path." << endl << flush;
      regionCfg = (*samples.begin());
    }
    else
      regionCfg = startNode;
    CfgType displacement;
    cout << "Got 132!!!" << endl << flush;
    displacement.GetRandomRay(maxDist, this->GetMPProblem()->GetEnvironment(), this->GetMPProblem()->GetDistanceMetric(m_dmLabel));

    CfgType offset = regionCfg - (displacement + startNode);
    offset = offset/offset.Magnitude();
    displacement += offset;

    CfgType newNode = startNode + displacement;

    LPOutput<MPTraits> lpOutput;
    CfgType collision;
    if(lp->IsConnected(newNode, startNode, collision, &lpOutput,
          env->GetPositionRes(), env->GetOrientationRes()))
    {
      VID nNew = g->AddVertex(newNode);
      g->AddEdge(g->GetVID(startNode), nNew, lpOutput.m_edge);
      (*this->m_currentTree).push_back(nNew);
      // After doing this, if the current region(index) is not the environment
      // Check if the new Cfg is in that region. If it is, delete the region.

    }
    // Finally, update roadmap like region steering
    GetVizmo().GetMap()->RefreshMap();
  }

  // Randomize Current Tree
  this->m_currentTree = this->m_trees.begin() + LRand()% this->m_trees.size();
  //cout << "m_trees.size() = " << m_trees.size() << ", currentTree = " << m_currentTree - m_trees.begin() << endl;

  VID recent = this-> ExpandTree(dir);
  if(recent != INVALID_VID){
    //connect various trees together
    this->ConnectTrees(recent);
    //see if tree is connected to goals
    if(this->m_evaluateGoal)
      this->EvaluateGoals();

    //evaluate the roadmap
    bool evalMap = this->EvaluateMap(this->m_evaluators);
    if(!this->m_growGoals){
      mapPassedEvaluation = this->m_trees.size()==1 && evalMap && ((this->m_evaluateGoal && this->m_goalsNotFound.size()==0) || !this->m_evaluateGoal);
      if(this->m_debug && this->m_goalsNotFound.size()==0)
        cout << "RRT FOUND ALL GOALS" << endl;
    }
    else
      mapPassedEvaluation = evalMap && this->m_trees.size()==1;
  }
  else
    mapPassedEvaluation = false;

  stats->StopClock("RRT Generation");
  if(this->m_debug) {
    stats->PrintClock("RRT Generation", cout);
    cout<<"\nEnd Running RegionRRT::" << endl;
  }
}

#endif
