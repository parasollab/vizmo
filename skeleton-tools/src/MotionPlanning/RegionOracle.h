#ifndef REGION_ORACLE_H_
#define REGION_ORACLE_H_

#include "OracleStrategy.h"

#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief Provide one or more regions to the planner prior to execution.
////////////////////////////////////////////////////////////////////////////////
template<class MPTraits>
class RegionOracle : public OracleStrategy<MPTraits> {

  public:

    ///\name Motion Planning Types
    ///@{

    typedef typename MPTraits::MPProblemType MPProblemType;

    ///@}
    ///\name Construction
    ///@{

    RegionOracle(const string& _strategy = "", const string& _inputRegions = "");
    RegionOracle(MPProblemType* _problem, XMLNode& _node);

    ///@}
    ///\name MPStrategyMethod Overrides
    ///@{

    virtual void Initialize() override;

    ///@}
};

/*------------------------------- Construction -------------------------------*/

template<class MPTraits>
RegionOracle<MPTraits>::
RegionOracle(const string& _strategy, const string& _inputRegions) :
    OracleStrategy<MPTraits>(_strategy, _inputRegions) {
  this->SetName("RegionOracle");
}


template<class MPTraits>
RegionOracle<MPTraits>::
RegionOracle(MPProblemType* _problem, XMLNode& _node) :
    OracleStrategy<MPTraits>(_problem, _node) {
  this->SetName("RegionOracle");
  this->m_oracleInput = _node.Read("regionFile", false, "", "Region Filename");
}

/*------------------------- MPStrategyMethod Overrides -----------------------*/

template<class MPTraits>
void
RegionOracle<MPTraits>::
Initialize() {
  OracleStrategy<MPTraits>::Initialize();
  GetVizmo().GetEnv()->LoadRegions(this->m_oracleInput);
}

/*----------------------------------------------------------------------------*/

#endif
