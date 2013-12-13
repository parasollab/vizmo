#include "BodyModel.h"

#include "ConnectionModel.h"
#include "Models/PolyhedronModel.h"
#include "Utilities/IO.h"

BodyModel::Base
BodyModel::GetBaseFromTag(const string& _tag){
  if(_tag == "PLANAR")
    return PLANAR;
  else if(_tag == "VOLUMETRIC")
    return VOLUMETRIC;
  else if(_tag == "FIXED")
    return FIXED;
  else if(_tag == "JOINT")
    return JOINT;
  else
    throw ParseException(WHERE,
        "Failed parsing robot base type. Choices are Planar, Volumetric, Fixed, or Joint.");
}

BodyModel::BaseMovement
BodyModel::GetMovementFromTag(const string& _tag){
  if(_tag == "ROTATIONAL")
    return ROTATIONAL;
  else if (_tag == "TRANSLATIONAL")
    return TRANSLATIONAL;
  else
    throw ParseException(WHERE,
        "Failed parsing robot movement type. Choices are Rotational or Translational.");
}

BodyModel::BodyModel(bool _isSurface) : TransformableModel("Body"),
  m_polyhedronModel(NULL),
  m_isFixed(true), m_isSurface(_isSurface), m_isBase(true),
  m_baseType(PLANAR), m_baseMovementType(TRANSLATIONAL),
  m_currentTransform(), m_transformDone(false) {
  }

BodyModel::~BodyModel() {
  delete m_polyhedronModel;
  for(ConnectionIter cit = Begin(); cit!=End(); ++cit)
    delete *cit;
}

void
BodyModel::Print(ostream& _os) const {
  m_polyhedronModel->Print(_os);
}

void
BodyModel::GetChildren(list<Model*>& _models) {
  _models.push_back(m_polyhedronModel);
}

void
BodyModel::SetRenderMode(RenderMode _mode) {
  Model::SetRenderMode(_mode);
  m_polyhedronModel->SetRenderMode(_mode);
}

void
BodyModel::SetColor(const Color4& _c) {
  Model::SetColor(_c);
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
BodyModel::Draw() {
  glColor4fv(GetColor());
  glPushMatrix();
  Transform();
  m_polyhedronModel->Draw();
  glPopMatrix();
}

void
BodyModel::DrawSelect() {
  glLineWidth(2);
  glPushMatrix();
  Transform();
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
    m_modelFilename = _modelDataDir + "/";
  }

  m_modelFilename += m_filename;

  //Read for Base Type. If Planar or Volumetric, read in two more strings
  //If Joint skip this stuff. If Fixed read in positions like an obstacle
  string baseTag = ReadFieldString(_is, WHERE,
      "Failed reading base tag.");
  m_baseType = GetBaseFromTag(baseTag);

  if(m_baseType == VOLUMETRIC || m_baseType == PLANAR){
    m_isBase = true;
    string rotationalTag = ReadFieldString(_is, WHERE,
        "Failed reading rotation tag.");
    m_baseMovementType = GetMovementFromTag(rotationalTag);
  }
  else if(m_baseType == FIXED){
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

void
BodyModel::DeleteConnection(ConnectionModel* _c){
  int index=0;
  for(ConnectionIter cit=Begin(); cit!=End(); cit++){
    if((*cit)==_c){
      m_connections.erase(m_connections.begin()+index);
      return;
    }
    index++;
  }
}
