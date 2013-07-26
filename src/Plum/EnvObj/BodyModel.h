#ifndef BODYMODEL_H_
#define BODYMODEL_H_

#include <Transformation.h>
using namespace mathtool;

#include "Utilities/Color.h"
#include "RobotInfo.h"

class ConnectionModel;

class BodyModel {
  public:
    BodyModel();
    ~BodyModel();

    //geometry filename and color
    const string& GetFilename() const {return m_filename;}
    const string& GetModelFilename() const {return m_modelFilename;}
    const Color4& GetColor() const {return m_color;}

    //base properties
    bool IsFixed() const {return m_isFixed;}
    bool IsSurface() const {return m_isSurface;}
    bool IsBase() {return m_isBase;};
    Robot::Base GetBase() {return m_baseType;};
    Robot::BaseMovement GetBaseMovement() {return m_baseMovementType;};

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
    void ComputeTransform(const BodyModel& _body, int _nextBody);

    //file IO
    void ParseActiveBody(istream& _is, const string& _modelDataDir, const Color4 _color);
    void ParseOtherBody(istream& _is, const string& _modelDataDir, const Color4 _color);
    friend ostream& operator<<(ostream& _os, const BodyModel& _b);

  private:
    string m_directory, m_filename, m_modelFilename; //dir, file, dir+'/'+file
    Color4 m_color; //store initial color (read from env. file)

    bool m_isFixed, m_isSurface, m_isBase;
    Robot::Base m_baseType;
    Robot::BaseMovement m_baseMovementType;

    vector<ConnectionModel*> m_connections;

    Transformation m_currentTransform, m_prevTransform;
    bool m_transformDone; //has current transform been computed?
};

#endif
