#include "MultiBodyModel.h"

namespace plum{

  //////////////////////////////////////////////////////////////////////
  // Cons/Des
  MultiBodyModel::MultiBodyModel(const CMultiBodyInfo & MBInfo)
    : m_MBInfo(MBInfo){
      SetColor(0.4f,0.4f,0.4f,1);
      m_bFixed = !MBInfo.m_active;
      m_radius = 0;
    }

  MultiBodyModel::~MultiBodyModel() {}

  //////////////////////////////////////////////////////////////////////
  // Core
  //////////////////////////////////////////////////////////////////////
  bool MultiBodyModel::BuildModels() {
    
    //build for each body and compute com     
    for(size_t i = 0; i< m_MBInfo.m_cNumberOfBody; i++){
      CBodyInfo& info = m_MBInfo.m_pBodyInfo[i];
      //only build fixed, free body will not be built (when m_bFixed is set)
      if(!info.m_bIsFixed && m_bFixed == true)
        continue; 
      m_poly.push_back(PolyhedronModel(info));

      if(m_poly[i].BuildModels() == false){
        cout<<"Couldn't build models in Polyhedron class"<<endl;
        return false;
      }

      m_poly[i].SetColor(info.rgb[0],info.rgb[1],info.rgb[2],1);

      m_COM[0]+=info.m_X;
      m_COM[1]+=info.m_Y;
      m_COM[2]+=info.m_Z;
    }   

    for(int i=0; i<3; i++) 
      m_COM[i] /= (double)m_poly.size();
    
    //set position of multi-body as com
    tx()=m_COM[0]; ty()=m_COM[1]; tz()=m_COM[2];

    //compute radius
    for(size_t i=0; i<m_poly.size(); i++ ){
      CBodyInfo & info=m_MBInfo.m_pBodyInfo[i];
      //only build fixed, free body will not be build (when m_bFixed is set)
      if( !info.m_bIsFixed && m_bFixed==true )
        continue;

      double dist = (Point3d(info.m_X,info.m_Y,info.m_Z)-m_COM).norm()
        + m_poly[i].GetRadius();

      if(m_radius < dist) m_radius = dist;
      
      //change to local coorindate of multibody
      m_poly[i].tx()-=tx(); 
      m_poly[i].ty()-=ty(); 
      m_poly[i].tz()-=tz();
    }
    return true;
  }

  void MultiBodyModel::Select(unsigned int* index, vector<gliObj>& sel){
    if(index!=NULL)
      sel.push_back(this);
  }


  //Draw
  void MultiBodyModel::Draw( GLenum mode ) {
    
    float* arr_m_RGBA = &m_RGBA[0];
    glColor4fv(arr_m_RGBA);
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
      m_poly[i].Draw(mode);
    glPopMatrix();
  }



  void MultiBodyModel::DrawSelect() {
    glPushMatrix();
    glTranslated(m_poly[0].tx(), m_poly[0].ty(), m_poly[0].tz());
    glTransform();
    glTranslated(-m_poly[0].tx(), -m_poly[0].ty(), -m_poly[0].tz());

    for(size_t i=0; i<m_poly.size(); i++)
      m_poly[i].DrawSelect();
    
    glPopMatrix();
  }

  void
    MultiBodyModel::SetRenderMode(RenderMode _mode) {
      m_renderMode = _mode;

      for(size_t i=0; i<m_poly.size(); i++)
        m_poly[i].SetRenderMode(_mode);
    }

  void MultiBodyModel::SetColor(float r, float g, float b, float a) {
    CGLModel::SetColor(r,g,b,a);
    
    for(size_t i=0; i<m_poly.size(); i++)
      m_poly[i].SetColor(r,g,b,a);
  }

  const float *  MultiBodyModel::GetColor() const{
    const float * c = m_MBInfo.m_pBodyInfo[0].rgb;
    return c;
  }

  void MultiBodyModel::Scale(double x, double y, double z) {
    CGLModel::Scale(x,y,z);
  }

  vector<string> 
  MultiBodyModel::GetInfo() const {	
    
    vector<string> info; 
    ostringstream temp, os;
    
    if(m_bFixed){
      info.push_back(string("Obstacle"));
      temp << "Position ( "<< tx()<<", "<<ty()<<", "<<tz()<<" )";
    }
    
    else {	
      info.push_back(string("Robot"));
      temp << m_poly.size();
      temp << "Cfg ( ";
      info.push_back(string("Cfg ( "));
      for(size_t i=0; i<m_cfg.size(); i++){
        temp<<m_cfg[i];
        if(i == m_cfg.size()-1)
          temp << " )";
        else
          temp << ", ";
      }
    }
    info.push_back(temp.str());
    return info;
  }


  //configuration of robot is got from OBPRMView_Robot::getFinalCfg()
  //queryCfg[i] used to print the robot's cfg.
  void MultiBodyModel::SetCfg(vector<double>& _cfg){
    m_cfg = _cfg;
  }
}//namespace plum
