#ifndef _GLI_CAMERA_H_
#define _GLI_CAMERA_H_

#include<Point.h>
#include<Vector.h>
using namespace mathtool;

#include <string>
#include <list>
using namespace std;

#include <qgl.h>

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
      Point3d getCameraPos() const { return m_CameraPos+m_deltaDis; }
      void setCameraPos(const Point3d& pos) { m_CameraPos=pos; }
      const string& getCameraName() const { return m_CamName; }
      const Vector3d& getWindowX() const { return m_WindowX; }
      const Vector3d& getWindowY() const { return m_WindowY; }
      Vector3d getWindowZ() const { return m_WindowX%m_WindowY; }



   private:

      Point3d m_CameraPos;
      Vector3d m_deltaDis; /// displacement caused by user
      Vector3d m_Up;

      double m_currentAzim, m_deltaAzim;
      double m_currentElev, m_deltaElev;
      double m_speed;
      double m_angle;


      QPoint m_PressedPt;
      bool m_MousePressed;
      bool m_cartesian;//if true then camera moves in cartesian coords,
      //if false then camera moves in spherical coords

      Vector3d m_WindowX;
      Vector3d m_WindowY;

      string m_CamName; /// camera name
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
      list<gliCamera> m_Cameras;
      gliCamera * m_CurrentCam;
};

///////////////////////////////////////////////////////////////////////////////
gliCameraFactory& gliGetCameraFactory();

#endif //_GLI_CAMERA_H_

