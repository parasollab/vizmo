#ifndef CFG_ORACLE_H_
#define CFG_ORACLE_H_

#include "MPStrategies/MPStrategyMethod.h"

#include "GUI/MainWindow.h"
#include "GUI/ModelSelectionWidget.h"

#include "Models/Vizmo.h"

#include "MotionPlanning/VizmoTraits.h"

template<class MPTraits>
class CfgOracle : public MPStrategyMethod<MPTraits> {

  public:

    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPProblemType::RoadmapType::VID VID;
    typedef typename MPProblemType::RoadmapType::GraphType GraphType;
    typedef typename GraphType::VI VI;
    typedef typename GraphType::EID EID;
    typedef EnvModel::RegionModelPtr RegionModelPtr;

    CfgOracle();
    CfgOracle(MPProblemType* _problem, XMLNode& _node);

    void Initialize();
    void Run();
    void Finalize();

  private:
};


template<class MPTraits>
CfgOracle<MPTraits>::
CfgOracle() : MPStrategyMethod<MPTraits>() {
  this->SetName("CfgOracle");
}


template<class MPTraits>
CfgOracle<MPTraits>::
CfgOracle(MPProblemType* _problem, XMLNode& _node) :
    MPStrategyMethod<MPTraits>(_problem, _node) {
  this->SetName("CfgOracle");
}


template<class MPTraits>
void
CfgOracle<MPTraits>::
Initialize() {
  cout << "Initializing " << this->GetNameAndLabel() << "." << endl;

  //Make non-region objects non-selectable
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
  GetVizmo().GetRobot()->SetSelectable(false);
}


template<class MPTraits>
void
CfgOracle<MPTraits>::Run() {
  cout << "Running CfgOracle." << endl;

  GetVizmo().StartClock("CfgOracle");
  this->GetStatClass()->StartClock("CfgOracleMP");

  //stop clock
  GetVizmo().StopClock("CfgOracle");
  this->GetStatClass()->StopClock("CfgOracleMP");
}


template<class MPTraits>
void
CfgOracle<MPTraits>::
Finalize() {
  //set up variables
  StatClass* stats = this->GetStatClass();
  string basename = this->GetBaseFilename();

  cout << "Finalizing CfgOracle." << endl;

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //print clocks
  GetVizmo().PrintClock("Pre-regions", cout);
  GetVizmo().PrintClock("CfgOracle", cout);
  stats->PrintClock("CfgOracleMP", cout);

  //output stat class
  ofstream ostats((basename + ".stat").c_str());

  ostats << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(ostats, this->GetRoadmap());

  GetVizmo().PrintClock("Pre-regions", ostats);
  GetVizmo().PrintClock("CfgOracle", ostats);
  stats->PrintClock("CfgOracleMP", ostats);

  //output roadmap
  this->GetRoadmap()->Write(basename + ".map", this->GetEnvironment());

  //show results pop-up
  ostringstream results;
  results << "Planning Complete!" << endl;
  GetVizmo().PrintClock("Pre-regions", results);
  GetVizmo().PrintClock("CfgOracle", results);

  GetMainWindow()->AlertUser(results.str());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
  GetVizmo().GetRobot()->SetSelectable(true);
}

#endif
