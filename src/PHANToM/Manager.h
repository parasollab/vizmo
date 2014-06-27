#ifndef MANAGER_H_
#define MANAGER_H_

#ifdef USE_PHANTOM

#include <vector>
using namespace std;

#include <HD/hd.h>
#include <HDU/hduError.h>
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

    hduVector3Dd GetPosition();
    hduVector3Dd GetRotation();
    hduVector3Dd GetVelocity();

  private:

    void Initialize();

    void Clean();

    HHD m_hhd; //haptic device handle
    HDSchedulerHandle m_schedulerCallback; //callback handle of scheduler
    HHLRC m_hhlrc; //hl rendering context

    hduVector3Dd m_position, m_velocity, m_force, m_rotation; //Device State
    hduVector3Dd m_position, m_rotation; //World State

    //TODO: Move to private
  public:
    vector<double> validpos; //Previous Valid cfg
    vector<double> fpos; //Potential Valid cfg
    int Collision;
    bool UseFeedback;
    bool CDOn;
    bool proceed;
    double phantomforce; //force factor
};

}

#endif
#endif
