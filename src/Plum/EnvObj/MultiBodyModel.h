#ifndef MULTIBODYMODEL_H_
#define MULTIBODYMODEL_H_

#include "Plum/GLModel.h"
#include "RobotInfo.h"

class BodyModel;

class MultiBodyModel : public GLModel{
  public:
    MultiBodyModel();
    ~MultiBodyModel();

    //access properties
    virtual const string GetName() const{return "MultiBody";}
    virtual vector<string> GetInfo() const;
    virtual void GetChildren(list<GLModel*>& _models);
    bool IsActive() const{return m_active;}
    const Point3d& GetCOM() const {return m_com;}
    double GetRadius() const {return m_radius;}
    virtual void SetRenderMode(RenderMode _mode);
    virtual void SetColor(const Color4& _c);

    //access bodies
    typedef vector<BodyModel*>::const_iterator BodyIter;
    BodyIter Begin() const {return m_bodies.begin();}
    BodyIter End() const {return m_bodies.end();}

    //access joints
    const Robot::JointMap& GetJointMap() const {return m_jointMap;}

    //drawing
    virtual void BuildModels();
    virtual void Draw(GLenum _mode);
    virtual void DrawSelect();
    virtual void Select(unsigned int* _index, vector<GLModel*>& sel);

    //IO
    void ParseMultiBody(istream& _is, const string& _modelDir);

  private:
    bool m_active; //Active or passive?
    bool m_surface;//is surface? default is false
    Point3d m_com; // center of mass
    double m_radius; //Radius

    vector<BodyModel*> m_bodies;
    Robot::JointMap m_jointMap;
};

#endif
