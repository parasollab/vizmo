#ifndef ENVMODEL_H_
#define ENVMODEL_H_

#include "MPProblem/Environment.h"

#include "BoundaryModel.h"
#include "Model.h"
#include "MultiBodyModel.h"
#include "RegionModel.h"

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

    vector<MultiBodyModel*> GetMultiBodies() {return m_multibodies;}

    //Boundary
    BoundaryModel* GetBoundary() {return m_boundary;}
    void SetBoundary(BoundaryModel* _b) {m_boundary = _b;}

    //Regions
    bool IsNonCommitRegion(RegionModel* _r) const;
    const vector<RegionModel*>& GetAttractRegions() const {return m_attractRegions;}
    const vector<RegionModel*>& GetAvoidRegions() const {return m_avoidRegions;}
    const vector<RegionModel*>& GetNonCommitRegions() const {return m_nonCommitRegions;}
    void AddAttractRegion(RegionModel* _r) {m_attractRegions.push_back(_r);}
    void AddAvoidRegion(RegionModel* _r) {m_avoidRegions.push_back(_r);}
    void AddNonCommitRegion(RegionModel* _r) {m_nonCommitRegions.push_back(_r);}
    void ChangeRegionType(RegionModel* _r, bool _attract);
    void DeleteRegion(RegionModel* _r);

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

    virtual void GetChildren(list<Model*>& _models);

    void BuildModels();
    virtual void SetSelectable(bool _s); //propagate selectability to children
    void Select(GLuint* _index, vector<Model*>& _sel);
    void Draw();
    void DrawSelect() {}
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

    //PMPL environment
    Environment* m_environment;
};

#endif
