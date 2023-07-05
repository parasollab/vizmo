#ifndef CAMERA_H_
#define CAMERA_H_

#include <Vector.h>
using namespace mathtool;

#include <string>
using namespace std;

#include <QObject>
#include <QMouseEvent>

////////////////////////////////////////////////////////////////////////////////
/// \brief A gluLookAt camera that is controllable with Qt input events.
////////////////////////////////////////////////////////////////////////////////
class Camera : public QObject {

  Q_OBJECT

  public:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Default constructor.
    /// \param[in] _eye  The camera's initial position.
    /// \param[in] _at   The camera's initial viewing target.
    /// \param[in] _up   The camera's initial up-direction.
    Camera(const Point3d& _eye, const Point3d& _at,
        const Vector3d& _up = Vector3d(0, 1, 0));

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the camera position and viewing target.
    /// \param[in] _eye The new position.
    /// \param[in] _at  The new viewing target.
    /// \param[in] _up  The new up direction.
    void Set(const Vector3d& _eye, const Vector3d& _at,
        const Vector3d& _up = Vector3d(0, 1, 0));

    void Draw(); ///< Apply the camera's viewpoint to the GL scene.

    //event handling
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Handle mouse button press events, which initiate a camera
    /// control.
    /// \param[in] _e The mouse event to handle.
    /// \return A \c bool indicating whether the event was handled or ignored.
    bool MousePressed(QMouseEvent* _e);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Handle mouse button release events, which terminate a camera
    /// control.
    /// \param[in] _e The mouse event to handle.
    /// \return A \c bool indicating whether the event was handled or ignored.
    bool MouseReleased(QMouseEvent* _e);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Handle active mouse move events, which are used to control the
    /// camera position and viewing direction.
    /// \param[in] _e The mouse event to handle.
    /// \return A \c bool indicating whether the event was handled or ignored.
    bool MouseMotion(QMouseEvent* _e);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Handle keyboard events, which are used to control the camera
    /// position and viewing direction.
    /// \param[in] _e The keyboard event to handle.
    /// \return A \c bool indicating whether the event was handled or ignored.
    bool KeyPressed(QKeyEvent* _e);

    //get the position, viewing direction, up direction, and viewing target
    const Vector3d& GetEye() const {return m_eye;} ///< Get the camera position.
    const Vector3d& GetDir() const {return m_dir;} ///< Get the viewing direction.
    const Vector3d& GetUp() const {return m_up;}   ///< Get the up direction.
    Vector3d GetAt() const {return m_eye + m_dir;} ///< Get the viewing target.

    //get the viewing coordinate frame
    Vector3d GetWindowX() const; ///< Get the screen-right direction.
    Vector3d GetWindowY() const; ///< Get the screen-up direction.
    Vector3d GetWindowZ() const; ///< Get the screen-out direction.

    //controls for free-floating mode
    void ToggleFreeFloat();      ///< Toggle free-floating mode.
    void ResetUp();              ///< Reset up to default.

    // Helper for changing frames
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Project a given vector from the camera frame to the world frame.
    /// \param[in] _v The vector in camera coordinates.
    /// \return       The same vector in world coordinates.
    Vector3d ProjectToWorld(const Vector3d& _v) const;

  public slots:

    // Slots for cross-thread camera control. Qt passes the parameters of these
    // functions by reference for signals originating from the same thread, or
    // by value for signals originating from another thread.
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Move the camera position and viewing target.
    /// \param[in] _delta The desired displacement relative to the current
    ///                   orientation.
    void Translate(const Vector3d& _delta);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Rotate the camera view counter-clockwise about a reference axis.
    /// \param[in] _axis  The axis of rotation relative to the current
    ///                   orientation.
    /// \param[in] _theta The angle of rotation in radians.
    void Rotate(const Vector3d& _axis, double _theta);

  private:

    // Position and orientation.
    Point3d m_eye;       ///< The camera position.
    Vector3d m_dir;      ///< The viewing direction.
    Vector3d m_up;       ///< The screen-up direction.

    // Speed.
    double m_speed;      ///< Speed of camera movement in relation to pixels.

    // Mouse event tracking.
    QPoint m_pressedPt;  ///< Stores the mouse-click location currently in use.
    bool m_mousePressed; ///< Indicates whether a mouse event is in progress.

    // Free-floating mode.
    bool m_freeFloating; ///< Indicates free-floating or classic mode.
};


#endif
