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
///
/// The manager emits a signal whenever device input is detected by the reader
/// object. The current implementation also has the manager connect/disconnect
/// external input consumers in order to keep the space mouse from controlling
/// more than one object at a time (qt doesn't offer a mutually exclusive
/// signal/slot pairing).
///
/// To use this manager, the following yum packages are needed:
/// \arg spacenavd       This is a free and open-source driver for the device.
/// \arg libspnav        The corresponding library for spacenavd.
/// \arg libspnav-devel  Development headers for libspnav.
/// \arg spnavcfg        Optional configuration utility.
/// Compile with spacemouse=1 to build the manager. Before starting vizmo, the
/// driver daemon (spacenavd) must be started using root privelages.
///
/// \todo Find a way to handle mutually exclusive connections externally so
///       that we don't need a pair of Enable/Disable functions here for every
///       input consumer.
////////////////////////////////////////////////////////////////////////////////
class SpaceMouseManager : public QObject {

  Q_OBJECT

  private:

    // Device control
    bool m_enable{false};       ///< Enable or disable device polling.
    bool m_enableCamera{false}; ///< Enable or disable camera control.
    bool m_enableCursor{false}; ///< Enable or disable cursor control.
    double m_speed{1.};         ///< Environment-based speed scaling factor.

    // Auxiliary structures
    QThread* m_thread;          ///< Separate thread for reading the device.
    SpaceMouseReader* m_reader; ///< Device input handler (runs in m_thread).
    QReadWriteLock m_lock{};    ///< Locking mutex for thread-safe data access.

  public:

    // Construction.
    SpaceMouseManager();
    ~SpaceMouseManager();

    // Controls
    void Enable();           ///< Collect spacemouse input.
    void Disable();          ///< Ignore spacemouse input.
    void EnableCamera();     ///< Start sending spacemouse input to the camera.
    void DisableCamera();    ///< Stop sending spacemouse input to the camera.
    void EnableCursor();     ///< Start sending spacemouse input to the 3D cursor.
    void DisableCursor();    ///< Stop sending spacemouse input to the 3D cursor.
    void SetSpeed();         ///< Set the speed scaling factor.

    // Data access
    bool IsEnabled();       ///< Check whether device is in use.
    bool IsEnabledCamera() {return m_enableCamera;} ///< Check camera control.
    bool IsEnabledCursor() {return m_enableCursor;} ///< Check cursor control.

    // Public, thread safe data (also used by m_reader)
    const double m_pollRate{120.};    ///< The device polling rate in Hz.
    const bool m_debug{false};        ///< Enable debugging messages.

  signals:

    // Event handling
    void run();                          ///< Signal the reader to execute.
    void TranslateEvent(Vector3d);       ///< Indicates a translation event.
    void RotateEvent(Vector3d, double);  ///< Indicates a rotation event.
    void ButtonEvent(int, bool);         ///< Indicates a button event.

  private slots:

    // Event handling
    void TranslateHandler(const Vector3d&); ///< Handle translation input.
    void RotateHandler(const Vector3d&);    ///< Handle rotation input.
};


/*-------------------------- Input Gathering Worker --------------------------*/

////////////////////////////////////////////////////////////////////////////////
/// \brief Worker unit for reading device input on behalf of the
///        SpaceMouseManager.
///
/// This object uses a poll-and-sleep style event loop to check for input from
/// the space mouse. The device is polled at (m_manager->m_pollRate) Hz.
/// Whenever an event is detected, the reader sends a signal to the manager with
/// the raw data for pre-use processing. Events that occur between polling
/// cycles are discarded.
////////////////////////////////////////////////////////////////////////////////
class SpaceMouseReader : public QObject {

  Q_OBJECT

  SpaceMouseManager* m_manager;           ///< Pointer to the owning manager.

  public:

    // Construction
    SpaceMouseReader(SpaceMouseManager* _manager);
    ~SpaceMouseReader();

  public slots:

    // Input collection
    void InputLoop();                    ///< Poll the device and process input.

  signals:

    // Input signals
    void TranslateInput(Vector3d);       ///< Signal a translation input.
    void RotateInput(Vector3d);          ///< Signal a rotation input.
    void ButtonInput(int, bool);         ///< Signal a button press/release.
};


#else
/*------------------------------- Dummy Manager ------------------------------*/

////////////////////////////////////////////////////////////////////////////////
/// \brief Placeholder for space mouse manager when no device is in use.
////////////////////////////////////////////////////////////////////////////////
class SpaceMouseManager {

  public:

    // Construction.
    SpaceMouseManager() {}
    ~SpaceMouseManager() {}

    // Controls
    void Enable() {}                  ///< No-op when no device is in use.
    void Disable() {}                 ///< No-op when no device is in use.
    void EnableCamera() {}            ///< No-op when no device is in use.
    void DisableCamera() {}           ///< No-op when no device is in use.
    void EnableCursor() {}            ///< No-op when no device is in use.
    void DisableCursor() {}           ///< No-op when no device is in use.

    // Data access.
    const bool IsEnabled() {return false;}       ///< Return false for no device.
    const bool IsEnabledCamera() {return false;} ///< Return false for no device.
    const bool IsEnabledCursor() {return false;} ///< Return false for no device.
};


#endif
#endif
