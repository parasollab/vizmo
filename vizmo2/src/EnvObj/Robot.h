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
    virtual void Select( unsigned int * index, vector<gliObj>& sel );

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
    //Keep initial Cfg. 
    void InitialCfg(double * cfg);
    void RestoreInitCfg();

 /*    ///Translation */
/*     //@{ */
/*     double& tx(){ ObjName="Robot"; */
/*       m_poly = m_RobotModel->GetPolyhedron(); */
/*       //printf("x = %.2f", m_poly[1].tx()); */
/*       return m_poly[0].tx(); */
/*     } */
/*     double& ty(){ */
/*       //m_poly = m_RobotModel->GetPolyhedron(); */
/*       // printf("y = %.2f", m_poly[1].ty()); */
/*       return m_poly[0].ty(); */
/*       } */
/*     double& tz(){ ObjName="Robot"; */
/*     m_poly = m_RobotModel->GetPolyhedron(); */
/*     //printf("z = %.2f", m_poly[1].tz()); */
/*     //m_poly = m_RobotModel->GetPolyhedron(); */
/*     return m_poly[0].tz(); */
/*     } */
   
    //@}

/*     ///Sacle */
/*     ///@{ */
/*     const double& sx() const { return m_Scale[0]; } */
/*     const double& sy() const { return m_Scale[1]; } */
/*     const double& sz() const { return m_Scale[2]; } */
/*     ///@} */

    ///Rotation
    ///@{
/*     double& rx(){ m_poly = m_RobotModel->GetPolyhedron(); */
/*     cout<<"RX: "<<m_poly[0].rx()<<endl; return m_poly[0].rx();} */
/*     double& ry(){  m_poly = m_RobotModel->GetPolyhedron(); */
/*     cout<<"RY: "<<m_poly[0].ry()<<endl;  */
/*     return m_poly[0].ry(); } */
/*     double& rz(){ m_poly = m_RobotModel->GetPolyhedron(); */
/*     cout<<"RZ: "<<m_poly[0].rz()<<endl; return m_poly[0].rz(); } */
/*     //@} */

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

    //////////////////////////////////////////////////////
    //To store start/goal cfgs for NEW Query
    /////////////////////////////////////////////////////
    vector<double *> StartCfg;
    vector<double *> GoalCfg;
    ////   storeCfg:: 
    //           cfg the cfg. to store    
    //           c tell if cfg. is start or goal     
    void storeCfg( vector<double *> cfg, char c, int dof){
      typedef vector<double*>::iterator IC;
      double * iCfg;

      printf("Cfg inRobot::");

      if(c == 's'){
	StartCfg.clear();
	for(IC ic=cfg.begin(); ic!=cfg.end(); ic++){
	  iCfg = (double *)(*ic);
	  StartCfg.push_back(iCfg);
	}

      for(int i=0; i<dof; i++){
	printf("%2f ",StartCfg[0][i]);
      }
      cout<<endl;

      }
      
      else{
	GoalCfg.clear();
	for(IC ic=cfg.begin(); ic!=cfg.end(); ic++){
	  iCfg = (double *)(*ic);
	  GoalCfg.push_back(iCfg);
	}

      for(int i=0; i<dof; i++){
	printf("%2f ",GoalCfg[0][i]);
      }
      cout<<endl;
      }  
     

    }


    vector<double *> getNewStartAndGoal(){

      vector<double *> v;
      if(!StartCfg.empty())
	v.push_back(StartCfg[0]);
      if(!GoalCfg.empty())
	v.push_back(GoalCfg[0]);
      return v;

    }

    double *returnCurrCfg(int dof);

    void getCurrCfg(int dof, bool hand, double * cfg, CGLModel * gl){
      
      list<CGLModel*> robotList;
      CGLModel * rl;
      double *currCfg = new double[dof];
      //get robot polyhedron
      m_RobotModel->GetChildren(robotList);
      if(!hand){
	typedef list<CGLModel *>::iterator RI;
 	for(RI i=robotList.begin(); i!=robotList.end(); i++){
	  rl = (CGLModel*)(*i);
	  currCfg[0]=rl->tx();currCfg[1]=rl->ty();currCfg[2]=rl->tz();
	  currCfg[3]=rl->rx();currCfg[4]=rl->ry();currCfg[5]=rl->rz();
	}
      }
      else{ //user moved robot by hand
	//this is for rigid bodies only!!
	currCfg[0] = gl->tx() + cfg[0];
	currCfg[1] = gl->ty() + cfg[1];
	currCfg[2] = gl->tz() + cfg[2];
	currCfg[3] = gl->rx();
	currCfg[4] = gl->ry();
	currCfg[5] = gl->rz();
      }
      
      vCfg.push_back(currCfg);
      
    }
    //////////////////////////////////////////////////////////////////////
    // Private Stuff
    //////////////////////////////////////////////////////////////////////
private:
    
    CEnvLoader * m_pEnvLoader;
    CMultiBodyModel * m_RobotModel;
    CMultiBodyInfo * m_RobotInfo;

    CPolyhedronModel * pPolyBackUp, * Poly;// * m_poly;
    //CMultiBodyModel * mbRobotBackUp;
    double * mbRobotBackUp;
    double * RotFstBody; //keeps rot. angles of body0
    Quaternion MBq; //keeps rot. for robotModel

    vector<double *> vCfg; //to store start/goal cfgs.
    int dof;
    //to store the START cfg. It comes from Query or Path files
    double * StCfg;
    //store ORIGINAL size and color
    double originalR, originalG, originalB, originalSize[3];

};

#endif

