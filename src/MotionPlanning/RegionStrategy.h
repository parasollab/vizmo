#ifndef REGIONSTRATEGY_H_
#define REGIONSTRATEGY_H_

#include "MPStrategies/MPStrategyMethod.h"

#include "Models/RegionModel.h"
#include "Models/Vizmo.h"

template<class MPTraits>
class RegionStrategy : public MPStrategyMethod<MPTraits> {
  public:
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPProblemType::RoadmapType::VID VID;

    RegionStrategy();
    RegionStrategy(MPProblemType* _problem, XMLNodeReader& _node);

    void Initialize();
    void Run();
    void Finalize();

  protected:
    //helper functions
    int SelectRegion();
    void SampleRegion(int _index, vector<CfgType>& _samples);
    void AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids);
    void Connect(vector<VID>& _vids);

  private:
    vector<shared_ptr<Boundary> > m_boundaries;
};

template<class MPTraits>
RegionStrategy<MPTraits>::RegionStrategy() {
  this->SetName("RegionStrategy");
}

template<class MPTraits>
RegionStrategy<MPTraits>::RegionStrategy(MPProblemType* _problem, XMLNodeReader& _node) {
  cerr << "Error. This constructor should not be called." << endl;
  exit(1);
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::Initialize() {
  cout << "Initializing Region Strategy." << endl;

  //parse regions into PMPL boundaries
  const vector<RegionModel*>& regions = GetVizmo().GetEnv()->GetRegions();
  for(vector<RegionModel*>::const_iterator RIT = regions.begin();
      RIT != regions.end(); RIT++) {
    m_boundaries.push_back((*RIT)->GetBoundary());
    cout << *m_boundaries.back() << endl;
  }
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::Run() {
  cout << "Running Region Strategy." << endl;

  //set up map evaluator. Currently, NodesEval is used unless a query file is
  //loaded.
  vector<string> evalLabel;
  if(GetVizmo().IsQueryLoaded())
    evalLabel.push_back("Query");
  else
    evalLabel.push_back("NodesEval");

  size_t iter = 0;
  while(!this->EvaluateMap(evalLabel)) {
    //pick a region
    int regionIndex = SelectRegion();

    //sample region
    vector<CfgType> samples;
    SampleRegion(regionIndex, samples);

    //add sample to map
    vector<VID> vids;
    AddToRoadmap(samples, vids);

    //connect roadmap
    Connect(vids);

    if(++iter % 20 == 0) {
      //recreate map model
      GetVizmo().GetMap()->RefreshMap();
    }
  }
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::Finalize() {
  cout << "Finalizing Region Strategy." << endl;

  GetVizmo().GetMap()->RefreshMap();

  //ofstream ofs("test.map");
  //this->GetMPProblem()->GetRoadmap()->Write(ofs, this->GetMPProblem()->GetEnvironment());
}

template<class MPTraits>
int
RegionStrategy<MPTraits>::SelectRegion() {
  return rand() % m_boundaries.size();
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::SampleRegion(int _index, vector<CfgType>& _samples) {
  typename MPProblemType::SamplerPointer sp = this->GetMPProblem()->GetSampler("uniform");

  sp->Sample(this->GetMPProblem()->GetEnvironment(), m_boundaries[_index],
      *this->GetMPProblem()->GetStatClass(), 1, 100, back_inserter(_samples));
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids) {
  GetVizmo().GetMap()->AcquireLock();
  _vids.clear();
  for(typename vector<CfgType>::iterator cit = _samples.begin();
      cit != _samples.end(); cit++) {
    _vids.push_back(this->GetMPProblem()->GetRoadmap()->GetGraph()->AddVertex(*cit));
  }
  GetVizmo().GetMap()->ReleaseLock();
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::Connect(vector<VID>& _vids) {
  GetVizmo().GetMap()->AcquireLock();
  stapl::sequential::
      vector_property_map<typename MPProblemType::GraphType::GRAPH, size_t> cMap;
  typename MPProblemType::ConnectorPointer cp =
      this->GetMPProblem()->GetConnector("Neighborhood Connector");
  cp->Connect(this->GetMPProblem()->GetRoadmap(),
      *(this->GetMPProblem()->GetStatClass()), cMap, _vids.begin(), _vids.end());
  GetVizmo().GetMap()->ReleaseLock();
}

#endif
