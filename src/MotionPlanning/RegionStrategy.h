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
    size_t SelectRegion();
    void SampleRegion(size_t _index, vector<CfgType>& _samples);
    void AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids);
    void Connect(vector<VID>& _vids);
    void UpdateRegionNodeCount();
    void UpdateRegionColor();
    bool EvaluateMap();

  private:
    RegionModel* m_samplingRegion;
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
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::Run() {
  cout << "Running Region Strategy." << endl;

  size_t iter = 0;
  while(!EvaluateMap()) {
    //pick a region
    size_t index = SelectRegion();

    //sample region
    vector<CfgType> samples;
    SampleRegion(index, samples);

    //add sample to map
    vector<VID> vids;
    AddToRoadmap(samples, vids);

    //connect roadmap
    Connect(vids);

    if(++iter % 20 == 0) {
      //recreate map model
      GetVizmo().GetMap()->RefreshMap();
    }
    usleep(10000);
  }
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::Finalize() {
  cout << "Finalizing Region Strategy." << endl;

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();

  //ofstream ofs("test.map");
  //this->GetMPProblem()->GetRoadmap()->Write(ofs, this->GetMPProblem()->GetEnvironment());
}

template<class MPTraits>
size_t
RegionStrategy<MPTraits>::SelectRegion() {
  //get regions from vizmo
  const vector<RegionModel*>& regions = GetVizmo().GetEnv()->GetRegions();

  //randomly choose a region
  return rand() % (regions.size() + 1);
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::SampleRegion(size_t _index, vector<CfgType>& _samples) {
  //setup access pointers
  shared_ptr<Boundary> samplingBoundary;
  const vector<RegionModel*>& regions = GetVizmo().GetEnv()->GetRegions();
  typename MPProblemType::SamplerPointer sp = this->GetMPProblem()->GetSampler("uniform");

  //check if the selected region is a region or the environment boundary.  if it
  //is the env boundary, set m_samplingRegion to null
  if(_index == regions.size()) {
    m_samplingRegion = NULL;
    samplingBoundary = this->GetMPProblem()->GetEnvironment()->GetBoundary();
  }
  else {
    m_samplingRegion = regions[_index];
    samplingBoundary = m_samplingRegion->GetBoundary();
  }

  //sample the region. track failures in col for density calculation.
  vector<CfgType> col;
  sp->Sample(this->GetMPProblem()->GetEnvironment(), samplingBoundary,
      *this->GetMPProblem()->GetStatClass(), 1, 100,
      back_inserter(_samples), back_inserter(col));

  //if this region is not the environment boundary, update failure count
  if(m_samplingRegion != NULL)
    m_samplingRegion->IncreaseFACount(col.size());
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids) {
  //lock map data
  GetVizmo().GetMap()->AcquireLock();

  //add nodes in _samples to graph. store VID's in _vids for connecting
  _vids.clear();
  for(typename vector<CfgType>::iterator cit = _samples.begin();
      cit != _samples.end(); cit++) {
    VID addedNode = this->GetMPProblem()->GetRoadmap()->GetGraph()->AddVertex(*cit);
    _vids.push_back(addedNode);
  }

  //if this region is not the environment boundary, update count/color
  if(m_samplingRegion != NULL) {
    UpdateRegionNodeCount();
    UpdateRegionColor();
  }

  //release map lock
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

template<class MPTraits>
void
RegionStrategy<MPTraits>::UpdateRegionNodeCount() {
  if(m_samplingRegion != NULL) {
    //set up access pointers
    Environment* ep = this->GetMPProblem()->GetEnvironment();
    typename MPProblemType::GraphType* g = this->GetMPProblem()->GetRoadmap()->GetGraph();
    shared_ptr<Boundary> bbx = m_samplingRegion->GetBoundary();

    //clear m_numVertices
    m_samplingRegion->ClearNodeCount();

    //iterate through graph to find which vertices are in the modified region
    for(typename MPProblemType::GraphType::iterator git = g->begin(); git != g->end(); git++) {
      if(ep->InBounds(g->GetVertex(git), bbx))
        m_samplingRegion->IncreaseNodeCount(1);
    }
  }
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::UpdateRegionColor() {
  if(m_samplingRegion != NULL) {
    //update region color based on node density
    double densityRatio = 1 - exp(-sqr(m_samplingRegion->Density()));
    m_samplingRegion->SetColor(Color4(densityRatio, 1 - densityRatio, 0., 1.));
  }
}

template<class MPTraits>
bool
RegionStrategy<MPTraits>::EvaluateMap() {
  //set up map evaluator. Currently, NodesEval is used unless a query file is
  //loaded.
  vector<string> evalLabel;
  if(GetVizmo().IsQueryLoaded())
    evalLabel.push_back("Query");
  else
    evalLabel.push_back("NodesEval");

  GetVizmo().GetMap()->AcquireLock();
  bool eval = MPStrategyMethod<MPTraits>::EvaluateMap(evalLabel);
  GetVizmo().GetMap()->ReleaseLock();
  return eval;
}

#endif
