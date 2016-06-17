#include "EnvModel.h"

#include <fstream>

#include "Environment/BoundingBox.h"
#include "Environment/BoundingSphere.h"
#include "Environment/ActiveMultiBody.h"
#include "Environment/StaticMultiBody.h"
#include "Environment/SurfaceMultiBody.h"

#include "ActiveMultiBodyModel.h"
#include "AvatarModel.h"
#include "BodyModel.h"
#include "BoundingBoxModel.h"
#include "BoundingSphereModel.h"
#include "CfgModel.h"
#include "ReebGraphModel.h"
#include "RegionBoxModel.h"
#include "RegionBox2DModel.h"
#include "RegionSphereModel.h"
#include "RegionSphere2DModel.h"
#include "StaticMultiBodyModel.h"
#include "SurfaceMultiBodyModel.h"
#include "TempObjsModel.h"
#include "TetGenDecompositionModel.h"
#include "UserPathModel.h"
#include "Utilities/VizmoExceptions.h"

EnvModel::
EnvModel(const string& _filename) : Model("Environment"),
  m_radius(0), m_boundary(nullptr),
  m_tetgenModel(nullptr), m_reebGraphModel(nullptr) {

    m_environment = new Environment();
    m_environment->Read(_filename);

    Build();

    //create avatar
    m_avatar = new AvatarModel(AvatarModel::None);
  }

EnvModel::
EnvModel(Environment* _env) : Model("Environment"),
  m_radius(0), m_boundary(nullptr),
  m_tetgenModel(nullptr), m_reebGraphModel(nullptr),
  m_environment(_env) {

    Build();

    //create avatar
    m_avatar = new AvatarModel(AvatarModel::None);
  }

EnvModel::
~EnvModel() {
  delete m_avatar;
  for(auto& p : m_userPaths)
    delete p;
  delete m_tetgenModel;
  delete m_reebGraphModel;
}

const Point3d&
EnvModel::
GetCenter() const {
  return m_boundary->GetCenter();
}

bool
EnvModel::
IsPlanar() const {
  for(auto& r : m_robots)
    if(!r->IsPlanar())
      return false;
  return true;
}

void
EnvModel::
PlaceRobots(vector<CfgModel>& _cfgs, bool _invisible) {
  for(auto& r : m_robots) {
    vector<double> cfg(r->Dofs(), 0);
    r->SetInitialCfg(cfg);
    if(_invisible)
      r->SetRenderMode(INVISIBLE_MODE);
    r->BackUp();
    m_avatar->SetCfg(cfg);
  }
  for(const auto& cfg : _cfgs) {
    m_robots[cfg.GetRobotIndex()]->SetInitialCfg(cfg.GetData());
    m_avatar->SetCfg(cfg.GetData());
  }
}

void
EnvModel::
ConfigureRender(const CfgModel& _c) {
  m_robots[_c.GetRobotIndex()]->ConfigureRender(_c.GetData());
}

shared_ptr<StaticMultiBodyModel>
EnvModel::
AddObstacle(const string& _dir, const string& _filename,
    const Transformation& _t) {
  m_centerOfMass *= m_obstacles.size() + m_surfaces.size();

  shared_ptr<StaticMultiBody> o =
    m_environment->AddObstacle(_dir, _filename, _t).second;
  m_obstacles.emplace_back(new StaticMultiBodyModel(o));
  m_obstacles.back()->Build();

  m_centerOfMass += m_obstacles.back()->GetCOM();
  m_centerOfMass /= m_obstacles.size() + m_surfaces.size();

  double dist = (m_obstacles.back()->GetCOM() - m_centerOfMass).norm() +
    m_obstacles.back()->GetRadius();
  if(dist > m_radius)
    m_radius = dist;

  return m_obstacles.back();
}

void
EnvModel::
DeleteObstacle(StaticMultiBodyModel* _m) {
  for(auto it = m_obstacles.begin(); it != m_obstacles.end(); ++it) {
    if(_m == it->get()) {
      m_environment->RemoveObstacle((*it)->GetStaticMultiBody());
      m_obstacles.erase(it);
      break;
    }
  }
}

void
EnvModel::
SetBoundary(shared_ptr<BoundaryModel> _b) {
  m_boundary = _b;
  m_environment->SetBoundary(m_boundary->GetBoundary());
}

bool
EnvModel::
IsNonCommitRegion(RegionModelPtr _r) const {
  return find(m_nonCommitRegions.begin(), m_nonCommitRegions.end(), _r)
    != m_nonCommitRegions.end();
}

void
EnvModel::
AddAttractRegion(RegionModelPtr _r) {
  QMutexLocker lock(&m_regionLock);
  _r->SetColor(Color4(0, 1, 0, 0.5));
  m_attractRegions.push_back(_r);
  VDAddRegion(_r.get());
}

void
EnvModel::
AddAvoidRegion(RegionModelPtr _r) {
  QMutexLocker lock(&m_regionLock);
  _r->SetColor(Color4(0, 0, 0, 0.5));
  m_avoidRegions.push_back(_r);
  VDAddRegion(_r.get());
}

void
EnvModel::
AddNonCommitRegion(RegionModelPtr _r) {
  QMutexLocker lock(&m_regionLock);
  _r->SetColor(Color4(0, 0, 1, 0.8));
  m_nonCommitRegions.push_back(_r);
}

void
EnvModel::
ChangeRegionType(RegionModelPtr _r, bool _attract) {
  QMutexLocker lock(&m_regionLock);
  vector<RegionModelPtr>::iterator rit;
  rit = find(m_nonCommitRegions.begin(), m_nonCommitRegions.end(), _r);
  if(rit != m_nonCommitRegions.end()) {
    m_nonCommitRegions.erase(rit);
    if(_attract)
      AddAttractRegion(_r);
    else
      AddAvoidRegion(_r);
  }
}

void
EnvModel::
DeleteRegion(RegionModelPtr _r) {
  QMutexLocker lock(&m_regionLock);
  VDRemoveRegion(_r.get());

  vector<RegionModelPtr>::iterator rit;
  rit = find(m_attractRegions.begin(), m_attractRegions.end(), _r);
  if(rit != m_attractRegions.end()) {
    m_attractRegions.erase(rit);
  }
  else {
    rit = find(m_avoidRegions.begin(), m_avoidRegions.end(), _r);
    if(rit != m_avoidRegions.end()) {
      m_avoidRegions.erase(rit);
    }
    else {
      rit = find(m_nonCommitRegions.begin(), m_nonCommitRegions.end(), _r);
      if(rit != m_nonCommitRegions.end()) {
        m_nonCommitRegions.erase(rit);
      }
    }
  }
}

EnvModel::RegionModelPtr
EnvModel::
GetRegion(Model* _model) {
  if(_model->Name().find("Region") != string::npos) {
    RegionModel* rm = (RegionModel*)_model;

    vector<RegionModelPtr>::iterator rit;
    for(auto& r : m_attractRegions)
      if(r.get() == rm)
        return r;
    for(auto& r : m_avoidRegions)
      if(r.get() == rm)
        return r;
    for(auto& r : m_nonCommitRegions)
      if(r.get() == rm)
        return r;
  }
  return RegionModelPtr();
}

void
EnvModel::
SaveRegions(const string& _filename) {
  ofstream ofs(_filename);
  ofs << "#####RegionFile#####" << endl;

  typedef vector<RegionModelPtr>::const_iterator CRIT;
  for(auto& r : m_nonCommitRegions)
    r->OutputDebugInfo(ofs);
  for(auto& r : m_attractRegions)
    r->OutputDebugInfo(ofs);
  for(auto& r : m_avoidRegions)
    r->OutputDebugInfo(ofs);
}

void
EnvModel::
LoadRegions(const string& _filename) {
  ifstream ifs(_filename);

  m_nonCommitRegions.clear();
  m_attractRegions.clear();
  m_avoidRegions.clear();

  string line;
  getline(ifs,line);

  while(getline(ifs,line)) {

    istringstream iss(line);

    RegionModelPtr _mod;

    int tempType;
    iss >> tempType;

    string modelShapeName;
    iss >> modelShapeName;

    RegionModel::Type modelType = static_cast<RegionModel::Type>(tempType);

    if(modelShapeName == "BOX") {

      Point3d min, max;
      iss >> min >> max;

      pair<double, double> xPair(min[0], max[0]);
      pair<double, double> yPair(min[1], max[1]);
      pair<double, double> zPair(min[2], max[2]);

      _mod = RegionModelPtr(new RegionBoxModel(xPair, yPair, zPair));
      _mod->SetType(modelType);
      _mod->ChangeColor();
    }
    else if(modelShapeName == "BOX2D") {

      Point2d min, max;
      iss >> min >> max;

      pair<double, double> xPair(min[0], max[0]);
      pair<double, double> yPair(min[1], max[1]);

      _mod = RegionModelPtr(new RegionBox2DModel(xPair, yPair));
      _mod->SetType(modelType);
      _mod->ChangeColor();

    }
    else if(modelShapeName == "SPHERE") {

      Point3d tempCenter;
      iss >> tempCenter;

      double radius = -1;
      iss >> radius;

      _mod = RegionModelPtr(new RegionSphereModel(tempCenter, radius));
      _mod->SetType(modelType);
      _mod->ChangeColor();
    }
    else if(modelShapeName == "SPHERE2D") {

      Point3d tempCenter;
      iss >> tempCenter;

      double radius = -1;
      iss >> radius;

      _mod = RegionModelPtr(new RegionSphere2DModel(tempCenter, radius));
      _mod->SetType(modelType);
      _mod->ChangeColor();
    }

    if(_mod != NULL) {
      //Checks what type of region then adds
      if(_mod->GetType() == 0)
        AddAttractRegion(_mod);
      else if(_mod->GetType() == 1)
        AddAvoidRegion(_mod);
      else if(_mod->GetType() == 2)
        AddNonCommitRegion(_mod);
    }
  }
}

void
EnvModel::
DeleteUserPath(UserPathModel* _p) {
  vector<UserPathModel*>::iterator pit;
  pit = find(m_userPaths.begin(), m_userPaths.end(), _p);
  if(pit != m_userPaths.end()) {
    delete *pit;
    m_userPaths.erase(pit);
  }
}

void
EnvModel::
SaveUserPaths(const string& _filename) {
  ofstream ofs(_filename);
  ofs << "#####PathsFile#####" << endl << endl;

  ofs << "NumPaths " << m_userPaths.size() << endl << endl;

  for(const auto& path : m_userPaths)
    ofs << *path << endl ;
}

void
EnvModel::
LoadUserPaths(const string& _filename) {
  ifstream ifs(_filename);

  //read num paths
  string temp;
  size_t number;
  ifs >> temp >> temp >> number;

  //read paths
  m_userPaths.clear();
  m_userPaths.resize(number);
  for(auto& path : m_userPaths) {
    path = new UserPathModel;
    ifs >> *path;
  }
  GetVizmo().GetEnv()->GetAvatar()->Disable();
}

void
EnvModel::
RemoveTempObjs(TempObjsModel* _t) {
  //remove TempObjs container from EnvModel
  for(vector<TempObjsModel*>::iterator tit = m_tempObjs.begin();
      tit != m_tempObjs.end(); ++tit) {
    if(*tit == _t) {
      m_tempObjs.erase(tit);
      break;
    }
  }
}

void
EnvModel::
AddTetGenDecompositionModel(TetGenDecomposition* _tetgen) {
  m_tetgenModel = new TetGenDecompositionModel(_tetgen);
}

void
EnvModel::
AddReebGraphModel(ReebGraphConstruction* _reebGraph) {
  m_reebGraphModel = new ReebGraphModel(_reebGraph);
}

void
EnvModel::
Build() {
  //construct boundary
  if(shared_ptr<BoundingBox> b = dynamic_pointer_cast<BoundingBox>(m_environment->GetBoundary()))
    m_boundary = shared_ptr<BoundingBoxModel>(new BoundingBoxModel(b));
  else if(shared_ptr<BoundingSphere> b = dynamic_pointer_cast<BoundingSphere>(m_environment->GetBoundary()))
    m_boundary = shared_ptr<BoundingSphereModel>(new BoundingSphereModel(b));
  else
    throw RunTimeException(WHERE, "Failed casting Boundary.");

  //construct multibody model
  for(size_t i = 0; i < m_environment->NumRobots(); ++i)
    m_robots.emplace_back(
        new ActiveMultiBodyModel(m_environment->GetRobot(i)));
  for(size_t i = 0; i < m_environment->NumObstacles(); ++i)
    m_obstacles.emplace_back(
        new StaticMultiBodyModel(m_environment->GetObstacle(i)));
  for(size_t i = 0; i < m_environment->NumSurfaces(); ++i)
    m_surfaces.emplace_back(
        new SurfaceMultiBodyModel(m_environment->GetSurface(i)));

  //Build boundary model
  if(!m_boundary)
    throw BuildException(WHERE, "Boundary is NULL");
  m_boundary->Build();

  //Build each
  //MultiBodyModel::ClearDOFInfo();
  for(auto& r : m_robots)
    r->Build();
  for(auto& o : m_obstacles) {
    o->Build();
    m_centerOfMass += o->GetCOM();
  }
  for(auto& s : m_surfaces) {
    s->Build();
    m_centerOfMass += s->GetCOM();
  }

  m_centerOfMass /= m_obstacles.size() + m_surfaces.size();

  //Compute radius
  for(auto& o : m_obstacles) {
    double dist = (o->GetCOM() - m_centerOfMass).norm() + o->GetRadius();
    if(dist > m_radius)
      m_radius = dist;
  }
  for(auto& s : m_surfaces) {
    double dist = (s->GetCOM() - m_centerOfMass).norm() + s->GetRadius();
    if(dist > m_radius)
      m_radius = dist;
  }
  double dist = m_boundary->GetMaxDist() / 2;
  if(dist > m_radius)
    m_radius = dist;
}


void
EnvModel::
Select(GLuint* _index, vector<Model*>& _sel) {
  if(!_index)
    return; //input error

  GLuint indx = *_index;

  switch(indx) {
    case EnvObjectName::Boundary:
      m_boundary->Select(_index + 1, _sel);
      break;
    case EnvObjectName::Robots:
      m_robots[*(_index + 1)]->Select(_index + 2, _sel);
      break;
    case EnvObjectName::Obstacles:
      m_obstacles[*(_index + 1)]->Select(_index + 2, _sel);
      break;
    case EnvObjectName::Surfaces:
      m_surfaces[*(_index + 1)]->Select(_index + 2, _sel);
      break;
    case EnvObjectName::AttractRegions:
      m_attractRegions[*(_index + 1)]->Select(_index + 2, _sel);
      break;
    case EnvObjectName::AvoidRegions:
      m_avoidRegions[*(_index + 1)]->Select(_index + 2, _sel);
      break;
    case EnvObjectName::NonCommitRegions:
      m_nonCommitRegions[*(_index + 1)]->Select(_index + 2, _sel);
      break;
    case EnvObjectName::UserPaths:
      m_userPaths[*(_index + 1)]->Select(_index + 2, _sel);
      break;
    case EnvObjectName::TetGen:
      m_tetgenModel->Select(_index + 1, _sel);
      break;
    case EnvObjectName::ReebGraph:
      m_reebGraphModel->Select(_index + 1, _sel);
      break;
    default:
      break;
  }
}


void
EnvModel::
DrawRender() {
  m_avatar->DrawRender();

  m_boundary->DrawRender();

  glLineWidth(1);
  for(auto& r : m_robots) {
    r->Restore();
    r->DrawRender();
  }
  for(auto& o : m_obstacles)
    o->DrawRender();
  for(auto& s : m_surfaces)
    s->DrawRender();

  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  {
    QMutexLocker lock(&m_regionLock);
    for(auto& r : m_attractRegions)
      r->DrawRender();
    for(auto& r : m_avoidRegions)
      r->DrawRender();
    for(auto& r : m_nonCommitRegions)
      r->DrawRender();
  }
  for(auto& p : m_userPaths)
    p->DrawRender();
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);

  for(auto& t : m_tempObjs)
    t->DrawRender();

  if(m_tetgenModel)
    m_tetgenModel->DrawRender();
  if(m_reebGraphModel)
    m_reebGraphModel->DrawRender();
}


void
EnvModel::
DrawSelect() {
  glLineWidth(1);

  size_t nameIndx = 0;

  glPushName(EnvObjectName::Robots);
  for(auto& r : m_robots) {
    glPushName(nameIndx++);
    r->Restore();
    r->DrawSelect();
    glPopName();
  }
  glPopName();

  nameIndx = 0;
  glPushName(EnvObjectName::Obstacles);
  for(auto& o : m_obstacles) {
    glPushName(nameIndx++);
    o->DrawSelect();
    glPopName();
  }
  glPopName();

  nameIndx = 0;
  glPushName(EnvObjectName::Surfaces);
  for(auto& s : m_surfaces) {
    glPushName(nameIndx++);
    s->DrawSelect();
    glPopName();
  }
  glPopName();

  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  {
    QMutexLocker lock(&m_regionLock);

    nameIndx = 0;
    glPushName(EnvObjectName::AttractRegions);
    for(auto& r : m_attractRegions) {
      glPushName(nameIndx++);
      r->DrawSelect();
      glPopName();
    }
    glPopName();

    nameIndx = 0;
    glPushName(EnvObjectName::AvoidRegions);
    for(auto& r : m_avoidRegions) {
      glPushName(nameIndx++);
      r->DrawSelect();
      glPopName();
    }
    glPopName();

    nameIndx = 0;
    glPushName(EnvObjectName::NonCommitRegions);
    for(auto& r : m_nonCommitRegions) {
      glPushName(nameIndx++);
      r->DrawSelect();
      glPopName();
    }
    glPopName();
  }

  nameIndx = 0;
  glPushName(EnvObjectName::UserPaths);
  for(auto& p : m_userPaths) {
    glPushName(nameIndx++);
    p->DrawSelect();
    glPopName();
  }
  glPopName();
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);

  glPushName(EnvObjectName::TetGen);
  if(m_tetgenModel)
    m_tetgenModel->DrawSelect();
  glPopName();

  glPushName(EnvObjectName::ReebGraph);
  if(m_reebGraphModel)
    m_reebGraphModel->DrawSelect();
  glPopName();

  glPushName(EnvObjectName::Boundary);
  m_boundary->DrawSelect();
  glPopName();
}


void
EnvModel::
Print(ostream& _os) const {
  _os << Name() << endl
    << "\t" << m_environment->GetEnvFileName() << endl
    << "\t" << m_robots.size() << " robots" << endl;
  if(!m_obstacles.empty())
    _os << "\t" << m_obstacles.size() << " obstacles" << endl;
  if(!m_surfaces.empty())
    _os << "\t" << m_surfaces.size() << " surfaces" << endl;
}


void
EnvModel::
ChangeColor() {
  for(const auto& r : m_robots)
    r->SetColor(Color4(DRand(), DRand(), DRand(), 1));
  for(const auto& o : m_obstacles)
    o->SetColor(Color4(DRand(), DRand(), DRand(), 1));
  for(const auto& s : m_surfaces)
    s->SetColor(Color4(DRand(), DRand(), DRand(), 1));
}


void
EnvModel::
SetSelectable(bool _s) {
  m_selectable = _s;
  for(const auto& r : m_robots)
    r->SetSelectable(_s);
  for(const auto& o : m_obstacles)
    o->SetSelectable(_s);
  for(const auto& s : m_surfaces)
    s->SetSelectable(_s);
  if(m_tetgenModel)
    m_tetgenModel->SetSelectable(_s);
  if(m_reebGraphModel)
    m_reebGraphModel->SetSelectable(_s);
  m_boundary->SetSelectable(_s);
}


void
EnvModel::
GetChildren(list<Model*>& _models) {
  for(const auto& r : m_robots)
    _models.push_back(r.get());
  for(const auto& o : m_obstacles)
    _models.push_back(o.get());
  for(const auto& s : m_surfaces)
    _models.push_back(s.get());
  {
    QMutexLocker lock(&m_regionLock);
    for(const auto& r : m_attractRegions)
      _models.push_back(r.get());
    for(const auto& r : m_avoidRegions)
      _models.push_back(r.get());
    for(const auto& r : m_nonCommitRegions)
      _models.push_back(r.get());
  }
  typedef vector<UserPathModel*>::iterator PIT;
  for(const auto& p : m_userPaths)
    _models.push_back(p);
  _models.push_back(m_boundary.get());
  if(m_tetgenModel)
    _models.push_back(m_tetgenModel);
  if(m_reebGraphModel)
    _models.push_back(m_reebGraphModel);
}

void
EnvModel::
SaveFile(const string& _filename) const {
  ofstream ofs(_filename);
  m_environment->Write(ofs);
}

