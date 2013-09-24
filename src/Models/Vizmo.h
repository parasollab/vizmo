#ifndef VIZMO_H_
#define VIZMO_H_

#include <vector>
#include <string>
using namespace std;

#include <Plum/EnvObj/MultiBodyModel.h>
#include <Utilities/CollisionDetection.h>

class Box;
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
    Vizmo();
    ~Vizmo();

    //Create VizmoObj.
    bool InitModels();
    void Clean();

    //Display OpenGL Scene
    void Display();

    //Select Objects in OpenGL Scene
    void Select(const Box& _box);

    //Animate Robot motion
    void Animate(int frame);
    //Animate map creation
    void AnimateDebug(int frame);

    // Environment Related Functions
    // Change the appearance of an object - Hidden/ Wire / Soid
    void ChangeAppearance(int );
    void RefreshEnv();
    void RandomizeEnvColors();
    bool SaveEnv(const char *filename); //save env. file
    bool SaveQry(const char *filename); //save query file
    void SaveQryStart(); //save start cfg
    //void SaveQryGoal(); //save goal cfg.
    void SaveQryCfg(char ch); //save start or goal cfg.
    void DeleteObject(MultiBodyModel * mbl);
    //called form main_win::autoMkmp()
    double GetEnvRadius();
    EnvModel* GetEnv() const { return m_envModel; }

    // Robot Related Functions
    RobotModel* GetRobot() const {  return m_robotModel; }

    // Roadmap Related Functions
    void ShowRoadMap(bool _show = true);
    bool IsRoadMapShown() const { return m_showMap; }
    bool IsRoadMapLoaded(){return m_mapModel;}
    //void ChangeNodeColor(double _r, double _g, double _b, string _s); //May be
    //used later?
    void RandomizeCCColors();
    MapModel<CfgModel, EdgeModel>* GetMap() const { return m_mapModel; }
    void SetMapObj(MapModel<CfgModel, EdgeModel>* _mm) { m_mapModel = _mm; }

    // Query Related Functions
    void ShowQueryFrame( bool bshow = true );   // to know if the Query has to be showed
    bool IsQueryLoaded(){ return m_queryModel; }
    QueryModel* GetQry() const { return m_queryModel; }

    // Path Related Functions
    void ShowPathFrame( bool bShow=true );
    int GetPathSize();
    bool IsPathLoaded(){ return m_pathModel; }
    PathModel* GetPath() const { return m_pathModel; }

    // Debug Related Functions
    int GetDebugSize();
    bool IsDebugLoaded(){return m_debugModel != NULL;}
    DebugModel* GetDebug() const {return m_debugModel;}

    // Collision Detection Related Functions
    CollisionDetection CD;
    void TurnOn_CD();
    bool IsCDOn();
    CfgModel * m_cfg;
    bool is_collison; //used in roadmap.cpp:printRobCfg()
    bool getCD_value(){ return is_collison;}//used in roadmap.cpp:printRobCfg()
    void Node_CD(double *cfg);
    void Node_CD(CfgModel * cfg);
    vector<double> m_nodeCfg;
    bool m_isNode;

    vector<GLModel*>& GetLoadedModels(){ return m_loadedModels; }
    vector<GLModel*>& GetSelectedModels() {return m_selectedModels;}

    //Miscellaneous
    bool StringToInt(const string &s, int &i);
    void SetDoubleClickStatus(bool _setting) { m_doubleClick = _setting; }
    bool GetDoubleClickStatus() { return m_doubleClick; }

    ////////////////////////////////////////////////////////////////
    // Variables used to change color of objects in the environment.
    float mR, mG, mB;

    /////////////////////////////////////////////////////////////////////
    // Filenames
    const string& getEnvFileName() const { return m_envFilename; }
    const string& getMapFileName() const { return m_mapFilename; }
    const string& getQryFileName() const { return m_queryFilename; }
    const string& getPathFileName() const { return m_pathFilename; }
    const string& getDebugFileName() const { return m_debugFilename; }

    void setEnvFileName(const string& name){ m_envFilename=name; }
    void setMapFileName(const string& name){ m_mapFilename=name; }
    void setQryFileName(const string& name){ m_queryFilename=name; }
    void setPathFileName(const string& name) { m_pathFilename=name; }
    void setDebugFileName(const string& name) { m_debugFilename=name; }

  protected:
    //Put robot in start configuration if possible
    void PlaceRobot();

    //Parse the Hit Buffer. Store selected obj into m_selectedModels.
    //hit is the number of hit by this selection
    //buffer is the hit buffer
    //if all, all obj select will be put into m_selectedItems,
    //otherwise only the closest will be selected.
    void SearchSelectedItems(int _hit, void* _buffer, bool _all);

  private:
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

    typedef vector<GLModel*>::iterator MIT;
    vector<GLModel*> m_loadedModels, m_selectedModels;
    bool m_doubleClick;
};

#endif
