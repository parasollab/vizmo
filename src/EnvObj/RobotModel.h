#ifndef OBPRMVIEWROBOT_H_
#define OBPRMVIEWROBOT_H_

#include <QKeyEvent>

#include <Vector.h>
#include <Matrix.h>
#include <Transformation.h>

using namespace mathtool;

#include <Plum/EnvObj/MultiBodyModel.h>
#include <Models/EnvModel.h>
#include <Plum/GLModel.h>
#include <Plum/EnvObj/MultiBodyInfo.h>
#include <Utilities/GL/gliCamera.h>

class RobotModel : public GLModel {

  public:

    void Print(){
      std::cout << m_RobotModel->tx() << " "
        << m_RobotModel->ty() << " "
        << m_RobotModel->tz() << " "
        << m_RobotModel->rx() << " "
        << m_RobotModel->ry() << " "
        << m_RobotModel->rz() << endl;
    }


    RobotModel(EnvModel* _env);
    RobotModel(const RobotModel& _otherRobot);
    ~RobotModel();

    EnvModel* GetEnvModel()const { return m_envModel; }

    virtual void BuildModels();
    virtual void Draw(GLenum _mode);
    virtual void DrawSelect();
    virtual void Select( unsigned int * index, vector<GLModel*>& sel );

    //set wire/solid to all items
    virtual void SetRenderMode(RenderMode _mode){
      GLModel::SetRenderMode(_mode);
      if(m_RobotModel!=NULL) m_RobotModel->SetRenderMode(_mode);
    }

    virtual void SetColor( float r, float g, float b, float a ){
      GLModel::SetColor(r,g,b,a);
      if(m_RobotModel!=NULL)
        m_RobotModel->SetColor(r,g,b,a);
    }
    void keepColor(float r, float g, float b);

    float Get_R() {return m_rR;}//return originalR;
    float Get_G() {return m_rG;}//return originalG;
    float Get_B() {return m_rB;}//return originalB;

    virtual const string GetName() const {
      if( m_RobotModel!=NULL ) return "Robot";
      return "No Robot";
    }

    virtual void GetChildren( list<GLModel*>& models ){
      if( m_RobotModel!=NULL )
        models.push_back(m_RobotModel);
    }

    void SaveQry(vector<vector<double> >& cfg, char ch);

    //copy query configuration
    //this info. is used to set the *actual* position and
    //orientation of the robot.
    //void setQueryCfg(double * CfgModel);

    int returnDOF(){ return dof; }
    //////////////////////////////////////////////////////////////////////
    // Access Functions
    //////////////////////////////////////////////////////////////////////
    //configure the Robot to cfg
    void Configure(double * cfg);
    void Configure(vector<double>& _cfg);
    //return current configuration
    vector<double> getFinalCfg();

    MultiBodyModel* getRobotModel() const;
    int getNumJoints();
    void BackUp();
    void Restore();
    //Keep initial Cfg.
    void InitialCfg(vector<double>& cfg);
    void RestoreInitCfg();

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
    vector<vector<double> > StartCfg;
    vector<vector<double> > GoalCfg;
    //////////////////////////////////////////////
    // storeCfg::
    //    cfg the cfg. to store
    //    c tells if cfg. is start or goal
    //////////////////////////////////////////////
    void storeCfg(vector<vector<double> >& cfg, char c, int dof){
      typedef vector<vector<double> >::iterator IC;
      if(c == 's'){
        StartCfg.clear();
        for(IC ic=cfg.begin(); ic!=cfg.end(); ic++){
          StartCfg.push_back(*ic);
        }
      }
      else{
        GoalCfg.clear();
        for(IC ic=cfg.begin(); ic!=cfg.end(); ic++){
          GoalCfg.push_back(*ic);
        }
      }
    }

    double gettx(){
      if(m_RobotModel!=NULL)
        return m_RobotModel->tx();
      else
        return 0;
    }

    double getty(){
      if(m_RobotModel!=NULL)
        return m_RobotModel->ty();
      else
        return 0;
    }

    double gettz(){
      if(m_RobotModel!=NULL)
        return m_RobotModel->tz();
      else
        return 0;
    }




    vector<vector<double> > getNewStartAndGoal(){

      vector<vector<double> > v;
      if(!StartCfg.empty())
        v.push_back(StartCfg[0]);
      if(!GoalCfg.empty())
        v.push_back(GoalCfg[0]);
      return v;

    }

    bool KP( QKeyEvent * e );
    void Transform(int dir);

    vector<double> returnCurrCfg(int dof);

    double * RotFstBody; //keeps rot. angles of body0
    double * queryCfg; //stores query cfg.
    //////////////////////////////////////////////////////////////////////
    // Private Stuff
    //////////////////////////////////////////////////////////////////////
  public:
    int mode;
    double delta;
    double phantomdelta;

    EnvModel* m_envModel;
    const CMultiBodyInfo * m_RobotInfo;
    MultiBodyModel * m_RobotModel;

    double m_polyXBack, m_polyYBack, m_polyZBack;
    vector<PolyhedronModel> m_poly;
    //MultiBodyModel * mbRobotBackUp;
    double * mbRobotBackUp;
    double * tempCfg;
    double * currentCfg;
    double * rotation_axis;
    //double * RotFstBody; //keeps rot. angles of body0
    Quaternion MBq; //keeps rot. for robotModel
    pthread_mutex_t mutex;

    vector<double *> vCfg; //to store start/goal cfgs.
    int dof;
    //to store the START cfg. It comes from Query or Path files
    double * StCfg;
    //store ORIGINAL size and color
    double originalR, originalG, originalB, originalSize[3];
    float m_rR, m_rG, m_rB;
    RenderMode m_RenderModeBackUp;
};

#endif

