#ifndef MANAGER_H_
#define MANAGER_H_

#ifdef USE_HAPTICS

#include <vector>
using namespace std;

#include "Vector.h"
using namespace mathtool;

#include <HDU/hduVector.h>
#include <HL/hl.h>

namespace Haptics {

//Define singleton
class Manager;
Manager& GetManager();

inline const bool UsingPhantom() {return true;}

class Manager {
  public:
    Manager();
    ~Manager();

    void HapticRender();
    void DrawRender();

    void UpdateWorkspace();

    //state access functions
    const hduVector3Dd& GetPos() const {return m_pos;}
    const hduVector3Dd& GetRot() const {return m_rot;}
    const hduVector3Dd& GetVel() const {return m_vel;}
    const hduVector3Dd& GetForce() const {return m_force;}
    Point3d GetWorldPos() const
        {return Point3d(m_worldPos[0], m_worldPos[1], m_worldPos[2]);}
    Point3d GetWorldRot() const
        {return Point3d(m_worldRot[0], m_worldRot[1], m_worldRot[2]);}
    bool IsInsideBBX();

    //control functions
    void ToggleForceOutput();

  private:

    void Initialize();

    void Clean();

    void GetState();

    void BoundaryRender();
    void ObstacleRender();

    HHD m_hhd; //haptic device handle
    HHLRC m_hhlrc; //hl rendering context

    hduVector3Dd m_pos, m_rot, m_vel, m_force; //Device State
    hduVector3Dd m_worldPos, m_worldRot; //World State
    vector<pair<double, double> > m_worldRange;

    HLuint m_boundaryId, m_obstaclesId;
};

}

#else

#include "Vector.h"
using namespace mathtool;

namespace Haptics {

inline const bool UsingPhantom() {return false;}

class Manager {
  public:
    void HapticRender() {}
    void DrawRender() {}
    void UpdateWorkspace() {}

    //state access functions
    Point3d GetWorldPos() {return Point3d();}
    Point3d GetWorldRot() {return Point3d();}

    //control functions
    void ToggleForceOutput() {}
};
}

#endif
#endif
