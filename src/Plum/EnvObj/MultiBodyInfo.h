#ifndef MULTIBODYINFO_H_
#define MULTIBODYINFO_H_

#include <iostream>
#include <string>
using namespace std;

#include "Transformation.h"
using namespace mathtool;

#include "RobotInfo.h"

class BodyInfo;
class ConnectionInfo;
//////////////////////////////////////////////////////////////////////
//
// MultiBodyInfo
//
//////////////////////////////////////////////////////////////////////
class MultiBodyInfo{

  public:
    //////////////////////////////////////////////////////////////////////
    // Constructor/Destructor
    //////////////////////////////////////////////////////////////////////
    MultiBodyInfo();
    MultiBodyInfo(const MultiBodyInfo& m_other);
    ~MultiBodyInfo();
    void operator=(const MultiBodyInfo& _other);
    friend ostream & operator<<(ostream& _out, const MultiBodyInfo& _body);

    int m_numberOfBody; //Total number of bodies
    int m_numberOfConnections; //how many connections there are
    bool m_active; //Active or passive?
    bool m_surface;//is surface? default is false

    BodyInfo* m_mBodyInfo;

    const Robot::JointMap& GetJointMap() const {return jointMap;}
    Robot::JointMap& GetJointMap() {return jointMap;}
    Robot::JointMap jointMap;
};

//////////////////////////////////////////////////////////////////////
//
// BodyInfo
//
//////////////////////////////////////////////////////////////////////
class BodyInfo{

  public:
    //////////////////////////////////////////////////////////////////////
    // Constructor/Destructor
    //////////////////////////////////////////////////////////////////////
    BodyInfo();
    BodyInfo(const BodyInfo& m_other);
    ~BodyInfo();

    /////////////////////////////////////////////////////////////////////
    // Methods to perform transformations:current and previous are used
    // to compute the transformation of THIS body
    //////////////////////////////////////////////////////////////////////////

    ///@If m_index==0, then transform m_x, m_y, m_z,m_alpha, m_beta, m_gamma
    void doTransform();
    ///@Copy t to m_prevTransform
    void setPrevTransform(Transformation _t);
    ///@Return actual (previous*current) transformation
    Transformation getTransform();
    ///@Compute prevtranform * TDH * dh * TBody2
    ///@receives the Body THIS body is connected to, the id of that connection and the new thetha
    void computeTransform(BodyInfo& BodyInfo, int _nextBody);

    void operator=(const BodyInfo& _other);
    friend ostream & operator<<(ostream& _out, const BodyInfo& _body);

    bool m_isFixed;
    bool m_isBase;
    bool m_transformDone;
    int  m_index;
    string m_modelDataFileName;
    string m_fileName;
    string m_directory;
    bool m_isNew; // to detect whether this body has been added by the user
    double m_x, m_y, m_z;
    double m_alpha, m_beta, m_gamma;
    float rgb[3]; //store initial color (read from env. file)
    bool m_isSurface;

    int m_numberOfConnection;
    ConnectionInfo* m_connectionInfo;
    Transformation m_currentTransform, m_prevTransform;

    bool IsBase() {return isBase;};
    Robot::Base GetBase() {return baseType;};
    Robot::BaseMovement GetBaseMovement() {return baseMovementType;};
    void SetBase(Robot::Base _baseType) {baseType = _baseType;};
    void SetBaseMovement(Robot::BaseMovement _baseMovementType) {baseMovementType = _baseMovementType;};

    bool isBase;                           ///<Is this a base?
  private:
    Robot::Base baseType;                  ///<If its a base, this needs to be set later
    Robot::BaseMovement baseMovementType;  ///<If its a base, this also needs to be set
};

//////////////////////////////////////////////////////////////////////
//
// ConnectionInfo
//
//////////////////////////////////////////////////////////////////////

class ConnectionInfo{
  public:
    enum JointType {REVOLUTE, SPHERICAL}; //1dof vs 2dof rotational joints
    //////////////////////////////////////////////////////////////////////
    // Constructor/Destructor
    //////////////////////////////////////////////////////////////////////
    ConnectionInfo();
    ConnectionInfo(const ConnectionInfo& m_other);
    ///@Return DH params. as a Vecto4d
    Transformation DHTransform();
    ///@Compute ad return the transformation from body1 to body2
    Transformation  transformToBody2();
    ///@Compute and return the transformation to DH frame
    Transformation  transformToDHframe();
    ///@Return id of the body THIS body is connected to
    int GetNextBody(){return m_nextIndex;}
    int GetPreviousBody(){return m_preIndex;}
    JointType GetJointType(){return m_jointType;}
    void operator=(const ConnectionInfo& _other);
    static JointType GetJointTypeFromTag(const string _tag);
    friend ostream& operator<<(ostream& _out, const ConnectionInfo& m_con);

    int m_preIndex;
    int m_nextIndex;
    //translation and rotation to NextBody
    double m_posX, m_posY, m_posZ;
    double m_orientX, m_orientY, m_orientZ;
    double alpha, theta, a, d;
    // this will keep the original theta value read
    //for Revolute joints, theta is variable
    double m_theta;
    bool m_actuated;
    //translation and rotation to DHFrame
    double m_pos2X, m_pos2Y, m_pos2Z;
    double m_orient2X, m_orient2Y, m_orient2Z;

    JointType m_jointType;

    //ordering information
    size_t m_globalIndex;
    static size_t m_globalCounter;

};

#endif
