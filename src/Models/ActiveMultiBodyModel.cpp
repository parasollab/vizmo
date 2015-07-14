#include "ActiveMultiBodyModel.h"

#include "BodyModel.h"
#include "BoundaryModel.h"
#include "EnvModel.h"
#include "Vizmo.h"

ActiveMultiBodyModel::
ActiveMultiBodyModel(shared_ptr<ActiveMultiBody> _a) :
  MultiBodyModel("ActiveMultiBody", _a), m_activeMultiBody(_a),
  m_initCfg(m_activeMultiBody->DOF()) {
  }

size_t
ActiveMultiBodyModel::
Dofs() const {
  return m_activeMultiBody->DOF();
}

bool
ActiveMultiBodyModel::
IsPlanar() const {
  return m_activeMultiBody->GetBaseType() == FreeBody::BodyType::Planar;
}

void
ActiveMultiBodyModel::
BackUp() {
  m_renderModeBackUp = m_renderMode;
  m_colorBackUp.clear();
  for(size_t i = 0; i < m_activeMultiBody->NumFreeBody(); ++i) {
    shared_ptr<FreeBody> body = m_activeMultiBody->GetFreeBody(i);
    if(body->IsColorLoaded())
      m_colorBackUp.push_back(body->GetColor());
    else
      m_colorBackUp.push_back(GetColor());
  }
}

void
ActiveMultiBodyModel::
ConfigureRender(const vector<double>& _cfg) {
  m_currCfg = _cfg;
  m_activeMultiBody->ConfigureRender(_cfg);

  for(size_t i = 0; i < m_activeMultiBody->NumFreeBody(); ++i)
    GetBodies()[i]->SetTransform(
        m_activeMultiBody->GetFreeBody(i)->RenderTransformation());
}

bool
ActiveMultiBodyModel::
InCSpace(const vector<double>& _cfg) {
  return m_activeMultiBody->InCSpace(_cfg, GetVizmo().GetEnv()->GetBoundary()->GetBoundary());
}

void
ActiveMultiBodyModel::
RestoreColor() {
  for(size_t i = 0; i < m_bodies.size(); ++i)
    m_bodies[i]->SetColor(m_colorBackUp[i]);
}

void
ActiveMultiBodyModel::
Restore() {
  SetRenderMode(m_renderModeBackUp);
  RestoreColor();
  ConfigureRender(m_initCfg);
}

void
ActiveMultiBodyModel::
Print(ostream& _os) const {
  _os << Name() << endl;
}

void
ActiveMultiBodyModel::
Build() {
  for(size_t i = 0; i < m_activeMultiBody->NumFreeBody(); ++i)
    m_bodies.emplace_back(new BodyModel(m_activeMultiBody->GetFreeBody(i)));
}

void
ActiveMultiBodyModel::
DrawSelected() {
  Restore();
  DrawSelectedImpl();
}

void
ActiveMultiBodyModel::
DrawSelectedImpl() {
  MultiBodyModel::DrawSelected();
}

const vector<ActiveMultiBody::DOFInfo>&
ActiveMultiBodyModel::
GetDOFInfo() const {
  return m_activeMultiBody->GetDOFInfo();
}
