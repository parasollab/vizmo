#include "gliTransTool.h"

#include <Quaternion.h>
using namespace mathtool;

#include <QMouseEvent>
#include <QKeyEvent>

#include "Camera.h"
#include "PickBox.h"
#include "GLUtilities.h"
#include "Models/Model.h"

///////////////////////////////////////////////////////////////////////////////
// gliTToolBase
///////////////////////////////////////////////////////////////////////////////

TransformableModel* gliTToolBase::m_pSObj = NULL;
Point3d gliTToolBase::m_sObjPrj;
Point3d gliTToolBase::m_xPrj;
Point3d gliTToolBase::m_yPrj;
Point3d gliTToolBase::m_zPrj;
int gliTToolBase::m_h=0;
int gliTToolBase::m_w=0;
int gliTToolBase::m_hitX=0;
int gliTToolBase::m_hitY=0;

void gliTToolBase::Draw(void)
{
    if( m_pSObj==NULL ) return;
    glMatrixMode(GL_PROJECTION); //change to Ortho view
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,m_w,0,m_h,-100,100);
    Draw(false);

    //pop GL_PROJECTION
    glMatrixMode(GL_PROJECTION); //change to Pers view
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void gliTToolBase::Project2Win(){
  if(m_pSObj) {
    const Vector3d& pos = m_pSObj->Translation();

    Point3d pts[4]={pos, pos + Point3d(0.1, 0, 0),
      pos + Point3d(0, 0.1, 0), pos + Point3d(0, 0, 0.1)};
    ProjectToWindow(pts, 4);
    m_sObjPrj = pts[0]; m_xPrj = pts[1]; m_yPrj = pts[2]; m_zPrj = pts[3];
  }
}

///////////////////////////////////////////////////////////////////////////////
// class gliMoveTool
///////////////////////////////////////////////////////////////////////////////

void gliMoveTool::Draw(bool bSelect)
{
    glDisable(GL_LIGHTING);
    //draw reference axis
    double ox=m_sObjPrj[0]; double oy=m_sObjPrj[1];
    Point3d x_dir=m_sObjPrj+(m_xPrj-m_sObjPrj).normalize()*50;
    Point3d y_dir=m_sObjPrj+(m_yPrj-m_sObjPrj).normalize()*50;
    Point3d z_dir=m_sObjPrj+(m_zPrj-m_sObjPrj).normalize()*50;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    if( m_selType!=NON ){
        glPointSize(3);
        glBegin(GL_LINES);
        glColor3d(0.6,0.6,0.6);
        glVertex3d(m_sObjPrjC[0],m_sObjPrjC[1],-5);
        glVertex3d(ox,oy,-5);
        glEnd();
        glBegin(GL_POINTS);
        glVertex2d(m_sObjPrjC[0],m_sObjPrjC[1]);
        glEnd();
    }

    //draw x axis
    if( bSelect ) glLoadName(X_AXIS);
    glBegin(GL_LINES);
    if( !bSelect ) glColor3f(1,0,0);
    glVertex2d(ox,oy);
    glVertex2d(x_dir[0],x_dir[1]);
    glEnd();

    //draw y axis
    if( bSelect ) glLoadName(Y_AXIS);
    glBegin(GL_LINES);
    if( !bSelect ) glColor3f(0,1,0);
    glVertex2d(ox,oy);
    glVertex2d(y_dir[0],y_dir[1]);
    glEnd();

    //draw z axis
    if( bSelect ) glLoadName(Z_AXIS);
    glBegin(GL_LINES);
    if( !bSelect ) glColor3f(0,0,1);
    glVertex2d(ox,oy);
    glVertex2d(z_dir[0],z_dir[1]);
    glEnd();

    //draw center
    if( !bSelect ){
        glBegin(GL_LINE_LOOP);
        glColor3f(0.9f,0.9f,0);
        glVertex2d(ox-5,oy+5);
        glVertex2d(ox+5,oy+5);
        glVertex2d(ox+5,oy-5);
        glVertex2d(ox-5,oy-5);
        glEnd();
    }

    glPopMatrix();
}


//Handle Mouse
bool gliMoveTool::MP( QMouseEvent * e ) //mouse button pressed
{
    m_selType=NON;
    if( m_pSObj==NULL ) return false;
    if( e->button()&Qt::LeftButton ) return false; //not LMB
    if( Select(e->pos().x(),e->pos().y()) ){
        m_sObjPosC = m_pSObj->Translation(); //store old pos
        m_sObjPrjC=m_sObjPrj;
        m_hitX=e->pos().x(); m_hitY=m_h-e->pos().y();
        m_hitUnPrj = ProjectToWorld(m_hitX, m_hitY, m_sObjPosC);
        return true;
    }
    return false;
}

bool gliMoveTool::MR( QMouseEvent * e ) //mouse button released
{
    SelType oldSelT=m_selType; m_selType=NON;
    return oldSelT!=NON;
}

bool gliMoveTool::MM( QMouseEvent * e )  //mouse motion
{
    if( m_selType==NON ) return false; //nothing selected
    int x=e->pos().x();
    int y=m_h-e->pos().y();

    Point3d cur_pos = ProjectToWorld(x, y, m_sObjPosC);
    Vector3d v=cur_pos-m_hitUnPrj;

    if(m_selType == X_AXIS) {
      v(v[0], 0, 0);
      m_pSObj->Translation()[0] = m_sObjPosC[0]+v[0];
    }
    if(m_selType == Y_AXIS) {
      v(0, v[1], 0);
      m_pSObj->Translation()[1] = m_sObjPosC[1]+v[1];
    }
    if(m_selType == Z_AXIS) {
      v(0, 0, v[2]);
      m_pSObj->Translation()[2] = m_sObjPosC[2]+v[2];
    }
    if(m_selType == VIEW_PLANE) {
        Point3d npos = m_sObjPosC+v;
        m_pSObj->Translation() += npos;
    }

    Project2Win();
    return true;
}

bool gliMoveTool::Select(int x, int y)
{
    y=m_h-y;
    if( fabs(x-m_sObjPrj[0])<10 && fabs(y-m_sObjPrj[1])<10 ){
        m_selType=VIEW_PLANE; return true;
    }

    //do selection buffer
    GLuint hitBuffer[1000];
    GLint viewport[4];

    // prepare for selection mode
    glSelectBuffer(1000, hitBuffer);
    glRenderMode(GL_SELECT);

    // get view port
    glGetIntegerv(GL_VIEWPORT, viewport);

    // number stact
    glInitNames();
    glPushName(0);

    // change view volume
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluPickMatrix( x, y, 10, 10, viewport);
    glOrtho(0,m_w,0,m_h,-100,100);

    //draw
    Draw(true);
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glFlush();

    //check result
    if( glRenderMode( GL_RENDER )==0 ) return false;
    m_selType=(SelType)(hitBuffer[3]);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Scale Tool
void gliScaleTool::Draw(bool bSelect)
{
    glDisable(GL_LIGHTING);
    //draw reference axis
    double ox=m_sObjPrj[0]; double oy=m_sObjPrj[1];
    Point3d x_dir=m_sObjPrj+(m_xPrj-m_sObjPrj).normalize()*50;
    Point3d y_dir=m_sObjPrj+(m_yPrj-m_sObjPrj).normalize()*50;
    Point3d z_dir=m_sObjPrj+(m_zPrj-m_sObjPrj).normalize()*50;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    //draw x axis
    if( bSelect ) glLoadName(X_AXIS);
    glBegin(GL_LINE_LOOP);
    glColor3f(1,0,0);
    glVertex2d(x_dir[0]-3,x_dir[1]-3);
	glVertex2d(x_dir[0]+3,x_dir[1]-3);
	glVertex2d(x_dir[0]+3,x_dir[1]+3);
	glVertex2d(x_dir[0]-3,x_dir[1]+3);
    //glVertex2d(m_xPrj[0],m_xPrj[1]);
    glEnd();

    //draw y axis
    if( bSelect ) glLoadName(Y_AXIS);
    glBegin(GL_LINE_LOOP);
    glColor3f(0,1,0);
    glVertex2d(y_dir[0]-3,y_dir[1]-3);
	glVertex2d(y_dir[0]+3,y_dir[1]-3);
	glVertex2d(y_dir[0]+3,y_dir[1]+3);
	glVertex2d(y_dir[0]-3,y_dir[1]+3);    //glVertex2d(m_yPrj[0],m_yPrj[1]);
    glEnd();

    //draw z axis
    if( bSelect ) glLoadName(Z_AXIS);
    glBegin(GL_LINE_LOOP);
    glColor3f(0,0,1);
    glVertex2d(z_dir[0]-3,z_dir[1]-3);
	glVertex2d(z_dir[0]+3,z_dir[1]-3);
	glVertex2d(z_dir[0]+3,z_dir[1]+3);
	glVertex2d(z_dir[0]-3,z_dir[1]+3);
    glEnd();

    //draw axis and center
    if( !bSelect ){

		glBegin(GL_LINES);
		//x
		glColor3f(1,0,0);
		glVertex2d(ox,oy);
		glVertex2d(x_dir[0],x_dir[1]);
		//y
		glColor3f(0,1,0);
		glVertex2d(ox,oy);
        glVertex2d(y_dir[0],y_dir[1]);
		//z
		glColor3f(0,0,1);
        glVertex2d(ox,oy);
        glVertex2d(z_dir[0],z_dir[1]);
		glEnd();
		//center
        glBegin(GL_LINE_LOOP);
        glColor3f(0.9f,0.9f,0);
        glVertex2d(ox-5,oy+5);
        glVertex2d(ox+5,oy+5);
        glVertex2d(ox+5,oy-5);
        glVertex2d(ox-5,oy-5);
        glEnd();
    }

    glPopMatrix();
}


//Handle Mouse Movement
bool gliScaleTool::MP( QMouseEvent * e ) //mouse button pressed
{
    m_selType=NON;
    if( m_pSObj==NULL ) return false;
    if( e->button()&Qt::LeftButton ) return false; //not LMB
    if( Select(e->pos().x(),e->pos().y()) ){
        m_sObjPosC = m_pSObj->Translation(); //store old pos
        m_sObjPrjC = m_sObjPrj;
        m_hitX=e->pos().x(); m_hitY=m_h-e->pos().y();
        m_hitUnPrj = ProjectToWorld(m_hitX, m_hitY, m_sObjPosC);
        m_osX = m_pSObj->Scale()[0];
        m_osY = m_pSObj->Scale()[1];
        m_osZ = m_pSObj->Scale()[2];
        return true;
    }
    return false;
}

bool gliScaleTool::MR( QMouseEvent * e ) //mouse button released
{
    SelType oldSelT=m_selType; m_selType=NON;
    return oldSelT!=NON;
}

bool gliScaleTool::MM( QMouseEvent * e )  //mouse motion
{
    if( m_selType==NON ) return false; //nothing selected
    int x=e->pos().x();
    int y=m_h-e->pos().y();

    Point3d cur_pos = ProjectToWorld(x, y, m_sObjPosC);
    Vector3d v=(cur_pos-m_hitUnPrj)/10;

    if(m_selType==X_AXIS) {
      if(m_osX+v[0]>0)
        m_pSObj->Scale()[0] = m_osX+v[0];
    }
    if(m_selType==Y_AXIS) {
      if(m_osY+v[1]>0)
        m_pSObj->Scale()[1] = m_osY+v[1];
    }
    if(m_selType==Z_AXIS) {
      if(m_osZ+v[2]>0)
        m_pSObj->Scale()[2] = m_osZ+v[2];
    }
    if(m_selType==VIEW_PLANE) {
      if(m_osX+v[0]>0 && m_osY+v[0]>0 && m_osZ+v[0]>0)
        m_pSObj->Scale()(m_osX+v[0], m_osY+v[0], m_osZ+v[0]);
    }

    Project2Win();
    return true;
}

bool gliScaleTool::Select(int x, int y)
{
    y=m_h-y;
    if( fabs(x-m_sObjPrj[0])<10 && fabs(y-m_sObjPrj[1])<10 ){
        m_selType=VIEW_PLANE; return true;
    }

    //do selection buffer
    GLuint hitBuffer[1000];
    GLint viewport[4];

    // prepare for selection mode
    glSelectBuffer(1000, hitBuffer);
    glRenderMode(GL_SELECT);

    // get view port
    glGetIntegerv(GL_VIEWPORT, viewport);

    // number stact
    glInitNames();
    glPushName(0);

    // change view volum
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluPickMatrix( x, y, 10, 10, viewport);
    glOrtho(0,m_w,0,m_h,-100,100);

    //draw
    Draw(true);
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glFlush();

    //check result
    if( glRenderMode( GL_RENDER )==0 ) return false;
    m_selType=(SelType)(hitBuffer[3]);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// class gliRotateTool
///////////////////////////////////////////////////////////////////////////////

void gliRotateTool::Draw(bool bSelect)
{
    glDisable(GL_LIGHTING);
    //draw reference axis
    Camera* cam = GetCameraFactory().GetCurrentCamera();
    double ox=m_sObjPrj[0]; double oy=m_sObjPrj[1];

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glTranslatef(ox,oy,0);

    glPushMatrix();
    glRotated(cam->GetCameraElev(), 1.0, 0.0, 0.0);
    glRotated(cam->GetCameraAzim(), 0.0, 1.0, 0.0);

    if( m_selType!=NON ){
        Vector3d v1,v2;
        if( m_selType==X_AXIS ){v1=m_lAC[1]; v2=m_lAC[2]; }
        else if( m_selType==Y_AXIS ){v1=m_lAC[2]; v2=m_lAC[0]; }
        else if( m_selType==Z_AXIS ){v1=m_lAC[0]; v2=m_lAC[1]; }
        else if( m_selType==OUTLINE ){
            v1 = cam->GetWindowX(); v2 = cam->GetWindowY();
        }
        Vector3d s=cos(m_hitAngle)*v1*m_r+sin(m_hitAngle)*v2*m_r;
        Vector3d e=cos(m_curAngle)*v1*m_r+sin(m_curAngle)*v2*m_r;
        //draw 2 points
        glPointSize(4);
        glBegin(GL_POINTS);
        glColor4f(0.6f,0.6f,0.6f,0.6f);
        glVertex3d(s[0],s[1],s[2]);
        glVertex3d(e[0],e[1],e[2]);
        glEnd();
    }

    //draw circle around x axis
    if( bSelect ) glLoadName(X_AXIS);
    if( !bSelect ) glColor3f(1,0,0);
    DrawArc(m_r, m_arc[0][0], m_arc[0][1], m_lA[1], m_lA[2]);

    //draw y axis
    if( bSelect ) glLoadName(Y_AXIS);
    if( !bSelect ) glColor3f(0,1,0);
    DrawArc(m_r,m_arc[1][0],m_arc[1][1],m_lA[2],m_lA[0]);

    //draw z axis
    if( bSelect ) glLoadName(Z_AXIS);
    if( !bSelect ) glColor3f(0,0,1);
    DrawArc(m_r,m_arc[2][0],m_arc[2][1],m_lA[0],m_lA[1]);

    glPopMatrix(); //pop camera rotation

    //draw center
    if( !bSelect ){
        glColor3f(0.9f,0.9f,0);
        DrawCircle(m_r+5);
        glColor3f(0.6f,0.6f,0.6f);
        DrawCircle(m_r);
    }

    glPopMatrix();
}

Point3d gliRotateTool::UnProj2World
(const Point3d& ref,const Vector3d& n,int x, int y)
{
    GLint viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport);

    // change view volum
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,m_w,0,m_h,-100,100);
    Point3d p = ProjectToWorld(x, y, ref, n);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    return p;
}

bool gliRotateTool::MP( QMouseEvent * e ) //mouse button pressed
{
    //SelType oldSelT=m_selType; m_selType=NON;
    if( m_pSObj==NULL ) return false;
    if( e->button()&Qt::LeftButton ) return false; //not LMB

    int x=e->pos().x(); int y=e->pos().y();
    if( Select(x,y) ){
        y=m_h-y;
        //gliCamera* pcam=gliGetCameraFactory().getCurrentCamera();
        m_sObjPosC = m_pSObj->Translation();
        m_sObjQC = m_pSObj->RotationQ();
        for( int id=0;id<3;id++ ) m_lAC[id]=m_lA[id];
        Vector3d axis, v1, v2; //rotation axis
        if( m_selType==X_AXIS ){
            axis=m_lAC[0]; v1=m_lAC[1]; v2=m_lAC[2];
        }
        else if( m_selType==Y_AXIS ){
            axis=m_lAC[1]; v1=m_lAC[2]; v2=m_lAC[0];
        }
        else if( m_selType==Z_AXIS ){
            axis=m_lAC[2]; v1=m_lAC[0]; v2=m_lAC[1];
        }
        else if( m_selType==OUTLINE ){
            Camera* cam = GetCameraFactory().GetCurrentCamera();
            axis = cam->GetWindowZ();
            v1 = cam->GetWindowX();
            v2 = cam->GetWindowY();
        }

        Point3d prj = ProjectToWorld(x, y, m_sObjPosC, axis);
        Vector3d tmp=prj-m_sObjPosC;
        m_curAngle=m_hitAngle=ComputAngle(tmp,v1,v2);
        return true;
    }

    return false;
}

bool gliRotateTool::MR( QMouseEvent * e ) //mouse button released
{
    SelType oldSelT=m_selType; m_selType=NON;
    return oldSelT!=NON;
}


bool gliRotateTool::MM( QMouseEvent * e )  //mouse motion
{

    if( m_selType==NON ) return false; //nothing selected
    int x=e->pos().x();
    int y=m_h-e->pos().y();

    Vector3d axis, v1, v2; //rotation axis
    if( m_selType==X_AXIS ){
        axis=m_lAC[0]; v1=m_lAC[1]; v2=m_lAC[2];
    }
    else if( m_selType==Y_AXIS ){
        axis=m_lAC[1]; v1=m_lAC[2]; v2=m_lAC[0];
    }
    else if( m_selType==Z_AXIS ){
        axis=m_lAC[2]; v1=m_lAC[0]; v2=m_lAC[1];
    }
    else if( m_selType==OUTLINE ){
        Camera* cam = GetCameraFactory().GetCurrentCamera();
        axis = cam->GetWindowZ();
        v1 = cam->GetWindowX();
        v2 = cam->GetWindowY();
    }

    Point3d prj = ProjectToWorld(x, y, m_sObjPosC, axis);
    Vector3d tmp=prj-m_sObjPosC;
    m_curAngle=ComputAngle(tmp,v1,v2);

    double da=(m_curAngle-m_hitAngle); //displacement angle
    //clamp between PI and -PI
    if( da>PI ) da -= TWOPI;
    else if( da<-PI ) da += TWOPI;

    //compute new q
    Quaternion q(cos(da/2), sin(da/2) * axis);
    m_pSObj->RotationQ() = (q*m_sObjQC).normalized();

    //update rotation variables for this body
    //m_pSObj->Quaternion2Euler();
    /*Matrix3x3 m;
    m = m_pSObj->getMatrix();
    Vector3d v;
    v = m_pSObj->MatrixToEuler(m);
    */

    //values in radians
    //m_pSObj->m_rotPoly[0] = v[0];
    //m_pSObj->m_rotPoly[1] = v[1];
    //m_pSObj->m_rotPoly[2] = v[2];

    ComputLocalAxis();
    ComputAngles();
    return true;
}

bool gliRotateTool::Select(int x, int y)
{
    y=m_h-y;

    //do selection buffer
    GLuint hitBuffer[1000];
    GLint viewport[4];

    // prepare for selection mode
    glSelectBuffer(1000, hitBuffer);
    glRenderMode(GL_SELECT);

    // get view port
    glGetIntegerv( GL_VIEWPORT, viewport);

    // number stact
    glInitNames();
    glPushName(0);

    // change view volum
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluPickMatrix( x, y, 10, 10, viewport);
    glOrtho(0,m_w,0,m_h,-100,100);

    //draw
    Draw(true);
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glFlush();

    //check result
    if( glRenderMode( GL_RENDER )!=0 ){
        m_selType=(SelType)(hitBuffer[3]);
        return true;
    }

    double dx=m_sObjPrj[0]-x; double dy=m_sObjPrj[1]-y;
    double dist=sqrt(dx*dx+dy*dy);
    if( dist<m_r+10 && dist>m_r ){
        m_selType=OUTLINE;
        return true;
    }

    if( dist<m_r+5 ){
        m_selType=CENTER;
        return true;
    }

    return false;
}

void gliRotateTool::ComputAngles()
{
    Camera* cam = GetCameraFactory().GetCurrentCamera();
    Vector3d v = cam->GetWindowZ();
    ComputAngles(m_arc[0], m_lA[0], m_lA[1], m_lA[2], v);
    ComputAngles(m_arc[1], m_lA[1], m_lA[2], m_lA[0], v);
    ComputAngles(m_arc[2], m_lA[2], m_lA[0], m_lA[1], v);
}

void gliRotateTool::ComputAngles
(double angle[2],Vector3d& n,Vector3d& v1,Vector3d& v2,Vector3d& view)
{
    Vector3d s=view%n;
    Vector3d e=n%view;
    angle[0]=atan2(s*v2,s*v1);
    angle[1]=atan2(e*v2,e*v1);
    if( angle[1]<angle[0] ) angle[1]+=TWOPI;
}

void gliRotateTool::ComputLocalAxis()
{
    if( m_pSObj==NULL ) return;

    static Vector3d x(1,0,0);
    static Vector3d y(0,1,0);
    static Vector3d z(0,0,1);
    const Quaternion& q = m_pSObj->RotationQ();

    m_lA[0]=(q*x*(-q)).imaginary();
    m_lA[1]=(q*y*(-q)).imaginary();
    m_lA[2]=(q*z*(-q)).imaginary();
}

///////////////////////////////////////////////////////////////////////////////
// class GLTransformTool

void GLTransformTool::CheckSelectObject()
{
    m_rT.setSObject(NULL);

    //get selected objects
    const vector<Model*>& sobjs = GetPickedSceneObjs();
    if( sobjs.empty() ) return;
    m_rT.setSObject((TransformableModel*)sobjs.front());
}

void GLTransformTool::Draw(void)
{
    if( m_pTool!=NULL ) m_pTool->Draw();
}

bool GLTransformTool::MP( QMouseEvent * e ) //mouse button pressed
{
    if( m_pTool!=NULL ) return m_pTool->MP(e);
    return false;
}

bool GLTransformTool::MR( QMouseEvent * e ) //mouse button released
{
    if( m_pTool!=NULL ) return m_pTool->MR(e);
    return false;
}

bool GLTransformTool::MM( QMouseEvent * e )  //mouse motion
{
  if( m_pTool!=NULL ) return m_pTool->MM(e);
    return false;
}

bool GLTransformTool::KEY( QKeyEvent * e ) //Key
{
    switch(e->key()){
        case 'q': case 'Q':
            if(m_pTool!=NULL) m_pTool->Disable();
            m_pTool=NULL;  return true;
        case 'w': case 'W':
            if(m_pTool!=NULL) m_pTool->Disable();
            m_pTool=&m_mT; m_pTool->Enable(); return true;
        case 'e': case 'E':
            if(m_pTool!=NULL) m_pTool->Disable();
            m_pTool=&m_rT; m_pTool->Enable(); return true;
        case 'r': case 'R':
            if(m_pTool!=NULL) m_pTool->Disable();
            m_pTool=&m_sT; m_pTool->Enable(); return true;
        default: return false; //not handled
    }

}


///////////////////////////////////////////////////////////////////////////////
//Singleton for GLTransformTool
GLTransformTool g_TransformTool;
GLTransformTool& gliGetTransformTool()
{
    return g_TransformTool;
}
