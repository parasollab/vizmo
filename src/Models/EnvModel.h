#ifndef ENV_MODEL_H_
#define ENV_MODEL_H_

#include "MPProblem/Environment.h"

#include "BoundaryModel.h"
#include "Model.h"
#include "MultiBodyModel.h"
#include "RegionModel.h"

#include "Utilities/IO.h"

class TempObjsModel;
class UserPathModel;

class EnvModel : public LoadableModel {

  public:
    EnvModel(const string& _filename);
    ~EnvModel();

    //Access functions
    string GetModelDataDir() {return  m_modelDataDir;}
    int GetDOF() {return m_dof;}
    double GetRadius() const {return m_radius;}
    const Point3d& GetCOM() const {return m_centerOfMass;}
    Environment* GetEnvironment() {return m_environment;}

    const vector<MultiBodyModel*>& GetMultiBodies() const {return m_multibodies;}

    //Boundary
    BoundaryModel* GetBoundary() {return m_boundary;}
    void SetBoundary(BoundaryModel* _b) {m_boundary = _b;}

    //Regions
    bool IsNonCommitRegion(RegionModel* _r) const;
    const vector<RegionModel*>& GetAttractRegions() const {return m_attractRegions;}
    const vector<RegionModel*>& GetAvoidRegions() const {return m_avoidRegions;}
    const vector<RegionModel*>& GetNonCommitRegions() const {return m_nonCommitRegions;}
    void AddAttractRegion(RegionModel* _r);
    void AddAvoidRegion(RegionModel* _r);
    void AddNonCommitRegion(RegionModel* _r);
    void ChangeRegionType(RegionModel* _r, bool _attract);
    void DeleteRegion(RegionModel* _r);

    //Paths
    void AddUserPath(UserPathModel* _p) {m_userPaths.push_back(_p);}
    void DeleteUserPath(UserPathModel* _p);

    //Temporary Objects
    void AddTempObjs(TempObjsModel* _t) {m_tempObjs.push_back(_t);}
    void RemoveTempObjs(TempObjsModel* _t);

    //Load functions
    virtual void ParseFile();
    void SetModelDataDir(const string _modelDataDir);
    void NewModelDir();
    void ParseBoundary(ifstream& _ifs);

    //Display functions
    void ChangeColor(); //changes object's color randomly
    void DeleteMBModel(MultiBodyModel* _mbl);
    void AddMBModel(MultiBodyModel* _newMBI);

    // TESTING to save Env. file
    bool SaveFile(const char* _filename);

    virtual void SetSelectable(bool _s); //propagate selectability to children
    virtual void GetChildren(list<Model*>& _models);

    void Build();
    void Select(GLuint* _index, vector<Model*>& _sel);
    void DrawRender();
    void DrawSelect();
    void DrawSelected() {}
    void Print(ostream& _os) const;

  private:
    string m_modelDataDir;
    bool m_containsSurfaces;

    int m_dof;
    string SetTransformRight(string _transformString);

    vector<MultiBodyModel*> m_multibodies;

    double m_radius;
    Point3d m_centerOfMass;

    BoundaryModel* m_boundary;

    vector<RegionModel*> m_attractRegions, m_avoidRegions, m_nonCommitRegions;

    vector<UserPathModel*> m_userPaths;
    vector<TempObjsModel*> m_tempObjs;

    //PMPL environment
    Environment* m_environment;
};

#endif
