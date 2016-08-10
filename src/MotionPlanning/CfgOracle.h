#ifndef CFG_ORACLE_H_
#define CFG_ORACLE_H_

#include "OracleStrategy.h"

#include "Models/Vizmo.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief Provide one or more configurations to the planner prior to execution.
////////////////////////////////////////////////////////////////////////////////
template<class MPTraits>
class CfgOracle : public OracleStrategy<MPTraits> {

  public:

    ///\name Motion Planning Types
    ///@{

    typedef typename MPTraits::MPProblemType MPProblemType;

    ///@}
    ///\name Construction
    ///@{

    CfgOracle(const string& _strategy = "", const string& _inputCfgs = "");
    CfgOracle(MPProblemType* _problem, XMLNode& _node);

    ///@}
    ///\name MPStrategyMethod Overrides
    ///@{

    virtual void Initialize() override;

    ///@}
};

/*------------------------------- Construction -------------------------------*/

template<class MPTraits>
CfgOracle<MPTraits>::
CfgOracle(const string& _strategy, const string& _inputCfgs) :
    OracleStrategy<MPTraits>(_strategy, _inputCfgs) {
  this->SetName("CfgOracle");
}


template<class MPTraits>
CfgOracle<MPTraits>::
CfgOracle(MPProblemType* _problem, XMLNode& _node) :
    OracleStrategy<MPTraits>(_problem, _node) {
  this->SetName("CfgOracle");
  this->m_oracleInput = _node.Read("cfgFile", false, "", "Cfg Map Filename");
}

/*------------------------- MPStrategyMethod Overrides -----------------------*/

template<class MPTraits>
void
CfgOracle<MPTraits>::
Initialize() {
  OracleStrategy<MPTraits>::Initialize();

  if(!this->m_oracleInput.empty()) {
    auto r = this->GetRoadmap();
    r->Read(this->m_oracleInput);
    cout << "Input Map: " << this->m_oracleInput << endl;
    GetVizmo().GetMap()->RefreshMap();
  }
}

/*----------------------------------------------------------------------------*/

#endif
