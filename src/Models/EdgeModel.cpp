#include "EdgeModel.h"

#include <limits.h>
#include <cstdlib>

#include "CfgModel.h"

double EdgeModel::m_edgeThickness = 1;


EdgeModel::
EdgeModel(string _lpLabel, double _weight,
    const vector<CfgModel>& _intermediates) : Model(""),
    DefaultWeight<CfgModel>(_lpLabel, _weight, _intermediates),
    m_id(-1), m_isValid(true) { }


EdgeModel::
EdgeModel(const DefaultWeight<CfgModel>& _e) : Model(""),
    DefaultWeight<CfgModel>(_e), m_id(-1), m_isValid(true) { }


void
EdgeModel::
SetName() {
  ostringstream temp;
  temp << "Edge " << m_id;
  m_name = temp.str();
}


void
EdgeModel::
Set(CfgModel* _c1, CfgModel* _c2) {
  m_startCfg = _c1;
  m_endCfg = _c2;
}


void
EdgeModel::
Set(size_t _id, CfgModel* _c1, CfgModel* _c2) {
  m_id = _id;
  Set(_c1, _c2);
  SetName();
}


void
EdgeModel::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  if(m_isValid)
    glColor4fv(m_color);
  else
    glColor4fv(Color4(1.0-m_color[0], 1.0-m_color[1], 1.0-m_color[2], 0.0));

  glBegin(GL_LINE_STRIP);
  glVertex3dv(m_startCfg->GetPoint());
  for(const auto& c : m_intermediates)
    glVertex3dv(c.GetPoint());
  glVertex3dv(m_endCfg->GetPoint());
  glEnd();

  //draw intermediate configurations
  if(CfgModel::GetShape() == CfgModel::Robot) {
    for(auto& c : m_intermediates) {
      c.SetRenderMode(WIRE_MODE);
      c.DrawRender();
    }
  }
}


void
EdgeModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  glBegin(GL_LINE_STRIP);
  glVertex3dv(m_startCfg->GetPoint());
  for(const auto& c : m_intermediates)
    glVertex3dv(c.GetPoint());
  glVertex3dv(m_endCfg->GetPoint());
  glEnd();

  //draw intermediate configurations
  if(CfgModel::GetShape() == CfgModel::Robot) {
    for(auto& c : m_intermediates) {
      c.SetRenderMode(WIRE_MODE);
      c.DrawSelect();
    }
  }
}


void
EdgeModel::
DrawSelected() {
  glLineWidth(m_edgeThickness + 4);

  glBegin(GL_LINE_STRIP);
  glVertex3dv(m_startCfg->GetPoint());
  for(const auto& c : m_intermediates)
    glVertex3dv(c.GetPoint());
  glVertex3dv(m_endCfg->GetPoint());
  glEnd();
}


void
EdgeModel::
Print(ostream& _os) const {
  _os << "Edge ID= " << m_id << endl
      << "Connects Nodes: " << m_startCfg->GetIndex() << " and "
      << m_endCfg->GetIndex() << endl << "Intermediates: ";

  for(const auto& c : m_intermediates)
    _os << c << " | ";

  _os << endl << "Weight: " << m_weight << endl;

  if(!m_isValid)
    _os << "**** IS IN COLLISION!! ****" << endl;
}


void
EdgeModel::
DrawRenderInCC() {
  glVertex3dv(m_startCfg->GetPoint());
  for(const auto& c : m_intermediates) {
    /// \todo Implementation appears to have a duplicated line - can anyone
    ///       confirm that this is/isn't needed?
    glVertex3dv(c.GetPoint());
    glVertex3dv(c.GetPoint());
  }
  glVertex3dv(m_endCfg->GetPoint());
}
