#ifndef MULTI_BODY_MODEL_H_
#define MULTI_BODY_MODEL_H_

#include <algorithm>
#include <containers/sequential/graph/graph.h>

#include <Transformation.h>
using namespace mathtool;

#include "Environment/MultiBody.h"

#include "Model.h"

class BodyModel;
class ConnectionModel;
class EnvModel;

class MultiBodyModel : public Model {
  public:

    MultiBodyModel(string _name, shared_ptr<MultiBody> _m);
    ~MultiBodyModel();

    //access properties
    virtual void SetRenderMode(RenderMode _mode);
    virtual void SetSelectable(bool _s);
    virtual void SetColor(const Color4& _c);
    virtual void ToggleNormals();

    const Point3d& GetCOM() const {return m_multiBody->GetCenterOfMass();}
    double GetRadius() const {return m_multiBody->GetBoundingSphereRadius();}

    //access bodies
    typedef vector<BodyModel*>::const_iterator BodyIter;
    BodyIter begin() const { return m_bodies.begin(); }
    BodyIter end() const { return m_bodies.end(); }
    vector<BodyModel*>& GetBodies() {return m_bodies;}

    //drawing
    virtual void Build() = 0;
    virtual void Select(GLuint* _index, vector<Model*>& sel);
    virtual void DrawRender();
    virtual void DrawSelect();
    virtual void DrawSelected();
    virtual void DrawHaptics();
    virtual void Print(ostream& _os) const;

    //Edit Robot
    /*void DeleteBody(int _index);
    void DeleteRobot(int _index);
    void DeleteJoint(int _indexBase, int _indexJoint, int _bodyNumber);
    void DeleteJoints();
    void AddBase(BodyModel* _newBase);
    void AddBody(BodyModel* _b, int _index);
    void AddJoint(ConnectionModel* _c, int _indexBase, int _indexJoint, int _bodyNum);
    void ChangeDOF(int _dof);*/

  protected:
    vector<BodyModel*> m_bodies;       //complete list of all bodies
  private:
    shared_ptr<MultiBody> m_multiBody;
};

#endif
