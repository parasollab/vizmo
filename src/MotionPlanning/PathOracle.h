#ifndef PATH_ORACLE_H_
#define PATH_ORACLE_H_

#include "MPStrategies/MPStrategyMethod.h"

#include "GUI/MainWindow.h"
#include "GUI/ModelSelectionWidget.h"

#include "Models/Vizmo.h"

#include "MotionPlanning/VizmoTraits.h"

template<class MPTraits>
class PathOracle : public MPStrategyMethod<MPTraits> {

  public:

    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPProblemType::RoadmapType::VID VID;
    typedef typename MPProblemType::RoadmapType::GraphType GraphType;
    typedef typename GraphType::VI VI;
    typedef typename GraphType::EID EID;
    typedef EnvModel::RegionModelPtr RegionModelPtr;

    PathOracle();
    PathOracle(MPProblemType* _problem, XMLNode& _node);

    void Initialize();
    void Run();
    void Finalize();

  private:
};


template<class MPTraits>
PathOracle<MPTraits>::
PathOracle() : MPStrategyMethod<MPTraits>() {
  this->SetName("PathOracle");
}


template<class MPTraits>
PathOracle<MPTraits>::
PathOracle(MPProblemType* _problem, XMLNode& _node) :
    MPStrategyMethod<MPTraits>(_problem, _node) {
  this->SetName("PathOracle");
}


template<class MPTraits>
void
PathOracle<MPTraits>::
Initialize() {
  cout << "Initializing " << this->GetNameAndLabel() << "." << endl;

  //Make non-region objects non-selectable
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
}


template<class MPTraits>
void
PathOracle<MPTraits>::
Run() {
  cout << "Running PathOracle." << endl;

  GetVizmo().StartClock("PathOracle");
  this->GetStatClass()->StartClock("PathOracleMP");

  //stop clock
  GetVizmo().StopClock("PathOracle");
  this->GetStatClass()->StopClock("PathOracleMP");
}


template<class MPTraits>
void
PathOracle<MPTraits>::
Finalize() {
  //set up variables
  StatClass* stats = this->GetStatClass();
  string basename = this->GetBaseFilename();

  cout << "Finalizing PathOracle." << endl;

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //print clocks
  GetVizmo().PrintClock("Pre-regions", cout);
  GetVizmo().PrintClock("PathOracle", cout);
  stats->PrintClock("PathOracleMP", cout);

  //output stat class
  ofstream ostats((basename + ".stat").c_str());

  ostats << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(ostats, this->GetRoadmap());

  GetVizmo().PrintClock("Pre-regions", ostats);
  GetVizmo().PrintClock("PathOracle", ostats);
  stats->PrintClock("PathOracleMP", ostats);

  //output roadmap
  this->GetRoadmap()->Write(basename + ".map", this->GetEnvironment());

  //show results pop-up
  ostringstream results;
  results << "Planning Complete!" << endl;
  GetVizmo().PrintClock("Pre-regions", results);
  GetVizmo().PrintClock("PathOracle", results);

  GetMainWindow()->AlertUser(results.str());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
}

#endif
