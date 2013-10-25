#include "PickBox.h"

#include <cmath>

#include <GL/glut.h>

#include <QMouseEvent>

void
PickBox::Draw() {
  if(m_leftMouseButton) {
    //change to Ortho view
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, m_w, 0, m_h);

    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
    glLoadIdentity();

    //draw dotted square around pick box
    glDisable(GL_LIGHTING);
    glEnable(GL_LINE_STIPPLE);
    glLineWidth(1);
    glBegin(GL_LINE_LOOP);
    glColor3f(0,0,0);
    glVertex2f(m_pickBox.m_left, m_pickBox.m_bottom);
    glVertex2f(m_pickBox.m_right, m_pickBox.m_bottom);
    glVertex2f(m_pickBox.m_right, m_pickBox.m_top);
    glVertex2f(m_pickBox.m_left, m_pickBox.m_top);
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    glPopMatrix();

    //change back to pers view
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
  }
}

void
PickBox::MousePressed(QMouseEvent* _e) {
  if(_e->button() == Qt::LeftButton) {
    m_leftMouseButton=true;
    m_pickBox.m_right = m_pickBox.m_left = _e->pos().x();
    m_pickBox.m_bottom = m_pickBox.m_top = m_h - _e->pos().y();
  }
}

void
PickBox::MouseReleased(QMouseEvent* _e) {
  m_leftMouseButton=false;
}

void
PickBox::MouseMotion(QMouseEvent* _e) {
  if(m_leftMouseButton) {
    m_pickBox.m_right = _e->pos().x();
    m_pickBox.m_top = m_h - _e->pos().y();
  }
}

