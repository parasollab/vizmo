#include "ReebGraphConstruction.h"

#include <GL/gl.h>

#include "Utilities/MetricUtils.h"

#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

ostream& operator<<(ostream& _os, ReebGraphConstruction::RGEID _rgeid) {
  return _os << "(" << _rgeid.source() << ", " << _rgeid.target() << ", " << _rgeid.id() << ")";
}

ReebGraphConstruction::
ReebGraphConstruction(const vector<Vector3d>& _vertices,
    const vector<tuple<size_t, size_t, size_t>>& _triangles) :
  m_vertices(_vertices), m_triangles(_triangles),
  m_minBucket(numeric_limits<double>::max()),
  m_maxBucket(numeric_limits<double>::min()) {
    Construct();
  }

void
ReebGraphConstruction::
Draw() {
  glDisable(GL_LIGHTING);
  glPointSize(8);
  glLineWidth(5);

  //draw reeb graph
  glColor4f(0.0, 0.5, 0.2, 0.05);

  glBegin(GL_POINTS);
  for(auto v = m_reebGraph.begin(); v != m_reebGraph.end(); ++v) {
    glVertex3dv(m_vertices[v->property().m_vertex]);
  }
  glEnd();

  for(auto e = m_reebGraph.edges_begin(); e != m_reebGraph.edges_end(); ++e) {
  glBegin(GL_LINE_STRIP);
    /*glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->source())->property().m_vertex]);
    glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->target())->property().m_vertex]);*/
    //glVertex3dv(e->property().m_buckets.begin()->second.second);
    //glVertex3dv(e->property().m_buckets.rbegin()->second.second);
    /*glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->target())->property().m_vertex]);
    for(auto& bucket : e->property().m_buckets) {
      glVertex3dv(bucket.second.second);
      glVertex3dv(bucket.second.second);
    }
    glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->source())->property().m_vertex]);*/
    //glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->source())->property().m_vertex]);
    for(auto& v : e->property().m_path) {
      glVertex3dv(v);
      //glVertex3dv(v);
    }
    //glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->target())->property().m_vertex]);
  glEnd();
  }

  glColor4f(0.5, 0.0, 0.2, 0.05);
  glLineWidth(1);
  glBegin(GL_LINES);
  for(auto e = m_reebGraph.edges_begin(); e != m_reebGraph.edges_end(); ++e) {
    glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->source())->property().m_vertex]);
    glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->target())->property().m_vertex]);
  }
  glEnd();

  glEnable(GL_LIGHTING);

}

void
ReebGraphConstruction::
Construct() {
  auto f = [&](const Vector3d& v) {
    return v[1]; //take y value as morse function. TODO generalize
  };

  ClockClass clockTotal, clockAddVertices, clockTriangleSort, clockAddEdges,
             clockAddTriangles, clockDelete2Nodes, clockMakePaths;

  clockTotal.SetName("Total");
  clockAddVertices.SetName("Add Vertices");
  clockTriangleSort.SetName("Triangle Sort");
  clockAddEdges.SetName("Add Edges");
  clockAddTriangles.SetName("Add Triangles");
  clockDelete2Nodes.SetName("Delete 2-Nodes");
  clockMakePaths.SetName("Make Paths");

  clockTotal.StartClock();

  clockAddVertices.StartClock();
  for(size_t i = 0; i < m_vertices.size(); ++i) {
    double w = f(m_vertices[i]);
    CreateNode(i, w);
    if(w < m_minBucket)
      m_minBucket = w;
    if(w > m_maxBucket)
      m_maxBucket = w;
    //cout << "Vertex: " << i << " at " << m_vertices[i] << " f = " << f(m_vertices[i]) << endl;
  }
  //m_numBuckets = ceil((m_maxBucket - m_minBucket)/GetVizmo().GetEnv()->GetEnvironment()->GetPositionRes());
  m_bucketRes = (m_maxBucket - m_minBucket) / 65.;

  clockAddVertices.StopClock();

  //cout << "Buckets: " << m_numBuckets << " buckets from " << m_minBucket << " to " << m_maxBucket << endl;
  cout << "Buckets from " << m_minBucket << " to " << m_maxBucket << " at resolution " << m_bucketRes << endl;
  //cin.ignore();
  cout << "Add Vertices done." << endl;

  clockTriangleSort.StartClock();

  ReebNodeComp rnc(&m_vertices);
  for(auto tit = m_triangles.begin(); tit != m_triangles.end(); ++tit) {
    size_t v[3] = {get<0>(*tit), get<1>(*tit), get<2>(*tit)};
    sort(v, v+3, [&](size_t _a, size_t _b) {
        return rnc(m_reebGraph.find_vertex(_a)->property(),
            m_reebGraph.find_vertex(_b)->property());
        /*double fa = f(m_vertices[_a]);
        double fb = f(m_vertices[_b]);
        //return fa - fb > 0.000001 || (fabs(fa - fb) < 0.000001 && _a <= _b);
        return fa - fb > 0.000001 ||
        (fabs(fa - fb) < 0.000001 &&
         m_vertices[_a][0] - m_vertices[_b][0] > 0.000001) ||
        (fabs(fa - fb) < 0.000001 &&
         fabs(m_vertices[_a][0] - m_vertices[_b][0]) < 0.000001 &&
         m_vertices[_a][2] - m_vertices[_b][2] > 0.000001);
         */
        });

    *tit = make_tuple(v[0], v[1], v[2]);
  }
  cout << "triangles: " << m_triangles.size() << endl;
  sort(m_triangles.begin(), m_triangles.end());
  auto i = unique(m_triangles.begin(), m_triangles.end());
  m_triangles.resize(distance(m_triangles.begin(), i));
  cout << "triangles: " << m_triangles.size() << endl;
  //cin.ignore();

  clockTriangleSort.StopClock();

  cout << "Triangles Sort done." << endl;

  clockAddEdges.StartClock();

  for(auto& t : m_triangles) {
    size_t v[3] = {get<0>(t), get<1>(t), get<2>(t)};
    /*sort(v, v+3, [&](size_t _a, size_t _b) {
        double fa = f(m_vertices[_a]);
        double fb = f(m_vertices[_b]);
        return fa > fb || (fa == fb && _a < _b);
        });*/

    MeshEdge* e0 = CreateArc(v[0], v[2]);
    MeshEdge* e1 = CreateArc(v[0], v[1]);
    MeshEdge* e2 = CreateArc(v[1], v[2]);

    ++e0->m_numTris;
    ++e1->m_numTris;
    ++e2->m_numTris;
  }

  clockAddEdges.StopClock();

  cout << "Add Edges done." << endl;

  clockAddTriangles.StartClock();

  for(auto& t : m_triangles) {
    size_t v[3] = {get<0>(t), get<1>(t), get<2>(t)};
    /*sort(v, v+3, [&](size_t _a, size_t _b) {
        double fa = f(m_vertices[_a]);
        double fb = f(m_vertices[_b]);
        return fa - fb > 0.000001 || (fabs(fa - fb) < 0.000001 && _a < _b);
        });*/

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

    //cin.ignore();

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
      if(vit->predecessors().size() /*m_reebGraph.get_in_degree(vit->descriptor())*/ == 1 && vit->size()/*m_reebGraph.get_out_degree(vit->descriptor())*/ == 1) {
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
        ReebArc& newa = GetReebArc(neweid);
        for(auto& edge : ain.m_edges) {
          edge->m_arcs.insert(neweid);
          edge->m_arcs.erase(in);
          newa.m_edges.insert(edge);
        }
        for(auto& bucket : ain.m_buckets)
          newa.m_buckets[bucket.first].insert(bucket.second.begin(), bucket.second.end());
        /*for(auto& bucket : ain.m_buckets) {
          auto& b = newa.m_buckets[bucket.first];
          b.second = b.second*b.first + bucket.second.second*bucket.second.first;
          b.first += bucket.second.first;
          b.second /= b.first;
        }*/
        for(auto& edge : aout.m_edges) {
          edge->m_arcs.insert(neweid);
          edge->m_arcs.erase(out);
          newa.m_edges.insert(edge);
        }
        for(auto& bucket : aout.m_buckets)
          newa.m_buckets[bucket.first].insert(bucket.second.begin(), bucket.second.end());
        /*for(auto& bucket : aout.m_buckets) {
          auto& b = newa.m_buckets[bucket.first];
          b.second = b.second*b.first + bucket.second.second*bucket.second.first;
          b.first += bucket.second.first;
          b.second /= b.first;
        }*/
        m_reebGraph.delete_edge(in);
        m_reebGraph.delete_edge(out);
        m_reebGraph.delete_vertex(vit->descriptor());
        --vit;
        removed = true;
      }
    }
  } while(removed);
  for(auto vit = m_reebGraph.begin(); vit != m_reebGraph.end(); ++vit) {
    if(vit->predecessors().size() /*m_reebGraph.get_in_degree(vit->descriptor())*/ == 1 && vit->size()/*m_reebGraph.get_out_degree(vit->descriptor())*/ == 1) {
      cout << "Err: Can remove vid: " << vit->descriptor() << endl;
    }
  }

  clockDelete2Nodes.StopClock();

  cout << "Delete 2 Nodes done." << endl;

  clockMakePaths.StartClock();

  for(auto eit = m_reebGraph.edges_begin(); eit != m_reebGraph.edges_end(); ++eit) {
    //cout << "Edge: " << eit->descriptor() << endl;
    for(auto& bucket : eit->property().m_buckets) {
      //cout << "\tBucket: " << bucket.first << " with " << bucket.second.size() << endl;
      Vector3d v;
      for(auto& vid : bucket.second)
        v += m_vertices[vid];
      v /= bucket.second.size();
      //cout << "Averaged to: " << v << endl;
      eit->property().m_path.push_back(v);
    }
  }

  clockMakePaths.StopClock();

  clockTotal.StopClock();

  cout << "Final Graph: nodes = " << m_reebGraph.get_num_vertices()
    << " edges = " << m_reebGraph.get_num_edges() << endl;
  //for(auto eit = m_reebGraph.edges_begin(); eit != m_reebGraph.edges_end(); ++eit)
  //  cout << "\tEdge: " << eit->descriptor() << endl;

  clockAddVertices.PrintClock(cout);
  clockTriangleSort.PrintClock(cout);
  clockAddEdges.PrintClock(cout);
  clockAddTriangles.PrintClock(cout);
  clockDelete2Nodes.PrintClock(cout);
  clockMakePaths.PrintClock(cout);
  clockTotal.PrintClock(cout);
}

void
ReebGraphConstruction::
CreateNode(size_t _i, double _w) {
  m_reebGraph.add_vertex(_i, ReebNode(_i, _w));
}

ReebGraphConstruction::MeshEdge*
ReebGraphConstruction::
CreateArc(size_t _s, size_t _t) {
  MeshEdge m(_s, _t, &m_vertices, &m_reebGraph);
  if(!m_edges.count(&m)) {
    MeshEdge* m2 = new MeshEdge(_s, _t, &m_vertices, &m_reebGraph);
    m_edges.insert(m2);
    RGEID eid = m_reebGraph.add_edge(_s, _t, ReebArc(_s, _t, m2));
    //cout << "\tAdding edge: " << eid << endl;
    m2->m_arcs.insert(eid);

    ReebArc& ra = GetReebArc(eid);
    double ws = m_reebGraph.find_vertex(_s)->property().m_w;
    double wt = m_reebGraph.find_vertex(_t)->property().m_w;
    size_t bis = floor((ws - m_minBucket)/m_bucketRes);
    size_t bit = floor((wt - m_minBucket)/m_bucketRes);
    ra.m_buckets[bis].insert(_s);
    ra.m_buckets[bit].insert(_t);
    /*cout << "ws: " << ws << " bs: " << bis << " wt: " << wt << " bt: " << bit << endl;
    auto& bs = ra.m_buckets[bis];
    bs.second = bs.second*bs.first + m_vertices[_s];
    ++bs.first;
    bs.second /= bs.first;
    auto& bt = ra.m_buckets[bit];
    bt.second = bt.second*bt.first + m_vertices[_t];
    ++bt.first;
    bt.second /= bt.first;
    cout << ra.m_buckets.size() << endl;
    for(auto& bucket : ra.m_buckets)
      cout << "b: " << bucket.first << " " << bucket.second.second << endl;*/
    //cin.ignore();
  }
  return *m_edges.find(&m);
}

/*ReebGraphConstruction::MeshEdge&
  ReebGraphConstruction::
  GetMeshEdge(size_t _s, size_t _t) {
  ReebGraph::edge_descriptor ed(_s, _t);
  ReebGraph::vertex_iterator vi;
  ReebGraph::edge_iterator ei;

  m_reebGraph.find_edge(ed, ei, vi);

  return ei->property();
  }*/

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
  //bool valid = true;
  //while(valid) {

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

    ReebNode& n0 = m_reebGraph.find_vertex(GetReebArc(*asit0).m_target)->property();
    ReebNode& n1 = m_reebGraph.find_vertex(GetReebArc(*asit1).m_target)->property();

    /*cout << "\t\t\tn0: " << n0.m_vertex
      << " at " << m_vertices[n0.m_vertex]
      << " with f = " << n0.m_w << endl;;
    cout << "\t\t\tn1: " << n1.m_vertex
      << " at " << m_vertices[n1.m_vertex]
      << " with f = " << n1.m_w << endl;;
    */
    //if(n0.m_w - n1.m_w > 0.000001 ||
    //    (fabs(n0.m_w - n1.m_w) < 0.000001 && asit0->target() < asit1->target()) ||
    //    (fabs(n0.m_w - n1.m_w) < 0.000001 && asit0->target() == asit1->target() && asit0->id() < asit1->id())
    ReebNodeComp rnc(&m_vertices);
    if(rnc(n0, n1)
        /*rac(*asit0, *asit1)*/) {
      MergeArcs(*asit0, *asit1);
      ++asit0;
      asit1 = _a1.begin();
    }
    else /*if(n0.m_w < n1.m_w)*/ {
      MergeArcs(*asit1, *asit0);
      ++asit1;
      asit0 = _a0.begin();
    }
    //n0.m_w == n1.m_w && n0.m_vertex == n1.m_vertex
    //else {
      //merge special
      //cout << "\t\t\tMerging Special?" << endl;
      //MergeArcs(*asit1, *asit0);
      //++asit0, ++asit1;
    //}
    //  valid = false;
  }
}

void
ReebGraphConstruction::
MergeArcs(RGEID _a0, RGEID _a1) {
  /*if(_a0.source() != _a1.source()) {
    cout << "\t\t\tSources do not match. Skipping." << endl;
    return;
  }*/
  //cout << "\t\t\tMerging: _a0 = " << _a0 << " _a1 = " << _a1 << endl;
  ReebArc& a0 = GetReebArc(_a0);
  ReebArc& a1 = GetReebArc(_a1);
  for(auto& edge : a1.m_edges) {
    auto i = edge->m_arcs.insert(_a0);
    if(!i.second) {
      cout << "***Insert existed? " << _a0 << endl;
      for(auto& arc : edge->m_arcs)
        cout << arc << " ";
      cout << endl;
      cin.ignore();
    }
    edge->m_arcs.erase(_a1);
    a0.m_edges.insert(edge);
  }
  for(auto bit = a1.m_buckets.begin(); bit != a1.m_buckets.end();) {
    if(bit->first >= a0.m_buckets.rbegin()->first) {
      a0.m_buckets[bit->first].insert(bit->second.begin(), bit->second.end());
      /*auto& b = a0.m_buckets[bit->first];
      b.second = b.second*b.first + bit->second.second*bit->second.first;
      b.first += bit->second.first;
      b.second /= b.first;*/
      if(bit->first != a0.m_buckets.rbegin()->first)
        bit = a1.m_buckets.erase(bit);
      else
        ++bit;
    }
    else
      ++bit;
  }
  if(_a0.target() == _a1.target()) {
    //cout << "\t\t\tMerging multiedge" << endl;
  }
  else if(_a0.target() != _a1.target()) {
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
}

ReebGraphConstruction::ReebArc&
ReebGraphConstruction::
GetReebArc(RGEID _a) {
  ReebGraph::adj_edge_iterator ei;
  ReebGraph::vertex_iterator vi;
  m_reebGraph.find_edge(_a, vi, ei);
  return ei->property();
}

void
ReebGraphConstruction::
DeleteMeshEdge(MeshEdge* _e) {
  for(auto& a : _e->m_arcs)
    GetReebArc(a).m_edges.erase(_e);
}
