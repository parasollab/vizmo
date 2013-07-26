#ifndef MULTIBODYINFO_H_
#define MULTIBODYINFO_H_

#include <iostream>
#include <string>
using namespace std;

#include "Transformation.h"
using namespace mathtool;

#include "RobotInfo.h"

class BodyModel;
class ConnectionModel;
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

    BodyModel* m_mBodyInfo;

    const Robot::JointMap& GetJointMap() const {return jointMap;}
    Robot::JointMap& GetJointMap() {return jointMap;}
    Robot::JointMap jointMap;
};

#endif
