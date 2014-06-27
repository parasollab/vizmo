#ifndef MANAGER_H_
#define MANAGER_H_

#ifdef USE_PHANTOM

#include <vector>
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

    void Frame();

    const hduVector3Dd& GetPos() const {return m_pos;}
    const hduVector3Dd& GetRot() const {return m_rot;}
    const hduVector3Dd& GetVel() const {return m_vel;}
    const hduVector3Dd& GetForce() const {return m_force;}

  private:

    void Initialize();

    void Clean();

    HHD m_hhd; //haptic device handle
    HHLRC m_hhlrc; //hl rendering context

    hduVector3Dd m_pos, m_rot, m_vel, m_force; //Device State
    hduVector3Dd m_worldPos, m_worldRot; //World State
};

}

#endif
#endif
