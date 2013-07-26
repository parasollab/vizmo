#include "MultiBodyModel.h"

#include "ConnectionModel.h"
#include "BodyModel.h"
#include "Models/PolyhedronModel.h"
#include "Utilities/IOUtils.h"

//////////////////////////////////////////////////////////////////////
// Cons/Des
MultiBodyModel::MultiBodyModel() : m_active(false), m_surface(false) {}

MultiBodyModel::~MultiBodyModel() {
  for(BodyIter bit = BodiesBegin(); bit!=BodiesEnd(); ++bit)
    delete *bit;
  for(PolyhedronIter pit = PolyhedronsBegin(); pit!=PolyhedronsEnd(); ++pit)
    delete *pit;
}

//////////////////////////////////////////////////////////////////////
// Core
//////////////////////////////////////////////////////////////////////
void
MultiBodyModel::BuildModels() {

  //build for each body and compute com
  m_com(0, 0, 0);
  for(BodyIter bit = BodiesBegin(); bit!=BodiesEnd(); ++bit) {
    PolyhedronModel* p = new PolyhedronModel();
    p->SetBodyModel(*bit);
    p->BuildModels();
    p->SetColor((*bit)->GetColor());
    m_polyhedrons.push_back(p);
    m_com += (*bit)->GetTransform().translation();
  }

  m_com /= m_polyhedrons.size();

  //set position of multi-body as com
  tx() = m_com[0]; ty() = m_com[1]; tz() = m_com[2];

  //compute radius
  m_radius = 0;
  for(size_t i=0; i<m_polyhedrons.size(); i++){
    BodyModel* body = m_bodies[i];

    double dist = (body->GetTransform().translation()-m_com).norm() + m_polyhedrons[i]->GetRadius();

    if(m_radius < dist) m_radius = dist;

    //change to local coorindate of multibody
    m_polyhedrons[i]->tx() -= tx();
    m_polyhedrons[i]->ty() -= ty();
    m_polyhedrons[i]->tz() -= tz();
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
  glTranslated(m_polyhedrons[0]->tx(), m_polyhedrons[0]->ty(), m_polyhedrons[0]->tz());
  glTransform();
  glTranslated(-m_polyhedrons[0]->tx(), -m_polyhedrons[0]->ty(), -m_polyhedrons[0]->tz());

  for(PolyhedronIter pit = PolyhedronsBegin(); pit!=PolyhedronsEnd(); ++pit)
    (*pit)->Draw(_mode);

  glPopMatrix();
}

void
MultiBodyModel::DrawSelect(){
  glPushMatrix();
  glTranslated(m_polyhedrons[0]->tx(), m_polyhedrons[0]->ty(), m_polyhedrons[0]->tz());
  glTransform();
  glTranslated(-m_polyhedrons[0]->tx(), -m_polyhedrons[0]->ty(), -m_polyhedrons[0]->tz());

  for(PolyhedronIter pit = PolyhedronsBegin(); pit!=PolyhedronsEnd(); ++pit)
    (*pit)->DrawSelect();

  glPopMatrix();
}

void
MultiBodyModel::SetRenderMode(RenderMode _mode){
  GLModel::SetRenderMode(_mode);
  for(PolyhedronIter pit = PolyhedronsBegin(); pit!=PolyhedronsEnd(); ++pit)
    (*pit)->SetRenderMode(_mode);
}

void
MultiBodyModel::SetColor(const Color4& _c) {
  GLModel::SetColor(_c);
  for(PolyhedronIter pit = PolyhedronsBegin(); pit!=PolyhedronsEnd(); ++pit)
    (*pit)->SetColor(_c);
}

void
MultiBodyModel::GetChildren(list<GLModel*>& _models) {
  for(PolyhedronIter pit = PolyhedronsBegin(); pit!=PolyhedronsEnd(); ++pit)
    _models.push_back(*pit);
}


vector<string>
MultiBodyModel::GetInfo() const{
  vector<string> info;
  ostringstream temp, os;

  if(m_active){
    info.push_back(string("Robot"));
    temp << m_polyhedrons.size();
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

    BodyModel* b = new BodyModel();
    b->ParseOtherBody(_is, _modelDir, color);
    m_bodies.push_back(b);
  }
  else
    throw ParseException(WHERE,
        "Unsupported body type '" + multibodyType +
        "'. Choices are Active, Passive, Internal, or Surface.");
}
