#include "vizmo2.h"
#include "scene_win.h"
#include "main_win.h"
#include <GL/gli.h>
#include <GL/gliCamera.h>
#include <GL/gliFont.h>


///////////////////////////////////////////////////////////////////////////////
//This class handle opengl features

VizGLWin::VizGLWin(QWidget * parent, const char * name)
: QGLWidget(parent,name)
{ 
    setMinimumSize( 300, 300 );
    setFocusPolicy(QWidget::StrongFocus);
    
    R = G = B = 1;
    takingSnapShot=false;
}

void VizGLWin::getWidthHeight(int *w,int *h)
{
    *w=width();
    *h=height();
}

void VizGLWin::togleSlectionSlot()
{
    takingSnapShot=!takingSnapShot;
}

void VizGLWin::resetCamera()
{
    double R=GetVizmo().GetEnvRadius();
    gliGetCameraFactory().getCurrentCamera()->setCameraPos(Point3d(0,0,4*R));
}

//used as callback for gli
inline vector<gliObj> vizmo_select(const gliBox& box){
    GetVizmo().Select(box);
    return GetVizmo().GetSelectedItem();
}

void VizGLWin::initializeGL()
{
    
    // BSS - trying to make shadeing smooth
    glShadeModel(GL_SMOOTH);
    
    
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
    if( w>h ) glViewport( 0, 0, w, w );
    else  glViewport( 0, 0, h, h );

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
	drawText();
}

void VizGLWin::SetLight()
{
    //glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
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


void VizGLWin::simulateMouseUpSlot()
{
    gliSimMouseUp();
    //  updateGL();
}

void VizGLWin::getBoxDimensions(int *xOffset, int *yOffset,int *w,int *h)
{
    gliPickBoxDim(xOffset,yOffset,w,h);
    
}

void VizGLWin::mouseReleaseEvent( QMouseEvent * e )
{
    if( gliMR(e,takingSnapShot) ){ //handled by gli
		updateGL();	return; }

    updateGL();
    if( e->button()==Qt::RightButton ){
        vector<gliObj>& objs=gliGetPickedSceneObjs();
        objs2=&objs;
        if( !objs.empty() ){
            emit selectByRMB();
        }//empty
    }//not RMB
	emit selected();
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

void VizGLWin::drawText()
{
	vector<gliObj>& sel=GetVizmo().GetSelectedItem();
	typedef vector<gliObj>::iterator SIT;

    glPushAttrib(GL_CURRENT_BIT);

    //draw reference axis
    glMatrixMode(GL_PROJECTION); //change to Ortho view
    glPushMatrix(); 
    glLoadIdentity();
    gluOrtho2D(0,20,0,20);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
	glDisable(GL_LIGHTING);
    
    //Draw text
	glTranslated(0,20,0);
	for(SIT i=sel.begin();i!=sel.end();i++){
		CGLModel * gl=(CGLModel *)(*i);
		list<string> info=gl->GetInfo();
		drawText(info);
	}

	glPopMatrix();

    //pop GL_PROJECTION
    glMatrixMode(GL_PROJECTION); //change to Pers view
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
}

void VizGLWin::drawText(list<string>& info)
{
	typedef list<string>::iterator SIT;
	
	for(SIT i=info.begin();i!=info.end();i++){
		//////////////////////////////////////////////
		glTranslated(0,-0.5,0);
		glColor3f(0.2,0.2,0.5);
		drawstr(0.2,0,0,i->c_str());
	}
}












