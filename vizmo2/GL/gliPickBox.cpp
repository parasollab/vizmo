#include "gliPickBox.h"
#include "gliTransTool.h"

#include <GL/glut.h>

void gliPickBox::Draw( void )
{
    if( !m_LMB_DOWN ) return;

    //change to Ortho view
    glMatrixMode(GL_PROJECTION); 
    glPushMatrix(); 
    glLoadIdentity();
    gluOrtho2D(0,m_W,0,m_H);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glEnable(GL_LINE_STIPPLE);
	glLineWidth(1);
    glBegin(GL_LINE_LOOP);
    glColor3f(0,0,0);
    glVertex2f(m_PickBox.l,m_PickBox.b);
    glVertex2f(m_PickBox.r,m_PickBox.b);
    glVertex2f(m_PickBox.r,m_PickBox.t);
    glVertex2f(m_PickBox.l,m_PickBox.t);
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
    m_LMB_DOWN=true;
    m_PickBox.r=m_PickBox.l=e->pos().x(); 
    m_PickBox.b=m_PickBox.t=m_H-e->pos().y();
}

void gliPickBox::MR( QMouseEvent * e ) //mouse button
{
    m_LMB_DOWN=false;
}

void gliPickBox::MM( QMouseEvent * e )  //mouse motion
{   
    if( !m_LMB_DOWN ) return;
    m_PickBox.r=e->pos().x(); 
    m_PickBox.t=m_H-e->pos().y();
}

///////////////////////////////////////////////////////////////////////////
//Singleton
gliPickBox g_PickBox;
gliPickBox & gliGetPickBox(){ return g_PickBox; }
