#include "PickBox.h"

#include <cmath>
#include <iostream>

#include <GL/glut.h>

#include <QtGui>

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
    glColor3f(0, 0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(m_pickBox.m_left, m_pickBox.m_top);
    glVertex2f(m_pickBox.m_left, m_pickBox.m_bottom);
    glVertex2f(m_pickBox.m_right, m_pickBox.m_bottom);
    glVertex2f(m_pickBox.m_right, m_pickBox.m_top);
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    glPopMatrix();

    //change mouse cursor if highlighted
    if(m_highlightedPart == (LEFT | TOP) ||
        m_highlightedPart == (RIGHT | BOTTOM))
      QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
    else if(m_highlightedPart == (LEFT | BOTTOM) ||
        m_highlightedPart == (RIGHT | TOP))
      QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
    else if(m_highlightedPart & (LEFT | RIGHT))
      QApplication::setOverrideCursor(Qt::SizeHorCursor);
    else if(m_highlightedPart & (TOP | BOTTOM))
      QApplication::setOverrideCursor(Qt::SizeVerCursor);
    else
      QApplication::setOverrideCursor(Qt::ArrowCursor);

    //change back to pers view
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
  }
}

void
PickBox::MousePressed(QMouseEvent* _e) {
  if(_e->button() == Qt::LeftButton) {
    if(m_leftMouseButton) {
      m_resizing = true;
    }
    else {
      m_leftMouseButton = true;
      m_pickBox.m_right = m_pickBox.m_left = _e->pos().x();
      m_pickBox.m_bottom = m_pickBox.m_top = m_h - _e->pos().y();
    }
  }
}

void
PickBox::MouseReleased(QMouseEvent* _e) {
  m_leftMouseButton = false;
  m_resizing = false;
}

void
PickBox::MouseMotion(QMouseEvent* _e) {
  if(m_leftMouseButton) {
    if(m_resizing) {
      if(m_highlightedPart & LEFT)
        m_pickBox.m_left = _e->pos().x();
      if(m_highlightedPart & RIGHT)
        m_pickBox.m_right = _e->pos().x();
      if(m_highlightedPart & TOP)
        m_pickBox.m_top = m_h - _e->pos().y();
      if(m_highlightedPart & BOTTOM)
        m_pickBox.m_bottom = m_h - _e->pos().y();
    }
    else{
      m_pickBox.m_right = _e->pos().x();
      m_pickBox.m_top = m_h - _e->pos().y();
    }

    //ensure the top and left are the true top/left
    //if not swap the values and fix m_highlightedPart
    if(m_pickBox.m_bottom < m_pickBox.m_top) {
      swap(m_pickBox.m_bottom, m_pickBox.m_top);
      if(m_resizing)
        m_highlightedPart = m_highlightedPart ^ TOP ^ BOTTOM;
    }
    if(m_pickBox.m_left > m_pickBox.m_right) {
      swap(m_pickBox.m_left, m_pickBox.m_right);
      if(m_resizing)
        m_highlightedPart = m_highlightedPart ^ LEFT ^ RIGHT;
    }
  }
}

bool
PickBox::PassiveMouseMotion(QMouseEvent* _e) {
  if(m_leftMouseButton) {
    m_highlightedPart = 0;
    int x = _e->pos().x(), y = m_h - _e->pos().y();
    int bottom = min(m_pickBox.m_bottom, m_pickBox.m_top) - 5;
    int top = max(m_pickBox.m_bottom, m_pickBox.m_top) + 5;
    int left = min(m_pickBox.m_left, m_pickBox.m_right) - 5;
    int right = max(m_pickBox.m_left, m_pickBox.m_right) + 5;

    if(abs(x - m_pickBox.m_left) < 5 && y >= bottom && y <= top)
      m_highlightedPart |= LEFT;
    else if(abs(x - m_pickBox.m_right) < 5 && y >= bottom && y <= top)
      m_highlightedPart |= RIGHT;
    if(abs(y - m_pickBox.m_top) < 5 && x >= left && x <= right)
      m_highlightedPart |= TOP;
    else if(abs(y - m_pickBox.m_bottom) < 5 && x >= left && x <= right)
      m_highlightedPart |= BOTTOM;

    return true;
  }
  else
    return false;
}
