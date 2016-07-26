#include "CfgModel.h"
#include "EdgeModel.h"
#include "EnvModel.h"
#include "TempObjsModel.h"
#include "Vizmo.h"

TempObjsModel::
TempObjsModel() : Model("TempObjs"), m_tempCfgs(), m_tempEdges() {
  //Add self to env's temp objs list for display purposes.
  GetVizmo().GetEnv()->AddTempObjs(this);
}


TempObjsModel::
~TempObjsModel() {
  // Remove self from env's temp obj list.
  GetVizmo().GetEnv()->RemoveTempObjs(this);

  // Delete temporary cfgs.
  for(auto cfg : m_tempCfgs)
    delete cfg;
  m_tempCfgs.clear();

  // Delete temporary edges.
  for(auto edge : m_tempEdges)
    delete edge;
  m_tempEdges.clear();

  // Delete temporary models.
  for(auto model : m_tempModels)
    delete model;
  m_tempModels.clear();
}


void
TempObjsModel::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  glLineWidth(2);
  for(auto cfg : m_tempCfgs) {
    cfg->SetColor(Color4(0., 1., 0., 1.));
    cfg->SetRenderMode(WIRE_MODE);
    cfg->DrawRender();
  }
  for(auto edge : m_tempEdges) {
    edge->SetColor(Color4(0., 1., 0., 1.));
    edge->SetRenderMode(WIRE_MODE);
    edge->DrawRender();
  }
  for(auto model : m_tempModels)
    model->DrawRender();
}


void
TempObjsModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  for(auto cfg : m_tempCfgs)
    cfg->DrawSelect();
  for(auto edge : m_tempEdges)
    edge->DrawSelect();
}


void
TempObjsModel::
Print(ostream& _os) const {
  _os << Name() << endl;
}
