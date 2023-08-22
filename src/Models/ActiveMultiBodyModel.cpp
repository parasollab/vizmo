#include "ActiveMultiBodyModel.h"

#include "BodyModel.h"
#include "BoundaryModel.h"
#include "EnvModel.h"
#include "Vizmo.h"

ActiveMultiBodyModel::
ActiveMultiBodyModel(shared_ptr<MultiBody> _a) :
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
  return m_activeMultiBody->GetBaseType() == Body::Type::Planar;
}

void
ActiveMultiBodyModel::
BackUp() {
  m_renderModeBackUp = m_renderMode;
  m_colorBackUp.clear();
  for(size_t i = 0; i < m_activeMultiBody->GetNumBodies(); ++i) {
    Body* body = m_activeMultiBody->GetBody(i);
    if(body->IsTextureLoaded())
      m_colorBackUp.push_back(Color4(1, 1, 1, 1));
    else {
      auto color = body->GetColor();
      m_colorBackUp.push_back(Color4(color[0],color[1],color[2],color[3]));
    }
  }
}

void
ActiveMultiBodyModel::
ConfigureRender(const vector<double>& _cfg) {
  m_currCfg = _cfg;
  /*m_activeMultiBody->ConfigureRender(_cfg);

  for(size_t i = 0; i < m_activeMultiBody->GetNumBodies(); ++i)
    GetBodies()[i]->SetTransform(
        m_activeMultiBody->GetBody(i)->RenderTransformation());
  */
  m_activeMultiBody->Configure(_cfg);
}

bool
ActiveMultiBodyModel::
InCSpace(const vector<double>& _cfg) {
  //return m_activeMultiBody->InCSpace(_cfg, GetVizmo().GetEnv()->GetBoundary()->GetBoundary());
  return GetVizmo().GetEnv()->GetBoundary()->GetBoundary()->InBoundary(_cfg);
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
  for(size_t i = 0; i < m_activeMultiBody->GetNumBodies(); ++i) {
    m_bodies.emplace_back(new BodyModel(m_activeMultiBody->GetBody(i)));
    m_bodies.back()->Build();
  }
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

const vector<DofInfo>&
ActiveMultiBodyModel::
GetDOFInfo() const {
  return m_activeMultiBody->GetDofInfo();
}
