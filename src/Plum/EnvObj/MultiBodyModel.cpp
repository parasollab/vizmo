#include "MultiBodyModel.h"

#include "ConnectionModel.h"
#include "BodyModel.h"
#include "Utilities/IOUtils.h"

//////////////////////////////////////////////////////////////////////
// Cons/Des
MultiBodyModel::MultiBodyModel() : m_active(false), m_surface(false) {}

MultiBodyModel::~MultiBodyModel() {
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    delete *bit;
}

//////////////////////////////////////////////////////////////////////
// Core
//////////////////////////////////////////////////////////////////////
void
MultiBodyModel::BuildModels() {
  //build for each body and compute com
  m_com(0, 0, 0);
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    m_com += (*bit)->GetTransform().translation();
  m_com /= m_bodies.size();

  //set position of multi-body as com
  tx() = m_com[0]; ty() = m_com[1]; tz() = m_com[2];

  //compute radius
  m_radius = 0;
  for(BodyIter bit = Begin(); bit!=End(); ++bit) {
    double dist = ((*bit)->GetTransform().translation()-m_com).norm() + (*bit)->GetRadius();
    if(m_radius < dist)
      m_radius = dist;
  }
}

void
MultiBodyModel::Select(unsigned int* _index, vector<GLModel*>& sel){
  if(_index!= NULL)
    sel.push_back(this);
}

//Draw
void
MultiBodyModel::Draw(GLenum _mode){
  glColor4fv(GetColor());
  glPushMatrix();

  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    (*bit)->Draw(_mode);

  glPopMatrix();
}

void
MultiBodyModel::DrawSelect(){
  glPushMatrix();

  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    (*bit)->DrawSelect();

  glPopMatrix();
}

void
MultiBodyModel::SetRenderMode(RenderMode _mode){
  GLModel::SetRenderMode(_mode);
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    (*bit)->SetRenderMode(_mode);
}

void
MultiBodyModel::SetColor(const Color4& _c) {
  GLModel::SetColor(_c);
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    (*bit)->SetColor(_c);
}

void
MultiBodyModel::GetChildren(list<GLModel*>& _models) {
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    _models.push_back(*bit);
}


vector<string>
MultiBodyModel::GetInfo() const{
  vector<string> info;
  ostringstream temp, os;

  if(m_active){
    info.push_back(string("Robot"));
    temp << m_bodies.size();
  }
  else {
    info.push_back(string("Obstacle"));
    temp << "Position ( "<< tx()<<", "<<ty()<<", "<<tz()<<" )";
  }
  info.push_back(temp.str());
  return info;
}

void
MultiBodyModel::ParseMultiBody(istream& _is, const string& _modelDir) {
  string multibodyType = ReadFieldString(_is, WHERE,
      "Failed reading Multibody type.");

  if(multibodyType == "ACTIVE")
    m_active = true;
  else if(multibodyType == "SURFACE")
    m_surface = true;

  if(multibodyType == "ACTIVE") {
    size_t numberOfBody = ReadField<size_t>(_is, WHERE,
        "Failed reading body count");

    Color4 color = GetColorFromComment(_is);

    for(size_t i = 0; i < numberOfBody; i++) {
      BodyModel* b = new BodyModel();
      b->ParseActiveBody(_is, _modelDir, color);
      m_bodies.push_back(b);
    }

    //Get connection info
    string connectionTag = ReadFieldString(_is, WHERE,
        "Failed reading connections tag.");
    size_t numberOfConnections = ReadField<size_t>(_is, WHERE,
        "Failed reading number of connections");

    if(numberOfConnections != 0){
      for(size_t i = 0; i < numberOfConnections; ++i) {
        ConnectionModel* c = new ConnectionModel();
        _is >> *c;
        //Increment m_numberOfConnection for each body
        BodyModel* previousBody = m_bodies[c->GetPreviousIndex()];
        previousBody->AddConnection(c);
        m_jointMap.push_back(c);
      }
    }
  }
  else if(multibodyType == "INTERNAL" ||
      multibodyType == "SURFACE" ||
      multibodyType == "PASSIVE") {

    Color4 color = GetColorFromComment(_is);

    BodyModel* b = new BodyModel(m_surface);
    b->ParseOtherBody(_is, _modelDir, color);
    m_bodies.push_back(b);
  }
  else
    throw ParseException(WHERE,
        "Unsupported body type '" + multibodyType +
        "'. Choices are Active, Passive, Internal, or Surface.");
}
