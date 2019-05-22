#include "CfgModel.h"
#include "EdgeModel.h"
#include "EnvModel.h"
#include "TempObjsModel.h"
#include "Vizmo.h"

/*---------------------------- Construction ----------------------------------*/

TempObjsModel::
TempObjsModel() : Model("TempObjs") {
  //Add self to env's temp objs list for display purposes.
  GetVizmo().GetEnv()->AddTempObjs(this);
}


TempObjsModel::
~TempObjsModel() {
  Clear();

  // Grab lock for remainder of object life.
  m_lock.lock();

  // Remove self from env's temp obj list.
  GetVizmo().GetEnv()->RemoveTempObjs(this);
}

/*------------------------------ Interface -----------------------------------*/

void
TempObjsModel::
AddModel(Model* _m, const Color4& _col) {
  m_lock.lock();
  _m->SetColor(_col);
  _m->SetRenderMode(WIRE_MODE);
  m_models.push_back(_m);
  m_lock.unlock();
}


void
TempObjsModel::
RemoveModel(Model* _m) {
  m_lock.lock();
  auto m = find(m_models.begin(), m_models.end(), _m);
  if(m == m_models.end())
    throw RunTimeException(WHERE, "Removal of temporary model with name '" +
        _m->Name() + "requested, but the model wasn't found.");
  m_models.erase(m);
  delete _m;
  m_lock.unlock();
}


void
TempObjsModel::
Clear() {
  m_lock.lock();

  // Delete temporary models.
  for(auto model : m_models)
    delete model;
  m_models.clear();

  m_lock.unlock();
}

/*---------------------------- Model Functions -------------------------------*/

void
TempObjsModel::
DrawRender() {
  // Don't draw if mutex is already locked or render mode is invisible.
  if(!m_lock.try_lock() || m_renderMode == INVISIBLE_MODE)
    return;

  glLineWidth(2);
  for(auto model : m_models)
    model->DrawRender();

  m_lock.unlock();
}

/*----------------------------------------------------------------------------*/
