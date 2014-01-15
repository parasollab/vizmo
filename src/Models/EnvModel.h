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

    BoundaryModel* GetBoundary() {return m_boundary;}
    void SetBoundary(BoundaryModel* _b) {m_boundary = _b;}

    const vector<RegionModel*>& GetRegions() const {return m_regions;}
    void AddRegion(RegionModel* _r) {m_regions.push_back(_r);}
    void DeleteRegion(RegionModel* _r) {m_regions.erase(find(m_regions.begin(), m_regions.end(), _r));}

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

    vector<RegionModel*> m_regions;

    //PMPL environment
    Environment* m_environment;
};

#endif
