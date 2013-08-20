#include "BodyModel.h"

#include "ConnectionModel.h"
#include "Models/PolyhedronModel.h"
#include "Utilities/IOUtils.h"

BodyModel::BodyModel(bool _isSurface) :
  m_polyhedronModel(NULL),
  m_isFixed(true), m_isSurface(_isSurface), m_isBase(true),
  m_baseType(Robot::PLANAR), m_baseMovementType(Robot::TRANSLATIONAL),
  m_currentTransform(), m_transformDone(false) {
  }

BodyModel::~BodyModel() {
  delete m_polyhedronModel;
  for(ConnectionIter cit = Begin(); cit!=End(); ++cit)
    delete *cit;
}

vector<string>
BodyModel::GetInfo() const {
  return m_polyhedronModel->GetInfo();
}

void
BodyModel::GetChildren(list<GLModel*>& _models) {
  _models.push_back(m_polyhedronModel);
}

void
BodyModel::SetRenderMode(RenderMode _mode) {
  GLModel::SetRenderMode(_mode);
  m_polyhedronModel->SetRenderMode(_mode);
}

void
BodyModel::SetColor(const Color4& _c) {
  GLModel::SetColor(_c);
  m_polyhedronModel->SetColor(_c);
}

void
BodyModel::ComputeTransform(const BodyModel* _body, size_t _nextBody){
  for(ConnectionIter cit = _body->Begin(); cit!=_body->End(); ++cit) {
    if((*cit)->GetNextIndex() == _nextBody) {
      Transformation dh = (*cit)->DHTransform();
      const Transformation& tdh = (*cit)->TransformToDHFrame();
      const Transformation& tbody2 = (*cit)->TransformToBody2();
      m_currentTransform = m_prevTransform * tdh * dh * tbody2;
      return;
    }
  }
  cerr << "Compute transform error. Connection not found." << endl;
}

void
BodyModel::Draw(GLenum _mode) {
  glColor4fv(GetColor());
  glPushMatrix();
  glTransform();
  glScale();
  m_polyhedronModel->Draw(_mode);
  glPopMatrix();
}

void
BodyModel::DrawSelect() {
  glLineWidth(2);
  glPushMatrix();
  glTransform();
  m_polyhedronModel->DrawSelect();
  glPopMatrix();
}

void
BodyModel::ParseActiveBody(istream& _is, const string& _modelDataDir, const Color4 _color) {
  m_isFixed = false;

  m_filename = ReadFieldString(_is, WHERE,
      "Failed reading geometry filename.", false);

  if(!_modelDataDir.empty()) {
    //store just the path of the current directory
    m_directory = _modelDataDir;
    m_modelFilename += _modelDataDir + "/";
  }

  m_modelFilename += m_filename;

  //Read for Base Type. If Planar or Volumetric, read in two more strings
  //If Joint skip this stuff. If Fixed read in positions like an obstacle
  string baseTag = ReadFieldString(_is, WHERE,
      "Failed reading base tag.");
  m_baseType = Robot::GetBaseFromTag(baseTag);

  if(m_baseType == Robot::VOLUMETRIC || m_baseType == Robot::PLANAR){
    m_isBase = true;
    string rotationalTag = ReadFieldString(_is, WHERE,
        "Failed reading rotation tag.");
    m_baseMovementType = Robot::GetMovementFromTag(rotationalTag);
  }
  else if(m_baseType == Robot::FIXED){
    m_isBase = true;
    m_currentTransform = ReadField<Transformation>(_is, WHERE, "Failed reading body transformation");
  }

  m_polyhedronModel = new PolyhedronModel(this);

  //Set color information
  SetColor(_color);
}

void
BodyModel::ParseOtherBody(istream& _is, const string& _modelDataDir, const Color4 _color) {
  m_isFixed = m_isBase = true;

  //read and set up geometry filename data
  m_filename = ReadFieldString(_is, WHERE,
      "Failed reading geometry filename.", false);

  if(!_modelDataDir.empty()) {
    //store just the path of the current directory
    m_directory = _modelDataDir;
    m_modelFilename += _modelDataDir + "/";
  }
  m_modelFilename += m_filename;

  //read transformation
  m_currentTransform = ReadField<Transformation>(_is, WHERE, "Failed reading body transformation");

  m_polyhedronModel = new PolyhedronModel(this);

  //Set color information
  SetColor(_color);
}

ostream&
operator<<(ostream& _os, const BodyModel& _b) {
  return _os;
}

