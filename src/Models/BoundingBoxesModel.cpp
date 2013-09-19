#include "BoundingBoxesModel.h"

#include <cstdlib>
#include <cmath>

#include "Models/BoundingBoxModel.h"

BoundingBoxesModel::BoundingBoxesModel() {}

BoundingBoxesModel::~BoundingBoxesModel() {
  typedef vector<BoundingBoxModel*>::iterator BIT;
  for(BIT bit = m_bbxModels.begin(); bit != m_bbxModels.end(); ++bit)
    delete *bit;
  for(BIT bit = m_bbxOverlaps.begin(); bit != m_bbxOverlaps.end(); ++bit)
    delete *bit;
  m_bbxModels.clear();
  m_bbxOverlaps.clear();
}

vector<string>
BoundingBoxesModel::GetInfo() const {
  vector<string> info;
  info.push_back("Bounding Box");
  info.push_back("");
  string name = "Num Bounding Boxes=";
  ostringstream temp;
  temp << m_bbxModels.size();
  info.push_back(name + temp.str());
  return info;
}

void
BoundingBoxesModel::GetChildren(list<GLModel*>& _models) {
  typedef vector<BoundingBoxModel*>::iterator BIT;
  for(BIT bit = m_bbxModels.begin(); bit != m_bbxModels.end(); ++bit)
    _models.push_back(*bit);
}

void
BoundingBoxesModel::BuildModels() {
  typedef vector<BoundingBoxModel*>::iterator BIT;
  for(BIT bit = m_bbxModels.begin(); bit != m_bbxModels.end(); ++bit)
    (*bit)->BuildModels();

  for(BIT bit1 = m_bbxModels.begin(); bit1 != m_bbxModels.end(); ++bit1)
    for(BIT bit2 = bit1+1; bit2 != m_bbxModels.end(); ++bit2)
      BuildOverlapModel(*bit1, *bit2);
}

void
BoundingBoxesModel::Draw(GLenum _mode) {
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  typedef vector<BoundingBoxModel*>::iterator BIT;
  for(BIT bit = m_bbxModels.begin(); bit != m_bbxModels.end(); ++bit)
    (*bit)->Draw(_mode);
  glDisable(GL_CULL_FACE);

  for(BIT bit = m_bbxOverlaps.begin(); bit != m_bbxOverlaps.end(); ++bit)
    (*bit)->DrawSelect();
}

void
BoundingBoxesModel::Select(unsigned int* _index, vector<GLModel*>& _sel) {
  if(!_index || *_index >= m_bbxModels.size())
    return;
  m_bbxModels[*_index]->Select(_index+1, _sel);
}

void
BoundingBoxesModel::BuildOverlapModel(BoundingBoxModel* _a, BoundingBoxModel* _b) {
  OverlapType x = ClassifyOverlap(_a->m_bbx[0].first, _a->m_bbx[0].second, _b->m_bbx[0].first, _b->m_bbx[0].second);
  OverlapType y = ClassifyOverlap(_a->m_bbx[1].first, _a->m_bbx[1].second, _b->m_bbx[1].first, _b->m_bbx[1].second);
  OverlapType z = ClassifyOverlap(_a->m_bbx[2].first, _a->m_bbx[2].second, _b->m_bbx[2].first, _b->m_bbx[2].second);

  if(!((x==D && y==D && z==D) || (x==EP && y==EP && z==EP) || (x==EN && y==EN && y==EN))){
    vector<double> ov = Overlap(_a, _b);
    BoundingBoxModel* cbbm = new BoundingBoxModel();
    cbbm->m_bbx[0].first = ov[0];
    cbbm->m_bbx[0].second = ov[1];
    cbbm->m_bbx[1].first = ov[2];
    cbbm->m_bbx[1].second = ov[3];
    cbbm->m_bbx[2].first = ov[4];
    cbbm->m_bbx[2].second = ov[5];
    cbbm->BuildModels();
    m_bbxOverlaps.push_back(cbbm);
  }
}

BoundingBoxesModel::OverlapType
BoundingBoxesModel::ClassifyOverlap(double _min1, double _max1, double _min2, double _max2){
  double a = _max1 - _min1;
  double b = _max2 - _min2;
  double apb = a + b;
  double amb = abs(a - b);
  double dist = abs((_max1 + _min1)/2 - (_max2 + _min2)/2);
  if(dist <= amb) {
    if(_min1 < _min2)
      return EP;
    else
      return EN;
  }
  else if(dist>=apb)
    return D;
  else
    return O;
}

vector<double>
BoundingBoxesModel::Overlap(BoundingBoxModel* _a, BoundingBoxModel* _b){
  vector<double> ov = vector<double>(6);
  ov[0] = max(_a->m_bbx[0].first, _b->m_bbx[0].first);
  ov[2] = max(_a->m_bbx[1].first, _b->m_bbx[1].first);
  ov[4] = max(_a->m_bbx[2].first, _b->m_bbx[2].first);
  ov[1] = min(_a->m_bbx[0].second, _b->m_bbx[0].second);
  ov[3] = min(_a->m_bbx[1].second, _b->m_bbx[1].second);
  ov[5] = min(_a->m_bbx[2].second, _b->m_bbx[2].second);
  return ov;
}

