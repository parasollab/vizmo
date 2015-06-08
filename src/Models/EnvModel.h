#ifndef ENV_MODEL_H_
#define ENV_MODEL_H_

#include "MPProblem/Environment.h"

#include "BoundaryModel.h"
#include "Model.h"
#include "MultiBodyModel.h"
#include "RegionModel.h"

#include "Utilities/IO.h"

class AvatarModel;
class TempObjsModel;
class UserPathModel;

////////////////////////////////////////////////////////////////////////////////
/// \brief   Owns and renders the PMPL Environment.
/// \details Also owns several auxiliary items, including user input (regions,
///          paths, avatar) and temporary objects.
////////////////////////////////////////////////////////////////////////////////
class EnvModel : public LoadableModel {

  public:

    // Construction
    EnvModel(const string& _filename);
    ~EnvModel();

    // IO functions
    virtual void ParseFile();
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the name of the directory from which the environment was
    ///        loaded.
    string GetModelDataDir() {return  m_modelDataDir;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the name of the directory from which the environment is
    ///        loaded.
    void SetModelDataDir(const string _modelDataDir);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Parse a boundary file input stream to create the environment
    ///        boundary.
    void ParseBoundary(ifstream& _ifs);

    // Access functions
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the PMPL environment.
    Environment* GetEnvironment() {return m_environment;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the total DOF count for all active bodies.
    int GetDOF() {return m_dof;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the center of mass for all bodies in the environment.
    const Point3d& GetCOM() const {return m_centerOfMass;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the approximate environment radius.
    double GetRadius() const {return m_radius;}

    // Multibodies
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the list of all multibodies in the environment.
    const vector<MultiBodyModel*>& GetMultiBodies() const {return m_multibodies;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Add a multibody to the environment.
    void AddMBModel(MultiBodyModel* _newMBI);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Remove a multibody from the environment.
    void DeleteMBModel(MultiBodyModel* _mbl);

    // Boundary
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the environment BoundaryModel.
    BoundaryModel* GetBoundary() {return m_boundary;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the environment BoundaryModel.
    void SetBoundary(BoundaryModel* _b) {m_boundary = _b;}

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
    void AddAttractRegion(RegionModelPtr _r);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Add a new avoid region.
    void AddAvoidRegion(RegionModelPtr _r);
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

    // Temporary Objects
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Add a set of temporary objects to the environment.
    void AddTempObjs(TempObjsModel* _t) {m_tempObjs.push_back(_t);}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Remove a set of temporary objects from the environment.
    void RemoveTempObjs(TempObjsModel* _t);

    // Display functions
    void ChangeColor(); ///< Change all objects' colors randomly.

    // TESTING to save Env. file
    bool SaveFile(const char* _filename); ///< Experimental environment saving.

    // Model functions
    virtual void SetSelectable(bool _s);
    virtual void GetChildren(list<Model*>& _models);
    void Build();
    void Select(GLuint* _index, vector<Model*>& _sel);
    void DrawRender();
    void DrawSelect();
    void DrawSelected() {}
    void Print(ostream& _os) const;

  private:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Questionable helper function for saving environments.
    string SetTransformRight(string _transformString);

    string m_modelDataDir;   ///< Stores the environment data directory.
    bool m_containsSurfaces; ///< Indicates whether the environment has surfaces.

    int m_dof;  ///< The total DOF count for all active bodies.

    vector<MultiBodyModel*> m_multibodies;  ///< Stores all multibodies.

    double m_radius;        ///< Stores an approximate environment radius.
    Point3d m_centerOfMass; ///< Stores the COM for all loaded multibodies.

    BoundaryModel* m_boundary;    ///< Stores the environment boundary model.

    AvatarModel* m_avatar;                     ///< Stores the user avatar.

    vector<RegionModelPtr> m_attractRegions;   ///< Stores attract regions.
    vector<RegionModelPtr> m_avoidRegions;     ///< Stores avoid regions.
    vector<RegionModelPtr> m_nonCommitRegions; ///< Stores non-commit regions.

    vector<UserPathModel*> m_userPaths; ///< Stores user paths.
    vector<TempObjsModel*> m_tempObjs;  ///< Stores temporary objects.

    Environment* m_environment; ///< The PMPL environment.
};

#endif
