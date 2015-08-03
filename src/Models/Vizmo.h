#ifndef VIZMO_H_
#define VIZMO_H_

#include <vector>
#include <string>
using namespace std;

#include "Models/CfgModel.h"
#include "Models/EdgeModel.h"

//class ActiveMultiBodyModel;
class Box;
class DebugModel;
class EnvModel;
template<typename, typename> class MapModel;
class Model;
class PathModel;
class QueryModel;
namespace Haptics {class Manager;}
class SpaceMouseManager;

//Define singleton
class Vizmo;
Vizmo& GetVizmo();

////////////////////////////////////////////////////////////////////////////////
/// \brief Vizmo is the main class that manages interface with PMPL and owns the
/// data for this problem instance.
////////////////////////////////////////////////////////////////////////////////
class Vizmo {

  public:

    Vizmo();
    ~Vizmo();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create the Vizmo components.
    /// \return A \c bool indicating whether the initialization succeeded.
    bool InitModels();
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Initialize PMPL structures from an XML input file.
    /// \param[in] _xmlFilename The input XML filename.
    void InitPMPL(string _xmlFilename);
    void InitPMPL(); ///< Initialize PMPL structures with no input file.
    void Clean();    ///< Clear all models and data.

    void Draw();     ///< Display the OpenGL scene.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Select all objects in the OpenGL scene that lie within the
    /// cropped window area defined by \c _box.
    /// \param[in] _box The cropped window area from which to select objects.
    void Select(const Box& _box);

    // Environment Related Functions
    EnvModel* GetEnv() const {return m_envModel;}
    const string& GetEnvFileName() const {return m_envFilename;}
    void SetEnvFileName(const string& _name) {m_envFilename = _name;}
    void PlaceRobots(); ///< Position the robot in the Environment.

    // Input device manager access
    Haptics::Manager* GetPhantomManager() const {return m_phantomManager;}
    SpaceMouseManager* GetSpaceMouseManager() const {return m_spaceMouseManager;}

    // Roadmap Related Functions
    MapModel<CfgModel, EdgeModel>* GetMap() const {return m_mapModel;}
    const string& GetMapFileName() const {return m_mapFilename;}
    void SetMapFileName(const string& _name) {m_mapFilename = _name;}
    bool IsRoadMapLoaded(){return m_mapModel;}

    // Query Related Functions
    QueryModel* GetQry() const {return m_queryModel;}
    const string& GetQryFileName() const {return m_queryFilename;}
    void SetQryFileName(const string& _name) {m_queryFilename = _name;}
    bool IsQueryLoaded(){ return m_queryModel; }

    // Path Related Functions
    PathModel* GetPath() const {return m_pathModel;}
    const string& GetPathFileName() const {return m_pathFilename;}
    void SetPathFileName(const string& name) {m_pathFilename = name;}
    bool IsPathLoaded() const {return m_pathModel;}

    // Debug Related Functions
    DebugModel* GetDebug() const {return m_debugModel;}
    const string& GetDebugFileName() const {return m_debugFilename;}
    void SetDebugFileName(const string& _name) {m_debugFilename = _name;}
    bool IsDebugLoaded() const {return m_debugModel;}

    const string& GetXMLFileName() const {return m_xmlFilename;}
    void SetXMLFileName(const string& _name) {m_xmlFilename = _name;}

    // Collision Detection Related Functions
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Check a configuration for collision with the environment.
    /// \param[in] _c The configuration to check.
    bool CollisionCheck(CfgModel& _c1);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Check for a valid local plan from one configuration to another.
    /// \param[in] _c1 The start configuration.
    /// \param[in] _c2 The end configuration.
    /// \return A \c pair indicating validity and edge weight.
    pair<bool, double> VisibilityCheck(CfgModel& _c1, CfgModel& _c2);

    vector<Model*>& GetLoadedModels() {return m_loadedModels;}
    vector<Model*>& GetSelectedModels() {return m_selectedModels;}

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the name and label of all Samplers specified in the
    ///        vizmo problem
    /// \return All Sampler name and labels.
    vector<string> GetAllSamplers() const;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the name and label of all MP strategies specified in the
    ///        vizmo problem
    /// \return All MP strategy name and labels.
    vector<string> GetAllStrategies() const;

    // Motion planning related functions
    void SetSeed(long _l) {m_seed = _l;}
    long GetSeed() {return m_seed;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Start a timer.
    /// \param[in] _c The label of the timer to start.
    void StartClock(const string& _c);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Stop a timer.
    /// \param[in] _c The label of the timer to stop.
    void StopClock(const string& _c);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Print the last interval measured by a timer.
    /// \param[in] _c The label of the timer to print.
    /// \param[in] _os The ostream to print to.
    void PrintClock(const string& _c, ostream& _os);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Offset a timer by the last measured interval of another timer.
    /// \param[in] _c1 The label of the timer to adjust.
    /// \param[in] _c2 The label of the reference timer to adjust by.
    /// \param[in] _op The adjustment operation (either \c '+' or \c '-').
    void AdjustClock(const string& _c1, const string& _c2, const string& _op);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the Vizmo MapModel to use the current PMPL RoadmapGraph.
    void SetPMPLMap();
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Execute an interactive or PMPL strategy.
    /// \param[in] _strategy The label of the strategy to use.
    void Solve(const string& _strategy);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the largest distance realized in the current environment.
    /// \return The maximum distance.
    double GetMaxEnvDist();

  private:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Parse the hit buffer and store selected objects into
    /// \ref m_selectedModels.
    /// \param[in] _hit The number of objects in the selection.
    /// \param[in] _buffer The hit buffer.
    /// \param[in] _all Determines whether all objects or only the closest
    ///                 object will be stored in the hit buffer.
    void SearchSelectedItems(int _hit, void* _buffer, bool _all);

    //environment
    EnvModel* m_envModel;   ///< The current environment model.
    string m_envFilename;   ///< The current environment filename.

    //input device managers
    Haptics::Manager* m_phantomManager;     ///< The PHANToM manager.
    SpaceMouseManager* m_spaceMouseManager; ///< The space mouse manager.

    //map
    MapModel<CfgModel, EdgeModel>* m_mapModel;  ///< The current map model.
    string m_mapFilename;                       ///< The current map filename.

    //query
    QueryModel* m_queryModel;   ///< The current query model.
    string m_queryFilename;     ///< The current query filename.

    //path
    PathModel* m_pathModel;     ///< The current path model.
    string m_pathFilename;      ///< The current path filename.

    //debug
    DebugModel* m_debugModel;   ///< The current debug model.
    string m_debugFilename;     ///< The current debug filename.

    //XML File
    string m_xmlFilename;       ///< The current XML filename.

    vector<Model*> m_loadedModels;    ///< The currently loaded models.
    vector<Model*> m_selectedModels;  ///< The currently selected models.

    long m_seed;    ///< The program's random seed.
    map<string, pair<QTime, double> > m_timers; ///< A set of timers for stop-
                                                ///< watch like functionality.
};

#endif
