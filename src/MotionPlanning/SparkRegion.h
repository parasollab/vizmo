#ifndef SPARKREGION_H_
#define SPARKREGION_H_

#include "MPStrategies/MPStrategyMethod.h"
#include "MPStrategies/SparkPRM.h"

#include "Models/RegionModel.h"
#include "Models/RegionSphereModel.h"
#include "Models/RegionSphere2DModel.h"
#include "Models/Vizmo.h"

#include "RegionStrategy.h"

template<class MPTraits>
class SparkRegion : public RegionStrategy<MPTraits> {
  public:
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPProblemType::RoadmapType::VID VID;
    typedef typename MPProblemType::GraphType GraphType;

    SparkRegion();
    SparkRegion(MPProblemType* _problem, XMLNodeReader& _node);

    void Initialize();
    void Run();
    void Finalize();

  protected:
    //helper functions
    bool ConstructRRT(vector<VID>& _vids);
    void DeleteRegion(size_t _index);
};

template<class MPTraits>
SparkRegion<MPTraits>::
SparkRegion() {
  this->SetName("SparkRegion");
}

template<class MPTraits>
SparkRegion<MPTraits>::
SparkRegion(MPProblemType* _problem, XMLNodeReader& _node) : RegionStrategy<MPTraits>(_problem, _node) {
  this->SetName("SparkRegion");
}

template<class MPTraits>
void
SparkRegion<MPTraits>::
Initialize() {
  cout << "Initializing Spark Region Strategy." << endl;
  if(GetVizmo().IsQueryLoaded())

    string basename = this->GetBaseFilename();

  this->SetupTools();

  //Make non-region objects non-selectable
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
  GetVizmo().GetRobot()->SetSelectable(false);
}

template<class MPTraits>
void
SparkRegion<MPTraits>::
Run() {
  cout << "Running Spark Region Strategy." << endl;

  //start clock
  GetVizmo().StartClock("SparkRegion");
  this->GetMPProblem()->GetStatClass()->StartClock("SparkRegionMP");

  size_t iter = 0;
  while(!this->EvaluateMap()) {

    vector<CfgType> samples;
    size_t index = this->SelectRegion();
    this->SampleRegion(index, samples);

    this->ProcessAvoidRegions();

    vector<VID> vids;
    this->AddToRoadmap(samples, vids);

    this->Connect(vids, iter);

    if(samples.size() && index != GetVizmo().GetEnv()->GetAttractRegions().size()) {
      ConstructRRT(vids);
      DeleteRegion(index);
      GetVizmo().GetMap()->RefreshMap();
    }

    if(++iter % 20 == 0)
      GetVizmo().GetMap()->RefreshMap();
    usleep(10000);
  }

  //stop clock
  GetVizmo().StopClock("SparkRegion");
  this->GetMPProblem()->GetStatClass()->StopClock("SparkRegionMP");
}

template<class MPTraits>
void
SparkRegion<MPTraits>::
Finalize() {
  cout << "Finalizing Spark Region Strategy." << endl;
  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //base filename
  string basename = this->GetBaseFilename();

  //print clocks + output a stat class
  StatClass* stats = this->GetMPProblem()->GetStatClass();
  GetVizmo().PrintClock("Pre-regions", cout);
  GetVizmo().PrintClock("SparkRegion", cout);
  //stats->PrintClock("Pre-regions", cout);
  stats->PrintClock("SparkRegionMP", cout);

  ofstream ostats((basename + ".stats").c_str());
  ostats << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(ostats, this->GetMPProblem()->GetRoadmap());
  GetVizmo().PrintClock("Pre-regions", ostats);
  GetVizmo().PrintClock("SparkRegion", ostats);
  //stats->PrintClock("Pre-regions", ostats);
  stats->PrintClock("SparkRegionMP", ostats);

  //output roadmap
  ofstream ofs((basename + ".map").c_str());
  this->GetMPProblem()->GetRoadmap()->Write(ofs, this->GetMPProblem()->GetEnvironment());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
  GetVizmo().GetRobot()->SetSelectable(true);
}

template<class MPTraits>
bool
SparkRegion<MPTraits>::
ConstructRRT(vector<VID>& _vids) {
  typedef typename vector<VID>::iterator VIT;
  for(VIT vit = _vids.begin(); vit != _vids.end(); ++vit) {
    this->CheckNarrowPassageSample(*vit);
  }
  return false;
}

template<class MPTraits>
void
SparkRegion<MPTraits>::
DeleteRegion(size_t _index) {
  const vector<RegionModel*>& regions = GetVizmo().GetEnv()->GetAttractRegions();
  GetVizmo().GetSelectedModels().clear();
  GetMainWindow()->GetGLWidget()->SetCurrentRegion(NULL);
  GetVizmo().GetEnv()->DeleteRegion(regions[_index-1]);
}

#endif
