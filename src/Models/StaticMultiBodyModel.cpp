#include "StaticMultiBodyModel.h"

#include "Environment/FixedBody.h"
#include "Environment/StaticMultiBody.h"

#include "BodyModel.h"

StaticMultiBodyModel::
StaticMultiBodyModel(shared_ptr<StaticMultiBody> _s) :
  MultiBodyModel("StaticMultiBody", _s), m_staticMultiBody(_s) {
  }

StaticMultiBodyModel::
StaticMultiBodyModel(string _name, shared_ptr<StaticMultiBody> _s) :
  MultiBodyModel(_name, _s), m_staticMultiBody(_s) {
  }

void
StaticMultiBodyModel::Build() {
  shared_ptr<FixedBody> body = m_staticMultiBody->GetFixedBody(0);
  m_bodies.emplace_back(new BodyModel(body));
  m_bodies.back()->Build();
  if(body->IsColorLoaded())
    SetColor(body->GetColor());
  if(m_staticMultiBody->IsInternal())
    SetRenderMode(INVISIBLE_MODE);
}

size_t
StaticMultiBodyModel::
GetNumVertices() const {
  return m_bodies[0]->GetPolyhedronModel()->GetNumVertices();
}

size_t
StaticMultiBodyModel::
GetNumFacets() const {
  return m_bodies[0]->GetPolyhedronModel()->GetNumFacets();
}

void
StaticMultiBodyModel::
AddToTetGen(tetgenio* _tetModel,
    size_t _pOff, size_t _fOff, size_t _hOff) const {
  cout << "Adding obstacle: " << _hOff << endl;
  cout << "pOff: " << _pOff << "\tfOff: " << _fOff << "\thOff: " << _hOff << endl;

  shared_ptr<FixedBody> body = m_staticMultiBody->GetFixedBody(0);
  GMSPolyhedron& poly = body->GetWorldPolyhedron();

  //Add vertices to tetModel
  size_t n = 0;
  for(auto& v : poly.m_vertexList) {
    //cout << "Adding vertex " << n << ": " << v << endl;
    _tetModel->pointlist[3*_pOff + 3*n + 0] = v[0];
    _tetModel->pointlist[3*_pOff + 3*n + 1] = v[1];
    _tetModel->pointlist[3*_pOff + 3*n + 2] = v[2];
    ++n;
  }

  //Add triagles as facets to tetModel
  size_t m = 0;
  for(auto& t : poly.m_polygonList) {
    tetgenio::facet* f = &_tetModel->facetlist[_fOff + m];
    _tetModel->facetmarkerlist[_fOff + m] = _hOff + 1;
    f->numberofpolygons = 1;
    f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
    f->numberofholes = 0;
    f->holelist = NULL;
    tetgenio::polygon* p = &f->polygonlist[0];
    p->numberofvertices = t.m_vertexList.size();
    p->vertexlist = new int[p->numberofvertices];
    size_t i = 0;
    for(auto& v : t.m_vertexList)
      p->vertexlist[i++] = _pOff + v;
    ++m;
  }

  //Add hole to tetModel
  Vector3d com = body->GetCenterOfMass();
  Vector3d hole = com;
  GMSPolyhedron& modpoly = body->GetPolyhedron();
  for(auto& v : modpoly.m_vertexList) {
    if(body->IsConvexHullVertex(v)) {
      if((v - com).norm() > (hole - com).norm())
        hole = v;
    }
  }
  body->ComputeCenterOfMass();
  hole = body->GetWorldTransformation() * hole;
  hole = hole + (com - hole).normalize() * 0.0001;
  _tetModel->holelist[3*_hOff + 0] = hole[0];
  _tetModel->holelist[3*_hOff + 1] = hole[1];
  _tetModel->holelist[3*_hOff + 2] = hole[2];
}
