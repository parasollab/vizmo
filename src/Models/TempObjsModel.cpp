#include "CfgModel.h"
#include "EdgeModel.h"
#include "EnvModel.h"
#include "TempObjsModel.h"
#include "Vizmo.h"

TempObjsModel::
TempObjsModel() : Model("TempObjs"), m_tempCfgs(), m_tempEdges() {
  //Add self to env's temp objs list for display purposes
  GetVizmo().GetEnv()->AddTempObjs(this);
}

TempObjsModel::
~TempObjsModel() {
  //Remove self from env's temp obj list
  GetVizmo().GetEnv()->RemoveTempObjs(this);

  //Delete temporary cfgs
  for(vector<CfgModel*>::iterator cit = m_tempCfgs.begin();
      cit != m_tempCfgs.end(); ++cit)
    delete *cit;
  m_tempCfgs.clear();

  //Delete temporary edges
  for(vector<EdgeModel*>::iterator eit = m_tempEdges.begin();
      eit != m_tempEdges.end(); ++eit)
    delete *eit;
  m_tempEdges.clear();
}

void
TempObjsModel::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  glLineWidth(2);
  for(vector<CfgModel*>::iterator cit = m_tempCfgs.begin();
      cit != m_tempCfgs.end(); ++cit) {
    (*cit)->SetColor(Color4(0., 1., 0., 1.));
    (*cit)->SetRenderMode(WIRE_MODE);
    (*cit)->DrawRender();
  }
  for(vector<EdgeModel*>::iterator eit = m_tempEdges.begin();
      eit != m_tempEdges.end(); ++eit) {
    (*eit)->SetColor(Color4(0., 1., 0., 1.));
    (*eit)->SetRenderMode(WIRE_MODE);
    (*eit)->DrawRender();
  }
}

void
TempObjsModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  for(vector<CfgModel*>::iterator cit = m_tempCfgs.begin();
      cit != m_tempCfgs.end(); ++cit)
    (*cit)->DrawSelect();
  for(vector<EdgeModel*>::iterator eit = m_tempEdges.begin();
      eit != m_tempEdges.end(); ++eit)
    (*eit)->DrawSelect();
}

void
TempObjsModel::
Print(ostream& _os) const {
  _os << Name() << endl;
}
