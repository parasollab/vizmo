#ifndef PATH_STRATEGY_H_
#define PATH_STRATEGY_H_

#include "MPStrategies/MPStrategyMethod.h"

#include "Models/UserPathModel.h"
#include "Models/Vizmo.h"

template<class MPTraits>
class PathStrategy : public MPStrategyMethod<MPTraits> {
  public:
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPProblemType::RoadmapType::VID VID;

    PathStrategy();
    PathStrategy(MPProblemType* _problem, XMLNodeReader& _node);

    void Initialize();
    void Run();
    void Finalize();

  protected:
    //helper functions
    void AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids);
    void Connect(vector<VID>& _vids, size_t _i);

  private:
    UserPathModel* m_samplingPath;
};

template<class MPTraits>
PathStrategy<MPTraits>::
PathStrategy() {
  this->SetName("PathStrategy");
}

template<class MPTraits>
PathStrategy<MPTraits>::
PathStrategy(MPProblemType* _problem, XMLNodeReader& _node) {
  cerr << "Error. This constructor should not be called yet." << endl;
  exit(1);
}

template<class MPTraits>
void
PathStrategy<MPTraits>::
Initialize() {
  cout << "Initializing Path Strategy." << endl;

  //set base filename
  ostringstream oss;
  oss << GetVizmo().GetEnv()->GetModelDataDir() << "/PathStrategy." << GetVizmo().GetSeed();
  string basename = oss.str();
  this->SetBaseFilename(basename);

  //Make non-user objects non-selectable while PathStrategy is running
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
  GetVizmo().GetRobot()->SetSelectable(false);

  //start clocks
  GetVizmo().StartClock("PathStrategy");
  this->GetMPProblem()->GetStatClass()->StartClock("PathStrategyMP");
}

template<class MPTraits>
void
PathStrategy<MPTraits>::
Run() {
  cout << "Running Path Strategy." << endl;
}

template<class MPTraits>
void
PathStrategy<MPTraits>::
Finalize() {
  //stop clocks
  GetVizmo().StopClock("PathStrategy");
  this->GetMPProblem()->GetStatClass()->StopClock("PathStrategyMP");

  cout << "Finalizing Path Strategy." << endl;

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();

  //base filename
  string basename = this->GetBaseFilename();

  //print clocks + output a stat class
  StatClass* stats = this->GetMPProblem()->GetStatClass();
  GetVizmo().PrintClock("PathStrategy", cout);
  stats->PrintClock("PathStrategyMP", cout);

  ofstream ostats((basename + ".stats").c_str());
  ostats << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(ostats, this->GetMPProblem()->GetRoadmap());
  GetVizmo().PrintClock("PathStrategy", ostats);
  stats->PrintClock("PathStrategyMP", ostats);

  //output roadmap
  ofstream ofs((basename + ".map").c_str());
  this->GetMPProblem()->GetRoadmap()->Write(ofs, this->GetMPProblem()->GetEnvironment());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
  GetVizmo().GetRobot()->SetSelectable(true);
}

template<class MPTraits>
void
PathStrategy<MPTraits>::AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids) {
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
PathStrategy<MPTraits>::Connect(vector<VID>& _vids, size_t _i) {
  QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
  stapl::sequential::
      vector_property_map<typename MPProblemType::GraphType::GRAPH, size_t> cMap;
  typename MPProblemType::ConnectorPointer cp =
      this->GetMPProblem()->GetConnector("Neighborhood Connector");
  cp->Connect(this->GetMPProblem()->GetRoadmap(),
      *(this->GetMPProblem()->GetStatClass()), cMap, _vids.begin(), _vids.end());
}

#endif
