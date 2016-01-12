#include "BoundingBoxModel.h"

#include <limits>
#include <sstream>

#include <glut.h>

#include "Environment/BoundingBox.h"

BoundingBoxModel::
BoundingBoxModel(shared_ptr<BoundingBox> _b) :
  BoundaryModel("Bounding Box", _b),
  m_boundingBox(_b) {
    Build();
  }

BoundingBoxModel::
BoundingBoxModel(const pair<double, double>& _x, const pair<double, double>& _y,
    const pair<double, double>& _z) :
  BoundaryModel("Bounding Box", NULL) {
    m_boundingBox = shared_ptr<BoundingBox>(new BoundingBox(_x, _y, _z));
    m_boundary = m_boundingBox;
    Build();
  }

void
BoundingBoxModel::
Build() {
  const pair<double, double>* const bbx = m_boundingBox->GetBox();
  double zmin = bbx[2].second == numeric_limits<double>::max() ?
    -1 : bbx[2].first;
  double zmax = bbx[2].second == numeric_limits<double>::max() ?
    1 : bbx[2].second;

  // Compute center
  m_center[0] = (bbx[0].first + bbx[0].second) / 2.;
  m_center[1] = (bbx[1].first + bbx[1].second) / 2.;
  m_center[2] = (zmin + zmax) / 2.;

  GLdouble vertices[] = {
    bbx[0].first,  bbx[1].first,  zmin,
    bbx[0].second, bbx[1].first,  zmin,
    bbx[0].second, bbx[1].first,  zmax,
    bbx[0].first,  bbx[1].first,  zmax,
    bbx[0].first,  bbx[1].second, zmin,
    bbx[0].second, bbx[1].second, zmin,
    bbx[0].second, bbx[1].second, zmax,
    bbx[0].first,  bbx[1].second, zmax
  };

  //Face index
  GLubyte id1[] = { 3, 2, 1, 0 }; //buttom
  GLubyte id2[] = { 4, 5, 6, 7 }; //top
  GLubyte id3[] = { 2, 6, 5, 1 }; //left
  GLubyte id4[] = { 0, 4, 7, 3 }; //right
  GLubyte id5[] = { 1, 5, 4, 0 }; //back
  GLubyte id6[] = { 7, 6, 2, 3 }; //front

  //line index
  GLubyte lineid[] = {
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
  };

  //set properties for this box
  m_displayID = glGenLists(1);
  glNewList(m_displayID, GL_COMPILE);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glPolygonMode(GL_FRONT, GL_FILL);

  //setup points
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_DOUBLE, 0, vertices);

  glColor3d(0.95, 0.95, 0.95);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id1);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id2);

  glColor3d(0.85, 0.85, 0.85);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id3);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id4);

  glColor3d(0.75, 0.75, 0.75);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id5);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id6);
  glDisable(GL_POLYGON_OFFSET_FILL);

  //Draw lines
  glLineWidth(1);
  glColor3d(0.5, 0.5, 0.5);
  glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, lineid);

  glDisable(GL_CULL_FACE);
  glEndList();

  m_linesID = glGenLists(1);
  glNewList(m_linesID, GL_COMPILE);
  glVertexPointer(3, GL_DOUBLE, 0, vertices);
  //Draw lines
  glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, lineid);
  glEndList();

  glDisableClientState(GL_VERTEX_ARRAY);
}


void
BoundingBoxModel::
DrawHaptics() {
  const pair<double, double>* const bbx = m_boundingBox->GetBox();
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  glScalef( bbx[0].second - bbx[0].first,
      bbx[1].second - bbx[1].first,
      bbx[2].second - bbx[2].first);
  glutSolidCube(1);
  glPopMatrix();
}


void
BoundingBoxModel::
Print(ostream& _os) const {
  _os << Name() << endl
    << *m_boundingBox << endl;
}

vector<pair<double, double> >
BoundingBoxModel::
GetRanges() const {
  const pair<double, double>* const bbx = m_boundingBox->GetBox();
  return vector<pair<double, double>>(bbx, bbx+3);
}

double
BoundingBoxModel::
GetMaxDist() {
  return m_boundingBox->GetMaxDist();
}

size_t
BoundingBoxModel::
GetNumVertices() const {
  return 8;
}

size_t
BoundingBoxModel::
GetNumFacets() const {
  return 6;
}

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_set_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/centroid.h>
#include <../src/CGAL/io.cpp>
#include <../src/CGAL/kernel.cpp>
typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef Kernel::Point_2                                   Point2;
typedef CGAL::Polygon_2<Kernel>                           Polygon2;
typedef CGAL::Polygon_with_holes_2<Kernel>                PolygonWithHoles2;
typedef CGAL::Polygon_set_2<Kernel>                       PolygonSet2;
typedef std::vector<PolygonWithHoles2>                    PWH2Set;
//#include "print_utils.h"

vector<pair<vector<size_t>, vector<pair<vector<size_t>, Vector3d>>>>
Merge(tetgenio* _tetModel, vector<size_t>& verts,
    vector<pair<vector<size_t>, Vector3d>> _holes) {

  cout << "Merging Facet: ";
  for(auto& x : verts)
    cout << x << " ";
  cout << endl;

  Vector3d v0(&_tetModel->pointlist[3*verts[0]]);
  Vector3d v1(&_tetModel->pointlist[3*verts[1]]);
  Vector3d v2(&_tetModel->pointlist[3*verts[2]]);
  Vector3d vx = (v0 - v1).normalize();
  Vector3d vy = (v2 - v1).normalize();

  auto planeProj = [&](Vector3d& _p) {
    double x = (_p - v1).proj(vx).norm();
    double y = (_p - v1).proj(vy).norm();
    //cout << "Projecting: " << _p << " to (" << x << ", " << y << ")" << endl;
    return Point2(x, y);
  };

  auto makePolygon2 = [&](vector<size_t>& vs) {
    Polygon2 p;
    for(auto& x : vs) {
      Vector3d v(&_tetModel->pointlist[3*x]);
      //cout << "Vertex " << x << " at " << v << endl;
      p.push_back(planeProj(v));
    }
    return p;
  };

  auto planeUnproj = [&](const Point2& p) {
    //cout << "Unprojecting: " << p << " to " << v1 + vx*to_double(p[0]) + vy*to_double(p[1]) << endl;
    return v1 + vx*to_double(p[0]) + vy*to_double(p[1]);
  };

  auto getVert = [&](const Vector3d& _p1) {
    size_t closest = -1;
    double dist = 1e6;
    for(size_t i = 0; i < (size_t)_tetModel->numberofpoints; ++i) {
      Vector3d p2(&_tetModel->pointlist[3*i]);
      double d = (_p1-p2).norm();
      if(d < dist) {
        closest = i;
        dist = d;
      }
    }
    return closest;
  };

  Polygon2 bounds = makePolygon2(verts);

  PWH2Set diff;
  diff.push_back(PolygonWithHoles2(bounds));

  PWH2Set diff2;
  for(size_t i = 0; i < _holes.size(); ++i) {
    cout << "i: " << diff.size() << " " << diff[0].number_of_holes() << endl;
    Polygon2 hole = makePolygon2(_holes[i].first);
    cout << "With Hole: ";
    for(auto& x : _holes[i].first)
      cout << x << " ";
    cout << endl;
    for(auto& p : diff)
      CGAL::difference(p, hole, back_inserter(diff2));
    diff.swap(diff2);
    diff2.clear();
  }

  /*PolygonSet2 s;
  s.insert(bounds);
  for(auto& h : _holes) {
    Polygon2 hole = makePolygon2(h.first);
    s.difference(hole);
  }
  PWH2Set diff;
  s.polygons_with_holes(back_inserter(diff));*/

  //int i = 0;
  /*for(auto& p : diff) {
    cout << p << endl;
  }
  cin.ignore();
  */

  cout << "Final return: " << diff.size() << endl;
  vector<pair<vector<size_t>, vector<pair<vector<size_t>, Vector3d>>>> polys;
  for(auto& p : diff) {
    vector<size_t> ext;
    vector<pair<vector<size_t>, Vector3d>> holes;
    Polygon2 outer = p.outer_boundary();
    for(auto pit = outer.vertices_begin(); pit != outer.vertices_end(); ++pit) {
      ext.push_back(getVert(planeUnproj(*pit)));
    }
    cout << "Outer: ";
    for(auto& x : ext)
      cout << x << " ";
    cout << endl;
    for(auto hit = p.holes_begin(); hit != p.holes_end(); ++hit) {
      vector<size_t> hole;
      for(auto pit = hit->vertices_begin(); pit != hit->vertices_end(); ++pit) {
        hole.push_back(getVert(planeUnproj(*pit)));
      }
      Point2 cent = CGAL::centroid(hit->vertices_begin(), hit->vertices_end());
      Vector3d c = planeUnproj(cent);
      holes.push_back(make_pair(hole, c));
      cout << "Hole: ";
      for(auto& x : hole)
        cout << x << " ";
      cout << ": " << c << endl;
    }
    polys.push_back(make_pair(ext, holes));
  }

  return polys;
}

void
BoundingBoxModel::
AddToTetGen(tetgenio* _tetModel, size_t _pOff, size_t _fOff) const {

  cout << "\nAdding boundary" << endl;

  const pair<double, double>* const ranges = m_boundingBox->GetBox();
  _tetModel->pointlist[3*_pOff +  0] = ranges[0].first;
  _tetModel->pointlist[3*_pOff +  1] = ranges[1].first;
  _tetModel->pointlist[3*_pOff +  2] = ranges[2].first;
  _tetModel->pointlist[3*_pOff +  3] = ranges[0].first;
  _tetModel->pointlist[3*_pOff +  4] = ranges[1].first;
  _tetModel->pointlist[3*_pOff +  5] = ranges[2].second;
  _tetModel->pointlist[3*_pOff +  6] = ranges[0].first;
  _tetModel->pointlist[3*_pOff +  7] = ranges[1].second;
  _tetModel->pointlist[3*_pOff +  8] = ranges[2].first;
  _tetModel->pointlist[3*_pOff +  9] = ranges[0].first;
  _tetModel->pointlist[3*_pOff + 10] = ranges[1].second;
  _tetModel->pointlist[3*_pOff + 11] = ranges[2].second;
  _tetModel->pointlist[3*_pOff + 12] = ranges[0].second;
  _tetModel->pointlist[3*_pOff + 13] = ranges[1].first;
  _tetModel->pointlist[3*_pOff + 14] = ranges[2].first;
  _tetModel->pointlist[3*_pOff + 15] = ranges[0].second;
  _tetModel->pointlist[3*_pOff + 16] = ranges[1].first;
  _tetModel->pointlist[3*_pOff + 17] = ranges[2].second;
  _tetModel->pointlist[3*_pOff + 18] = ranges[0].second;
  _tetModel->pointlist[3*_pOff + 19] = ranges[1].second;
  _tetModel->pointlist[3*_pOff + 20] = ranges[2].first;
  _tetModel->pointlist[3*_pOff + 21] = ranges[0].second;
  _tetModel->pointlist[3*_pOff + 22] = ranges[1].second;
  _tetModel->pointlist[3*_pOff + 23] = ranges[2].second;

  auto addFacet = [&](size_t _fIndx, size_t _v0, size_t _v1, size_t _v2, size_t _v3) {
    tetgenio::facet* f;
    tetgenio::polygon* p;
    f = &_tetModel->facetlist[_fIndx];
    _tetModel->facetmarkerlist[_fIndx] = 0;
    //for each facet...determine if lies on this facet of boundary
    vector<pair<vector<size_t>, Vector3d>> holes;
    vector<size_t> verts = {_pOff + _v0, _pOff + _v1, _pOff + _v2, _pOff + _v3};
    for(size_t i = 0; i < _fOff; ++i) {
      vector<size_t> tri(3);
      tri[0] = _tetModel->facetlist[i].polygonlist[0].vertexlist[0];
      tri[1] = _tetModel->facetlist[i].polygonlist[0].vertexlist[1];
      tri[2] = _tetModel->facetlist[i].polygonlist[0].vertexlist[2];
      Vector3d x0(&_tetModel->pointlist[3*tri[0]]);
      Vector3d x1(&_tetModel->pointlist[3*tri[1]]);
      Vector3d x2(&_tetModel->pointlist[3*tri[2]]);
      Vector3d x3(&_tetModel->pointlist[3*verts[0]]);
      Vector3d x4(&_tetModel->pointlist[3*verts[1]]);
      Vector3d x5(&_tetModel->pointlist[3*verts[2]]);
      Vector3d x6(&_tetModel->pointlist[3*verts[3]]);
      /*cout << "x0: " << x0 << endl;
      cout << "x1: " << x1 << endl;
      cout << "x2: " << x2 << endl;
      cout << "x3: " << x3 << endl;
      cout << "Co: " << fabs((x2-x0)*((x1-x0)%(x3-x2))) << endl;*/
      if(fabs((x2-x0)*((x1-x0)%(x3-x2))) < 0.00000001 &&
          fabs((x2-x0)*((x1-x0)%(x4-x2))) < 0.00000001 &&
          fabs((x2-x0)*((x1-x0)%(x5-x2))) < 0.00000001 &&
          fabs((x2-x0)*((x1-x0)%(x6-x2))) < 0.00000001) {
        holes.push_back(make_pair(tri, (x0 + x1 + x2)/3));
      }
    }
    cout << "\nFacet " << _fIndx << endl;
    auto polys = Merge(_tetModel, verts, holes);
    size_t numHoles = 0;
    for(auto& poly : polys) {
      numHoles += poly.second.size();
    }
    size_t numPoly = polys.size() + numHoles;

    f->numberofpolygons = numPoly;
    f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
    f->numberofholes = numHoles;
    f->holelist = new REAL[3*f->numberofholes];

    size_t polyIndx = 0;
    size_t holeIndx = 0;
    for(auto& poly : polys) {
      p = &f->polygonlist[polyIndx++];
      p->numberofvertices = poly.first.size();
      p->vertexlist = new int[p->numberofvertices];
      for(size_t i = 0; i < poly.first.size(); ++i)
        p->vertexlist[i] = poly.first[i];

      //add each self-intersect as hole
      //cout << "Num holes in facet " << _fIndx << ": " << poly.second.size() << endl;
      for(auto& h : poly.second) {
        p = &f->polygonlist[polyIndx++];
        p->numberofvertices = h.first.size();
        p->vertexlist = new int[p->numberofvertices];
        for(size_t i = 0; i < h.first.size(); ++i)
          p->vertexlist[i] = h.first[i];
        f->holelist[3*holeIndx + 0] = h.second[0];
        f->holelist[3*holeIndx + 1] = h.second[1];
        f->holelist[3*holeIndx + 2] = h.second[2];
        ++holeIndx;
      }
    }

    /*
    p = &f->polygonlist[0];
    p->numberofvertices = verts.size();
    p->vertexlist = new int[p->numberofvertices];
    for(size_t i = 0; i < verts.size(); ++i)
      p->vertexlist[i] = verts[i];

    //add each self-intersect as hole
    size_t i = 0;
    cout << "Num holes in facet " << _fIndx << ": " << holes.size() << endl;
    for(auto& h : holes) {
      p = &f->polygonlist[i + 1];
      p->numberofvertices = 3;
      p->vertexlist = new int[p->numberofvertices];
      p->vertexlist[0] = h.first[0];
      p->vertexlist[1] = h.first[1];
      p->vertexlist[2] = h.first[2];
      f->holelist[3*i + 0] = h.second[0];
      f->holelist[3*i + 1] = h.second[1];
      f->holelist[3*i + 2] = h.second[2];
      ++i;
    }*/

    /*for(size_t i = 0; i < verts.size(); ++i) {
      //outter segment
      Vector3d p0(&_tetModel->pointlist[3*verts[i]]);
      Vector3d p1(&_tetModel->pointlist[3*verts[(i+1)%verts.size()]]);
      if(holes.size())
        cout << "p0: " << p0 << endl
          << "p1: " << p1 << endl;
      //check each hole
      for(size_t j = 0; j < holes.size(); ++j) {
        //test if any segments coincide
        for(size_t k = 0; k < holes[j].first.size(); ++k) {
          Vector3d p2(&_tetModel->pointlist[3*holes[j].first[k]]);
          Vector3d p3(&_tetModel->pointlist[3*holes[j].first[(k+1)%holes[j].first.size()]]);
          //if coincide
          cout << "p2: " << p2 << endl
            << "p3: " << p3 << endl;
          cout << "do1: " << (p2-(p0+(p2-p0).proj(p1-p0))) << endl
            << "do2: " << (p3-(p0+(p3-p0).proj(p1-p0))) << endl;
          if(
              (p2-p0).proj(p1-p0).norm() <= (p1-p0).norm() + 0.00000001 &&
              (p3-p0).proj(p1-p0).norm() <= (p1-p0).norm() + 0.00000001 &&
              (p2-p1).proj(p0-p1).norm() <= (p0-p1).norm() + 0.00000001 &&
              (p3-p1).proj(p0-p1).norm() <= (p0-p1).norm() + 0.00000001 &&
              (p2-(p0+(p2-p0).proj(p1-p0))).norm() < 0.00000001 &&
              (p3-(p0+(p3-p0).proj(p1-p0))).norm() < 0.00000001) {
            cout << "verts: ";
            for(auto& x : verts)
              cout << x << " ";
            cout << endl;
            cout << "hole: ";
            for(auto& x : holes[j].first)
              cout << x << " ";
            cout << endl;
            cout << "i: " << i << endl;
            cout << "k: " << k << endl;
            //merge into outter polygon
            if((p2-p0).norm() < (p3-p0).norm()) {
              cout << "k is closer" << endl;
              for(size_t l = 0; l < holes[j].first.size(); ++l)
                verts.insert(verts.begin() + i + 1, holes[j].first[(k + 1 + l)%holes[j].first.size()]);
            }
            else {
              cout << "k+1 is closer" << endl;
              for(size_t l = 0; l < holes[j].first.size(); ++l)
                verts.insert(verts.begin() + i + 1 + l, holes[j].first[(k + 1 + l)%holes[j].first.size()]);
            }
            verts.resize(distance(verts.begin(), unique(verts.begin(), verts.end())));
            cout << "final: ";
            for(auto& x : verts)
              cout << x << " ";
            cout << endl;
            //verts.insert(verts.begin() + i, holes[j].first.begin(), holes[j].first.end());
            //remove hole
            holes.erase(holes.begin() + j);
            cout << "Removing hole!" << endl;
            i = 0; break; break;
          }
        }
      }
    }
    */
    /*f->numberofpolygons = 1 + holes.size();
    f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
    f->numberofholes = holes.size();
    f->holelist = new REAL[3*f->numberofholes];
    p = &f->polygonlist[0];
    p->numberofvertices = verts.size();
    p->vertexlist = new int[p->numberofvertices];
    for(size_t i = 0; i < verts.size(); ++i)
      p->vertexlist[i] = verts[i];

    //add each self-intersect as hole
    size_t i = 0;
    cout << "Num holes in facet " << _fIndx << ": " << holes.size() << endl;
    for(auto& h : holes) {
      p = &f->polygonlist[i + 1];
      p->numberofvertices = 3;
      p->vertexlist = new int[p->numberofvertices];
      p->vertexlist[0] = h.first[0];
      p->vertexlist[1] = h.first[1];
      p->vertexlist[2] = h.first[2];
      f->holelist[3*i + 0] = h.second[0];
      f->holelist[3*i + 1] = h.second[1];
      f->holelist[3*i + 2] = h.second[2];
      ++i;
    }*/
  };

  addFacet(_fOff + 0, 0, 1, 3, 2);
  addFacet(_fOff + 1, 0, 2, 6, 4);
  addFacet(_fOff + 2, 4, 5, 7, 6);
  addFacet(_fOff + 3, 5, 1, 3, 7);
  addFacet(_fOff + 4, 7, 3, 2, 6);
  addFacet(_fOff + 5, 0, 4, 5, 1);

  /*tetgenio::facet* f;
  tetgenio::polygon* p;
  //side 1
  f = &_tetModel->facetlist[0];
  f->numberofpolygons = 1;
  f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
  f->numberofholes = 0;
  f->holelist = NULL;
  p = &f->polygonlist[0];
  p->numberofvertices = 4;
  p->vertexlist = new int[p->numberofvertices];
  p->vertexlist[0] = 3*_pOff + 0;
  p->vertexlist[1] = 3*_pOff + 1;
  p->vertexlist[2] = 3*_pOff + 3;
  p->vertexlist[3] = 3*_pOff + 2;

  //side 2
  f = &_tetModel->facetlist[1];
  f->numberofpolygons = 1;
  f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
  f->numberofholes = 0;
  f->holelist = NULL;
  p = &f->polygonlist[0];
  p->numberofvertices = 4;
  p->vertexlist = new int[p->numberofvertices];
  p->vertexlist[0] = 3*_pOff + 0;
  p->vertexlist[1] = 3*_pOff + 2;
  p->vertexlist[2] = 3*_pOff + 6;
  p->vertexlist[3] = 3*_pOff + 4;

  //side 3
  f = &_tetModel->facetlist[2];
  f->numberofpolygons = 1;
  f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
  f->numberofholes = 0;
  f->holelist = NULL;
  p = &f->polygonlist[0];
  p->numberofvertices = 4;
  p->vertexlist = new int[p->numberofvertices];
  p->vertexlist[0] = 3*_pOff + 4;
  p->vertexlist[1] = 3*_pOff + 5;
  p->vertexlist[2] = 3*_pOff + 7;
  p->vertexlist[3] = 3*_pOff + 6;

  //side 4
  f = &_tetModel->facetlist[3];
  f->numberofpolygons = 1;
  f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
  f->numberofholes = 0;
  f->holelist = NULL;
  p = &f->polygonlist[0];
  p->numberofvertices = 4;
  p->vertexlist = new int[p->numberofvertices];
  p->vertexlist[0] = 3*_pOff + 5;
  p->vertexlist[1] = 3*_pOff + 1;
  p->vertexlist[2] = 3*_pOff + 3;
  p->vertexlist[3] = 3*_pOff + 7;

  //side 5
  f = &_tetModel->facetlist[4];
  f->numberofpolygons = 1;
  f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
  f->numberofholes = 0;
  f->holelist = NULL;
  p = &f->polygonlist[0];
  p->numberofvertices = 4;
  p->vertexlist = new int[p->numberofvertices];
  p->vertexlist[0] = 3*_pOff + 7;
  p->vertexlist[1] = 3*_pOff + 3;
  p->vertexlist[2] = 3*_pOff + 2;
  p->vertexlist[3] = 3*_pOff + 6;

  //side 6
  f = &_tetModel->facetlist[5];
  f->numberofpolygons = 1;
  f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
  f->numberofholes = 0;
  f->holelist = NULL;
  p = &f->polygonlist[0];
  p->numberofvertices = 4;
  p->vertexlist = new int[p->numberofvertices];
  p->vertexlist[0] = 3*_pOff + 0;
  p->vertexlist[1] = 3*_pOff + 4;
  p->vertexlist[2] = 3*_pOff + 5;
  p->vertexlist[3] = 3*_pOff + 1;

  for(size_t i = 0; i < 6; ++i)
    _tetModel->facetmarkerlist[i] = 0;
  */
}

