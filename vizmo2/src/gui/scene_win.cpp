#include "vizmo2.h"
#include "scene_win.h"
#include "main_win.h"
#include <GL/gli.h>
#include <GL/gliCamera.h>

///////////////////////////////////////////////////////////////////////////////
//This class handle opengl features

VizGLWin::VizGLWin(QWidget * parent, const char * name)
: QGLWidget(parent,name)
{ 
    setMinimumSize( 500, 500 ); 
    setFocusPolicy(QWidget::StrongFocus);

    R = G = B = 1;
}

void VizGLWin::resetCamera()
{
    double R=GetVizmo().GetEnvRadius();
    gliGetCameraFactory().getCurrentCamera()->setCameraPos(Point3d(0,0,2*R));
}

//used as callback for gli
inline vector<gliObj> vizmo_select(const gliBox& box){
    GetVizmo().Select(box);
    return GetVizmo().GetSelectedItem();
}

void VizGLWin::initializeGL()
{
    /*Setup light and material properties*/
    SetLight();

    /*others*/
    glEnable( GL_DEPTH_TEST);
    // glClearColor( 1 , 1, 1, 0 );
    glClearColor( R , G, B, 0 );
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glLineStipple(2,0xAAAA);

    //create models
    gliSetPickingFunction(vizmo_select);
}

void VizGLWin::resizeGL( int w, int h)
{
    gliWS(w,h);
    glViewport( 0, 0, (GLsizei)w, (GLsizei)h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 60, 1, 1, 1500 );
}

#include <GL/glut.h>
void VizGLWin::paintGL()
{
    //Init Draw
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glClearColor( R , G, B, 0 );

    ((VizmoMainWin*)parentWidget())->InitVizmo();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gliDraw();
    SetLightPos();
    GetVizmo().Display();
}

void VizGLWin::SetLight()
{
    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    GLfloat WhiteLight[] =  { 0.9f, 0.9f, 0.9f, 1.0f };
    glLightfv(GL_LIGHT0,GL_DIFFUSE,WhiteLight);
    glLightfv(GL_LIGHT1,GL_DIFFUSE,WhiteLight);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
}

void VizGLWin::mousePressEvent( QMouseEvent * e )
{
    if( gliMP(e) ){ updateGL(); return; }//handled by gli
    updateGL();
}

void VizGLWin::mouseReleaseEvent( QMouseEvent * e )
{
    if( gliMR(e) ){ updateGL(); return; }//handled by gli
    updateGL();
    if( e->button()==Qt::RightButton ){
      vector<gliObj>& objs=gliGetPickedSceneObjs();
	
      objs2=&objs;

        if( !objs.empty() ){


	  emit selectByRMB();
        }//empty
    }//not RMB
}

void VizGLWin::mouseMoveEvent ( QMouseEvent * e )
{
    if( gliMM(e) ){ updateGL(); return; }//handled by gli
    updateGL();
}

void VizGLWin::keyPressEvent ( QKeyEvent * e )
{
    if( gliKEY(e) ){ updateGL(); return; }//handled by gli
    e->ignore(); //not handled
}














