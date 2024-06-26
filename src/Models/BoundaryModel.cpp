#include "BoundaryModel.h"

#include "Environment/Boundary.h"

BoundaryModel::
BoundaryModel(const string& _name, shared_ptr<Boundary> _b) :
  Model(_name), m_boundary(_b),
  m_displayID(-1), m_linesID(-1) {
  }


BoundaryModel::
~BoundaryModel() {
  glDeleteLists(m_displayID, 1);
  glDeleteLists(m_linesID, 1);
}


void
BoundaryModel::
Select(GLuint* _index, vector<Model*>& _sel) {
  if(m_selectable && _index)
    _sel.push_back(this);
}


void
BoundaryModel::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return;
  glDisable(GL_LIGHTING);
  glCallList(m_displayID);
  glEnable(GL_LIGHTING);
}


void
BoundaryModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;
  glCallList(m_displayID);
}


void
BoundaryModel::
DrawSelected() {
  glDisable(GL_LIGHTING);
  glLineWidth(2);
  glCallList(m_linesID);
  glEnable(GL_LIGHTING);
}
