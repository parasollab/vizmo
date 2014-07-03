#ifndef MANAGER_H_
#define MANAGER_H_

#ifdef USE_PHANTOM

#include <vector>

#include "Vector.h"

using namespace std;

#include <HDU/hduVector.h>
#include <HL/hl.h>

namespace PHANToM {

//Define singleton
class Manager;
Manager& GetManager();

class Manager {
  public:
    Manager();
    ~Manager();

    void HapticRender();
    void DrawRender();

    void UpdateWorkspace();

    const hduVector3Dd& GetPos() const {return m_pos;}
    const hduVector3Dd& GetRot() const {return m_rot;}
    const hduVector3Dd& GetVel() const {return m_vel;}
    const hduVector3Dd& GetForce() const {return m_force;}

    mathtool::Point3d GetWorldPos() const
        {return mathtool::Point3d(m_worldPos[0], m_worldPos[1], m_worldPos[2]);}
    mathtool::Point3d GetWorldRot() const
        {return mathtool::Point3d(m_worldRot[0], m_worldRot[1], m_worldRot[2]);}

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

    HLuint m_boundaryId, m_obstaclesId;
};

}

#endif
#endif
