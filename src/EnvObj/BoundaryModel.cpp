#include "BoundaryModel.h"

BoundaryModel::BoundaryModel() : m_displayID(-1), m_linesID(-1) {
}

void
BoundaryModel::Select(unsigned int* _index, vector<gliObj>& _sel){
  if(_index)
    _sel.push_back(this);
}

void
BoundaryModel::Draw(GLenum _mode){
  if(m_RenderMode == INVISIBLE_MODE) return;
  if(m_displayID != -1){
    glDisable(GL_LIGHTING);
    glCallList(m_displayID);
    glEnable(GL_LIGHTING);
  }
}

void
BoundaryModel::DrawSelect(){
  if(m_linesID != -1){
    glDisable(GL_LIGHTING);
    glLineWidth(2);
    glColor3d(1, 1, 0);
    glCallList(m_linesID);
    glEnable(GL_LIGHTING);
  }
}

