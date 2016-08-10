#ifndef ORACLE_STRATEGY_H_
#define ORACLE_STRATEGY_H_

#include "MPStrategies/MPStrategyMethod.h"

#include "GUI/MainWindow.h"
#include "GUI/ModelSelectionWidget.h"

#include "Models/Vizmo.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief Base strategy for supplying oracle information to the planner prior
///        to runtime.
////////////////////////////////////////////////////////////////////////////////
template<class MPTraits>
class OracleStrategy : public MPStrategyMethod<MPTraits> {

  public:

    ///\name Motion Planning Types
    ///@{

    typedef typename MPTraits::MPProblemType MPProblemType;

    ///@}
    ///\name Construction
    ///@{

    OracleStrategy(const string& _strategy = "", const string& _input = "");
    OracleStrategy(MPProblemType* _problem, XMLNode& _node);
    virtual ~OracleStrategy() = default;

    ///@}
    ///\name MPStrategyMethod Overrides
    ///@{

    virtual void Initialize() override;
    virtual void Run() override;
    virtual void Finalize() override;

    ///@}

  protected:

    ///\name Internal State
    ///@{

    string m_strategy;    ///< The MPStrategy to run.
    string m_oracleInput; ///< The oracle input file name.

    ///@}
};

/*------------------------------- Construction -------------------------------*/

template<class MPTraits>
OracleStrategy<MPTraits>::
OracleStrategy(const string& _strategy, const string& _input) :
    MPStrategyMethod<MPTraits>(), m_strategy(_strategy), m_oracleInput(_input) {
  this->SetName("OracleStrategy");
}

template<class MPTraits>
OracleStrategy<MPTraits>::
OracleStrategy(MPProblemType* _problem, XMLNode& _node) :
    MPStrategyMethod<MPTraits>(_problem, _node) {
  this->SetName("OracleStrategy");
  m_strategy = _node.Read("mps", true, "", "MPStrategy from VizmoXML");
}

/*------------------------- MPStrategyMethod Overrides -----------------------*/

template<class MPTraits>
void
OracleStrategy<MPTraits>::
Initialize() {
  cout << "Initializing " << this->GetNameAndLabel() << "." << endl;

  //Make non-region objects non-selectable
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
}


template<class MPTraits>
void
OracleStrategy<MPTraits>::
Run() {
  string name = this->GetNameAndLabel();

  GetVizmo().StartClock(name);
  this->GetStatClass()->StartClock(name + "MP");
  GetVizmo().StopClock("Pre-input");

  cout << "Running " << name << ". Beginning Strategy: " << m_strategy << endl;

  (*this->GetMPStrategy(m_strategy))();

  GetVizmo().GetMap()->RefreshMap();

  //stop clock
  GetVizmo().StopClock(name);
  this->GetStatClass()->StopClock(name + "MP");
}


template<class MPTraits>
void
OracleStrategy<MPTraits>::
Finalize() {
  //set up variables
  StatClass* stats = this->GetStatClass();
  string basename = this->GetBaseFilename();
  string name = this->GetNameAndLabel();
  cout << "Finalizing " << name << "." << endl;

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //print clocks
  GetVizmo().PrintClock("Pre-input", cout);
  GetVizmo().PrintClock(name, cout);
  stats->PrintClock(name + "MP", cout);

  //output stat class
  ofstream ostats(basename + ".stat");

  ostats << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(ostats, this->GetRoadmap());

  GetVizmo().PrintClock("Pre-input", ostats);
  GetVizmo().PrintClock(name, ostats);
  stats->PrintClock(name + "MP", ostats);

  //output roadmap
  this->GetRoadmap()->Write(basename + ".map", this->GetEnvironment());

  //show results pop-up
  ostringstream results;
  results << "Planning Cfg Complete!" << endl;
  GetVizmo().PrintClock("Pre-input", results);
  GetVizmo().PrintClock(name, results);

  GetMainWindow()->AlertUser(results.str());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
}

/*----------------------------------------------------------------------------*/

#endif
