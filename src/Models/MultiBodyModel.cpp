#include "MultiBodyModel.h"

#include "BodyModel.h"
#include "BoundingBoxModel.h"
#include "BoundaryModel.h"
#include "BoundingSphereModel.h"
#include "CfgModel.h"
#include "EnvModel.h"
#include "Vizmo.h"
#include "Utilities/IO.h"

#include <containers/sequential/graph/algorithms/connected_components.h>

//vector<MultiBodyModel::DOFInfo> MultiBodyModel::m_dofInfo = vector<MultiBodyModel::DOFInfo>();

MultiBodyModel::
MultiBodyModel(string _name, MultiBody* _m) :
  Model(_name), m_multiBody(_m) {
  }

MultiBodyModel::
~MultiBodyModel() {
  for(auto& body : *this)
    delete body;
}

void
MultiBodyModel::
SetRenderMode(RenderMode _mode){
  Model::SetRenderMode(_mode);
  for(auto& body : *this)
    body->SetRenderMode(_mode);
}

void
MultiBodyModel::
SetSelectable(bool _s){
  m_selectable = _s;
  for(auto& body : *this)
    body->SetSelectable(_s);
}

void
MultiBodyModel::
SetColor(const Color4& _c) {
  Model::SetColor(_c);
  for(auto& body : *this)
    body->SetColor(_c);
}

void
MultiBodyModel::
ToggleNormals() {
  Model::ToggleNormals();
  for(auto& body : *this)
    body->ToggleNormals();
}

void
MultiBodyModel::
Build() {
}

void
MultiBodyModel::
Select(unsigned int* _index, vector<Model*>& sel){
  if(m_selectable && _index)
    sel.push_back(this);
}

void
MultiBodyModel::
DrawRender() {
  glColor4fv(GetColor());
  for(auto& body : *this)
    body->DrawRender();
}

void
MultiBodyModel::
DrawSelect() {
  for(auto& body : *this)
    body->DrawSelect();
}

void
MultiBodyModel::
DrawSelected(){
  for(auto& body : *this)
    body->DrawSelected();
}

void
MultiBodyModel::
DrawHaptics() {
  for(auto& body : *this)
    body->DrawHaptics();
}

void
MultiBodyModel::
Print(ostream& _os) const {
  _os << Name() << endl;
  //if(m_active) {
    _os << "Active" << endl;
    _os << m_bodies.size() << endl;
  //}
  //else {
    _os << "Passive" << endl;
    m_bodies[0]->Print(_os);
  //}
}

/*void
MultiBodyModel::AddJoint(ConnectionModel* _c, int _indexBase, int _indexJoint, int _bodyNum){
  m_robots.at(_indexBase).second.insert(m_robots.at(_indexBase).second.begin()+_indexJoint, _c);
  m_joints.insert(m_joints.begin()+(_bodyNum-(_indexBase+1)), _c);
}

void
MultiBodyModel::AddBody(BodyModel* _b, int _index){
  if(_index <= (int)m_bodies.size())
    m_bodies.insert(m_bodies.begin()+_index, _b);
  else
    m_bodies.push_back(_b);
}

void
MultiBodyModel::DeleteBody(int _index){
  m_bodies.erase(m_bodies.begin()+_index);
}

void
MultiBodyModel::DeleteJoint(int _indexBase, int _indexJoint, int _bodyNumber){
  m_joints.erase(m_joints.begin()+(_bodyNumber-(_indexBase+1)));
  m_robots.at(_indexBase).second.erase(m_robots.at(_indexBase).second.begin()+_indexJoint);
}

void
MultiBodyModel::DeleteJoints(){
  while(!m_joints.empty())
    m_joints.pop_back();
}

void
MultiBodyModel::DeleteRobot(int _index){
  m_robots.erase(m_robots.begin()+_index);
}

void
MultiBodyModel::AddBase(BodyModel* _newBase){
  m_robots.push_back(make_pair(_newBase, vector<ConnectionModel*>()));
}*/

/*void
MultiBodyModel::ChangeDOF(int _dof){
  m_dof = _dof;
  CfgModel().SetDOF(m_dof);
}*/
