#ifndef _OBPRMVIEW_ROBOT_H_
#define _OBPRMVIEW_ROBOT_H_

//////////////////////////////////////////////////////////////////////
//Plum Headers
#include <EnvObj/MultiBodyModel.h>
#include <EnvObj/EnvLoader.h>
#include <GLModel.h>
using namespace plum;

class OBPRMView_Robot : public CGLModel
{
    
public:
    //////////////////////////////////////////////////////////////////////
    // Constructor/Destructor
    //////////////////////////////////////////////////////////////////////
    OBPRMView_Robot(CEnvLoader * pEnv);
    OBPRMView_Robot(OBPRMView_Robot &other_robot);
    ~OBPRMView_Robot();
    
    //////////////////////////////////////////////////////////////////////
    // Action functions
    //////////////////////////////////////////////////////////////////////
    virtual bool BuildModels();
    
    virtual void Draw( GLenum mode);

    //void Draw( GLenum mode, float s);
    
    //set wire/solid to all items
    virtual void SetRenderMode( int mode ){ 
        CGLModel::SetRenderMode(mode);
	if(m_RobotModel!=NULL) m_RobotModel->SetRenderMode(mode); 
    }
    
    virtual void SetColor( float r, float g, float b, float a ){
        CGLModel::SetColor(r,g,b,a);
	if(m_RobotModel!=NULL) m_RobotModel->SetColor(r,g,b,a);
    }
    //new Aimee 12/11/02
/*      virtual void SetSize(float size = 1.0){ */
/*        CGLModel::SetSize(size); */
/*        if(m_RobotModel!=NULL) m_RobotModel->SetSize(size); */
/*      } */
    
    //////////////////////////////////////////////////////////////////////
    // Access Functions
    //////////////////////////////////////////////////////////////////////
    void Configure( double * cfg ) { //assum this is rigid
        //setup rotation and translation
        tx()=cfg[0]; ty()=cfg[1]; tz()=cfg[2];
		double cx_2=cos(cfg[3]/2); double sx_2=sin(cfg[3]/2);
        double cy_2=cos(cfg[4]/2); double sy_2=sin(cfg[4]/2);
        double cz_2=cos(cfg[5]/2); double sz_2=sin(cfg[5]/2);
        Quaternion qx(cx_2,sx_2*Vector3d(1,0,0));
        Quaternion qy(cy_2,sy_2*Vector3d(0,1,0));
        Quaternion qz(cz_2,sz_2*Vector3d(0,0,1));
        Quaternion nq=qz*qy*qx; //new q
        this->q(nq.normalize()); //set new q
    }

    //variable used to change the size of the robot
    float size; 
    bool change;
    char come;

    CEnvLoader * getEnvLoader();
    CMultiBodyModel * getRobotModel();

    //////////////////////////////////////////////////////////////////////
    // Private Stuff
    //////////////////////////////////////////////////////////////////////
private:
    
    CEnvLoader * m_pEnvLoader;
    CMultiBodyModel * m_RobotModel;
};

#endif

