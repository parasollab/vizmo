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
    void Set(const Point3d& _pos, double _azim, double _elev);

    void Draw();

    //event handling
    bool MousePressed(QMouseEvent* _e);
    bool MouseReleased(QMouseEvent* _e);
    bool MouseMotion(QMouseEvent* _e);
    bool KeyPressed(QKeyEvent* _e);

    const string& GetCameraName() const {return m_camName;}
    Point3d GetCameraPos() const {return m_cameraPos + m_deltaDis;}
    void SetCameraPos(const Point3d& _pos) {m_cameraPos = _pos;}
    double GetCameraAzim() const {return m_currentAzim + m_deltaAzim;}
    double GetCameraElev() const {return m_currentElev + m_deltaElev;}

    //computing window coordinateframe based on Elev and Azim
    Vector3d GetWindowX() const;
    Vector3d GetWindowY() const;
    Vector3d GetWindowZ() const;

  private:
    string m_camName;

    Point3d m_cameraPos;
    Vector3d m_deltaDis; // displacement caused by user
    Vector3d m_up;

    double m_currentAzim, m_deltaAzim;
    double m_currentElev, m_deltaElev;
    double m_speed;
    double m_angle;

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
