#ifndef REGION_ORACLE_H_
#define REGION_ORACLE_H_

#include "OracleStrategy.h"

#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

template<class MPTraits>
class RegionOracle : public OracleStrategy<MPTraits> {
  public:
    typedef typename MPTraits::MPProblemType MPProblemType;

    RegionOracle(const string& _inputRegions = "", const string& _strategy = "");
    RegionOracle(MPProblemType* _problem, XMLNode& _node);

    void Initialize();

  private:
    string m_inputRegions;
};


template<class MPTraits>
RegionOracle<MPTraits>::
RegionOracle(const string& _inputRegions, const string& _strategy) :
  OracleStrategy<MPTraits>(_strategy), m_inputRegions(_inputRegions) {
    this->SetName("RegionOracle");
  }

template<class MPTraits>
RegionOracle<MPTraits>::
RegionOracle(MPProblemType* _problem, XMLNode& _node) :
  OracleStrategy<MPTraits>(_problem, _node) {
    this->SetName("RegionOracle");
    m_inputRegions = _node.Read("regionFile", false, "", "Region Filename");
  }

template<class MPTraits>
void
RegionOracle<MPTraits>::
Initialize() {
  OracleStrategy<MPTraits>::Initialize();

  GetVizmo().GetEnv()->LoadRegions(m_inputRegions);
}

#endif
