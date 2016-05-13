#include "EdgeModel.h"

#include <limits.h>
#include <cstdlib>

#include "CfgModel.h"

double EdgeModel::m_edgeThickness = 1;
double EdgeModel::m_numIntermediates = .3;

EdgeModel::
EdgeModel(string _lpLabel, double _weight,
    const vector<CfgModel>& _intermediates) : Model(""),
    EdgeType(_lpLabel, _weight, _intermediates),
    m_id(-1), m_isValid(true) { }

EdgeModel::
EdgeModel(const DefaultWeight<CfgModel>& _e) : Model(""),
    EdgeType(_e), m_id(-1), m_isValid(true) { }

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

  RecalculateEdges(*_c1);
}

void
EdgeModel::
RecalculateEdges(CfgModel _c) {
  double timeRes = CfgModel::GetTimeRes();
  CfgModel tick = _c;

  m_intermediates.clear();
  for(int i = 0; i < m_timeStep; ++i) {
    tick = tick.Apply(m_control, timeRes);
    m_intermediates.push_back(tick);
  }
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
  if(m_id != (size_t)-1)
    _os << "EID: " << m_id << endl;

  if(m_startCfg->GetIndex() != (size_t)-1) {
    _os << "Edge Connects: " << m_startCfg->GetIndex() << " to "
      << m_endCfg->GetIndex() << endl;
  }
  else {
    _os << "Edge Connects: " << endl;
    m_startCfg->Print(_os);
    _os << " to " << endl;
    m_endCfg->Print(_os);
  }

  _os << "Weight: " << m_weight << endl;

#ifdef PMPCfg
  _os << "Intermediates: ";
  for(const auto& c : m_intermediates)
    _os << "\t" << c << endl;
#elif defined(PMPState)
  _os << "Control: ";
  for(const auto& c : m_control)
    _os << c << " ";
  _os << endl;

  _os << "Time step: " << m_timeStep;
#endif

  if(!m_isValid)
    _os << "**** Invalid! ****" << endl;
}

void
EdgeModel::
DrawRenderInCC() {
  glVertex3dv(m_startCfg->GetPoint());

  //Number of intermediates to keep
  float k = m_numIntermediates == 0 ? 0 : 1 / m_numIntermediates;

  for(float i = 0; i < m_intermediates.size() - k; i += k) {
    int index = round(i);
    glVertex3dv(m_intermediates[index].GetPoint());
    glVertex3dv(m_intermediates[index].GetPoint());
  }

  glVertex3dv(m_endCfg->GetPoint());
}
