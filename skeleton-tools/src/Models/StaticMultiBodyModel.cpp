#include "StaticMultiBodyModel.h"

#include "Environment/FixedBody.h"
#include "Environment/StaticMultiBody.h"

#include "BodyModel.h"

StaticMultiBodyModel::
StaticMultiBodyModel(shared_ptr<StaticMultiBody> _s) :
  MultiBodyModel("StaticMultiBody", _s), m_staticMultiBody(_s) {
  }

StaticMultiBodyModel::
StaticMultiBodyModel(string _name, shared_ptr<StaticMultiBody> _s) :
  MultiBodyModel(_name, _s), m_staticMultiBody(_s) {
  }

void
StaticMultiBodyModel::Build() {
  shared_ptr<FixedBody> body = m_staticMultiBody->GetFixedBody(0);
  m_bodies.emplace_back(new BodyModel(body));
  m_bodies.back()->Build();
  if(body->IsColorLoaded())
    SetColor(body->GetColor());
  if(m_staticMultiBody->IsInternal())
    SetRenderMode(INVISIBLE_MODE);
}

void
StaticMultiBodyModel::
SetTransform(Transformation& _t) {
  m_bodies[0]->SetTransform(_t);
  m_staticMultiBody->GetFixedBody(0)->PutWorldTransformation(_t);
}
