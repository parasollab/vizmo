#ifndef DEVICE_STATE_H_
#define DEVICE_STATE_H_

#include <HD/hd.h>
#include <HDU/hduVector.h>

namespace PHANToM {

/* Haptic device record. */
struct DeviceState {
  HHD m_hhd;
  hduVector3Dd m_position;
  hduVector3Dd m_velocity;
  hduVector3Dd m_force;
  hduVector3Dd m_rotation;
};

}

#endif
