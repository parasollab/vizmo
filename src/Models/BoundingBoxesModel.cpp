#include "BoundingBoxesModel.h"

#include <cstdlib>
#include <cmath>

#include "Models/BoundingBoxModel.h"

BoundingBoxesModel::BoundingBoxesModel() : Model("BoundingBoxes") {}

BoundingBoxesModel::~BoundingBoxesModel() {
  typedef vector<BoundingBoxModel*>::iterator BIT;
  for(BIT bit = m_bbxModels.begin(); bit != m_bbxModels.end(); ++bit)
    delete *bit;
  for(BIT bit = m_bbxOverlaps.begin(); bit != m_bbxOverlaps.end(); ++bit)
    delete *bit;
  m_bbxModels.clear();
  m_bbxOverlaps.clear();
}

void
BoundingBoxesModel::GetChildren(list<Model*>& _models) {
  typedef vector<BoundingBoxModel*>::iterator BIT;
  for(BIT bit = m_bbxModels.begin(); bit != m_bbxModels.end(); ++bit)
    _models.push_back(*bit);
}

void
BoundingBoxesModel::Build() {
  typedef vector<BoundingBoxModel*>::iterator BIT;
  for(BIT bit = m_bbxModels.begin(); bit != m_bbxModels.end(); ++bit)
    (*bit)->Build();

  for(BIT bit1 = m_bbxModels.begin(); bit1 != m_bbxModels.end(); ++bit1)
    for(BIT bit2 = bit1+1; bit2 != m_bbxModels.end(); ++bit2)
      BuildOverlapModel(*bit1, *bit2);
}

void
BoundingBoxesModel::Select(GLuint* _index, vector<Model*>& _sel) {
  if(!m_selectable || !_index || *_index >= m_bbxModels.size())
    return;
  m_bbxModels[*_index]->Select(_index+1, _sel);
}

void
BoundingBoxesModel::DrawRender() {
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  typedef vector<BoundingBoxModel*>::iterator BIT;
  for(BIT bit = m_bbxModels.begin(); bit != m_bbxModels.end(); ++bit)
    (*bit)->DrawRender();
  glDisable(GL_CULL_FACE);

  for(BIT bit = m_bbxOverlaps.begin(); bit != m_bbxOverlaps.end(); ++bit)
    (*bit)->DrawSelect();
}

void
BoundingBoxesModel::DrawSelect() {
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  typedef vector<BoundingBoxModel*>::iterator BIT;
  for(BIT bit = m_bbxModels.begin(); bit != m_bbxModels.end(); ++bit)
    (*bit)->DrawRender();
  glDisable(GL_CULL_FACE);

  for(BIT bit = m_bbxOverlaps.begin(); bit != m_bbxOverlaps.end(); ++bit)
    (*bit)->DrawSelected();
}

void
BoundingBoxesModel::Print(ostream& _os) const {
  _os << Name() << endl << endl
    << "Num Bounding Boxes: " << m_bbxModels.size() << endl;
}

void
BoundingBoxesModel::BuildOverlapModel(BoundingBoxModel* _a, BoundingBoxModel* _b) {
  OverlapType x = ClassifyOverlap(_a->m_bbx[0].first, _a->m_bbx[0].second, _b->m_bbx[0].first, _b->m_bbx[0].second);
  OverlapType y = ClassifyOverlap(_a->m_bbx[1].first, _a->m_bbx[1].second, _b->m_bbx[1].first, _b->m_bbx[1].second);
  OverlapType z = ClassifyOverlap(_a->m_bbx[2].first, _a->m_bbx[2].second, _b->m_bbx[2].first, _b->m_bbx[2].second);

  if(!(x==y && y==z && x!=O)){
    vector<double> ov = Overlap(_a, _b);
    BoundingBoxModel* cbbm = new BoundingBoxModel();
    for(int i=0; i<3; i++){
    cbbm->m_bbx[i].first = ov[2*i];
    cbbm->m_bbx[i].second = ov[2*i+1];
    }
    cbbm->Build();
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
  for(int i=0; i<3; i++ ){
  ov[2*i] = max(_a->m_bbx[i].first, _b->m_bbx[i].first);
  ov[2*i+1] = min(_a->m_bbx[i].second, _b->m_bbx[i].second);
  }
  return ov;
}

