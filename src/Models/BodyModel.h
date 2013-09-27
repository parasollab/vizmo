#ifndef BODYMODEL_H_
#define BODYMODEL_H_

#include <Transformation.h>
using namespace mathtool;

#include "Utilities/Color.h"
#include "Models/PolyhedronModel.h"

class ConnectionModel;

class BodyModel : public Model {
  public:
    enum Base {PLANAR, VOLUMETRIC, FIXED, JOINT}; //2D, 3D, 0D, not a base
    enum BaseMovement {ROTATIONAL, TRANSLATIONAL}; //rotation+translation, just translation, no movement

    static Base GetBaseFromTag(const string& _tag);
    static BaseMovement GetMovementFromTag(const string& _tag);

    BodyModel(bool _isSurface = false);
    ~BodyModel();

    //properties
    void GetChildren(list<Model*>& _models);
    void SetRenderMode(RenderMode _mode);
    void SetColor(const Color4& _c);

    const string& GetFilename() const {return m_filename;}
    const string& GetModelFilename() const {return m_modelFilename;}
    const Point3d& GetCOM() const {return m_polyhedronModel->GetCOM();}
    double GetRadius() const {return m_polyhedronModel->GetRadius();}
    RAPID_model* GetRapidModel() const {return m_polyhedronModel->GetRapidModel();}

    //base properties
    bool IsFixed() const {return m_isFixed;}
    bool IsSurface() const {return m_isSurface;}
    bool IsBase() const {return m_isBase;}
    bool IsBaseFixed() const {return m_baseType == FIXED;}
    bool IsBasePlanar() const {return m_baseType == PLANAR;}
    bool IsBaseVolumetric() const {return m_baseType == VOLUMETRIC;}
    Base GetBase() const {return m_baseType;}
    BaseMovement GetBaseMovement() const {return m_baseMovementType;}
    bool IsBaseRotational() const {return m_baseMovementType == ROTATIONAL;}

    //access to connections
    typedef vector<ConnectionModel*>::const_iterator ConnectionIter;
    ConnectionIter Begin() const {return m_connections.begin();}
    ConnectionIter End() const {return m_connections.end();}
    void AddConnection(ConnectionModel* _c) {m_connections.push_back(_c);}

    //access to transformations
    const Transformation& GetTransform() const {return m_currentTransform;}
    void SetTransform(const Transformation& _t) {m_currentTransform = _t;}
    bool IsTransformDone() const {return m_transformDone;}
    void SetTransformDone(bool _t) {m_transformDone = _t;}
    void SetPrevTransform(const Transformation& _t) {m_prevTransform = _t;}
    void ResetTransform() {m_currentTransform = m_prevTransform = Transformation(); m_transformDone = false;}

    //Return actual (previous*current) transformation
    //Compute prevtranform * TDH * dh * TBody2
    //receives the Body THIS body is connected to, the id of that connection
    void ComputeTransform(const BodyModel* _body, size_t _nextBody);

    void BuildModels() {}
    void Select(GLuint* _index, vector<Model*>& sel) {m_polyhedronModel->Select(_index, sel);}
    void Draw(GLenum _mode);
    void DrawSelect();
    void Print(ostream& _os) const;

    //file IO
    void ParseActiveBody(istream& _is, const string& _modelDataDir, const Color4 _color);
    void ParseOtherBody(istream& _is, const string& _modelDataDir, const Color4 _color);
    friend ostream& operator<<(ostream& _os, const BodyModel& _b);

  private:
    string m_directory, m_filename, m_modelFilename; //dir, file, dir+'/'+file
    PolyhedronModel* m_polyhedronModel;

    bool m_isFixed, m_isSurface, m_isBase;
    Base m_baseType;
    BaseMovement m_baseMovementType;

    vector<ConnectionModel*> m_connections;

    Transformation m_currentTransform, m_prevTransform;
    bool m_transformDone; //has current transform been computed?
};

#endif
