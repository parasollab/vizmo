#ifndef CAMERA_H_
#define CAMERA_H_

#include<Vector.h>
using namespace mathtool;

#include <string>
using namespace std;

#include <qgl.h>

class Camera {
  public:

    Camera(const string& _name, const Point3d& _eye, const Point3d& _at);

    //allow user to explicitly specify camera position/orientation
    void Set(const Vector3d& _eye, const Vector3d& _at);
    void Draw();

    //event handling
    bool MousePressed(QMouseEvent* _e);
    bool MouseReleased(QMouseEvent* _e);
    bool MouseMotion(QMouseEvent* _e);
    bool KeyPressed(QKeyEvent* _e);

    const string& GetName() const {return m_name;}
    const Vector3d& GetEye() const {return m_currEye;}
    Vector3d GetAt() const {return m_currEye + m_currDir;}

    //computing window coordinateframe based on Elev and Azim
    Vector3d GetWindowX() const;
    Vector3d GetWindowY() const;
    Vector3d GetWindowZ() const;

  private:
    void Rotate(Vector3d& _vec, const Vector3d& _axis, double _theta);

    string m_name;

    const Vector3d m_up; //always y-axis
    Point3d m_eye, m_currEye; //position of camera
    Vector3d m_dir, m_currDir; //unit vector where camera is facing

    double m_speed; //speed of camera movement in relation to pixels

    QPoint m_pressedPt;
    bool m_mousePressed;
};


//Camera Factory creates an instance of camera
class CameraFactory {
  public:
    CameraFactory();

    void AddCamera(const Camera& _camera);

    Camera* GetCurrentCamera() const {return m_currentCam;}
    void SetCurrentCamera(const string& _name);

  private:
    map<string, Camera> m_cameras;
    Camera* m_currentCam;
};

#endif
