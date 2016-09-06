#ifndef ENV_MODEL_H_
#define ENV_MODEL_H_

#include <QMutex>
#include <QMutexLocker>

#include "Environment/Environment.h"

#include "Model.h"
#include "BoundaryModel.h"

#include "Utilities/IO.h"

class ActiveMultiBodyModel;
class AvatarModel;
class BoundaryModel;
class CfgModel;
class ReebGraphConstruction;
class ReebGraphModel;
class StaticMultiBodyModel;
class SurfaceMultiBodyModel;
class TempObjsModel;
class WorkspaceDecomposition;
class WorkspaceDecompositionModel;
class UserPathModel;

////////////////////////////////////////////////////////////////////////////////
/// \brief   Owns and renders the PMPL Environment.
///
/// Also owns several auxiliary items, including user input (regions, paths,
/// avatar) and temporary objects.
////////////////////////////////////////////////////////////////////////////////
class EnvModel : public Model {

  public:

    enum EnvObjectName : GLuint {
      BoundaryObj,
      Robots,
      Obstacles,
      Surfaces,
      AttractRegions,
      AvoidRegions,
      NonCommitRegions,
      UserPaths,
      Decomposition,
      ReebGraph
    };

    // Construction
    EnvModel(const string& _filename);
    EnvModel(Environment* _env);
    ~EnvModel();

    // Access functions
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the PMPL environment.
    Environment* GetEnvironment() {return m_environment;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the center of mass for all bodies in the environment.
    const Point3d& GetCOM() const {return m_centerOfMass;}
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get center of boundary
    const Point3d& GetCenter() const;
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the approximate environment radius.
    double GetRadius() const {return m_radius;}
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Are all robots planar?
    bool IsPlanar() const;

    // Robot functions
    ////////////////////////////////////////////////////////////////////////////
    /// @param _i Index of robot
    /// @return Robot model of index @p _i
    shared_ptr<ActiveMultiBodyModel> GetRobot(size_t _i) {return m_robots[_i];}
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Initialize robot model positions in environment
    /// @param _cfgs CfgModels to initialize
    /// @param _invisible Are robot models initially invisible?
    void PlaceRobots(vector<CfgModel>& _cfgs, bool _invisible);
    ////////////////////////////////////////////////////////////////////////////
    /// @param _c CfgModel to position in environment
    void ConfigureRender(const CfgModel& _c);

    // Obstacles
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Add obstacle to environment
    /// @param _dir Directory for geometry file
    /// @param _filename Geometry filename
    /// @param _t Transformation of object
    /// @return Pointer to newly created obstacle
    shared_ptr<StaticMultiBodyModel> AddObstacle(const string& _dir,
        const string& _filename, const Transformation& _t);
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Remove obstacle from the environment
    void DeleteObstacle(StaticMultiBodyModel* _m);

    // Boundary
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the environment BoundaryModel.
    shared_ptr<BoundaryModel> GetBoundary() {return m_boundary;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the environment BoundaryModel.
    void SetBoundary(shared_ptr<BoundaryModel> _b);
    bool InBoundary(const Point3d& _p) const {
      return m_boundary->GetBoundary()->InBoundary(_p);
    }

    // Avatar
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the user avatar.
    AvatarModel* GetAvatar() {return m_avatar;}

    // Regions
    typedef shared_ptr<RegionModel> RegionModelPtr; ///< Shared region pointer.
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Check if a region is non-commit.
    bool IsNonCommitRegion(RegionModelPtr _r) const;
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get all attract regions.
    const vector<RegionModelPtr>& GetAttractRegions() const {
      return m_attractRegions;
    }
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get all avoid regions.
    const vector<RegionModelPtr>& GetAvoidRegions() const {
      return m_avoidRegions;
    }
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get all non-commit regions.
    const vector<RegionModelPtr>& GetNonCommitRegions() const {
      return m_nonCommitRegions;
    }
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Add a new attract region.
    void AddAttractRegion(RegionModelPtr _r, bool _lock = true);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Add a new avoid region.
    void AddAvoidRegion(RegionModelPtr _r, bool _lock = true);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Add a new non-commit region.
    void AddNonCommitRegion(RegionModelPtr _r);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Change a non-commit region to attract or avoid.
    void ChangeRegionType(RegionModelPtr _r, bool _attract);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Delete a region of any type.
    void DeleteRegion(RegionModelPtr _r);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get a region pointer from a region model base pointer.
    RegionModelPtr GetRegion(Model* _model);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Save all regions to file
    /// \param _filename File name
    void SaveRegions(const string& _filename);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Load regions from file
    /// \param _filename File name
    void LoadRegions(const string& _filename);

    //Paths
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get all user paths.
    const vector<UserPathModel*>& GetUserPaths() const {return m_userPaths;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Add a new user path.
    void AddUserPath(UserPathModel* _p) {m_userPaths.push_back(_p);}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Remove a user path.
    void DeleteUserPath(UserPathModel* _p);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Save all user paths to file
    /// \param _filename File name
    void SaveUserPaths(const string& _filename);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Load user path from file
    /// \param _filename File name
    void LoadUserPaths(const string& _filename);

    // Temporary Objects
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Add a set of temporary objects to the environment.
    void AddTempObjs(TempObjsModel* _t) {m_tempObjs.push_back(_t);}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Remove a set of temporary objects from the environment.
    void RemoveTempObjs(TempObjsModel* _t);

    // Decomposition
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Add workspace decomposition model to environment
    void AddWorkspaceDecompositionModel(const WorkspaceDecomposition* _wd);
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Add Reeb Graph Model to environment
    void AddReebGraphModel(ReebGraphConstruction* _reebGraph);

    // Display functions
    void ChangeColor(); ///< Change all objects' colors randomly.

    // Model functions
    virtual void SetSelectable(bool _s);
    virtual void GetChildren(list<Model*>& _models);
    void Build();
    void Select(GLuint* _index, vector<Model*>& _sel);
    void DrawRender();
    void DrawSelect();
    void DrawSelected() {}
    void Print(ostream& _os) const;

    void SaveFile(const string& _filename) const;

  private:

    vector<shared_ptr<ActiveMultiBodyModel>> m_robots;    ///< All robots.
    vector<shared_ptr<StaticMultiBodyModel>> m_obstacles; ///< All obstacles.
    vector<shared_ptr<SurfaceMultiBodyModel>> m_surfaces; ///< All surfaces.

    double m_radius{0};     ///< Stores an approximate environment radius.
    Point3d m_centerOfMass; ///< Stores the COM for all loaded multibodies.

    shared_ptr<BoundaryModel> m_boundary;    ///< Stores the environment boundary model.

    AvatarModel* m_avatar{nullptr};            ///< The user avatar.

    vector<RegionModelPtr> m_attractRegions;   ///< Attract regions.
    vector<RegionModelPtr> m_avoidRegions;     ///< Avoid regions.
    vector<RegionModelPtr> m_nonCommitRegions; ///< Non-commit regions.
    mutable QMutex m_regionLock;               ///< Region Lock

    vector<UserPathModel*> m_userPaths;        ///< User paths.
    vector<TempObjsModel*> m_tempObjs;         ///< Temporary objects.

    WorkspaceDecompositionModel* m_decompositionModel{nullptr}; ///< Decomp Model.
    ReebGraphModel* m_reebGraphModel{nullptr};        ///< Reeb Graph Model.

    Environment* m_environment{nullptr}; ///< The PMPL environment.
};

#endif
