#include "RegionBoxModel.h"

#include "MPProblem/BoundingBox.h"

RegionBoxModel::RegionBoxModel() : RegionModel("Box Region") {}

shared_ptr<Boundary>
RegionBoxModel::GetBoundary() const {
  return shared_ptr<Boundary>(new BoundingBox());
}

//initialization of gl models
void
RegionBoxModel::BuildModels() {
}

//determing if _index is this GL model
void
RegionBoxModel::Select(GLuint* _index, vector<Model*>& _sel) {
  if(_index)
    _sel.push_back(this);
}

//draw is called for the scene.
void
RegionBoxModel::Draw() {
}

//DrawSelect is only called if item is selected
void
RegionBoxModel::DrawSelect() {
}

//output model info
void
RegionBoxModel::Print(ostream& _os) const {
}

bool
RegionBoxModel::MousePressed(QMouseEvent* _e) {
  return false;
}

bool
RegionBoxModel::MouseReleased(QMouseEvent* _e) {
  return false;
}

bool
RegionBoxModel::MouseMotion(QMouseEvent* _e) {
  return false;
}

bool
RegionBoxModel::PassiveMouseMotion(QMouseEvent* _e) {
  return false;
}
