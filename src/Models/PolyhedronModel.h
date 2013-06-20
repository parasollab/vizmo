#ifndef POLYHEDRONMODEL_H_
#define POLYHEDRONMODEL_H_

#include "Plum/GLModel.h"
#include "Plum/EnvObj/MultiBodyInfo.h"

#include <Point.h>
using namespace mathtool;

class RAPID_model;

class PolyhedronModel : public plum::CGLModel {
  public:
    typedef Vector<int,  3> Tri;
    typedef vector<Point3d> PtVector;
    typedef vector<Tri> TriVector;

    PolyhedronModel(plum::CBodyInfo& _bodyInfo);
    ~PolyhedronModel();

    const string GetName() const;
    vector<string> GetInfo() const;

    double GetRadius() const {return m_radius;}
    const Point3d& GetCOM() const {return m_com;}

    RAPID_model* GetRapidModel() {return m_rapidModel;}

    bool BuildModels();
    void Draw(GLenum _mode);
    void DrawSelect();

  protected:
    //build models, given points and triangles
    void ComputeNormals(const PtVector& _points, const TriVector& _tris, vector<Vector3d>& _norms);
    void BuildRapid(const PtVector& _points, const TriVector& _tris);
    void BuildSolid(const PtVector& _points, const TriVector& _tris, const vector<Vector3d>& _norms);
    bool BuildWired(const PtVector& _points, const TriVector& _tris, const vector<Vector3d>& _norms);

    //set m_com to center of mass of _points
    void COM(const PtVector& _points);
    //set m_radius to distance furthest point in _points to m_com
    void Radius(const PtVector& _points);

  private:
    GLuint m_solidID; //the compiled model id for solid model
    GLuint m_wiredID; //the compiled model id for wire frame

    double m_radius; //radius
    Point3d m_com; //Center of Mass

    plum::CBodyInfo m_bodyInfo;
    RAPID_model* m_rapidModel;
};

#endif
