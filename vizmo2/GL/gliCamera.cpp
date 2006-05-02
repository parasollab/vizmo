#include "gliCamera.h"

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
}

void gliCamera::Draw( void )
{
    glTranslatef( (m_CameraPos[0]+m_deltaDis[0]),
                  (m_CameraPos[1]+m_deltaDis[1]), 
                  -(m_CameraPos[2]+m_deltaDis[2]));

    glRotated(m_currentElev+m_deltaElev, 1.0, 0.0, 0.0);
    glRotated(m_currentAzim+m_deltaAzim, 0.0, 1.0, 0.0);
    /*
    gluLookAt( (m_CameraPos[0]+m_deltaDis[0]), //eye pos
              -(m_CameraPos[1]+m_deltaDis[1]),
              -(m_CameraPos[2]+m_deltaDis[2]),
               0,0,0, //center
               m_Up[0],m_Up[1],m_Up[2]);
    */
}

///////////////////////////////////////////////////////////////////////////////
//Handle Mouse Movement
bool gliCamera::MP( QMouseEvent * e )
{
    if( e->state()&Qt::ControlButton ){
        m_MousePressed=true;
        m_PressedPt=e->pos();
        return true; //handled
    }
    return false;
}

bool gliCamera::MR( QMouseEvent * e )
{
    if( !m_MousePressed ) 
        return false; //mouse is not pressed

    m_MousePressed=false;
    for( int iD=0;iD<3;iD++ ){
        m_CameraPos[iD]+=m_deltaDis[iD];
        m_deltaDis[iD]=0;
    }
    
    m_currentElev += m_deltaElev;
    m_currentAzim += m_deltaAzim;
            
    m_deltaElev = 0.0;
    m_deltaAzim = 0.0;

    return true;
}

//defined later
void RotateY(Vector3d& v, double degree);
void RotateX(Vector3d& v, double degree);
bool gliCamera::MM( QMouseEvent * e )  //mouse motion
{       

    if( !m_MousePressed ) return false; //mouse is not pressed

    Qt::ButtonState state=e->state();
    QPoint drag=e->pos()-m_PressedPt;
    //displacement
    if( state&Qt::MidButton ){ //mid button only
        m_deltaDis[0] = ((m_CameraPos[0]>5)?m_CameraPos[0]:5)*drag.x()/10.0;
        m_deltaDis[1] = -(((m_CameraPos[1]>5)?m_CameraPos[1]:5)*drag.y()/10.0);
    }
    else if(state&Qt::RightButton){ //right button only
        m_deltaDis[2] = (((m_CameraPos[2]>10)?m_CameraPos[2]:10)*drag.y()/10);
    }
    else if(state&Qt::LeftButton){ //left button only
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

///////////////////////////////////////////////////////////////////////////////
// Help Function
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
// gliCameraFactory

//create default cameras
gliCameraFactory::gliCameraFactory()
{
    createDefaultCameras();
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

