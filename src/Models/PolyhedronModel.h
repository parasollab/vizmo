#ifndef POLYHEDRONMODEL_H_
#define POLYHEDRONMODEL_H_

#include "Model.h"

#include <containers/sequential/graph/graph.h>

#include "Matrix.h"

class IModel;

class PolyhedronModel : public Model {
  public:
    typedef Vector<int,  3> Tri;
    typedef vector<Point3d> PtVector;
    typedef vector<Tri> TriVector;

    PolyhedronModel(const string& _filename, double _mass = 1.0, bool _isSurface = false);
    PolyhedronModel(const PolyhedronModel& _p);
    ~PolyhedronModel();

    size_t GetNumVertices() const {return m_numVerts;}
    double GetRadius() const {return m_radius;}
    const Point3d& GetCOM() const {return m_com;}
    const Matrix3x3& GetMoment() const {return m_moment;}

    void Build();
    void Select(GLuint* _index, vector<Model*>& sel) {}
    void DrawRender();
    void DrawSelect();
    void DrawSelected();
    void DrawHaptics();
    void Print(ostream& _os) const;
    void DrawNormals();

  protected:
    //build models, given points and triangles
    void BuildSolidObj(IModel* _model);

    void ComputeNormals(IModel* _model, vector<Vector3d>& _norms);
    void BuildSolidBYU(IModel* _model, const vector<Vector3d>& _norms);

    void BuildNormalsObj(IModel* _model);
    void BuildNormalsBYU(IModel* _model, const vector<Vector3d>& _norms);

    void BuildModelGraph(IModel* _model);
    void BuildWired(IModel* _model, const vector<Vector3d>& _norms);

    //set m_com to center of mass of _points
    void COM(const PtVector& _points);
    //set m_moment to moment of inertia
    void Moment(const PtVector& _points, const TriVector& _tris);
    //set m_radius to distance furthest point in _points to m_com
    void Radius(const PtVector& _points);

  private:
    string m_filename;
    bool m_isSurface;

    size_t m_numVerts;

    GLuint m_solidID; //the compiled model id for solid model
    GLuint m_wiredID; //the compiled model id for wire frame
    GLuint m_normalsID; //the compiled model id for normals

    double m_radius; //radius
    Point3d m_com; //Center of Mass
    double m_mass; //Mass
    Matrix3x3 m_moment; //Moment of Inertia

    typedef stapl::sequential::graph<stapl::UNDIRECTED, stapl::NONMULTIEDGES, int, Vector<int, 2> > ModelGraph;
    ModelGraph m_modelGraph; //model graph for wired model
};

#endif
