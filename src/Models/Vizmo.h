#ifndef VIZMO_H_
#define VIZMO_H_

#include <vector>
#include <string>
using namespace std;

#include <Plum/Plum.h>
#include <Plum/EnvObj/MultiBodyModel.h>
#include <Utilities/CollisionDetection.h>
using namespace plum;

class EnvModel;
class RobotModel;
class CfgModel;
class EdgeModel;
template<typename, typename>
class MapModel;
class QueryModel;
class PathModel;
class DebugModel;

//Define singleton
class Vizmo;
Vizmo& GetVizmo();

/* Vizmo
 *
 * Main class that handles event from gui and
 * contains objects for data loaded in.
 */
class Vizmo {
  public:
    /* VizmoObj
     *
     * This class holds all Vizmo objects.
     * When an item is NULL, it means this item is not availiable for
     * current session.
     * Only Vizmo can create this object.
     */
    struct VizmoObj {
      VizmoObj();
      ~VizmoObj();

      void Clean();

      //environment
      EnvModel* m_envModel;
      string m_envFilename;
      //robot
      RobotModel* m_robotModel;
      //map
      MapModel<CfgModel, EdgeModel>* m_mapModel;
      bool m_showMap;
      string m_mapFilename;
      //query
      QueryModel* m_queryModel;
      bool m_showQuery;
      string m_queryFilename;
      //path
      PathModel* m_pathModel;
      bool m_showPath;
      string m_pathFilename;
      //debug
      DebugModel* m_debugModel;
      string m_debugFilename;
    };

    Vizmo();

    //This file locates all possible filename related to OBPRM.
    //For example, *.path, *.env, *.query...etc.
    void GetAccessFiles(const string& _filename);

    //Create VizmoObj.
    bool InitVizmoObj();

    //Display OpenGL Scene
    void Display(){ m_plum.Draw();}

    //Select Objects in OpenGL Scene
    void Select(const gliBox& _box){m_plum.Select(_box);}

    //Animate Robot motion
    void Animate(int frame);
    //Animate map creation
    void AnimateDebug(int frame);

    // Environment Related Functions
    // Change the appearance of an object - Hidden/ Wire / Soid
    void ChangeAppearance(int );
    void RefreshEnv();
    void envObjsRandomColor();
    bool SaveEnv(const char *filename); //save env. file
    bool SaveQry(const char *filename); //save query file
    void SaveQryStart(); //save start cfg
    //void SaveQryGoal(); //save goal cfg.
    void SaveQryCfg(char ch); //save start or goal cfg.
    void DeleteObject(MultiBodyModel * mbl);
    //called form main_win::autoMkmp()
    bool m_envChanged;
    bool EnvChanged();
    bool IsEnvChanged() { return m_envChanged; }
    double GetEnvRadius();
    EnvModel* GetEnv() const { return m_obj.m_envModel; }

    // Robot Related Functions
    // get the robot's cfg. to be drawn in the toolbar
    //getRoboCfg() called from scene_win::mouseMoveEvent()
    void getRoboCfg();
    vector<string> info;
    //getRobCfgText() called from roadmap::printRobCfg()
    vector<string> getRobCfgText(){return info;}
    int getNumJoints();
    RobotModel* GetRobot() const {  return m_obj.m_robotModel; }

    // Roadmap Related Functions
    void ShowRoadMap(bool _show = true);
    bool IsRoadMapShown() const { return m_obj.m_showMap; }
    void ChangeNodesSize(float _s, string _str);
    void ChangeEdgeThickness(size_t _t);
    void ChangeNodesShape(string _s);
    bool IsRoadMapLoaded(){return m_obj.m_mapModel;}
    void ChangeCCColor(double _r, double _g, double _b, string _s);
    void ChangeNodeColor(double _r, double _g, double _b, string _s);
    void UpdateSelection();
    void ChangeNodesRandomColor();
    MapModel<CfgModel, EdgeModel>* GetMap() const {return m_obj.m_mapModel;}
    void SetMapObj(MapModel<CfgModel, EdgeModel>* _mm);

    // Query Related Functions
    void ShowQueryFrame( bool bshow = true );   // to know if the Query has to be showed
    bool IsQueryLoaded(){ return m_obj.m_queryModel; }
    QueryModel* GetQry() const { return m_obj.m_queryModel; }

    // Path Related Functions
    void ShowPathFrame( bool bShow=true );
    int GetPathSize();
    bool IsPathLoaded(){ return m_obj.m_pathModel; }
    PathModel* GetPath() const { return m_obj.m_pathModel; }

    // Debug Related Functions
    int GetDebugSize();
    bool IsDebugLoaded(){return m_obj.m_debugModel != NULL;}
    DebugModel* GetDebug() const {return m_obj.m_debugModel;}

    // Collision Detection Related Functions
    CollisionDetection CD;
    void TurnOn_CD();
    bool IsCDOn();
    CfgModel * m_cfg;
    bool is_collison; //used in roadmap.cpp:printRobCfg()
    bool getCD_value(){ return is_collison;}//used in roadmap.cpp:printRobCfg()
    void Node_CD(double *cfg);
    void Node_CD(CfgModel * cfg);
    double * m_nodeCfg;
    bool m_IsNode;

    //plum
    vector<GLModel*>& GetGLModels(){ return m_plum.GetGLModels(); }
    vector<GLModel*>& GetSelectedItems() {return m_plum.GetSelectedItems();}

    //Miscelaneous
    bool StringToInt(const string &s, int &i);
    bool oneColor;

    ////////////////////////////////////////////////////////////////
    // Variables used to change color of objects in the environment.
    float mR, mG, mB;

    /////////////////////////////////////////////////////////////////////
    // Filenames
    const string& getEnvFileName() const { return m_obj.m_envFilename; }
    const string& getMapFileName() const { return m_obj.m_mapFilename; }
    const string& getQryFileName() const { return m_obj.m_queryFilename; }
    const string& getPathFileName() const { return m_obj.m_pathFilename; }
    const string& getDebugFileName() const { return m_obj.m_debugFilename; }

    void setEnvFileName(const string& name){ m_obj.m_envFilename=name; }
    void setMapFileName(const string& name){ m_obj.m_mapFilename=name; }
    void setQryFileName(const string& name){ m_obj.m_queryFilename=name; }
    void setPathFileName(const string& name) { m_obj.m_pathFilename=name; }
    void setDebugFileName(const string& name) { m_obj.m_debugFilename=name; }

  protected:
    //Put robot in start configuration if possible
    void PlaceRobot();

  private:
    VizmoObj m_obj;
    Plum m_plum;
};

#endif
