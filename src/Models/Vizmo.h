#ifndef VIZMO_H_
#define VIZMO_H_

#include <vector>
#include <string>
using namespace std;

#include "Models/CfgModel.h"
#include "Models/EdgeModel.h"

class Box;
class Model;
class EnvModel;
class RobotModel;
template<typename, typename> class MapModel;
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
    //initialize PMPL data structures for collision checking
    void InitPMPL();
    void Clean();

    //Display OpenGL Scene
    void Draw();

    //Select Objects in OpenGL Scene
    void Select(const Box& _box);

    // Environment Related Functions
    EnvModel* GetEnv() const {return m_envModel;}
    const string& GetEnvFileName() const {return m_envFilename;}
    void SetEnvFileName(const string& name) {m_envFilename = name;}

    // Robot Related Functions
    RobotModel* GetRobot() const {return m_robotModel;}
    //Put robot in start configuration if possible
    void PlaceRobot();

    // Roadmap Related Functions
    MapModel<CfgModel, EdgeModel>* GetMap() const {return m_mapModel;}
    const string& GetMapFileName() const {return m_mapFilename;}
    void SetMapFileName(const string& name) {m_mapFilename = name;}
    bool IsRoadMapLoaded(){return m_mapModel;}

    // Query Related Functions
    QueryModel* GetQry() const {return m_queryModel;}
    const string& GetQryFileName() const {return m_queryFilename;}
    void SetQryFileName(const string& name) {m_queryFilename = name;}
    bool IsQueryLoaded(){ return m_queryModel; }

    // Path Related Functions
    PathModel* GetPath() const {return m_pathModel;}
    const string& GetPathFileName() const {return m_pathFilename;}
    void SetPathFileName(const string& name) {m_pathFilename = name;}
    bool IsPathLoaded() const {return m_pathModel;}

    // Debug Related Functions
    DebugModel* GetDebug() const {return m_debugModel;}
    const string& GetDebugFileName() const {return m_debugFilename;}
    void SetDebugFileName(const string& name) {m_debugFilename = name;}
    bool IsDebugLoaded() const {return m_debugModel;}

    // Collision Detection Related Functions
    bool CollisionCheck(CfgModel& _c1);
    bool VisibilityCheck(CfgModel& _c1, CfgModel& _c2);

    vector<Model*>& GetLoadedModels() {return m_loadedModels;}
    vector<Model*>& GetSelectedModels() {return m_selectedModels;}

    // Motion planning related functions
    void Solve(const string& _strategy);

  private:

    //Parse the Hit Buffer. Store selected obj into m_selectedModels.
    //hit is the number of hit by this selection
    //buffer is the hit buffer
    //if all, all obj select will be put into m_selectedItems,
    //otherwise only the closest will be selected.
    void SearchSelectedItems(int _hit, void* _buffer, bool _all);

    //environment
    EnvModel* m_envModel;
    string m_envFilename;

    //robot
    RobotModel* m_robotModel;

    //map
    MapModel<CfgModel, EdgeModel>* m_mapModel;
    string m_mapFilename;

    //query
    QueryModel* m_queryModel;
    string m_queryFilename;

    //path
    PathModel* m_pathModel;
    string m_pathFilename;

    //debug
    DebugModel* m_debugModel;
    string m_debugFilename;

    typedef vector<Model*>::iterator MIT;
    vector<Model*> m_loadedModels, m_selectedModels;
};

#endif
