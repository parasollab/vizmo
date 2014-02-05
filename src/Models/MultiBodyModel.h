#ifndef MULTIBODYMODEL_H_
#define MULTIBODYMODEL_H_

#include <include/Graph.h>

#include <Transformation.h>
using namespace mathtool;

#include "Model.h"

class BodyModel;
class ConnectionModel;
class EnvModel;

class MultiBodyModel : public Model {
  public:
    typedef vector<ConnectionModel*> Joints;
    typedef pair<BodyModel*, Joints> Robot;
    typedef vector<Robot> Robots;

    struct DOFInfo{
      DOFInfo(string _n, double _min, double _max)
        :m_name(_n), m_minVal(_min), m_maxVal(_max){}

      string m_name;
      double m_minVal;
      double m_maxVal;
    };

    //constructor to be read in from file
    MultiBodyModel(EnvModel* _env);
    //constructor for obstacles
    MultiBodyModel(EnvModel* _env,
        const string& _modelDataDir, const string& _filename,
        const Transformation& _t);
    MultiBodyModel(const MultiBodyModel& _m);
    ~MultiBodyModel();

    //access properties
    virtual void SetRenderMode(RenderMode _mode);
    virtual void SetSelectable(bool _s);
    virtual void SetColor(const Color4& _c);

    bool IsActive() const{return m_active;}
    bool IsSurface() const {return m_surface;}
    const Point3d& GetCOM() const {return m_com;}
    double GetRadius() const {return m_radius;}
    int GetDOF() const {return m_dof;}
    int GetNbBodies() {return m_bodies.size();}
    Robots GetRobots() const {return m_robots;}

    static vector<DOFInfo>& GetDOFInfo() { return m_dofInfo; }
    static void ClearDOFInfo() { m_dofInfo.clear(); } //Clear/reset every time a file is opened

    //access bodies
    typedef vector<BodyModel*>::const_iterator BodyIter;
    BodyIter Begin() const { return m_bodies.begin(); }
    BodyIter End() const { return m_bodies.end(); }
    vector<BodyModel*>& GetBodies() {return m_bodies;}
    vector<ConnectionModel*>& GetJoints() {return m_joints;}

    //drawing
    virtual void Build();
    virtual void Select(GLuint* _index, vector<Model*>& sel);
    virtual void DrawRender();
    virtual void DrawSelect();
    virtual void DrawSelected();
    virtual void Print(ostream& _os) const;

    //IO
    void ParseMultiBody(istream& _is, const string& _modelDir);

    //Edit Robot
    void DeleteBody(int _index);
    void DeleteRobot(int _index);
    void DeleteJoint(int _indexBase, int _indexJoint, int _bodyNumber);
    void DeleteJoints();
    void AddBase(BodyModel* _newBase);
    void AddBody(BodyModel* _b, int _index);
    void AddJoint(ConnectionModel* _c, int _indexBase, int _indexJoint, int _bodyNum);
    void ChangeDOF(int _dof);

  private:
    void BuildRobotStructure();

    EnvModel* m_envModel;

    int m_dof;       //number of DOF for the body
    bool m_active;   //active or passive?
    bool m_surface;  //is surface? default is false
    Point3d m_com;   //center of mass
    double m_radius; //radius

    static vector<DOFInfo> m_dofInfo;  //types/ranges of each DOF
    vector<BodyModel*> m_bodies;       //complete list of all bodies
    vector<ConnectionModel*> m_joints; //complete list of all joints

    typedef stapl::sequential::graph<stapl::UNDIRECTED, stapl::NONMULTIEDGES, size_t> RobotGraph;
    RobotGraph m_robotGraph;
    vector<Robot> m_robots; //list of base/joint set pairs. One for each component of multibody
};

#endif
