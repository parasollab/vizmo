#ifndef _OBPRMVIEW_ROBOT_H_
#define _OBPRMVIEW_ROBOT_H_

//////////////////////////////////////////////////////////////////////
//Plum Headers
#include <EnvObj/MultiBodyModel.h>
#include <EnvObj/EnvLoader.h>
#include <GLModel.h>
#include <EnvObj/MultiBodyInfo.h>
#include <EnvObj/Transformation.h>
using namespace plum;

#include<Vector.h>
#include<Matrix.h>
using namespace mathtool;

class OBPRMView_Robot : public CGLModel
{
    
public:
    //////////////////////////////////////////////////////////////////////
    // Constructor/Destructor
    //////////////////////////////////////////////////////////////////////
    OBPRMView_Robot(CEnvLoader * pEnv);
    OBPRMView_Robot(const OBPRMView_Robot &other_robot);
    ~OBPRMView_Robot();
    
    //////////////////////////////////////////////////////////////////////
    // Action functions
    //////////////////////////////////////////////////////////////////////
    virtual bool BuildModels();
    
    virtual void Draw( GLenum mode);
    virtual void DrawSelect();

    //set wire/solid to all items
    virtual void SetRenderMode( int mode ){ 
        CGLModel::SetRenderMode(mode);
        if(m_RobotModel!=NULL) m_RobotModel->SetRenderMode(mode); 
    }
    
    virtual void SetColor( float r, float g, float b, float a ){
        CGLModel::SetColor(r,g,b,a);
        if(m_RobotModel!=NULL) m_RobotModel->SetColor(r,g,b,a);
    }

    virtual const string GetName() const { 
      if( m_RobotModel!=NULL ) return "Robot";
      return "No Robot"; 
    }
    
    virtual void GetChildren( list<CGLModel*>& models ){ 
      if( m_RobotModel!=NULL )
	models.push_back(m_RobotModel);
    }
    

    //////////////////////////////////////////////////////////////////////
    // Access Functions
    //////////////////////////////////////////////////////////////////////
    void Configure( double * cfg );
    
/*     void Configure( double * cfg ) { //assum this is rigid */
/*         //setup rotation and translation */
/*         tx()=cfg[0]; ty()=cfg[1]; tz()=cfg[2]; */
/*         rx()=cfg[3]; ry()=cfg[4]; rz()=cfg[5]; */
/*         Euiler2Quaternion(); */
/*     } */
        
    CEnvLoader * getEnvLoader() const;
    CMultiBodyModel * getRobotModel() const;

    void BackUp();
    void Restore();

    //variable used to change the size of the robot
    float size; 
    bool change;
    char come;
    double R, G, B, o_s[3], x, y, z, x1, y1, z1;

    Quaternion quat, q1, q2;

    void Scale(double x, double y, double z)
      {
	if(m_RobotModel!=NULL) 
	  m_RobotModel->Scale(x,y,z);
      }
    
    //////////////////////////////////////////////////////////////////////
    // Private Stuff
    //////////////////////////////////////////////////////////////////////
private:
    
    CEnvLoader * m_pEnvLoader;
    CMultiBodyModel * m_RobotModel;
    CMultiBodyInfo * m_RobotInfo;

    CPolyhedronModel * m_Poly, * pPolyBackUp, * Poly;

};

#endif

