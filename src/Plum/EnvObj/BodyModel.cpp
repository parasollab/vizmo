#include "BodyModel.h"

#include "ConnectionModel.h"
#include "Utilities/IOUtils.h"

BodyModel::BodyModel() : m_transform(), m_currentTransform() {
  m_isFixed = false;
  m_isBase = false;
  m_transformDone = false;
  m_index    = -1;

  m_modelFilename = "";
  m_isNew = false;
  m_isSurface = false;
}

BodyModel::BodyModel(const BodyModel& m_other){
  *this = m_other;
}

BodyModel::~BodyModel(){
  typedef vector<ConnectionModel*>::iterator CIT;
  for(CIT cit = m_connections.begin(); cit!=m_connections.end(); ++cit) {
    delete *cit;
  }
}

/*void
BodyModel::Transform(){
  if(m_isBase){
    m_currentTransform = m_transform;
  }
}*/

void
BodyModel::ComputeTransform(const BodyModel& _body, int _nextBody){
  typedef vector<ConnectionModel*>::const_iterator CIT;
  for(CIT cit = _body.m_connections.begin(); cit!=_body.m_connections.end(); ++cit) {
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
BodyModel::operator=(const BodyModel& _other){
  /*m_isFixed = _other.m_isFixed;
  m_isBase = _other.m_isBase;
  m_index = _other.m_index;
  m_x=_other.m_x;
  m_y=_other.m_y;
  m_z=_other.m_z;
  m_alpha=_other.m_alpha;
  m_beta=_other.m_beta;
  m_gamma=_other.m_gamma;
  m_isSurface=_other.m_isSurface;

  m_fileName = _other.m_fileName;
  m_directory = _other.m_directory;
  m_isNew = _other.m_isNew;

  m_modelDataFileName[0]='\0';
  m_modelDataFileName=_other.m_modelDataFileName;
  rgb[0] = _other.rgb[0];
  rgb[1] = _other.rgb[1];
  rgb[2] = _other.rgb[2];

  m_numberOfConnection =_other.m_numberOfConnection;

  m_currentTransform = _other.m_currentTransform;
  m_prevTransform = _other.m_prevTransform;

  for(int i = 0; i<m_numberOfConnection; i++){

    m_connections[i] = _other.m_connections[i];

  }
  */
  cerr << "Body assignment. Exiting." << endl;
  exit(1);
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
    m_transform = ReadField<Transformation>(_is, WHERE, "Failed reading body transformation");
    //Transform();
    m_currentTransform = m_transform;
  }

  //Set color information
  m_color = _color;
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
  m_transform = ReadField<Transformation>(_is, WHERE, "Failed reading body transformation");

  //Transform();
  m_currentTransform = m_transform;

  //Set color information
  m_color = _color;
}

ostream&
operator<<(ostream& _os, const BodyModel& _b) {
}

