#ifndef SPACE_MOUSE_MANAGER_H_
#define SPACE_MOUSE_MANAGER_H_

#include "Vector.h"
using namespace mathtool;

#ifdef USE_SPACEMOUSE

#include <QtCore>
class SpaceMouseReader;

/*-------------------------- Space Mouse Manager -----------------------------*/

////////////////////////////////////////////////////////////////////////////////
/// \brief Provides an encapsulated manager for using a 3dconnexion 3-D mouse
///        within Vizmo.
////////////////////////////////////////////////////////////////////////////////
class SpaceMouseManager : public QObject {

  Q_OBJECT

  // Device control.
  bool m_enable;              ///< Enable or disable device polling.
  bool m_enableCamera;        ///< Enable or disable camera control.

  // Device data.
  Vector3d m_pos;             ///< The most recent device position.
  Vector3d m_rot;             ///< The most recent device rotation.

  // Auxiliary structures.
  QThread* m_thread;          ///< Separate thread for reading the device.
  SpaceMouseReader* m_reader; ///< Device input handler (runs in m_thread).
  QReadWriteLock m_lock;      ///< Locking mutex for thread-safe data access.

  public:

    // Construction.
    SpaceMouseManager();
    ~SpaceMouseManager();

    // Controls
    void Enable();          ///< Collect spacemouse input.
    void Disable();         ///< Ignore spacemouse input.
    void EnableCamera();    ///< Start sending spacemouse input to the camera.
    void DisableCamera();   ///< Stop sending spacemouse input to the camera.

    // Data access.
    const bool IsEnabled();       ///< Check whether device is in use.
    const bool IsEnabledCamera(); ///< Check camera control.
    const Vector3d GetPos();      ///< Get most recent device position.
    const Vector3d GetRot();      ///< Get most recent device rotation.

    // Data feed.
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Update the last known device position. This method should only
    ///        be called by m_reader to report the most recent reading.
    void SetPos(const Vector3d& _pos);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Update the last known device rotation. This method should only
    ///        be called by m_reader to report the most recent reading.
    void SetRot(const Vector3d& _rot);

  signals:

    void run();   ///< Signal the reader to execute.
};


/*-------------------------- Input Gathering Worker --------------------------*/

////////////////////////////////////////////////////////////////////////////////
/// \brief Worker unit for reading device input on behalf of the
///        SpaceMouseManager.
/// \details This object executes a poll-and-sleep style event loop to check for
///          input from the space mouse. The device is polled at 1kHz.
////////////////////////////////////////////////////////////////////////////////
class SpaceMouseReader : public QObject {

  Q_OBJECT

  // Pointer to the owning manager.
  SpaceMouseManager* m_manager;

  // Camera speed factor
  double m_speed;

  public:

    // Construction.
    SpaceMouseReader(SpaceMouseManager* _manager);
    ~SpaceMouseReader();

  public slots:

    // Input collection.
    void InputLoop();                     ///< Poll the device and process input.

  signals:

    // Camera control signals.
    void TranslateCamera(Vector3d);       ///< Signal the camera to translate.
    void RotateCamera(Vector3d, double);  ///< Signal the camera to rotate.
};


#else

////////////////////////////////////////////////////////////////////////////////
/// \brief Placeholder for space mouse manager when no device is in use.
////////////////////////////////////////////////////////////////////////////////
class SpaceMouseManager {

  public:

    // Construction.
    SpaceMouseManager() {}
    ~SpaceMouseManager() {}

    // Controls
    void Enable() {}
    void Disable() {}
    void EnableCamera() {}
    void DisableCamera() {}

    // Data access.
    const bool IsEnabled() {return false;}
    const bool IsEnabledCamera() {return false;}
    const Vector3d GetPos() {return Vector3d();}
    const Vector3d GetRot() {return Vector3d();}

    // Data feed.
    void SetPos(const Vector3d&) {}
    void SetRot(const Vector3d&) {}
};


#endif
#endif
