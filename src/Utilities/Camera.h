#ifndef CAMERA_H_
#define CAMERA_H_

#include<Vector.h>
using namespace mathtool;

#include <string>
using namespace std;

#include <qgl.h>

class Camera {
  public:

    Camera(const string& _name, const Point3d& _pos, const Vector3d& _up);

    //allow user to explicitly specify camera position/orientation
    void Set(const Vector3d& _eye, const Vector3d& _at, const Vector3d& _up);
    void Draw();

    //event handling
    bool MousePressed(QMouseEvent* _e);
    bool MouseReleased(QMouseEvent* _e);
    bool MouseMotion(QMouseEvent* _e);
    bool KeyPressed(QKeyEvent* _e);

    const string& GetCameraName() const {return m_camName;}
    vector<Vector3d> GetCameraPos() const;
    double GetCameraAzim() const {return m_currentAzim + m_deltaAzim;}
    double GetCameraElev() const {return m_currentElev + m_deltaElev;}

    //computing window coordinateframe based on Elev and Azim
    Vector3d GetWindowX() const;
    Vector3d GetWindowY() const;
    Vector3d GetWindowZ() const;

  private:
    void KeyRotatePressed();

    string m_camName;

    Vector3d m_deltaDis; // displacement caused by user
    Vector3d m_up;
    Vector3d m_eye, m_at;
    Vector3d m_vector;

    double m_currentAzim, m_deltaAzim;
    double m_currentElev, m_deltaElev;
    double m_speed, m_defaultSpeed;

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
