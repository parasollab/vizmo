#ifndef _GLI_TOOL_H_
#define _GLI_TOOL_H_

#include "glut.h"

#include "gliCamera.h"
#include "Font.h"

//draw axis rotated accroding to camera's rotation
inline void gliDrawRotateAxis( const gliCamera& camera )
{
    static int gid=-1;
    if(gid==-1 ){ //no gid is created
        gid=glGenLists(1);
        glNewList(gid,GL_COMPILE);

        //create lines
        glLineWidth(2);
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        glColor3f(1,0,0);
        glVertex3f(0,0,0);
        glVertex3f(1,0,0);
        glColor3f(0,1,0);
        glVertex3f(0,0,0);
        glVertex3f(0,1,0);
        glColor3f(0,0,1);
        glVertex3f(0,0,0);
        glVertex3f(0,0,1);
        glEnd();

        //create letters
        glColor3f(1,0,0);
        DrawStr(1.2f,0,0,"x");
        glColor3f(0,1,0);
        DrawStr(0,1.2f,0,"y");
        glColor3f(0,0,1);
        DrawStr(0,0,1.4f,"z");

        glEndList();
    }

    //draw reference axis
    glMatrixMode(GL_PROJECTION); //change to Ortho view
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,20,0,20,-20,20);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glTranslatef(1.2f,1.2f,0);
    glRotated(camera.getCameraElev(), 1.0, 0.0, 0.0);
    glRotated(camera.getCameraAzim(), 0.0, 1.0, 0.0);
    glCallList(gid);
    glPopMatrix();

    //pop GL_PROJECTION
    glMatrixMode(GL_PROJECTION); //change to Pers view
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

//draw axis rotated accroding to camera's rotation
inline void gliDrawGrid()
{
    static int gid=-1;
    if(gid==-1 ){ //no gid is created
        gid=glGenLists(1);
        glNewList(gid,GL_COMPILE);
        glLineWidth(1);
        glDisable(GL_LIGHTING);
        glColor3f(0,0,0);

        glBegin(GL_LINE_LOOP);
        glVertex3f(-50,0,0);
        glVertex3f(-50,0,50);
        glVertex3f(50,0,50);
        glVertex3f(50,0,-50);
        glVertex3f(-50,0,-50);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(0,0,0);
        glVertex3f(-50,0,0);
        glVertex3f(0,0,0);
        glVertex3f(50,0,0);
        glVertex3f(0,0,0);
        glVertex3f(0,0,-50);
        glVertex3f(0,0,0);
        glVertex3f(0,0,50);
        glEnd();

        glEndList();
    }

    glCallList(gid);

}

inline Point3d gliProject2Window(const Point3d& pt)
{
    //Get matrix info
    int ViewPort[4]; double ModelViewM[16], ProjM[16];
    glGetIntegerv( GL_VIEWPORT, ViewPort );
    glGetDoublev( GL_MODELVIEW_MATRIX, ModelViewM );
    glGetDoublev( GL_PROJECTION_MATRIX, ProjM );

    Point3d proj;
    gluProject(pt[0], pt[1], pt[2],
               ModelViewM, ProjM, ViewPort,
               &proj[0],&proj[1],&proj[2]);
    return  proj;
}

inline void gliProject2Window(Point3d* pts, int size)
{
    //Get matrix info
    int ViewPort[4]; double ModelViewM[16], ProjM[16];
    glGetIntegerv( GL_VIEWPORT, ViewPort );
    glGetDoublev( GL_MODELVIEW_MATRIX, ModelViewM );
    glGetDoublev( GL_PROJECTION_MATRIX, ProjM );

    for( int i=0;i<size;i++ ){
        double proj[3];
        gluProject(pts[i][0], pts[i][1], pts[i][2],
                   ModelViewM, ProjM, ViewPort,
                   &proj[0],&proj[1],&proj[2]);
        pts[i] = proj;
    }
}

//x,y is window coord
//a reference point, ref, and a nornmal, n, defines a plane
//the unprojected point is intersetion of shooting ray from (x,y)
//and the plane defined by n and ref
inline Point3d gliUnProj2World
(const Point3d& ref,const Vector3d& n,int x, int y)
{
    Vector3d refv=ref-Point3d(0,0,0);
    int ViewPort[4]; double ModelViewM[16], ProjM[16];
    glGetIntegerv( GL_VIEWPORT, ViewPort );
    glGetDoublev( GL_MODELVIEW_MATRIX, ModelViewM );
    glGetDoublev( GL_PROJECTION_MATRIX, ProjM );

    Vector3d s,e; //start and end of ray

    /* unproject to plane defined by current x and y direction*/
    gluUnProject((GLdouble)x, (GLdouble)y, 0.0,ModelViewM,
                 ProjM, ViewPort, &s[0], &s[1], &s[2]);
    gluUnProject((GLdouble)x, (GLdouble)y, 1.0,ModelViewM,
                 ProjM, ViewPort, &e[0], &e[1], &e[2]);

    double t=0;
    double base=((e-s)*n);

    if( fabs(base)/(s-e).norm()<1e-2 ){
        Vector3d s_e=(s-e).normalize();
        Vector3d v=n%s_e;
        Point3d proj=gliProject2Window(ref);

        //find d1 and d2
        double d1=sqrt((proj[0]-x)*(proj[0]-x)+(proj[1]-y)*(proj[1]-y));
        Vector3d g=refv+0.1*v;
        Point3d proj_g=gliProject2Window(Point3d(0,0,0)+g);
        Point3d x_y(x,y,0);
        if( (proj_g-proj)*(x_y-proj)<0 ) d1=-d1;
        d1=fmod(d1,200);
        if( d1>100 ) d1=d1-200;
        else if( d1<-100 ) d1=200+d1;

        double d2;
        if( d1>50 ){ d1=100-d1; d2=-sqrt(2500-d1*d1); }
        else if (d1<-50 ){ d1=-100-d1; d2=-sqrt(2500-d1*d1); }
        else d2=sqrt(2500-d1*d1);
        g=refv+d1*v;
        return Point3d(0,0,0)+(g+d2*s_e);
    }
    else
        t=(refv*n-s*n)/base;

    return Point3d(0,0,0)+((1-t)*s+t*e);
}

inline Point3d gliUnProj2World
(const Point3d& ref, int x, int y)
{
    const gliCamera * pcam=gliGetCameraFactory().getCurrentCamera();
    Vector3d n=pcam->getWindowZ();
    return gliUnProj2World(ref,n,x,y);
}

inline void gliDrawCircle2D(double radius)
{
    float theta;
    glBegin(GL_LINE_LOOP);
    for(theta=0; theta<2 * PI; theta+=0.2f )
        glVertex2f( (float)(radius*cos(theta)), (float)(radius*sin(theta)) );
    glEnd();
}

//Draw arc from radius s to e with radius, r, on plane made by v1, v2
inline void
gliDrawArc(double r, double s, double e, const Vector3d& v1, const Vector3d& v2) {
  float t;
  Vector3d pt;
  glBegin(GL_LINE_STRIP);
  for(t=s; t<e; t+=0.2f) {
    pt = r*cos(t)*v1 + r*sin(t)*v2;
    glVertex3dv(pt);
  }
  pt = r*cos(e)*v1 + r*sin(e)*v2;
  glVertex3dv(pt);
  glEnd();
}

#endif //_GLI_TOOL_H_

