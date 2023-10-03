#include "StaticMultiBodyModel.h"

//#include "Environment/FixedBody.h"
//#include "Environment/StaticMultiBody.h"

#include "Geometry/Bodies/Body.h"
#include "Geometry/Bodies/MultiBody.h"

#include "BodyModel.h"

StaticMultiBodyModel::
StaticMultiBodyModel(MultiBody* _s) :
  MultiBodyModel("StaticMultiBody", _s), m_staticMultiBody(_s) {
  }

StaticMultiBodyModel::
StaticMultiBodyModel(string _name, MultiBody* _s) :
  MultiBodyModel(_name, _s), m_staticMultiBody(_s) {
  }

void
StaticMultiBodyModel::Build() {
  auto body = m_staticMultiBody->GetBody(0);
  m_bodies.emplace_back(new BodyModel(body));
  m_bodies.back()->Build();
  //if(body->IsColorLoaded()) {
    auto c = body->GetColor();
    SetColor(Color4(c[0],c[1],c[2],c[3]));
  //}
  if(m_staticMultiBody->IsInternal())
    SetRenderMode(INVISIBLE_MODE);
}

void
StaticMultiBodyModel::
SetTransform(Transformation& _t) {
  m_bodies[0]->SetTransform(_t);
  m_staticMultiBody->GetBody(0)->Configure(_t);
  //m_staticMultiBody->GetBody(0)->PutWorldTransformation(_t);
}
