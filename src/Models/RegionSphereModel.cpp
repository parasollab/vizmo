#include "RegionSphereModel.h"

#include "MPProblem/BoundingSphere.h"

RegionSphereModel::RegionSphereModel() : RegionModel("Sphere Region") {}

shared_ptr<Boundary>
RegionSphereModel::GetBoundary() const {
  return shared_ptr<Boundary>(new BoundingSphere());
}

//initialization of gl models
void
RegionSphereModel::BuildModels() {
}

//determing if _index is this GL model
void
RegionSphereModel::Select(GLuint* _index, vector<Model*>& _sel) {
  if(_index)
    _sel.push_back(this);
}

//draw is called for the scene.
void
RegionSphereModel::Draw() {
}

//DrawSelect is only called if item is selected
void
RegionSphereModel::DrawSelect() {
}

//output model info
void
RegionSphereModel::Print(ostream& _os) const {
}

bool
RegionSphereModel::MousePressed(QMouseEvent* _e) {
  return false;
}

bool
RegionSphereModel::MouseReleased(QMouseEvent* _e) {
  return false;
}

bool
RegionSphereModel::MouseMotion(QMouseEvent* _e) {
  return false;
}

bool
RegionSphereModel::PassiveMouseMotion(QMouseEvent* _e) {
  return false;
}
