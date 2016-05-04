#include "ReebGraphConstruction.h"

#include <unordered_map>

#include <containers/sequential/graph/algorithms/dijkstra.h>

#include <GL/gl.h>

#include "Utilities/MetricUtils.h"
#include "Utilities/TetGenDecomposition.h"

#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

ClockClass clockReebNodeComp;
ClockClass clockReebArcComp;
ClockClass clockGetReebArc;
ClockClass clockMergeArcs;
ClockClass clockGlueByMergeSorting;

ostream& operator<<(ostream& _os, ReebGraphConstruction::RGEID _rgeid) {
  return _os << "(" << _rgeid.source() << ", " << _rgeid.target() << ", " << _rgeid.id() << ")";
}

ReebGraphConstruction::
ReebGraphConstruction(TetGenDecomposition* _tetgen) {
  //Fill vertices vector
  size_t numPoints = _tetgen->GetNumPoints();
  const double* const points = _tetgen->GetPoints();
  m_vertices.reserve(numPoints);
  for(size_t i = 0; i < numPoints; ++i)
    m_vertices.emplace_back(&points[3*i]);

  //Fill triangle vector
  size_t numCorners = _tetgen->GetNumCorners();
  size_t numTetras = _tetgen->GetNumTetras();
  const int* const tetra = _tetgen->GetTetras();
  map<Triangle, unordered_set<size_t>> triangles;

  for(size_t i = 0; i < numTetras; ++i) {
    auto AddTriangle = [&](size_t _i, size_t _j, size_t _k) {
      int v[3] = {tetra[i*numCorners + _i], tetra[i*numCorners + _j], tetra[i*numCorners + _k]};
      sort(v, v+3);
      triangles[Triangle(v[0], v[1], v[2])].insert(i);
    };

    AddTriangle(0, 2, 1);
    AddTriangle(0, 3, 2);
    AddTriangle(0, 1, 3);
    AddTriangle(1, 2, 3);
  }

  m_triangles.reserve(triangles.size());
  copy(triangles.begin(), triangles.end(), back_inserter(m_triangles));

  clockReebNodeComp.SetName("Reeb Node Comp");
  clockReebArcComp.SetName("Reeb Arc Comp");
  clockGetReebArc.SetName("Get Reeb Arc");
  clockMergeArcs.SetName("Merge Arcs");
  clockGlueByMergeSorting.SetName("Glue By Merge Sorting");

  Construct();
  Embed(_tetgen);
}

void
ReebGraphConstruction::
Draw(const REAL* const _points, const int* const _tetra,
    size_t _numTetra, size_t _numCorners) {

  static size_t numCalls = 0;
  static size_t edge = 0;

  //cout << "Num calls: " << numCalls << endl;
  numCalls = (numCalls + 1) % 60;
  if(numCalls == 0) {
    edge = (edge + 1) % m_reebGraph.get_num_edges();
  }

  glDisable(GL_LIGHTING);
  glPointSize(8);
  glLineWidth(5);

  //draw reeb graph
  glColor4f(0.0, 0.5, 0.2, 0.05);

  /*glBegin(GL_POINTS);
  for(auto v = m_reebGraph.begin(); v != m_reebGraph.end(); ++v) {
    glVertex3dv(m_vertices[v->property().m_vertex]);
  }
  glEnd();

  glColor4f(0.0, 0.2, 0.5, 0.05);*/

  glBegin(GL_POINTS);
  for(auto v = m_reebGraph.begin(); v != m_reebGraph.end(); ++v) {
    glVertex3dv(v->property().m_vertex2);
  }
  glEnd();

  size_t i = 0;
  for(auto e = m_reebGraph.edges_begin(); e != m_reebGraph.edges_end(); ++e) {
    //if(i++ == edge) {
      glBegin(GL_LINE_STRIP);
      /*glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->source())->property().m_vertex]);
        glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->target())->property().m_vertex]);*/
      //glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->source())->property().m_vertex]);
      for(auto& v : e->property().m_path)
        glVertex3dv(v);
      //glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->target())->property().m_vertex]);
      glEnd();
    //}
  }

  /*glColor4f(0.5, 0.0, 0.2, 0.05);
  glLineWidth(3);
  glBegin(GL_LINES);
  i = 0;
  for(auto e = m_reebGraph.edges_begin(); e != m_reebGraph.edges_end(); ++e) {
    if(i++ == edge) {
      glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->source())->property().m_vertex]);
      glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->target())->property().m_vertex]);
    }
  }
  glEnd();*/

  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  i = 0;
  for(auto e = m_reebGraph.edges_begin(); e != m_reebGraph.edges_end(); ++e) {
    if(i++ == edge) {
      ReebArc& ra = e->property();
      glColor4fv(m_colors[e->descriptor()]);
      //cout << m_colors.size() << " Color: " << m_colors[e->descriptor()] << endl;
      glBegin(GL_TRIANGLES);
      for(const size_t& i : ra.m_tetra) {
        Vector3d vs[_numCorners];
        for(size_t j = 0; j < _numCorners; ++j)
          vs[j] = Vector3d(&_points[3*_tetra[i*_numCorners + j]]);
        glVertex3dv(vs[0]);
        glVertex3dv(vs[2]);
        glVertex3dv(vs[1]);
        glVertex3dv(vs[0]);
        glVertex3dv(vs[3]);
        glVertex3dv(vs[2]);
        glVertex3dv(vs[0]);
        glVertex3dv(vs[1]);
        glVertex3dv(vs[3]);
        glVertex3dv(vs[1]);
        glVertex3dv(vs[2]);
        glVertex3dv(vs[3]);
      }
      glEnd();
    }
  }

  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);

  glEnable(GL_LIGHTING);
}

pair<ReebGraphConstruction::FlowGraph, size_t>
ReebGraphConstruction::
GetFlowGraph(const Vector3d& _p, double _posRes) {
  typedef FlowGraph::vertex_descriptor FVD;
  FlowGraph f;

  enum Color {White, Gray, Black};
  unordered_map<FVD, Color> visited;

  //add vertices of reeb graph and find closest
  double closestDist = numeric_limits<double>::max();
  FVD closestID = -1;
  for(auto vit = m_reebGraph.begin(); vit != m_reebGraph.end(); ++vit) {
    Vector3d v = vit->property().m_vertex2;
    FVD vd = vit->descriptor();
    f.add_vertex(vd, v);
    //cout << "Adding vertex: " << vd << endl;
    //cout << "Has edges: " << endl;
    //for(auto eit = vit->begin(); eit != vit->end(); ++eit)
    //  cout << "\t" << eit->source() << " " << eit->target() << " " << eit->id() << endl;
    visited[vd] = White;
    double dist = (v - _p).norm();
    if(dist < closestDist) {
      closestDist = dist;
      closestID = vd;
    }
  }

  //Specialized BFS to make flow network
  //
  //Differs from regular BFS because:
  //  - Treats ReebGraph as undirected graph even though it is directed
  //  - Computes a graph instead of BFS tree, i.e., cross edges are added
  queue<FVD> q;
  q.push(closestID);
  visited[closestID] = Gray;
  while(!q.empty()) {
    FVD u = q.front();
    q.pop();
    auto uit = m_reebGraph.find_vertex(u);

    //process outgoing edges
    for(auto eit = uit->begin(); eit != uit->end(); ++eit) {
      FVD v = eit->target();
      switch(visited[v]) {
        case White:
          visited[v] = Gray;
          q.push(v);
        case Gray:
          f.add_edge(eit->descriptor(), eit->property().m_path);
          break;
        default:
          break;
      }
    }

    //process incoming edges
    set<FVD> processed;
    for(auto pit = uit->predecessors().begin();
        pit != uit->predecessors().end(); ++pit) {
      FVD v = *pit;
      if(processed.count(v) == 0) {
        auto vit = m_reebGraph.find_vertex(v);
        for(auto eit = vit->begin(); eit != vit->end(); ++eit) {
          if(eit->target() == u) {
            switch(visited[v]) {
              case White:
                visited[v] = Gray;
                q.push(v);
              case Gray:
                {
                  vector<Vector3d>& opath = eit->property().m_path;
                  vector<Vector3d> path(opath.rbegin(), opath.rend());
                  f.add_edge(ReebGraph::edge_descriptor(u, v, eit->descriptor().id()), path);
                  break;
                }
              default:
                break;
            }
          }
        }
      }
      processed.emplace(v);
    }
    visited[u] = Black;
  }

  return make_pair(f, closestID);
}

void
ReebGraphConstruction::
Construct() {
  auto f = [&](const Vector3d& v) {
    return v[1]; //take y value as morse function. TODO generalize
  };

  ClockClass clockTotal, clockAddVertices, clockVertexSort, clockTriangleSort,
             clockAddEdges, clockAddTriangles, clockDelete2Nodes, clockMakePaths;

  clockTotal.SetName("Total");
  clockAddVertices.SetName("Add Vertices");
  clockVertexSort.SetName("Vertex Sort");
  clockTriangleSort.SetName("Triangle Sort");
  clockAddEdges.SetName("Add Edges");
  clockAddTriangles.SetName("Add Triangles");
  clockDelete2Nodes.SetName("Delete 2-Nodes");
  clockMakePaths.SetName("Make Paths");

  clockTotal.StartClock();

  clockAddVertices.StartClock();
  for(size_t i = 0; i < m_vertices.size(); ++i) {
    double w = f(m_vertices[i]);
    CreateNode(i, m_vertices[i], w);
    //cout << "Vertex: " << i << " at " << m_vertices[i] << " f = " << f(m_vertices[i]) << endl;
  }

  clockAddVertices.StopClock();

  cout << "Add Vertices done." << endl;

  clockVertexSort.StartClock();

  vector<size_t> order;
  for(size_t i = 0; i < m_vertices.size(); ++i)
    order.emplace_back(i);
  ReebNodeComp rnc(&m_vertices);
  sort(order.begin(), order.end(), [&](size_t _a, size_t _b) {
        return rnc(m_reebGraph.find_vertex(_a)->property(), m_reebGraph.find_vertex(_b)->property());
      });
  for(size_t i = 0; i < order.size(); ++i)
    m_reebGraph.find_vertex(order[i])->property().m_order = i;

  clockVertexSort.StopClock();

  clockTriangleSort.StartClock();

  for(auto& t : m_triangles) {
    size_t v[3] = {get<0>(t.first), get<1>(t.first), get<2>(t.first)};
    sort(v, v+3, [&](size_t _a, size_t _b) {
        return m_reebGraph.find_vertex(_a)->property().m_order <
            m_reebGraph.find_vertex(_b)->property().m_order;
        });

    t.first = make_tuple(v[0], v[1], v[2]);
  }
  cout << "triangles: " << m_triangles.size() << endl;

  clockTriangleSort.StopClock();

  cout << "Triangles Sort done." << endl;

  clockAddEdges.StartClock();

  for(auto& t : m_triangles) {
    size_t v[3] = {get<0>(t.first), get<1>(t.first), get<2>(t.first)};

    MeshEdge* e0 = CreateArc(v[0], v[2], t.second);
    MeshEdge* e1 = CreateArc(v[0], v[1], t.second);
    MeshEdge* e2 = CreateArc(v[1], v[2], t.second);

    ++e0->m_numTris;
    ++e1->m_numTris;
    ++e2->m_numTris;
  }

  clockAddEdges.StopClock();

  cout << "Add Edges done." << endl;

  clockAddTriangles.StartClock();

  for(auto& t : m_triangles) {
    size_t v[3] = {get<0>(t.first), get<1>(t.first), get<2>(t.first)};

    //cout << "MergePaths with triangle {" << v[0] << ", " << v[1] << ", " << v[2] << "}" << endl;

    MeshEdge* e0 = CreateArc(v[0], v[2]);
    MeshEdge* e1 = CreateArc(v[0], v[1]);
    MeshEdge* e2 = CreateArc(v[1], v[2]);

    MergePaths(e0, e1, e2);

    //wait to continue for debug
    //cout << "Graph: nodes = " << m_reebGraph.get_num_vertices()
    //  << " edges = " << m_reebGraph.get_num_edges() << endl;
    //for(auto eit = m_reebGraph.edges_begin(); eit != m_reebGraph.edges_end(); ++eit)
    //  cout << "\tEdge: " << eit->descriptor() << endl;

    --e0->m_numTris;
    --e1->m_numTris;
    --e2->m_numTris;

    //delete edge optimization
    if(e0->m_numTris == 0)
      DeleteMeshEdge(e0);
    if(e1->m_numTris == 0)
      DeleteMeshEdge(e1);
    if(e2->m_numTris == 0)
      DeleteMeshEdge(e2);
  }

  clockAddTriangles.StopClock();

  cout << "Add Triangles done." << endl;

  clockDelete2Nodes.StartClock();

  bool removed;
  do {
    removed = false;
    for(auto vit = m_reebGraph.begin(); vit != m_reebGraph.end(); ++vit) {
      if(vit->predecessors().size() == 1 && vit->size() == 1) {
        //cout << "Can remove vid: " << vit->descriptor() << endl;
        //Remove2Node(vit);
        auto pred = m_reebGraph.find_vertex(vit->predecessors()[0]);
        RGEID in, out;
        for(auto eit : *pred) {
          if(eit.target() == vit->descriptor())
            in = eit.descriptor();
        }
        out = (*vit->begin()).descriptor();

        ReebArc& ain = GetReebArc(in);
        ReebArc& aout = GetReebArc(out);

        //do merge
        RGEID neweid = m_reebGraph.add_edge(ain.m_source, aout.m_target, ReebArc(ain.m_source, aout.m_target));

        m_colors[neweid] = Color4(DRand()*0.5 + 0.25, DRand()*0.5 + 0.25, DRand()*0.5 + 0.25, 0.2);

        ReebArc& newa = GetReebArc(neweid);
        for(auto& edge : ain.m_edges) {
          edge->m_arcs.insert(neweid);
          edge->m_arcs.erase(in);
          newa.m_edges.insert(edge);
        }
        newa.m_tetra.insert(ain.m_tetra.begin(), ain.m_tetra.end());
        for(auto& edge : aout.m_edges) {
          edge->m_arcs.insert(neweid);
          edge->m_arcs.erase(out);
          newa.m_edges.insert(edge);
        }
        newa.m_tetra.insert(aout.m_tetra.begin(), aout.m_tetra.end());

        m_reebGraph.delete_edge(in);
        m_reebGraph.delete_edge(out);
        m_reebGraph.delete_vertex(vit->descriptor());
        --vit;
        removed = true;
      }
    }
  } while(removed);
  for(auto vit = m_reebGraph.begin(); vit != m_reebGraph.end(); ++vit) {
    if(vit->predecessors().size() == 1 && vit->size() == 1)
      cout << "Err: Can remove vid: " << vit->descriptor() << endl;
  }

  clockDelete2Nodes.StopClock();

  cout << "Delete 2 Nodes done." << endl;

  clockMakePaths.StartClock();

  map<RGEID, Color4, RGEIDComp> tmpColors = m_colors;
  m_colors.clear();

  for(auto eit = m_reebGraph.edges_begin();
      eit != m_reebGraph.edges_end(); ++eit) {
    //cout << "Edge: " << eit->descriptor() << endl;
    m_colors[eit->descriptor()] = tmpColors[eit->descriptor()];
  }

  clockMakePaths.StopClock();

  clockTotal.StopClock();

  cout << "Final Graph: nodes = " << m_reebGraph.get_num_vertices()
    << " edges = " << m_reebGraph.get_num_edges() << endl;

  //clockReebNodeComp.PrintClock(cout);
  clockReebArcComp.PrintClock(cout);
  clockGetReebArc.PrintClock(cout);
  clockMergeArcs.PrintClock(cout);
  clockGlueByMergeSorting.PrintClock(cout);
  clockAddVertices.PrintClock(cout);
  clockVertexSort.PrintClock(cout);
  clockTriangleSort.PrintClock(cout);
  clockAddEdges.PrintClock(cout);
  clockAddTriangles.PrintClock(cout);
  clockDelete2Nodes.PrintClock(cout);
  clockMakePaths.PrintClock(cout);
  clockTotal.PrintClock(cout);
}

void
ReebGraphConstruction::
Embed(TetGenDecomposition* _tetgen) {
  size_t numCorners = _tetgen->GetNumCorners();
  const int* const tetras = _tetgen->GetTetras();
  TetGenDecomposition::DualGraph& tetraGraph = _tetgen->GetDualGraph();

  //embed ReebNodes in tetrahedralization
  for(auto nit = m_reebGraph.begin(); nit != m_reebGraph.end(); ++nit) {
    Vector3d& vn = nit->property().m_vertex2;
    double minDist = numeric_limits<double>::max();
    Vector3d closest;
    size_t closestID = -1;
    for(auto tit = tetraGraph.begin(); tit != tetraGraph.end(); ++tit) {
      Vector3d& vt = tit->property();
      double dist = (vt - vn).norm();
      if(dist < minDist) {
        minDist = dist;
        closest = vt;
        closestID = tit->descriptor();
      }
    }
    nit->property().m_vertex2 = closest;
    nit->property().m_tetra = closestID;
  }

  //embed ReebArcs in tetrahedralization
  for(auto eit = m_reebGraph.edges_begin(); eit != m_reebGraph.edges_end(); ++eit) {
    ReebArc& ra = eit->property();
    //weight all
    for(auto& tetraid : ra.m_tetra) {
      auto vit = tetraGraph.find_vertex(tetraid);
      for(auto eit = vit->begin(); eit != vit->end(); ++eit) {
        auto targetit = tetraGraph.find_vertex(eit->target());
        eit->property() = 0.01 * (vit->property() - targetit->property()).norm();
      }
    }

    //find path
    auto sourceit = m_reebGraph.find_vertex(eit->source());
    auto targetit = m_reebGraph.find_vertex(eit->target());

    vector<size_t> pathVID;
    stapl::sequential::find_path_dijkstra(tetraGraph,
        sourceit->property().m_tetra, targetit->property().m_tetra,
        pathVID, numeric_limits<double>::max());

    ra.m_path.clear();

    for(auto vit1 = pathVID.begin(), vit2 = vit1 + 1;
        vit2 != pathVID.end(); ++vit1, ++vit2) {
      Vector3d& v1 = tetraGraph.find_vertex(*vit1)->property();
      ra.m_path.push_back(v1);

      int t1[4] = {tetras[(*vit1)*numCorners + 0],
        tetras[(*vit1)*numCorners + 1],
        tetras[(*vit1)*numCorners + 2],
        tetras[(*vit1)*numCorners + 3]};
      int t2[4] = {tetras[(*vit2)*numCorners + 0],
        tetras[(*vit2)*numCorners + 1],
        tetras[(*vit2)*numCorners + 2],
        tetras[(*vit2)*numCorners + 3]};

      int tcommon[3];
      size_t j = 0;
      for(size_t i = 0; i < 4; ++i) {
        int* f = find(t2, t2+4, t1[i]);
        if(f != t2 + 4)
          tcommon[j++] = *f;
      }

      Vector3d c;
      for(size_t i = 0; i < 3; ++i)
        c += m_vertices[tcommon[i]];
      c /= 3;

      ra.m_path.push_back(c);
    }
    ra.m_path.push_back(tetraGraph.find_vertex(pathVID.back())->property());

    //unweight all
    for(auto& tetraid : ra.m_tetra) {
      auto vit = tetraGraph.find_vertex(tetraid);
      for(auto eit = vit->begin(); eit != vit->end(); ++eit) {
        auto targetit = tetraGraph.find_vertex(eit->target());
        eit->property() = (vit->property() - targetit->property()).norm();
      }
    }
  }
}

void
ReebGraphConstruction::
CreateNode(size_t _i, const Vector3d& _v, double _w) {
  m_reebGraph.add_vertex(_i, ReebNode(_i, _v, _w));
}

ReebGraphConstruction::MeshEdge*
ReebGraphConstruction::
CreateArc(size_t _s, size_t _t, const unordered_set<size_t>& _tetra) {
  MeshEdge m(_s, _t, &m_vertices, &m_reebGraph);
  if(!m_edges.count(&m)) {
    MeshEdge* m2 = new MeshEdge(_s, _t, &m_vertices, &m_reebGraph);
    m_edges.insert(m2);
    RGEID eid = m_reebGraph.add_edge(_s, _t, ReebArc(_s, _t, m2));
    //cout << "\tAdding edge: " << eid << endl;
    m2->m_arcs.insert(eid);
  }
  MeshEdge* e = *m_edges.find(&m);
  for(auto& a : e->m_arcs) {
    ReebArc& ra = GetReebArc(a);
    ra.m_tetra.insert(_tetra.begin(), _tetra.end());
  }
  return e;
}

void
ReebGraphConstruction::
MergePaths(MeshEdge* _e0, MeshEdge* _e1, MeshEdge* _e2) {
  //cout << "\tGraph: nodes = " << m_reebGraph.get_num_vertices()
  //  << " edges = " << m_reebGraph.get_num_edges() << endl;
  /*for(auto eit = m_reebGraph.edges_begin(); eit != m_reebGraph.edges_end(); ++eit)
    cout << "\t\tEdge: " << eit->descriptor() << endl;*/

  ArcSet& a0 = _e0->m_arcs;
  ArcSet& a1 = _e1->m_arcs;
  GlueByMergeSorting(a0, _e0, a1, _e1);

  //cout << "\tGraph: nodes = " << m_reebGraph.get_num_vertices()
  //  << " edges = " << m_reebGraph.get_num_edges() << endl;
  /*for(auto eit = m_reebGraph.edges_begin(); eit != m_reebGraph.edges_end(); ++eit)
    cout << "\t\tEdge: " << eit->descriptor() << endl;*/

  ArcSet& a2 = _e0->m_arcs;
  ArcSet& a3 = _e2->m_arcs;
  GlueByMergeSorting(a2, _e0, a3, _e2);
}

void
ReebGraphConstruction::
GlueByMergeSorting(ArcSet& _a0, MeshEdge* _e0, ArcSet& _a1, MeshEdge* _e1) {
  //while(a0 and a1 are "valid" arcs) do
  //  n0 <- BottomNode(a0)
  //  n1 <- BottomNode(a1)
  //  if f(n0) > f(n1) then
  //    MergeArcs(a0, a1)
  //  else
  //    MergeArcs(a1, a0)
  //  a0 <- NextArcMappedToEdge(a0, e0)
  //  a1 <- NextArcMappedToEdge(a1, e1)

  //cout << "\tGlueByMergeSorting" << endl;
  //cout << "\t\tEdge _e0: " << _e0->m_source << " " << _e0->m_target << " ArcSet _a0: ";
  //for(auto& a : _a0)
  //  cout << a << " ";
  //cout << endl;
  //cout << "\t\tEdge _e1: " << _e1->m_source << " " << _e1->m_target << " ArcSet _a1: ";
  //for(auto& a : _a1)
  //  cout << a << " ";
  //cout << endl;

  //cout << "\t\tBeginning merge" << endl;
  clockGlueByMergeSorting.StartClock();
  ReebArcComp rac(&m_vertices, &m_reebGraph);
  for(auto asit0 = _a0.begin(), asit1 = _a1.begin();
      asit0 != _a0.end() && asit1 != _a1.end();) {

    //cout << "\n\t\t\ta0: " << *asit0 << endl;
    //cout << "\t\t\ta1: " << *asit1 << endl;

    if(*asit0 == *asit1) {
      /*cout << "\t\t\tIdentical EIDs. Skipping." << endl;
      cout << "\t\t\t_e0: " << _e0 << " ArcSet _a0: ";
      for(auto& a : _a0)
        cout << a << " ";
      cout << endl;
      cout << "\t\t\t_e1: " << _e1 << " ArcSet _a1: ";
      for(auto& a : _a1)
        cout << a << " ";
      cout << endl;
      cin.ignore();
      */
      ++asit0; ++asit1;
      continue;
    }

    if(asit0->source() != asit1->source()) {
      //cout << "\t\t\tSources do not match. Skipping." << endl;
      if(rac(*asit0, *asit1))
        ++asit0;
      else
        ++asit1;
      continue;
    }

    ReebNode& n0 = m_reebGraph.find_vertex(asit0->target())->property();
    ReebNode& n1 = m_reebGraph.find_vertex(asit1->target())->property();

    /*cout << "\t\t\tn0: " << n0.m_vertex
      << " at " << m_vertices[n0.m_vertex]
      << " with f = " << n0.m_w << endl;;
    cout << "\t\t\tn1: " << n1.m_vertex
      << " at " << m_vertices[n1.m_vertex]
      << " with f = " << n1.m_w << endl;;
    */

    if(n0.m_order < n1.m_order) {
      MergeArcs(*asit0, *asit1);
      ++asit0;
      asit1 = _a1.begin();
    }
    else {
      MergeArcs(*asit1, *asit0);
      ++asit1;
      asit0 = _a0.begin();
    }
  }
  clockGlueByMergeSorting.StopClock();
}

void
ReebGraphConstruction::
MergeArcs(RGEID _a0, RGEID _a1) {
  clockMergeArcs.StartClock();
  /*if(_a0.source() != _a1.source()) {
    cout << "\t\t\tSources do not match. Skipping." << endl;
    return;
  }*/
  //cout << "\t\t\tMerging: _a0 = " << _a0 << " _a1 = " << _a1 << endl;
  ReebArc& a0 = GetReebArc(_a0);
  ReebArc& a1 = GetReebArc(_a1);

  for(auto& edge : a1.m_edges) {
    /*auto i =*/ edge->m_arcs.insert(_a0);
    /*if(!i.second) {
      cout << "***Insert existed? " << _a0 << endl;
      for(auto& arc : edge->m_arcs)
        cout << arc << " ";
      cout << endl;
      cin.ignore();
    }*/
    edge->m_arcs.erase(_a1);
    a0.m_edges.insert(edge);
  }
  a0.m_tetra.insert(a1.m_tetra.begin(), a1.m_tetra.end());

  //if(_a0.target() == _a1.target()) {
    //cout << "\t\t\tMerging multiedge" << endl;
  //}
  /*else */if(_a0.target() != _a1.target()) {
    ReebArc a = a1;
    a.m_source = a0.m_target;
    RGEID neweid = m_reebGraph.add_edge(a.m_source, a.m_target, a);
    //cout << "\t\t\tRenaming arc " << _a1 << " to " << neweid << endl;
    for(auto& edge : GetReebArc(neweid).m_edges)
      edge->m_arcs.insert(neweid);
  }
  /*else if(_a0.source() != _a1.source() && _a0.target() == _a1.target()) {
    cout << "***Merging: " << _a0 << " " << _a1 << endl;
    cin.ignore();
  }*/
  m_reebGraph.delete_edge(_a1);
  clockMergeArcs.StopClock();
}

ReebGraphConstruction::ReebArc&
ReebGraphConstruction::
GetReebArc(RGEID _a) {
  clockGetReebArc.StartClock();
  ReebGraph::adj_edge_iterator ei;
  ReebGraph::vertex_iterator vi;
  m_reebGraph.find_edge(_a, vi, ei);
  clockGetReebArc.StopClock();
  return ei->property();
}

void
ReebGraphConstruction::
DeleteMeshEdge(MeshEdge* _e) {
  for(auto& a : _e->m_arcs)
    GetReebArc(a).m_edges.erase(_e);
}
