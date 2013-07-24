#ifndef _GLI_CAMERA_H_
#define _GLI_CAMERA_H_

#include<Vector.h>
using namespace mathtool;

#include <string>
#include <list>
using namespace std;

#include <qgl.h>

class gliCamera;
gliCamera* GetCamera();
void SetCamera(gliCamera* _camera);

/**
 * Handle camera control
 */

class gliCamera{

  public:

    gliCamera(const string& name, const Point3d& pos, const Vector3d& up);

      ///////////////////////////////////////////////////////////////////////////
      /// Core
      void Transform(double &x,double &y,double &z);
      void ReverseTransform(double &x,double &y,double &z);
      //allow user to explicitly specify camera position/orientation
      void SetByUser(double _x, double _y, double _z, double _azim, double _elev);
      void Draw( void );
      bool MP( QMouseEvent * e ); /// mouse button press
      bool MR( QMouseEvent * e ); /// mouse button release
      bool MM( QMouseEvent * e ); /// mouse motion
      bool KP( QKeyEvent * e );   /// keyboard press
      bool KR( QKeyEvent * e );   /// keyboard release
      void Move( void );
      void SphericalToCartesian( void );
      void CartesianToSpherical( void );
      void PrintHelp( void );

      ///////////////////////////////////////////////////////////////////////////
      /// Access
      double getCameraAzim() const { return m_currentAzim+m_deltaAzim; }
      double getCameraElev() const { return m_currentElev+m_deltaElev; }
      double getCurrentElev() const {return m_currentElev;}
      double getCurrentAzim() const {return m_currentAzim;}
      Point3d getCameraPos() const { return m_cameraPos+m_deltaDis; }
      void setCameraPos(const Point3d& pos) { m_cameraPos=pos; }
      void setAzim(double _azim) { m_currentAzim = _azim; }
      void setElev(double _elev) { m_currentElev = _elev; }
      const string& getCameraName() const { return m_camName; }
      const Vector3d& getWindowX() const { return m_windowX; }
      const Vector3d& getWindowY() const { return m_windowY; }
      Vector3d getWindowZ() const { return m_windowX%m_windowY; }



   private:

      Point3d m_cameraPos;
      Vector3d m_deltaDis; /// displacement caused by user
      Vector3d m_up;

      double m_currentAzim, m_deltaAzim;
      double m_currentElev, m_deltaElev;
      double m_speed;
      double m_angle;


      QPoint m_pressedPt;
      bool m_mousePressed;
      bool m_cartesian;//if true then camera moves in cartesian coords,
      //if false then camera moves in spherical coords

      Vector3d m_windowX;
      Vector3d m_windowY;

      string m_camName; /// camera name
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Camera Factory creates an instance of camera
 */

class gliCameraFactory
{
   public:

      gliCameraFactory();

      ///////////////////////////////////////////////////////////////////////////
      /// Access
      void addCamera( const gliCamera& camera );
      gliCamera* getCurrentCamera();
      bool setCurrentCamera(const string& name);

   protected:

      void createDefaultCameras();
      gliCamera* findCamera(const string& name);

   private:
      /// default cameras
      list<gliCamera> m_cameras;
      gliCamera * m_currentCam;
};

///////////////////////////////////////////////////////////////////////////////
gliCameraFactory& gliGetCameraFactory();

#endif //_GLI_CAMERA_H_

