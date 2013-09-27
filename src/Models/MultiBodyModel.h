#ifndef MULTIBODYMODEL_H_
#define MULTIBODYMODEL_H_

#include <graph.h>

#include "Model.h"

class BodyModel;
class ConnectionModel;

class MultiBodyModel : public Model {
  public:
    typedef vector<ConnectionModel*> Joints;
    typedef pair<BodyModel*, Joints> Robot;
    typedef vector<Robot> Robots;

    MultiBodyModel();
    ~MultiBodyModel();

    //access properties
    virtual void GetChildren(list<Model*>& _models);
    virtual void SetRenderMode(RenderMode _mode);
    virtual void SetColor(const Color4& _c);

    bool IsActive() const{return m_active;}
    const Point3d& GetCOM() const {return m_com;}
    double GetRadius() const {return m_radius;}
    int GetDOF() const {return m_dof;}
    const Robots& GetRobots() const {return m_robots;}

    //access bodies
    typedef vector<BodyModel*>::const_iterator BodyIter;
    BodyIter Begin() const {return m_bodies.begin();}
    BodyIter End() const {return m_bodies.end();}

    //drawing
    virtual void BuildModels();
    virtual void Select(GLuint* _index, vector<Model*>& sel);
    virtual void Draw(GLenum _mode);
    virtual void DrawSelect();
    virtual void Print(ostream& _os) const;

    //IO
    void ParseMultiBody(istream& _is, const string& _modelDir);

  private:
    void BuildRobotStructure();

    int m_dof; //number of DOF for the body
    bool m_active; //Active or passive?
    bool m_surface;//is surface? default is false
    Point3d m_com; // center of mass
    double m_radius; //Radius

    vector<BodyModel*> m_bodies; //complete list of all bodies
    vector<ConnectionModel*> m_joints; //complete list of all joints

    typedef stapl::graph<stapl::UNDIRECTED, stapl::NONMULTIEDGES, size_t> RobotGraph;
    RobotGraph m_robotGraph;
    vector<Robot> m_robots; //list of base/joint set pairs. One for each component of multibody
};

#endif
