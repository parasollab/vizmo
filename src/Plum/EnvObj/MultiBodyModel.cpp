#include "MultiBodyModel.h"

//////////////////////////////////////////////////////////////////////
// Cons/Des
MultiBodyModel::MultiBodyModel(const MultiBodyInfo& m_mbInfo)
  : m_mbInfo(m_mbInfo){
    SetColor(0.4f,0.4f,0.4f,1);
    m_fixed = !m_mbInfo.m_active;
    m_radius = 0;
  }

//////////////////////////////////////////////////////////////////////
// Core
//////////////////////////////////////////////////////////////////////
void 
MultiBodyModel::BuildModels(){
  m_poly = vector<PolyhedronModel>(m_mbInfo.m_numberOfBody);

  //build for each body and compute com
  for(size_t i=0; i< size_t(m_mbInfo.m_numberOfBody); i++){
    BodyInfo& info = m_mbInfo.m_mBodyInfo[i];
    //only build fixed, free body will not be built (when m_fixed is set)
    if(!info.m_isFixed&& m_fixed)
      continue;
    m_poly[i].SetBodyInfo(info);
    m_poly[i].BuildModels();

    m_poly[i].SetColor(info.rgb[0],info.rgb[1],info.rgb[2],1);

    m_com[0]+=info.m_x;
    m_com[1]+=info.m_y;
    m_com[2]+=info.m_z;
  }
  for(int i=0; i<3; i++)
    m_com[i] /= (double)m_poly.size();

  //set position of multi-body as com
  tx()=m_com[0]; ty()=m_com[1]; tz()=m_com[2];

  //compute radius
  for(size_t i=0; i<m_poly.size(); i++){
    BodyInfo& info=m_mbInfo.m_mBodyInfo[i];
    //only build fixed, free body will not be build (when m_fixed is set)
    if(!info.m_isFixed&& m_fixed)
      continue;

    double dist= (Point3d(info.m_x,info.m_y,info.m_z)-m_com).norm()
      + m_poly[i].GetRadius();

    if(m_radius< dist) m_radius = dist;

    //change to local coorindate of multibody
    m_poly[i].tx()-=tx();
    m_poly[i].ty()-=ty();
    m_poly[i].tz()-=tz();
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
  glColor4fv(&m_rgba[0]);
  glPushMatrix();
  glTranslated(m_poly[0].tx(), m_poly[0].ty(), m_poly[0].tz());
  glTransform();
#ifdef USE_PHANTOM
  if(fabs(m_poly[0].tx())>.01||fabs(m_poly[0].ty())>.01||fabs(m_poly[0].tz())>.01){
    glBegin(GL_LINES);
    glVertex3f(-10*GetRobot()->rotation_axis[0],-10*GetRobot()->rotation_axis[1],-10*GetRobot()->rotation_axis[2]);
    glVertex3f(10*GetRobot()->rotation_axis[0],10*GetRobot()->rotation_axis[1],10*GetRobot()->rotation_axis[2]);
    glEnd();
  }
#endif
  glTranslated(-m_poly[0].tx(), -m_poly[0].ty(), -m_poly[0].tz());
  for(size_t i=0; i<m_poly.size(); i++)
    m_poly[i].Draw(_mode);
  glPopMatrix();
}

void 
MultiBodyModel::DrawSelect(){
  glPushMatrix();
  glTranslated(m_poly[0].tx(), m_poly[0].ty(), m_poly[0].tz());
  glTransform();
  glTranslated(-m_poly[0].tx(), -m_poly[0].ty(), -m_poly[0].tz());

  for(size_t i=0; i<m_poly.size(); i++)
    m_poly[i].DrawSelect();

  glPopMatrix();
}

void
MultiBodyModel::SetRenderMode(RenderMode _mode){
  m_renderMode = _mode;

  for(size_t i=0; i<m_poly.size(); i++)
    m_poly[i].SetRenderMode(_mode);
}

void 
MultiBodyModel::SetColor(float r, float g, float b, float a){
  GLModel::SetColor(r,g,b,a);

  for(size_t i=0; i<m_poly.size(); i++)
    m_poly[i].SetColor(r,g,b,a);
}

const float*  MultiBodyModel::GetColor() const{
  const float* c = m_mbInfo.m_mBodyInfo[0].rgb;
  return c;
}

void
MultiBodyModel::Scale(double x, double y, double z){
  GLModel::Scale(x,y,z);
}

vector<string>
MultiBodyModel::GetInfo() const{

  vector<string> info;
  ostringstream temp, os;

  if(m_fixed){
    info.push_back(string("Obstacle"));
    temp << "Position ( "<< tx()<<", "<<ty()<<", "<<tz()<<" )";
  }
  else{
    info.push_back(string("Robot"));
    temp << m_poly.size();
    temp << "Cfg ( ";
    info.push_back(string("Cfg ( "));
    for(size_t i=0; i<m_cfg.size(); i++){
      temp<<m_cfg[i];
      if(i== m_cfg.size()-1)
        temp << " )";
      else
        temp << ", ";
    }
  }
  info.push_back(temp.str());
  return info;
}

//configuration of robot is got from RobotModel::getFinalCfg()
//queryCfg[i] used to print the robot's cfg.
void 
MultiBodyModel::SetCfg(vector<double>& _cfg){
  m_cfg = _cfg;
}
