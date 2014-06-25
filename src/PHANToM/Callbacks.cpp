#include "Callbacks.h"

#include "Basic.h"
using namespace mathtool;

#include "DeviceState.h"
#include "Manager.h"

namespace PHANToM {

/* Client callback.
 *
 * Use this callback synchronously.
 * Gets data, in a thread safe manner, that is constantly being modified by the
 * haptics thread.
 */
HDCallbackCode
  GetDeviceState(void* _data) {

    cout << "GDS" << endl;

    DeviceState* deviceState = static_cast<DeviceState*>(_data);

    hdGetDoublev(HD_CURRENT_POSITION, deviceState->m_position);
    hdGetDoublev(HD_CURRENT_FORCE, deviceState->m_force);
    hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, deviceState->m_rotation);
    hdGetDoublev(HD_CURRENT_VELOCITY, deviceState->m_velocity);

    // execute this only once.
    return HD_CALLBACK_DONE;
  }

/* Main callback that calculates and sets the force.
*/
HDCallbackCode
  ForceFeedback(void* _data) {

    HHD hhd = hdGetCurrentDevice();

    hdBeginFrame(hhd);

    hduVector3Dd pos;
    hdGetDoublev(HD_CURRENT_POSITION, pos);

    hduVector3Dd force(0, 0, 0);
    hdSetDoublev(HD_CURRENT_FORCE, force);

    Manager& manager = GetManager();

    if(manager.CDOn) {
      if(manager.fpos.size() > 0) {
          if(manager.Collision > .5 && manager.validpos.size() > 0) {

            force[0] = manager.phantomforce*(manager.validpos[0] - manager.fpos[0]);
            force[1] = manager.phantomforce*(manager.validpos[1] - manager.fpos[1]);
            force[2] = manager.phantomforce*(manager.validpos[2] - manager.fpos[2]);
            //TODO FIX
            //GetCamera()->ReverseTransform(force[0],force[1],force[2]);
            double r = force.magnitude();
            if(r < 1)
              hdSetDoublev(HD_CURRENT_FORCE, force);
            else {
              force /= r;
              hdSetDoublev(HD_CURRENT_FORCE, force);
            }
          }

          if(fabs(manager.Collision) < .5)
            manager.validpos = manager.fpos;

          manager.proceed = false;
      }
    }

    hdEndFrame(hhd);

    HDErrorInfo error;
    if (HD_DEVICE_ERROR(error = hdGetError())) {
      hduPrintError(stderr, &error, "Error during scheduler callback");
      if (hduIsSchedulerError(&error))
        return HD_CALLBACK_DONE;
    }

    DeviceState ds;
    GetDeviceState(&ds);
    cout << "Pos::" << ds.m_position << endl;
    cout << "Force::" << ds.m_force << endl;
    cout << "Angles::" << ds.m_rotation << endl;
    cout << "Vel::" << ds.m_velocity << endl;

    return HD_CALLBACK_CONTINUE;
  }

}
