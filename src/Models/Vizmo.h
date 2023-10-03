#ifndef VIZMO_H_
#define VIZMO_H_

#include <vector>
#include <string>
using namespace std;

#include <QElapsedTimer>

#include "Models/CfgModel.h"
#include "Models/EdgeModel.h"

#ifdef PMPCfg
#include "MotionPlanning/VizmoTraits.h"
#elif defined(PMPState)
#include "MotionPlanning/VizmoStateTraits.h"
#endif

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
template <typename> class MPLibraryType;

//Define singleton
class Vizmo;
Vizmo& GetVizmo();

////////////////////////////////////////////////////////////////////////////////
/// \brief Vizmo is the main class that manages interface with PMPL and owns the
/// data for this problem instance.
////////////////////////////////////////////////////////////////////////////////
class Vizmo {

  public:

    ///\name Construction
    ///@{

    Vizmo() = default;
    ~Vizmo() {Clean();}

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create the Vizmo components.
    /// \return A \c bool indicating whether the initialization succeeded.
    bool InitModels();

    void InitPMPL(); ///< Initialize PMPL structures.
    void Clean();    ///< Clear all models and data.

    ///@}
    ///\name Rendering
    ///@{

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get all loaded models.
    vector<Model*>& GetLoadedModels() {return m_loadedModels;}

    void Draw();     ///< Display the OpenGL scene.

    ///@}
    ///\name Selection
    ///@{

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get all selected models.
    vector<Model*>& GetSelectedModels() {return m_selectedModels;}

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Select all objects in the OpenGL scene that lie within the
    /// cropped window area defined by \c _box.
    /// \param[in] _box The cropped window area from which to select objects.
    void Select(const Box& _box);

    ///@}
    ///\name Environment Functions
    ///@{

    EnvModel* GetEnv() const {return m_envModel;}
    const string& GetEnvFileName() const {return m_envFilename;}
    void SetEnvFileName(const string& _name) {m_envFilename = _name;}

    void PlaceRobots(); ///< Position the robot in the Environment.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the largest distance realized in the current environment.
    /// \return The maximum distance.
    double GetMaxEnvDist();

    ///@}
    ///\name Input Device Manager Accessors
    ///@{

    Haptics::Manager* GetPhantomManager() const {return m_phantomManager;}
    SpaceMouseManager* GetSpaceMouseManager() const {return m_spaceMouseManager;}

    ///@}
    ///\name Roadmap Functions
    ///@{

    MapModel<CfgModel, EdgeModel>* GetMap() const {return m_mapModel;}
    const string& GetMapFileName() const {return m_mapFilename;}
    void SetMapFileName(const string& _name) {m_mapFilename = _name;}
    void ReadMap(const string& _name);
    bool IsRoadMapLoaded(){return m_mapModel;}

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the Vizmo MapModel to use the current PMPL RoadmapGraph.
    void SetPMPLMap();

    ///@}
    ///\name Query Functions
    ///@{

    QueryModel* GetQry() const {return m_queryModel;}
    const string& GetQryFileName() const {return m_queryFilename;}
    void SetQryFileName(const string& _name) {m_queryFilename = _name;}
    bool IsQueryLoaded(){ return m_queryModel; }

    ///@}
    ///\name Path Functions
    ///@{

    PathModel* GetPath() const {return m_pathModel;}
    const string& GetPathFileName() const {return m_pathFilename;}
    void SetPathFileName(const string& name) {m_pathFilename = name;}
    bool IsPathLoaded() const {return m_pathModel;}

    ///@}
    ///\name Debug Functions
    ///@{

    DebugModel* GetDebug() const {return m_debugModel;}
    const string& GetDebugFileName() const {return m_debugFilename;}
    void SetDebugFileName(const string& _name) {m_debugFilename = _name;}
    bool IsDebugLoaded() const {return m_debugModel;}

    ///@}
    ///\name XML Functions
    ///@{

    const string& GetXMLFileName() const {return m_xmlFilename;}
    void SetXMLFileName(const string& _name) {m_xmlFilename = _name;}

    ///@}
    ///\name Collision Detection Functions
    ///@{

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Check a configuration for collision with the environment.
    /// \param[in] _c The configuration to check.
    bool CollisionCheck(CfgModel& _c1);
    
    bool IsInsideCheck(CfgModel& _c1);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Check for a valid local plan from one configuration to another.
    /// \param[in] _c1 The start configuration.
    /// \param[in] _c2 The end configuration.
    /// \return A \c pair indicating validity and edge weight.
    pair<bool, double> VisibilityCheck(CfgModel& _c1, CfgModel& _c2);

    ///@}
    ///\name Motion Planning Functions
    ///@{

    void SetSeed(long _l) {m_seed = _l;}  ///< Set the random seed for planning.
    long GetSeed() const {return m_seed;} ///< Get the random seed for planning.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the name and label of all Samplers specified in the
    ///        vizmo problem.
    /// \return All Sampler name and labels.
    vector<string> GetAllSamplers() const;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the name and label of all MPStrategies specified in the
    ///        vizmo problem.
    /// \return All MPStrategy name and labels.
    vector<string> GetAllStrategies() const;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Check each avoid region and remove any configurations found
    ///        within.
    void ProcessAvoidRegions();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Execute an interactive or PMPL strategy.
    /// \param[in] _strategy The label of the strategy to use.
    void Solve(const string& _strategy);

    ///@}
    ///\name Timing
    ///@{

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

    ///@}

    MPLibraryType<VizmoTraits>* GetMPLibrary() const {return m_library;}

  private:

    ////////////////////////////////////////////////////////////////////////////
    /// \ingroup Selection
    /// \brief Parse the hit buffer and store selected objects into
    ///        \ref m_selectedModels.
    /// \param[in] _hit The number of objects in the selection.
    /// \param[in] _buffer The hit buffer.
    /// \param[in] _all Determines whether all objects or only the closest
    ///                 object will be stored in the hit buffer.
    void SearchSelectedItems(int _hit, void* _buffer, bool _all);

    ///\name Loaded File Names
    ///@{

    string m_envFilename;       ///< The environment filename.
    string m_mapFilename;       ///< The map filename.
    string m_queryFilename;     ///< The query filename.
    string m_pathFilename;      ///< The path filename.
    string m_debugFilename;     ///< The debug filename.
    string m_xmlFilename;       ///< The XML filename. Defaults to CfgExamples.

    ///@}
    ///\name PMPL Object Models
    ///@{

    EnvModel* m_envModel{nullptr};                     ///< The environment model.
    MapModel<CfgModel, EdgeModel>* m_mapModel{nullptr};///< The map model.
    QueryModel* m_queryModel{nullptr};                 ///< The query model.
    PathModel* m_pathModel{nullptr};                   ///< The path model.
    DebugModel* m_debugModel{nullptr};                 ///< The debug model.

    ///@}
    ///\name GUI Model Containers
    ///@{

    vector<Model*> m_loadedModels;    ///< The currently loaded models.
    vector<Model*> m_selectedModels;  ///< The currently selected models.

    ///@}
    ///\name Input Device Managers
    ///@{

    Haptics::Manager* m_phantomManager{nullptr};     ///< The PHANToM manager.
    SpaceMouseManager* m_spaceMouseManager{nullptr}; ///< The space mouse manager.

    ///@}
    ///\name Other Internal State
    ///@{

    long m_seed;                               ///< The program's random seed.
    map<string, pair<QElapsedTimer, double>> m_timers; ///< Timers.

    MPLibraryType<VizmoTraits>* m_library{nullptr};

    ///@}
};

#endif
