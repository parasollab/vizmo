#ifndef _GLI_CAMERA_H_
#define _GLI_CAMERA_H_

#include<Point.h>
#include<Vector.h>
using namespace mathtool;

#include <string>
#include <list>
using namespace std;

#include <qgl.h>

//Handle the camera control
class gliCamera
{
public:
    
    gliCamera( const string& name, const Point3d& pos, const Vector3d& up );
    
    ///////////////////////////////////////////////////////////////////////////
    // Core
    void Draw( void );
    bool MP( QMouseEvent * e ); //mouse button press 
    bool MR( QMouseEvent * e ); //mouse button release
    bool MM( QMouseEvent * e );  //mouse motion

    ///////////////////////////////////////////////////////////////////////////
    // Access
    double getCameraAzim() const { return m_currentAzim+m_deltaAzim; }
    double getCameraElev() const { return m_currentElev+m_deltaElev; }
    Point3d getCameraPos() const { return m_CameraPos+m_deltaDis; }
    void setCameraPos(const Point3d& pos) { m_CameraPos=pos; }
    const string& getCameraName() const { return m_CamName; }
    const Vector3d& getWindowX() const { return m_WindowX; }
    const Vector3d& getWindowY() const { return m_WindowY; }
    Vector3d getWindowZ() const { return m_WindowX%m_WindowY; }

private:

    Point3d m_CameraPos;
    Vector3d m_deltaDis; //displacement caused by user
    Vector3d m_Up;

    double m_currentAzim, m_deltaAzim;
    double m_currentElev, m_deltaElev;

    QPoint m_PressedPt;
    bool m_MousePressed;

    Vector3d m_WindowX;
    Vector3d m_WindowY;

    string m_CamName; //camera name
};

///////////////////////////////////////////////////////////////////////////////
//Camera Factory
class gliCameraFactory
{
public:

    gliCameraFactory();

    ///////////////////////////////////////////////////////////////////////////
    // Access
    void addCamera( const gliCamera& camera );
    gliCamera* getCurrentCamera();
    bool setCurrentCamera(const string& name);

protected:

    void createDefaultCameras();
    gliCamera* findCamera(const string& name);

private:
    //default cameras
    list<gliCamera> m_Cameras;
    gliCamera * m_CurrentCam;
};

///////////////////////////////////////////////////////////////////////////////
gliCameraFactory& gliGetCameraFactory();

#endif //_GLI_CAMERA_H_

