#include "gliPickBox.h"
#include "gliTransTool.h"

#include <GL/glut.h>
//Added by qt3to4:
#include <QMouseEvent>

void gliPickBox::Draw( void )
{
    if( !m_lMB_DOWN ) return;

    //change to Ortho view
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0,m_w,0,m_h);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glEnable(GL_LINE_STIPPLE);
	glLineWidth(1);
    glBegin(GL_LINE_LOOP);
    glColor3f(0,0,0);
    glVertex2f(m_pickBox.l,m_pickBox.b);
    glVertex2f(m_pickBox.r,m_pickBox.b);
    glVertex2f(m_pickBox.r,m_pickBox.t);
    glVertex2f(m_pickBox.l,m_pickBox.t);
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    glPopMatrix();

    //change back to pers view
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void gliPickBox::MP( QMouseEvent * e ) //mouse button
{
    if( e->button()!=Qt::LeftButton ) return;
    m_lMB_DOWN=true;
    m_pickBox.r=m_pickBox.l=e->pos().x();
    m_pickBox.b=m_pickBox.t=m_h-e->pos().y();
}

void gliPickBox::MR( QMouseEvent * e ) //mouse button
{
    m_lMB_DOWN=false;
}

void gliPickBox::MM( QMouseEvent * e )  //mouse motion
{
    if( !m_lMB_DOWN ) return;
    m_pickBox.r=e->pos().x();
    m_pickBox.t=m_h-e->pos().y();
}

///////////////////////////////////////////////////////////////////////////
//Singleton
gliPickBox g_PickBox;
gliPickBox & gliGetPickBox(){ return g_PickBox; }
