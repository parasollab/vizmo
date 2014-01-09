#ifndef POLYHEDRONMODEL_H_
#define POLYHEDRONMODEL_H_

#include "Graph.h"

#include "Model.h"

class PolyhedronModel : public Model {
  public:
    typedef Vector<int,  3> Tri;
    typedef vector<Point3d> PtVector;
    typedef vector<Tri> TriVector;

    PolyhedronModel(const string& _filename, bool _isSurface = false);
    PolyhedronModel(const PolyhedronModel& _p);
    ~PolyhedronModel();

    double GetRadius() const {return m_radius;}
    const Point3d& GetCOM() const {return m_com;}

    void BuildModels();
    void Select(GLuint* _index, vector<Model*>& sel) {}
    void Draw();
    void DrawSelect();
    void Print(ostream& _os) const;

  protected:
    //build models, given points and triangles
    void ComputeNormals(const PtVector& _points, const TriVector& _tris, vector<Vector3d>& _norms);
    void BuildSolid(const PtVector& _points, const TriVector& _tris, const vector<Vector3d>& _norms);
    void BuildWired(const PtVector& _points, const TriVector& _tris, const vector<Vector3d>& _norms);
    void BuildModelGraph(const PtVector& _points, const TriVector& _tris);

    //set m_com to center of mass of _points
    void COM(const PtVector& _points);
    //set m_radius to distance furthest point in _points to m_com
    void Radius(const PtVector& _points);

  private:
    string m_filename;
    bool m_isSurface;

    GLuint m_solidID; //the compiled model id for solid model
    GLuint m_wiredID; //the compiled model id for wire frame

    double m_radius; //radius
    Point3d m_com; //Center of Mass

    typedef stapl::sequential::graph<stapl::UNDIRECTED, stapl::NONMULTIEDGES, int, Vector<int, 2> > ModelGraph;
    ModelGraph m_modelGraph; //model graph for wired model
};

#endif
