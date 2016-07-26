#include "PickBox.h"

#include <cmath>
#include <iostream>

#include "GLUtils.h"

void
PickBox::
Draw() {
  /// Rendering is done in a 2D ortho view that is defined in window
  /// coordinates, where X ranges from [0, window width] and Y ranges from
  /// [0, window height]. Also changes the cursor to reflect the current
  /// highlighted part for resize/move operations.
  if(m_picking) {
    //change to Ortho view
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, GLUtils::windowWidth, 0, GLUtils::windowHeight);

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
    if(m_highlightedPart == All)
      QApplication::setOverrideCursor(Qt::SizeAllCursor);
    else if(m_highlightedPart == (Left | Top) ||
        m_highlightedPart == (Right | Bottom))
      QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
    else if(m_highlightedPart == (Left | Bottom) ||
        m_highlightedPart == (Right | Top))
      QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
    else if(m_highlightedPart & (Left | Right))
      QApplication::setOverrideCursor(Qt::SizeHorCursor);
    else if(m_highlightedPart & (Top | Bottom))
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
PickBox::
MousePressed(QMouseEvent* _e) {
  if(_e->button() == Qt::LeftButton) {
    if(m_picking) {
      //resizing
      if(m_highlightedPart != All && m_highlightedPart != None) {
        m_resizing = true;
        m_translating = false;
      }
      //translating
      else {
        m_origBox = m_pickBox;
        m_clicked = _e->pos();
        m_translating = true;
        m_resizing = false;
      }
    }
    //initial drawing
    else {
      m_picking = true;
      m_resizing = true;
      m_highlightedPart = Top | Right;
      m_pickBox.m_right = m_pickBox.m_left = _e->pos().x();
      m_pickBox.m_bottom = m_pickBox.m_top = GLUtils::windowHeight - _e->pos().y();
    }
  }
}


void
PickBox::
MouseReleased(QMouseEvent* _e) {
  m_picking = false;
  m_resizing = false;
  m_translating = false;
  QApplication::setOverrideCursor(Qt::ArrowCursor);
}


void
PickBox::
MouseMotion(QMouseEvent* _e) {
  if(m_picking) {
    if(m_translating) {
      QPoint diff(_e->pos().x() - m_clicked.x(), m_clicked.y() - _e->pos().y());
      m_pickBox.m_left = max(1., m_origBox.m_left + diff.x());
      m_pickBox.m_right = min(double(GLUtils::windowWidth),
          m_origBox.m_right + diff.x());
      m_pickBox.m_bottom = max(1., m_origBox.m_bottom + diff.y());
      m_pickBox.m_top = min(double(GLUtils::windowHeight),
          m_origBox.m_top + diff.y());
    }
    else if(m_resizing) {
      if(m_highlightedPart & Left)
        m_pickBox.m_left = max(1, _e->pos().x());
      if(m_highlightedPart & Right)
        m_pickBox.m_right = min(GLUtils::windowWidth, _e->pos().x());
      if(m_highlightedPart & Bottom)
        m_pickBox.m_bottom = max(1, GLUtils::windowHeight - _e->pos().y());
      if(m_highlightedPart & Top)
        m_pickBox.m_top = min(GLUtils::windowHeight,
            GLUtils::windowHeight - _e->pos().y());
    }

    //ensure the top and left are the true top/left
    //if not swap the values and fix m_highlightedPart
    if(m_pickBox.m_bottom > m_pickBox.m_top) {
      swap(m_pickBox.m_bottom, m_pickBox.m_top);
      if(m_resizing)
        m_highlightedPart = m_highlightedPart ^ Top ^ Bottom;
    }
    if(m_pickBox.m_left > m_pickBox.m_right) {
      swap(m_pickBox.m_left, m_pickBox.m_right);
      if(m_resizing)
        m_highlightedPart = m_highlightedPart ^ Left ^ Right;
    }
  }
}

bool
PickBox::
PassiveMouseMotion(QMouseEvent* _e) {
  if(m_picking) {
    //clear highlighted part and get mouse position.
    m_highlightedPart = None;
    int x = _e->pos().x(), y = GLUtils::windowHeight - _e->pos().y();

    //define a pixle threshold. the cursor is considered to be touching a box
    //border if it is within pT of the border.
    int pT = 4;

    //inflate the borders by the pixle threshold. this ensures that corners can
    //be selected easily.
    int bottom = m_pickBox.m_bottom - pT;
    int top = m_pickBox.m_top + pT;
    int left = m_pickBox.m_left - pT;
    int right = m_pickBox.m_right + pT;

    //check for cursor in the center
    if(x > left + 2*pT && x < right - 2*pT && y > bottom + 2*pT && y < top - 2*pT)
      m_highlightedPart = All;
    else {
      if(y >= bottom && y <= top) {
        //if cursor is within y bounds, check for left/right border selection
        if(abs(x - m_pickBox.m_left) < pT)
          m_highlightedPart |= Left;
        else if(abs(x - m_pickBox.m_right) < pT)
          m_highlightedPart |= Right;
      }
      if(x >= left && x <= right) {
        //if cursor is within x bounds, check for top/bottom border selection
        if(abs(y - m_pickBox.m_top) < pT)
          m_highlightedPart |= Top;
        else if(abs(y - m_pickBox.m_bottom) < pT)
          m_highlightedPart |= Bottom;
      }
    }

    return true;
  }
  else
    return false;
}
