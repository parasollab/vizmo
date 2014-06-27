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

    DeviceState ds;
    GetDeviceState(&ds);
    cout << endl << "Haptic State" << endl;
    cout << "\tPos: " << ds.m_position << endl;
    cout << "\tForce: " << ds.m_force << endl;
    cout << "\tAngles: " << ds.m_rotation << endl;
    cout << "\tVel: " << ds.m_velocity << endl;

    //Manager& manager = GetManager();

    hduVector3Dd pos = ds.m_position;

    hduVector3Dd force(0, 0, 0);

    double r = 100;
    double dist = pos.magnitude();
    if(dist > r) {
      double penDist = r-dist;

      force = pos/dist;

      double k = 0.25;

      force = k * penDist * force;
    }

    //Try getting proxy pos/rotation
    //hduVector3Dd proxyPos;
    HLdouble proxyPos[3];
    hlGetDoublev(HL_PROXY_POSITION, proxyPos);

    cout << endl << "Workspace State" << endl;
    cout << "\tPos: " << proxyPos[0] << " " << proxyPos[1] << " " << proxyPos[2] << endl;

    /*
    if(manager.CDOn) {
      if(manager.fpos.size() > 0) {
          if(manager.Collision > .5 && manager.validpos.size() > 0) {

            force[0] = manager.phantomforce*(manager.validpos[0] - manager.fpos[0]);
            force[1] = manager.phantomforce*(manager.validpos[1] - manager.fpos[1]);
            force[2] = manager.phantomforce*(manager.validpos[2] - manager.fpos[2]);
            //TODO FIX
            //GetCamera()->ReverseTransform(force[0],force[1],force[2]);
            double r = force.magnitude();
            if(r >= 1)
              force /= r;
            }
          }

          if(fabs(manager.Collision) < .5)
            manager.validpos = manager.fpos;

          manager.proceed = false;
      }
    }*/

    //hdSetDoublev(HD_CURRENT_FORCE, force);

    hdEndFrame(hhd);

    HDErrorInfo error;
    if (HD_DEVICE_ERROR(error = hdGetError())) {
      hduPrintError(stderr, &error, "Error during scheduler callback");
      if (hduIsSchedulerError(&error))
        return HD_CALLBACK_DONE;
    }

    return HD_CALLBACK_CONTINUE;
  }

}
