#include "BodyModel.h"

#include <GL/glu.h>

#include "Environment/Body.h"
#include "MPProblem/MPProblemBase.h"

#include "Models/PolyhedronModel.h"
#include "Utilities/LoadTexture.h"

BodyModel::
BodyModel(shared_ptr<Body> _b) : TransformableModel("Body"), m_body(_b),
  m_polyhedronModel(new PolyhedronModel(
        (Body::m_modelDataDir == "/" ? "" : Body::m_modelDataDir) +
        _b->GetFileName(), _b->GetCOMAdjust())),
  m_textureID(-1) {
    SetTransform(_b->GetWorldTransformation());
  }

BodyModel::
~BodyModel() {
  delete m_polyhedronModel;
  glDeleteTextures(1, &m_textureID);
}

void
BodyModel::
Print(ostream& _os) const {
  m_polyhedronModel->Print(_os);
}

void
BodyModel::
GetChildren(list<Model*>& _models) {
  _models.push_back(m_polyhedronModel);
}

void
BodyModel::
SetRenderMode(RenderMode _mode) {
  Model::SetRenderMode(_mode);
  m_polyhedronModel->SetRenderMode(_mode);
}

void
BodyModel::
SetSelectable(bool _s) {
  m_selectable = _s;
  m_polyhedronModel->SetSelectable(_s);
}

void
BodyModel::
ToggleNormals() {
  Model::ToggleNormals();
  m_polyhedronModel->ToggleNormals();
}

void
BodyModel::
Select(GLuint* _index, vector<Model*>& sel) {
  if(m_selectable)
    m_polyhedronModel->Select(_index, sel);
}

void
BodyModel::
DrawRender() {
  glPushMatrix();

  Transform();

  if(m_textureID != GLuint(-1)) {
    glColor4fv(GetColor());
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
  }
  else
    glColor4fv(GetColor());

  m_polyhedronModel->DrawRender();

  if(m_body->IsTextureLoaded())
    glDisable(GL_TEXTURE_2D);

  m_polyhedronModel->DrawNormals();

  glPopMatrix();
}

void
BodyModel::
DrawSelect() {
  glPushMatrix();
  Transform();
  m_polyhedronModel->DrawSelect();
  glPopMatrix();
}

void
BodyModel::
DrawSelected() {
  glLineWidth(2);
  glPushMatrix();
  Transform();
  m_polyhedronModel->DrawSelected();
  glPopMatrix();
}

void
BodyModel::
DrawHaptics() {
  glPushMatrix();
  Transform();
  m_polyhedronModel->DrawHaptics();
  glPopMatrix();
}

ostream&
operator<<(ostream& _os, const BodyModel& _b) {
  return _os;
}

void
BodyModel::
SetTransform(const Transformation& _t) {
  m_currentTransform = _t;

  const Vector3d& p = _t.translation();
  const Orientation& o = _t.rotation();
  EulerAngle e;
  convertFromMatrix(e, o.matrix());
  Quaternion qua;
  convertFromMatrix(qua, o.matrix());

  Translation() = p;

  Rotation()[0] = e.alpha();
  Rotation()[1] = e.beta();
  Rotation()[2] = e.gamma();

  RotationQ() = qua;
}

void
BodyModel::
Build() {
  if(m_body->IsTextureLoaded()) {
    m_textureID = LoadTexture(MPProblemBase::GetPath(m_body->GetTexture()));
    SetColor(Color4(1, 1, 1, 1));
  }
}
