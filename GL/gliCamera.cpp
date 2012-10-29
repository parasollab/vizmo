#include "gliCamera.h"
//Added by qt3to4:
#include <QMouseEvent>
#include <iostream>
#include <math.h>
#include <src/vizmo2.h>

/**
 *  Constructor
 */
   gliCamera::gliCamera
( const string& name, const Point3d& pos, const Vector3d& up )
{
   m_CamName=name;
   m_CameraPos=pos;
   m_Up=up;
   m_currentAzim=m_deltaAzim=
      m_currentElev=m_deltaElev=0;
   m_MousePressed=false;
   m_WindowX.set(1,0,0);
   m_WindowY.set(0,1,0);
   m_speed=.5;
   m_angle=.5;
   m_cartesian=false;
}

/**
 *  Draw: Translate and Rotate camera
 */

void gliCamera::Transform(double &x,double &y,double &z){
   //cout << rand()%100 << endl;
   double dx,dy,dz;
   dx = x;dy = y;dz = z;
   double angle = (m_currentElev+m_deltaElev)*3.14159/180.0;
   //cout << "elev=" << angle << endl;
   y = dy*cos(angle) + dz*sin(angle);
   z = dz*cos(angle) - dy*sin(angle);
   dx = x;dy = y;dz = z;
   angle = (m_currentAzim+m_deltaAzim)*3.14159/180.0;
   //cout << "azim=" <<angle << endl;
   x = dx*cos(angle) - dz*sin(angle);
   z = dz*cos(angle) + dx*sin(angle);
}

void gliCamera::ReverseTransform(double &x,double &y,double &z){
   //cout << rand()%100 << endl;
   double dx,dy,dz;
   dx = x;dy = y;dz = z;
   double angle = -(m_currentElev+m_deltaElev)*3.14159/180.0;
   //cout << "elev=" << angle << endl;
   y = dy*cos(angle) + dz*sin(angle);
   z = dz*cos(angle) - dy*sin(angle);
   dx = x;dy = y;dz = z;
   angle = -(m_currentAzim+m_deltaAzim)*3.14159/180.0;
   //cout << "azim=" <<angle << endl;
   x = dx*cos(angle) - dz*sin(angle);
   z = dz*cos(angle) + dx*sin(angle);
}

void 
gliCamera::SetByUser(double _x, double _y, double _z, double _azim, double _elev){
  
  setCameraPos(Point3d(-1*(_x), -1*(_y), _z)); 
  m_currentAzim = _azim; 
  m_currentElev = _elev; 
}

void gliCamera::Draw( void )
{
   if(m_cartesian){//Cartesian motion
      glRotated(m_currentElev+m_deltaElev,1.0,0.0,0.0);  
      glRotated(m_currentAzim+m_deltaAzim,0.0,1.0,0.0);  
      glTranslatef(-m_CameraPos[0]-m_deltaDis[0],
            -m_CameraPos[1]-m_deltaDis[1],
            -m_CameraPos[2]-m_deltaDis[2]); 
   }else{//Polar motion
      glTranslatef( (m_CameraPos[0]+m_deltaDis[0]),
            (m_CameraPos[1]+m_deltaDis[1]), 
            -(m_CameraPos[2]+m_deltaDis[2]));

      glRotated(m_currentElev+m_deltaElev, 1.0, 0.0, 0.0);
      glRotated(m_currentAzim+m_deltaAzim, 0.0, 1.0, 0.0);
   }
   /*
      gluLookAt( (m_CameraPos[0]+m_deltaDis[0]), //eye pos
      -(m_CameraPos[1]+m_deltaDis[1]),
      -(m_CameraPos[2]+m_deltaDis[2]),
      0,0,0, //center
      m_Up[0],m_Up[1],m_Up[2]);
    */
}

///////////////////////////////////////////////////////////////////////
//**
//** Handle Mouse Movement
///////////////////////////////////////////////////////////////////////

bool gliCamera::MP(QMouseEvent* e)
{
   if(e->buttons() && (e->modifiers() == Qt::ControlModifier)){  
     m_MousePressed=true;
      m_PressedPt=e->pos();
      return true; //handled
   }     
   return false;
}

bool gliCamera::MR(QMouseEvent* e)
{
   if(!m_MousePressed) 
      return false; /// mouse is not pressed

   m_MousePressed=false;
   for(int iD=0;iD<3;iD++){
      m_CameraPos[iD]+=m_deltaDis[iD];
      m_deltaDis[iD]=0;
   }

   m_currentElev += m_deltaElev;
   m_currentAzim += m_deltaAzim;

   m_deltaElev = 0.0;
   m_deltaAzim = 0.0;

   return true;
}

/// defined later
void RotateY(Vector3d& v, double degree);
void RotateX(Vector3d& v, double degree);

//////////////////////////////////////
//**
//** Mouse motion
//////////////////////////////////////

bool 
gliCamera::MM(QMouseEvent* e){       

  if(!m_MousePressed)    
    return false; //mouse is not pressed
  
   //Qt::MouseButtons state=e->buttons();  
   QPoint drag=e->pos()-m_PressedPt;

   //displacement
   if(e->buttons() & Qt::MidButton){ //mid button only
      if(m_cartesian){
         m_deltaDis.set(0,0,0);
         m_deltaDis[0] = drag.x()/10.0;
         m_deltaDis[1] = -drag.y()/10.0;
         RotateX(m_deltaDis, -m_currentElev+m_deltaElev);
         RotateY(m_deltaDis, -m_currentAzim+m_deltaAzim);
      }else{
         m_deltaDis[0] = ((m_CameraPos[0]>5)?m_CameraPos[0]:5)*drag.x()/10.0;
        m_deltaDis[1] = -(((m_CameraPos[1]>5)?m_CameraPos[1]:5)*drag.y()/10.0);
      }
   }
   else if(e->buttons() & Qt::RightButton){ //right button only
      if(m_cartesian){//Cartesian movement
         double motion = drag.y()/10;
         m_deltaDis.set(0,0,motion);
         RotateX(m_deltaDis, -m_currentElev+m_deltaElev);
         RotateY(m_deltaDis, -m_currentAzim+m_deltaAzim);
      }else{//Spherical movement
         m_deltaDis[2] = (((m_CameraPos[2]>10)?m_CameraPos[2]:10)*drag.y()/100);
      }
   }
   else if((e->buttons() & Qt::LeftButton)){ //left button only

      m_deltaAzim = drag.x()/5.0;
      m_deltaElev = drag.y()/5.0;
      //compute window x, y dir
      m_WindowX.set(1,0,0);
      m_WindowY.set(0,1,0);
      RotateX(m_WindowX, m_currentElev+m_deltaElev);
      RotateY(m_WindowX, m_currentAzim+m_deltaAzim);
      m_WindowX[2]=-m_WindowX[2];
      RotateX(m_WindowY, m_currentElev+m_deltaElev);
      RotateY(m_WindowY, m_currentAzim+m_deltaAzim);  
      m_WindowY[2]=-m_WindowY[2];

   }
   else return false; //not handled
   return true;
}




//////////////////////////////////////
//**
//** Keyboard motion
//////////////////////////////////////

bool gliCamera::KP( QKeyEvent * e )  
{       




   switch(e->key()){
      case 'h': PrintHelp();return true;
      case '-': m_speed/=1.5;return true;
      case '+': m_speed*=1.5;return true;
      case '/': m_angle/=1.5;return true;
      case '*': m_angle*=1.5;return true;
      case '8': //forward
                m_deltaDis[0]=0;
                m_deltaDis[1]=0;
                m_deltaDis[2]=-m_speed;
                Move();
                return true;
      case '2': //back
                m_deltaDis[0]=0;
                m_deltaDis[1]=0;
                m_deltaDis[2]=m_speed;
                Move();
                return true;
      case '6': //left
                m_deltaDis[0]=m_speed*2*(m_cartesian-.5);
                m_deltaDis[1]=0;
                m_deltaDis[2]=0;
                Move();
                return true;
      case '4': //right
                m_deltaDis[0]=-m_speed*2*(m_cartesian-.5);
                m_deltaDis[1]=0;
                m_deltaDis[2]=0;
                Move();
                return true;
      case '7': //up
                m_deltaDis[0]=0;
                m_deltaDis[1]=m_speed*2*(m_cartesian-.5);
                m_deltaDis[2]=0;
                Move();
                return true;
      case '1': //down
                m_deltaDis[0]=0;
                m_deltaDis[1]=-m_speed*2*(m_cartesian-.5);
                m_deltaDis[2]=0;
                Move();
                return true;
      case '0': 
                if(m_cartesian){
                   m_cartesian=false;
                   //m_CameraPos[2]=pow(m_CameraPos[0]*m_CameraPos[0]+
                   //	      m_CameraPos[1]*m_CameraPos[1]+
                   //	      m_CameraPos[2]*m_CameraPos[2],.5);
                   //m_CameraPos[0]=0;
                   //m_CameraPos[1]=0;
                   CartesianToSpherical();
                }else{
                   m_cartesian=true;
                   //m_currentElev=0;
                   //m_currentAzim=0;
                   SphericalToCartesian();
                }
                return true;
      case Qt::Key_Left ://look left
                m_currentAzim-=m_angle*(m_cartesian-.5)*2;
                Move();
                return true;
      case Qt::Key_Right ://look right
                m_currentAzim+=m_angle*(m_cartesian-.5)*2;
                Move();
                return true;
      case Qt::Key_Up ://look up
                m_currentElev-=m_angle*(m_cartesian-.5);
                Move();
                return true;
      case Qt::Key_Down ://look down
                m_currentElev+=m_angle*(m_cartesian-.5);
                Move();
                return true;

   }






   return false;//not handled


}


void gliCamera::PrintHelp( void ){
   std::cout << 
      "numpad\n" <<
      "0 = switch between Spherical and Cartesian coordinates \n" <<
      "8 = move forward\n"<<
      "2 = move back\n"<<
      "4 = move left\n"<<
      "6 = move right\n"<<
      "7 = move up\n"<<
      "1 = move down\n"<<
      "- = decrease translation speed\n"<<
      "+ = increase translation speed\n"<<
      "/ = decrease rotation speed\n"<<
      "* = increase rotation speed\n"<<
      "arrow keys\n"
      "up = look up\n"<<
      "down = look down\n"<<
      "right = look right\n"<<
      "left = look left\n"<<
      ". = toggle Phantom force feedback\n"<<
      "page up = rotate the robot using Phantom\n"<<
      "page down = translate the robot using Phantom\n"<<
      "insert = increase motion step for Phantom interface\n"<<
      "delete = decrease motion step for Phantom interface\n"<<
      "home = increase force of Phantom force feedback\n"<<
      "end = decrease force of Phantom force feedback\n";

}


void gliCamera::SphericalToCartesian( void ){
   double DegToRad = 0.017453278;
   //m_CameraPos[0] *= -1;
   //m_CameraPos[1] *= -1;
   float R = m_CameraPos[2];
   //float elev = m_CameraPos[]
   m_CameraPos[1] = R * sin(m_currentElev*DegToRad);
   m_CameraPos[0] = -R * sin(m_currentAzim*DegToRad) * cos(m_currentElev*DegToRad);
   m_CameraPos[2] = R * cos(m_currentAzim*DegToRad) * cos(m_currentElev*DegToRad);
   //m_currentAzim = 0;
   //m_currentElev = 0;

   //m_CameraPos[2] *= -1;
}

void gliCamera::CartesianToSpherical( void ){
   double RadToDeg = 57.295827909;
   double R=pow(m_CameraPos[0]*m_CameraPos[0]+
         m_CameraPos[1]*m_CameraPos[1]+
         m_CameraPos[2]*m_CameraPos[2],.5);

   if(R > .0001)
      m_currentElev = RadToDeg*asin(m_CameraPos[1]/R);
   else m_currentElev = 0;
   m_currentAzim = RadToDeg*atan2(-m_CameraPos[0],m_CameraPos[2]);
   m_CameraPos[2]=R;
   m_CameraPos[0] = 0;
   m_CameraPos[1] = 0;

   //m_CameraPos[2] = 
}

void gliCamera::Move(void){
   m_WindowX.set(1,0,0);
   m_WindowY.set(0,1,0);
   RotateX(m_WindowX, m_currentElev);
   RotateY(m_WindowX, m_currentAzim);
   m_WindowX[2]=-m_WindowX[2];
   RotateX(m_WindowY, m_currentElev);
   RotateY(m_WindowY, m_currentAzim);  
   m_WindowY[2]=-m_WindowY[2];
   if(m_cartesian){
      RotateX(m_deltaDis, -m_currentElev-m_deltaElev);
      RotateY(m_deltaDis, -m_currentAzim-m_deltaAzim);
   }
   for( int iD=0;iD<3;iD++ ){
      m_CameraPos[iD]+=m_deltaDis[iD];
      m_deltaDis[iD]=0;
   }

}



////////////////////////////////////////////////////////////////////////////
//**
//** Help Function: handle x, y rotation
////////////////////////////////////////////////////////////////////////////

void RotateX(Vector3d& v, double degree){
   double c=cos(3.1415926*degree/180);
   double s=sin(3.1415926*degree/180);
   double v1=v[1]*c-v[2]*s;
   double v2=v[1]*s+v[2]*c;
   v[1]=v1; v[2]=v2;
}

void RotateY(Vector3d& v, double degree){
   double c=cos(3.1415926*degree/180);
   double s=sin(3.1415926*degree/180);
   double v0=v[0]*c+v[2]*s;
   double v2=-v[0]*s+v[2]*c;
   v[0]=v0; v[2]=v2;
}


///////////////////////////////////////////////////////////////////////////////
/// gliCameraFactory

/**
 * create default cameras
 */

gliCameraFactory::gliCameraFactory()
{
   createDefaultCameras();
   SetCamera(getCurrentCamera());
}

void gliCameraFactory::addCamera( const gliCamera& camera )
{ 
   gliCamera* cam=findCamera(camera.getCameraName());
   if(cam==NULL) m_Cameras.push_back(camera); //add if no old cam exit
}

gliCamera* gliCameraFactory::getCurrentCamera()
{
   return m_CurrentCam;
}

bool gliCameraFactory::setCurrentCamera(const string& name)
{
   gliCamera* cam=findCamera(name);
   if(cam==NULL) return false;
   m_CurrentCam=cam;
   return true;
}

///////////////////////////////////////////////////////////////////////////////
//protected

void gliCameraFactory::createDefaultCameras()
{
   gliCamera pers("pers",Point3d(0,0,500),Vector3d(0,1,0));    
   addCamera(pers);
   m_CurrentCam=&(m_Cameras.front());
}

gliCamera* gliCameraFactory::findCamera(const string& name)
{
   typedef list<gliCamera>::iterator CIT;
   for( CIT ic=m_Cameras.begin();ic!=m_Cameras.end();ic++ )
      if( ic->getCameraName()==name ) return &(*ic);

   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Singleton
gliCameraFactory g_CameraFactory;
gliCameraFactory& gliGetCameraFactory()
{
   return g_CameraFactory;
}

