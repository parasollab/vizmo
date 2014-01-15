#ifndef REGIONSTRATEGY_H_
#define REGIONSTRATEGY_H_

#include "MPStrategies/MPStrategyMethod.h"
#include "Models/RegionModel.h"

class Vizmo;

template<class Vizmo>
class RegionStrategy : public MPStrategyMethod<typename Vizmo::MPTraits> {
  public:
    typedef typename Vizmo::MPTraits::MPProblemType MPProblemType;
    typedef typename Vizmo::MPTraits::CfgType CfgType;
    typedef typename MPProblemType::RoadmapType::VID VID;

    RegionStrategy(Vizmo* _vizmo = NULL);
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
    Vizmo* m_vizmo;
    vector<shared_ptr<Boundary> > m_boundaries;
};

template<class Vizmo>
RegionStrategy<Vizmo>::RegionStrategy(Vizmo* _vizmo) : m_vizmo(_vizmo) {
  this->SetName("RegionStrategy");
}

template<class Vizmo>
RegionStrategy<Vizmo>::RegionStrategy(MPProblemType* _problem, XMLNodeReader& _node) {
  cerr << "Error. This constructor should not be called." << endl;
  exit(1);
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::Initialize() {
  cout << "Initializing Region Strategy." << endl;

  //parse regions into PMPL boundaries
  const vector<RegionModel*>& regions = m_vizmo->GetEnv()->GetRegions();
  for(vector<RegionModel*>::const_iterator RIT = regions.begin();
      RIT != regions.end(); RIT++) {
    m_boundaries.push_back((*RIT)->GetBoundary());
    cout << *m_boundaries.back() << endl;
  }
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::Run() {
  cout << "Running Region Strategy." << endl;

  //set up map evaluator. Currently, NodesEval is used unless a query file is
  //loaded.
  vector<string> evalLabel;
  if(m_vizmo->IsQueryLoaded())
    evalLabel.push_back("Query");
  else
    evalLabel.push_back("NodesEval");

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
  }
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::Finalize() {
  cout << "Finalizing Region Strategy." << endl;

  ofstream ofs("test.map");
  this->GetMPProblem()->GetRoadmap()->Write(ofs, this->GetMPProblem()->GetEnvironment());
}

template<class Vizmo>
int
RegionStrategy<Vizmo>::SelectRegion() {
  return rand() % m_boundaries.size();
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::SampleRegion(int _index, vector<CfgType>& _samples) {
  typename MPProblemType::SamplerPointer sp = this->GetMPProblem()->GetSampler("uniform");

  sp->Sample(this->GetMPProblem()->GetEnvironment(), m_boundaries[_index],
      *this->GetMPProblem()->GetStatClass(), 1, 100, back_inserter(_samples));
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids) {
  _vids.clear();
  for(typename vector<CfgType>::iterator cit = _samples.begin();
      cit != _samples.end(); cit++) {
    _vids.push_back(this->GetMPProblem()->GetRoadmap()->GetGraph()->AddVertex(*cit));
  }
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::Connect(vector<VID>& _vids) {
  stapl::sequential::
      vector_property_map<typename MPProblemType::GraphType::GRAPH, size_t> cMap;
  typename MPProblemType::ConnectorPointer cp =
      this->GetMPProblem()->GetConnector("Neighborhood Connector");
  cp->Connect(this->GetMPProblem()->GetRoadmap(),
      *(this->GetMPProblem()->GetStatClass()), cMap, _vids.begin(), _vids.end());
}

#endif
#ifndef REGIONSTRATEGY_H_
#define REGIONSTRATEGY_H_

#include "MPStrategies/MPStrategyMethod.h"
#include "Models/RegionModel.h"

class Vizmo;

template<class Vizmo>
class RegionStrategy : public MPStrategyMethod<typename Vizmo::MPTraits> {
  public:
    typedef typename Vizmo::MPTraits::MPProblemType MPProblemType;
    typedef typename Vizmo::MPTraits::CfgType CfgType;
    typedef typename MPProblemType::RoadmapType::VID VID;

    RegionStrategy(Vizmo* _vizmo = NULL);
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
    Vizmo* m_vizmo;
    vector<shared_ptr<Boundary> > m_boundaries;
};

template<class Vizmo>
RegionStrategy<Vizmo>::RegionStrategy(Vizmo* _vizmo) : m_vizmo(_vizmo) {
  this->SetName("RegionStrategy");
}

template<class Vizmo>
RegionStrategy<Vizmo>::RegionStrategy(MPProblemType* _problem, XMLNodeReader& _node) {
  cerr << "Error. This constructor should not be called." << endl;
  exit(1);
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::Initialize() {
  cout << "Initializing Region Strategy." << endl;

  //parse regions into PMPL boundaries
  const vector<RegionModel*>& regions = m_vizmo->GetEnv()->GetRegions();
  for(vector<RegionModel*>::const_iterator RIT = regions.begin();
      RIT != regions.end(); RIT++) {
    m_boundaries.push_back((*RIT)->GetBoundary());
    cout << *m_boundaries.back() << endl;
  }
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::Run() {
  cout << "Running Region Strategy." << endl;

  //set up map evaluator. Currently, NodesEval is used unless a query file is
  //loaded.
  vector<string> evalLabel;
  if(m_vizmo->IsQueryLoaded())
    evalLabel.push_back("Query");
  else
    evalLabel.push_back("NodesEval");

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
  }
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::Finalize() {
  cout << "Finalizing Region Strategy." << endl;

  ofstream ofs("test.map");
  this->GetMPProblem()->GetRoadmap()->Write(ofs, this->GetMPProblem()->GetEnvironment());
}

template<class Vizmo>
int
RegionStrategy<Vizmo>::SelectRegion() {
  return rand() % m_boundaries.size();
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::SampleRegion(int _index, vector<CfgType>& _samples) {
  typename MPProblemType::SamplerPointer sp = this->GetMPProblem()->GetSampler("uniform");

  sp->Sample(this->GetMPProblem()->GetEnvironment(), m_boundaries[_index],
      *this->GetMPProblem()->GetStatClass(), 1, 100, back_inserter(_samples));
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids) {
  _vids.clear();
  for(typename vector<CfgType>::iterator cit = _samples.begin();
      cit != _samples.end(); cit++) {
    _vids.push_back(this->GetMPProblem()->GetRoadmap()->GetGraph()->AddVertex(*cit));
  }
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::Connect(vector<VID>& _vids) {
  stapl::sequential::
      vector_property_map<typename MPProblemType::GraphType::GRAPH, size_t> cMap;
  typename MPProblemType::ConnectorPointer cp =
      this->GetMPProblem()->GetConnector("Neighborhood Connector");
  cp->Connect(this->GetMPProblem()->GetRoadmap(),
      *(this->GetMPProblem()->GetStatClass()), cMap, _vids.begin(), _vids.end());
}

#endif
#ifndef REGIONSTRATEGY_H_
#define REGIONSTRATEGY_H_

#include "MPStrategies/MPStrategyMethod.h"
#include "Models/RegionModel.h"

class Vizmo;

template<class Vizmo>
class RegionStrategy : public MPStrategyMethod<typename Vizmo::MPTraits> {
  public:
    typedef typename Vizmo::MPTraits::MPProblemType MPProblemType;
    typedef typename Vizmo::MPTraits::CfgType CfgType;
    typedef typename MPProblemType::RoadmapType::VID VID;

    RegionStrategy(Vizmo* _vizmo = NULL);
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
    Vizmo* m_vizmo;
    vector<shared_ptr<Boundary> > m_boundaries;
};

template<class Vizmo>
RegionStrategy<Vizmo>::RegionStrategy(Vizmo* _vizmo) : m_vizmo(_vizmo) {
  this->SetName("RegionStrategy");
}

template<class Vizmo>
RegionStrategy<Vizmo>::RegionStrategy(MPProblemType* _problem, XMLNodeReader& _node) {
  cerr << "Error. This constructor should not be called." << endl;
  exit(1);
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::Initialize() {
  cout << "Initializing Region Strategy." << endl;

  //parse regions into PMPL boundaries
  const vector<RegionModel*>& regions = m_vizmo->GetEnv()->GetRegions();
  for(vector<RegionModel*>::const_iterator RIT = regions.begin();
      RIT != regions.end(); RIT++) {
    m_boundaries.push_back((*RIT)->GetBoundary());
    cout << *m_boundaries.back() << endl;
  }
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::Run() {
  cout << "Running Region Strategy." << endl;

  //set up map evaluator. Currently, NodesEval is used unless a query file is
  //loaded.
  vector<string> evalLabel;
  if(m_vizmo->IsQueryLoaded())
    evalLabel.push_back("Query");
  else
    evalLabel.push_back("NodesEval");

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
  }
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::Finalize() {
  cout << "Finalizing Region Strategy." << endl;

  ofstream ofs("test.map");
  this->GetMPProblem()->GetRoadmap()->Write(ofs, this->GetMPProblem()->GetEnvironment());
}

template<class Vizmo>
int
RegionStrategy<Vizmo>::SelectRegion() {
  return rand() % m_boundaries.size();
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::SampleRegion(int _index, vector<CfgType>& _samples) {
  typename MPProblemType::SamplerPointer sp = this->GetMPProblem()->GetSampler("uniform");

  sp->Sample(this->GetMPProblem()->GetEnvironment(), m_boundaries[_index],
      *this->GetMPProblem()->GetStatClass(), 1, 100, back_inserter(_samples));
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids) {
  _vids.clear();
  for(typename vector<CfgType>::iterator cit = _samples.begin();
      cit != _samples.end(); cit++) {
    _vids.push_back(this->GetMPProblem()->GetRoadmap()->GetGraph()->AddVertex(*cit));
  }
}

template<class Vizmo>
void
RegionStrategy<Vizmo>::Connect(vector<VID>& _vids) {
  stapl::sequential::
      vector_property_map<typename MPProblemType::GraphType::GRAPH, size_t> cMap;
  typename MPProblemType::ConnectorPointer cp =
      this->GetMPProblem()->GetConnector("Neighborhood Connector");
  cp->Connect(this->GetMPProblem()->GetRoadmap(),
      *(this->GetMPProblem()->GetStatClass()), cMap, _vids.begin(), _vids.end());
}

#endif
