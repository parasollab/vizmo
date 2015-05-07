#ifndef AVOID_REGION_VALIDITY_H_
#define AVOID_REGION_VALIDITY_H_

#include "Models/RegionModel.h"
#include "Models/Vizmo.h"

#include "ValidityCheckers/ValidityCheckerMethod.h"

template<class MPTraits>
class AvoidRegionValidity : public ValidityCheckerMethod<MPTraits> {
  public:
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;

    AvoidRegionValidity();
    AvoidRegionValidity(MPProblemType* _problem, XMLNode& _node);

  protected:
    bool IsValidImpl(CfgType& _cfg, CDInfo& _cdInfo, const string& _callName);
};

template<class MPTraits>
AvoidRegionValidity<MPTraits>::
AvoidRegionValidity() : ValidityCheckerMethod<MPTraits>() {
  this->m_name = "AvoidRegionValidity";
}

template<class MPTraits>
AvoidRegionValidity<MPTraits>::
AvoidRegionValidity(MPProblemType* _problem, XMLNode& _node) :
    ValidityCheckerMethod<MPTraits>(_problem, _node) {
  this->m_name = "AvoidRegionValidity";
}

//checks whether robot is completely outside of all avoid regions.
//returns false if any part of the robot lies within any avoid region.
template<class MPTraits>
bool
AvoidRegionValidity<MPTraits>::
IsValidImpl(CfgType& _cfg, CDInfo& _cdInfo, const string& _callName) {
  //get environment, avoid regions, and robot
  Environment* env = this->GetMPProblem()->GetEnvironment();
  vector<EnvModel::RegionModelPtr> avoidRegions = GetVizmo().GetEnv()->GetAvoidRegions();
  shared_ptr<MultiBody> robot = env->GetMultiBody(_cfg.GetRobotIndex());

  _cfg.ConfigEnvironment(env); // Config the robot in the environment.

  //check each region to ensure _cfg does not enter it
  for(typename vector<EnvModel::RegionModelPtr>::iterator rit = avoidRegions.begin();
      rit != avoidRegions.end(); ++rit) {
    shared_ptr<Boundary> b = (*rit)->GetBoundary();

    //first check if robot's center is within the boundary
    if(b->InBoundary(_cfg.GetRobotCenterPosition()))
      return false;

    //if center is outside boundary, check each component of the robot to ensure
    //that none lie within the avoid region
    for(int m = 0; m < robot->GetFreeBodyCount(); ++m) {
      typedef vector<Vector3d>::const_iterator VIT;
      Transformation& worldTransformation = robot->GetFreeBody(m)->
        WorldTransformation();

      //first check bounding polyhedron for this component
      GMSPolyhedron &bb = robot->GetFreeBody(m)->GetBoundingBoxPolyhedron();
      bool bbValid = true;
      for(VIT v = bb.m_vertexList.begin(); v != bb.m_vertexList.end(); ++v)
        if(b->InBoundary(worldTransformation * (*v)))
          bbValid = false;

      //if the bounding polyhedron is valid, no need to check geometry
      if(bbValid)
        continue;

      //if the bounding polyhedron is not valid, check component's geometry
      GMSPolyhedron &poly = robot->GetFreeBody(m)->GetPolyhedron();
      for(VIT v = poly.m_vertexList.begin(); v != poly.m_vertexList.end(); ++v)
        if(b->InBoundary(worldTransformation * (*v)))
          return false;
    }
  }
  return true;
}

#endif
