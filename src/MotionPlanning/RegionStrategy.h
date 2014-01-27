#ifndef REGIONSTRATEGY_H_
#define REGIONSTRATEGY_H_

#include "MPStrategies/MPStrategyMethod.h"

#include "Models/RegionModel.h"
#include "Models/RegionSphereModel.h"
#include "Models/Vizmo.h"

template<class MPTraits>
class RegionStrategy : public MPStrategyMethod<MPTraits> {
  public:
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;
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
    void RejectSamples(vector<CfgType>& _samples);
    void AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids);
    void Connect(vector<VID>& _vids, size_t _i);
    void RecommendRegions(vector<VID>& _vids, size_t _i);
    void UpdateRegionStats();
    void UpdateRegionColor(size_t _i);
    bool EvaluateMap();

  private:
    RegionModel* m_samplingRegion;
    vector<VID> m_toDel;
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

  //base filename
  ostringstream oss;
  oss << GetVizmo().GetEnv()->GetModelDataDir() << "/RegionStrategy." << GetVizmo().GetSeed();
  string basename = oss.str();
  this->SetBaseFilename(basename);

  if(GetVizmo().IsQueryLoaded())
    boost::static_pointer_cast<Query<MPTraits> >(this->GetMPProblem()->GetMapEvaluator("Query"))->SetPathFile(basename + ".path");

  //Make non-region objects non-selectable
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
  GetVizmo().GetRobot()->SetSelectable(false);
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::Run() {
  cout << "Running Region Strategy." << endl;

  //start clock
  GetVizmo().StartClock("RegionStrategy");
  this->GetMPProblem()->GetStatClass()->StartClock("RegionStrategyMP");

  size_t iter = 0;
  while(!EvaluateMap()) {
    //pick a region
    size_t index = SelectRegion();

    //sample region
    vector<CfgType> samples;
    SampleRegion(index, samples);

    //reject samples
    RejectSamples(samples);

    //add sample to map
    vector<VID> vids;
    AddToRoadmap(samples, vids);

    //connect roadmap
    Connect(vids, iter);

    if(iter > 200) {
      //recommend regions based upon vids
      RecommendRegions(vids, iter);
    }

    if(++iter % 20 == 0) {
      //handle deletion of vertices
      {
        QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
        typedef typename MPProblemType::RoadmapType::GraphType GraphType;
        GraphType* g = this->GetMPProblem()->GetRoadmap()->GetGraph();
        typedef typename vector<VID>::iterator VIT;
        for(VIT vit = m_toDel.begin(); vit != m_toDel.end(); ++vit)
          g->delete_vertex(*vit);
        m_toDel.clear();
      }
      //recreate map model
      GetVizmo().GetMap()->RefreshMap();
    }
    usleep(10000);
  }

  //stop clock
  GetVizmo().StopClock("RegionStrategy");
  this->GetMPProblem()->GetStatClass()->StopClock("RegionStrategyMP");
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::Finalize() {
  cout << "Finalizing Region Strategy." << endl;

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();

  //base filename
  string basename = this->GetBaseFilename();

  //print clocks + output a stat class
  StatClass* stats = this->GetMPProblem()->GetStatClass();
  GetVizmo().PrintClock("Pre-regions", cout);
  GetVizmo().PrintClock("RegionStrategy", cout);
  //stats->PrintClock("Pre-regions", cout);
  stats->PrintClock("RegionStrategyMP", cout);

  ofstream ostats((basename + ".stats").c_str());
  ostats << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(ostats, this->GetMPProblem()->GetRoadmap());
  GetVizmo().PrintClock("Pre-regions", ostats);
  GetVizmo().PrintClock("RegionStrategy", ostats);
  //stats->PrintClock("Pre-regions", ostats);
  stats->PrintClock("RegionStrategyMP", ostats);

  //output roadmap
  ofstream ofs((basename + ".map").c_str());
  this->GetMPProblem()->GetRoadmap()->Write(ofs, this->GetMPProblem()->GetEnvironment());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
  GetVizmo().GetRobot()->SetSelectable(true);
}

template<class MPTraits>
size_t
RegionStrategy<MPTraits>::SelectRegion() {
  //get regions from vizmo
  const vector<RegionModel*>& regions = GetVizmo().GetEnv()->GetAttractRegions();

  //randomly choose a region
  return rand() % (regions.size() + 1);
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::SampleRegion(size_t _index, vector<CfgType>& _samples) {
  //setup access pointers
  shared_ptr<Boundary> samplingBoundary;
  const vector<RegionModel*>& regions = GetVizmo().GetEnv()->GetAttractRegions();
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

  //attempt to sample the region. track failures in col for density calculation.
  try {
    //track failures in col for density calculation
    vector<CfgType> col;
    sp->Sample(this->GetMPProblem()->GetEnvironment(), samplingBoundary,
        *this->GetMPProblem()->GetStatClass(), 1, 10,
        back_inserter(_samples), back_inserter(col));

    //if this region is not the environment boundary, update failure count
    if(m_samplingRegion != NULL)
      m_samplingRegion->IncreaseFACount(col.size());
  }
  //catch Boundary too small exception
  catch(PMPLException _e) {
    //static size_t numE = 0;
    //cout << _e.what() << endl << numE++ << endl;
    return;
  }
  //catch all others and exit
  catch(exception _e) {
    cerr << _e.what() << endl;
    exit(1);
  }
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::RejectSamples(vector<CfgType>& _samples) {
  Environment* env = this->GetMPProblem()->GetEnvironment();

  //first test to make sure all regions are processed
  typedef vector<RegionModel*>::const_iterator RIT;
  const vector<RegionModel*>& avoidRegions = GetVizmo().GetEnv()->GetAvoidRegions();
  for(RIT rit = avoidRegions.begin(); rit != avoidRegions.end(); ++rit) {
    if(!(*rit)->IsProcessed()) {
      //loop over the graph testing vertices for deletion
      typedef typename MPProblemType::RoadmapType::GraphType GraphType;
      typedef typename GraphType::VI VI;
      GraphType* g = this->GetMPProblem()->GetRoadmap()->GetGraph();
      for(VI vi = g->begin(); vi != g->end(); ++vi)
        if(env->InBounds(vi->property(), (*rit)->GetBoundary()))
          m_toDel.push_back(vi->descriptor());
      (*rit)->Processed();
    }
  }

  //check is any samples lies within any avoid region
  //if so, erase Cfg from _samples vector
  typedef typename vector<CfgType>::iterator CIT;
  for(CIT cit = _samples.begin(); cit != _samples.end();) {
    bool erase = false;
    for(RIT rit = avoidRegions.begin(); rit != avoidRegions.end(); ++rit) {
      if(env->InBounds(*cit, (*rit)->GetBoundary())) {
        erase = true;
        break;
      }
    }
    if(erase)
      cit = _samples.erase(cit);
    else
      ++cit;
  }
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids) {
  //lock map data
  QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());

  //add nodes in _samples to graph. store VID's in _vids for connecting
  _vids.clear();
  for(typename vector<CfgType>::iterator cit = _samples.begin();
      cit != _samples.end(); cit++) {
    VID addedNode = this->GetMPProblem()->GetRoadmap()->GetGraph()->AddVertex(*cit);
    _vids.push_back(addedNode);
  }
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::Connect(vector<VID>& _vids, size_t _i) {
  QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
  stapl::sequential::
      vector_property_map<typename MPProblemType::GraphType::GRAPH, size_t> cMap;
  typename MPProblemType::ConnectorPointer cp =
      this->GetMPProblem()->GetConnector("Neighborhood Connector");
  cp->Connect(this->GetMPProblem()->GetRoadmap(),
      *(this->GetMPProblem()->GetStatClass()), cMap, _vids.begin(), _vids.end());

  UpdateRegionStats();
  UpdateRegionColor(_i);
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::RecommendRegions(vector<VID>& _vids, size_t _i) {
  if(!m_samplingRegion) {
    typedef typename MPProblemType::RoadmapType::GraphType GraphType;
    GraphType* g = this->GetMPProblem()->GetRoadmap()->GetGraph();
    typedef typename vector<VID>::iterator VIT;
    for(VIT vit = _vids.begin(); vit != _vids.end(); ++vit) {
      if(g->get_degree(*vit) < 1) {
        //node is not connected to anything
        //recommend a region
        RegionSphereModel* r = new RegionSphereModel(
            g->find_vertex(*vit)->property().GetPoint(),
            this->GetMPProblem()->GetEnvironment()->GetPositionRes() * 100,
            false);
        r->SetCreationIter(_i);
        GetVizmo().GetEnv()->AddNonCommitRegion(r);
      }
    }
  }
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::UpdateRegionStats() {
  if(m_samplingRegion) {
    //set up access pointers
    Environment* ep = this->GetMPProblem()->GetEnvironment();
    typename MPProblemType::GraphType* g = this->GetMPProblem()->GetRoadmap()->GetGraph();
    shared_ptr<Boundary> bbx = m_samplingRegion->GetBoundary();

    //clear m_numVertices
    m_samplingRegion->ClearNodeCount();

    //iterate through graph to find which vertices are in the modified region
    //typedef CCModel<CfgType, WeightType>* CCM;
    //typedef set<CCM> SetCCs;
    //SetCCs ccs;
    for(typename MPProblemType::GraphType::iterator git = g->begin(); git != g->end(); git++) {
      const CfgType& c = g->GetVertex(git);
      if(ep->InBounds(c, bbx)) {
        m_samplingRegion->IncreaseNodeCount(1);
        //ccs.insert(c.GetCC());
      }
    }

    //typename SetCCs::iterator cit = find(ccs.begin(), ccs.end(), (CCM)NULL);
    //if(cit != ccs.end())
    //  ccs.erase(cit);

    //m_samplingRegion->SetCCCount(ccs.size());
  }
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::UpdateRegionColor(size_t _i) {
  if(m_samplingRegion) {
    //update region color based on node density
    double densityRatio = exp(-sqr(m_samplingRegion->NodeDensity()));
    //double densityRatio = 1 - exp(-sqr(m_samplingRegion->CCDensity()));
    m_samplingRegion->SetColor(Color4(1 - densityRatio, densityRatio, 0., 0.5));
  }
  typedef vector<RegionModel*>::const_iterator RIT;
  vector<RegionModel*> nonCommit = GetVizmo().GetEnv()->GetNonCommitRegions();
  for(RIT rit = nonCommit.begin(); rit != nonCommit.end(); ++rit) {
    if((*rit)->GetCreationIter() != size_t(-1)) {
      double iterCount = 1000 - _i + (*rit)->GetCreationIter();
      if(iterCount <= 0)
        GetVizmo().GetEnv()->DeleteRegion(*rit);
      else
        (*rit)->SetColor(Color4(0, 0, 1, iterCount/1250.));
    }
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

  QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
  bool eval = MPStrategyMethod<MPTraits>::EvaluateMap(evalLabel);
  return eval;
}

#endif
