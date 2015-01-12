#ifndef CAMERA_H_
#define CAMERA_H_

#include<Vector.h>
using namespace mathtool;

#include <string>
using namespace std;

#include <qgl.h>

////////////////////////////////////////////////////////////////////////////////
/// \brief A gluLookAt camera that is controllable with Qt input events.
////////////////////////////////////////////////////////////////////////////////
class Camera {

  public:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Default constructor.
    /// \param[in] _name The camera name.
    /// \param[in] _eye  The camera's initial position.
    /// \param[in] _at   The camera's initial viewing target.
    Camera(const string& _name, const Point3d& _eye, const Point3d& _at);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the camera position and viewing target.
    /// \param[in] _eye The new position.
    /// \param[in] _at  The new viewing target.
    void Set(const Vector3d& _eye, const Vector3d& _at);

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

    //get the name, position, viewing direction, and viewing target
    const string& GetName() const {return m_name;}
    const Vector3d& GetEye() const {return m_currEye;}
    const Vector3d& GetDir() const {return m_currDir;}
    Vector3d GetAt() const {return m_currEye + m_currDir;}

    //get the viewing coordinate frame
    Vector3d GetWindowX() const; ///< Get the screen-left direction.
    Vector3d GetWindowY() const; ///< Get the screen-up direction.
    Vector3d GetWindowZ() const; ///< Get the viewing direction (screen-in).

  private:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Rotate an input vector counter-clockwise about a reference axis.
    /// \param[in,out] _vec   The vector to be rotated.
    /// \param[in]     _axis  The axis of rotation.
    /// \param[in]     _theta The angle of rotation in radians.
    void Rotate(Vector3d& _vec, const Vector3d& _axis, double _theta);

    string m_name;       ///< The camera name.

    const Vector3d m_up; ///< Always the y-axis.
    Point3d m_currEye,   ///< The position of the camera.
            m_eye;       ///< Temporary position for camera control.
    Vector3d m_currDir,  ///< The unit vector where camera is facing.
             m_dir;      ///< Temporary facing vector for camera control.

    double m_speed;      ///< Speed of camera movement in relation to pixels.

    QPoint m_pressedPt;  ///< Stores the mouse-click location currently in use.
    bool m_mousePressed; ///< Indicates whether a mouse event is in progress.
};

////////////////////////////////////////////////////////////////////////////////
/// \brief A container class for managing multiple Camera objects.
////////////////////////////////////////////////////////////////////////////////
class CameraFactory {

  public:

    CameraFactory();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Copy another Camera into this container.
    /// \param[in] \c _camera The Camera to be copied.
    void AddCamera(const Camera& _camera);

    Camera* GetCurrentCamera() const {return m_currentCam;}
    void SetCurrentCamera(const string& _name);

  private:

    map<string, Camera> m_cameras; ///< Storage for multiple Camera.
    Camera* m_currentCam;          ///< A pointer to the Camera in use.
};

#endif
