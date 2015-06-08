#include "EnvModel.h"

#include <fstream>

#include "AvatarModel.h"
#include "BodyModel.h"
#include "BoundingBoxModel.h"
#include "BoundingSphereModel.h"
#include "CfgModel.h"
#include "ConnectionModel.h"
#include "TempObjsModel.h"
#include "UserPathModel.h"
#include "Utilities/VizmoExceptions.h"

EnvModel::
EnvModel(const string& _filename) : LoadableModel("Environment"),
    m_containsSurfaces(false), m_radius(0), m_boundary(NULL), m_tempObjs() {
  SetFilename(_filename);
  size_t sl = _filename.rfind('/');
  SetModelDataDir(_filename.substr(0, sl == string::npos ? 0 : sl));

  ParseFile();
  Build();

  m_environment = new Environment();
  m_environment->Read(_filename);
  m_environment->ComputeResolution();

  //create avatar
  m_avatar = new AvatarModel(AvatarModel::None);
}


EnvModel::
~EnvModel() {
  delete m_boundary;
  delete m_avatar;
  m_avatar = NULL;
  typedef vector<MultiBodyModel*>::const_iterator MIT;
  for(MIT mit = m_multibodies.begin(); mit!=m_multibodies.end(); ++mit)
    delete *mit;
  typedef vector<UserPathModel*>::iterator PIT;
  for(PIT pit = m_userPaths.begin(); pit != m_userPaths.end(); ++pit)
    delete *pit;
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
  _r->SetColor(Color4(0, 1, 0, 0.5));
  m_attractRegions.push_back(_r);
  VDAddRegion(_r.get());
}


void
EnvModel::
AddAvoidRegion(RegionModelPtr _r) {
  _r->SetColor(Color4(0, 0, 0, 0.5));
  m_avoidRegions.push_back(_r);
  VDAddRegion(_r.get());
}


void
EnvModel::
AddNonCommitRegion(RegionModelPtr _r) {
  _r->SetColor(Color4(0, 0, 1, 0.8));
  m_nonCommitRegions.push_back(_r);
}


void
EnvModel::
ChangeRegionType(RegionModelPtr _r, bool _attract) {
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
    for(rit = m_attractRegions.begin(); rit != m_attractRegions.end(); ++rit)
      if(rit->get() == rm)
        return *rit;
    for(rit = m_avoidRegions.begin(); rit != m_avoidRegions.end(); ++rit)
      if(rit->get() == rm)
        return *rit;
    for(rit = m_nonCommitRegions.begin(); rit != m_nonCommitRegions.end(); ++rit)
      if(rit->get() == rm)
        return *rit;
  }
  return RegionModelPtr();
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
ParseFile() {

  if(!FileExists(GetFilename()))
    throw ParseException(WHERE, "'" + GetFilename() + "' does not exist");

  //Open file for reading data
  ifstream ifs(GetFilename().c_str());

  //Read boundary
  string b = ReadFieldString(ifs, WHERE,
      "Failed reading boundary tag.");

  if(b != "BOUNDARY")
    throw ParseException(WHERE,
        "Failed reading boundary tag ' " + b + " '.");

  ParseBoundary(ifs);

  //Read number of multibodies
  string mb = ReadFieldString(ifs, WHERE,
      "Failed reading Multibodies tag");

  if(mb != "MULTIBODIES")
    throw ParseException(WHERE,
        "Failed reading multibodies tag. Read " + mb + ".");

  size_t numMultiBodies = ReadField<size_t>(ifs, WHERE,
      "Failed reading number of Multibodies.");

  for(size_t i = 0; i < numMultiBodies && ifs; i++) {
    MultiBodyModel* m = new MultiBodyModel(this);
    m->ParseMultiBody(ifs, m_modelDataDir);
    m_multibodies.push_back(m);
  }
}


void
EnvModel::
SetModelDataDir(const string _modelDataDir) {
  m_modelDataDir = _modelDataDir;
  cout << "- Geo Dir   : " << m_modelDataDir << endl;
}


void
EnvModel::
ParseBoundary(ifstream& _ifs) {
  string type = ReadFieldString(_ifs, WHERE, "Failed reading Boundary type.");

  if(type == "BOX")
    m_boundary = new BoundingBoxModel();
  else if(type == "SPHERE")
    m_boundary = new BoundingSphereModel();
  else
    throw ParseException(WHERE, "Failed reading boundary type '" + type +
        "'. Choices are BOX or SPHERE.");

  m_boundary->Parse(_ifs);
}


void
EnvModel::
Build() {
  //Build boundary model
  if(!m_boundary)
    throw BuildException(WHERE, "Boundary is NULL");
  m_boundary->Build();

  //Build each
  MultiBodyModel::ClearDOFInfo();
  for(const auto& mb : m_multibodies) {
    mb->Build();
    m_dof += mb->GetDOF();
    m_centerOfMass += mb->GetCOM();
  }

  m_centerOfMass /= m_multibodies.size();

  //Compute radius
  for(const auto& mb : m_multibodies) {
    double dist = (mb->GetCOM() - m_centerOfMass).norm() + mb->GetRadius();
    if(m_radius < dist)
      m_radius = dist;
  }
}


void
EnvModel::
Select(GLuint* _index, vector<Model*>& _sel) {
  size_t numMBs = m_multibodies.size();
  size_t numAttractRegions = m_attractRegions.size();
  size_t numAvoidRegions = m_avoidRegions.size();
  size_t numNonCommitRegions = m_nonCommitRegions.size();
  size_t numPaths = m_userPaths.size();

  //unselect old one
  if(!_index || *_index > numMBs + numAttractRegions + numAvoidRegions
      + numNonCommitRegions + numPaths) //input error
    return;
  else if(*_index == numMBs + numAttractRegions + numAvoidRegions
      + numNonCommitRegions + numPaths)
    m_boundary->Select(_index+1, _sel);
  else if(*_index < numMBs)
    m_multibodies[*_index]->Select(_index+1, _sel);
  else if(*_index < numMBs + numAttractRegions)
    m_attractRegions[*_index - numMBs]->Select(_index+1, _sel);
  else if(*_index < numMBs + numAttractRegions + numAvoidRegions)
    m_avoidRegions[*_index - numMBs - numAttractRegions]->Select(_index+1, _sel);
  else if(*_index < numMBs + numAttractRegions + numAvoidRegions
      + numNonCommitRegions)
    m_nonCommitRegions[*_index - numMBs - numAttractRegions - numAvoidRegions]->
        Select(_index+1, _sel);
  else
    m_userPaths[*_index - numMBs - numAttractRegions - numAvoidRegions
        - numNonCommitRegions]->Select(_index+1, _sel);
}


void
EnvModel::
DrawRender() {
  m_avatar->DrawRender();

  size_t numMBs = m_multibodies.size();
  size_t numAttractRegions = m_attractRegions.size();
  size_t numAvoidRegions = m_avoidRegions.size();
  size_t numNonCommitRegions = m_nonCommitRegions.size();
  size_t numPaths = m_userPaths.size();

  m_boundary->DrawRender();

  glLineWidth(1);
  for(size_t i = 0; i < numMBs; ++i)
    if(!m_multibodies[i]->IsActive())
      m_multibodies[i]->DrawRender();

  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  for(size_t i = 0; i < numAttractRegions; ++i)
    m_attractRegions[i]->DrawRender();
  for(size_t i = 0; i < numAvoidRegions; ++i)
    m_avoidRegions[i]->DrawRender();
  for(size_t i = 0; i < numNonCommitRegions; ++i)
    m_nonCommitRegions[i]->DrawRender();
  for(size_t i = 0; i < numPaths; ++i)
    m_userPaths[i]->DrawRender();
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);

  for(vector<TempObjsModel*>::iterator tit = m_tempObjs.begin();
      tit != m_tempObjs.end(); ++tit) {
    (*tit)->DrawRender();
  }
}


void
EnvModel::
DrawSelect() {
  size_t numMBs = m_multibodies.size();
  size_t numAttractRegions = m_attractRegions.size();
  size_t numAvoidRegions = m_avoidRegions.size();
  size_t numNonCommitRegions = m_nonCommitRegions.size();
  size_t numPaths = m_userPaths.size();

  glPushName(numMBs + numAttractRegions + numAvoidRegions + numNonCommitRegions
      + numPaths);
  m_boundary->DrawSelect();
  glPopName();

  glLineWidth(1);
  for(size_t i = 0; i < numMBs; ++i){
    if(!m_multibodies[i]->IsActive()){
      glPushName(i);
      m_multibodies[i]->DrawSelect();
      glPopName();
    }
  }

  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  for(size_t i = 0; i < numAttractRegions; ++i) {
    glPushName(numMBs + i);
    m_attractRegions[i]->DrawSelect();
    glPopName();
  }
  for(size_t i = 0; i < numAvoidRegions; ++i) {
    glPushName(numMBs + numAttractRegions + i);
    m_avoidRegions[i]->DrawSelect();
    glPopName();
  }
  for(size_t i = 0; i < numNonCommitRegions; ++i) {
    glPushName(numMBs + numAttractRegions + numAvoidRegions + i);
    m_nonCommitRegions[i]->DrawSelect();
    glPopName();
  }
  for(size_t i = 0; i < numPaths; ++i) {
    glPushName(numMBs + numAttractRegions + numAvoidRegions + numNonCommitRegions
        + i);
    m_userPaths[i]->DrawSelect();
    glPopName();
  }
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);

  for(vector<TempObjsModel*>::iterator tit = m_tempObjs.begin();
      tit != m_tempObjs.end(); ++tit) {
    (*tit)->DrawSelect();
  }
}


void
EnvModel::
Print(ostream& _os) const {
  _os << Name() << ": " << GetFilename() << endl
      << m_multibodies.size() << " multibodies" << endl;
}


void
EnvModel::
ChangeColor() {
  int numMBs = m_multibodies.size();
  for(int i = 0; i < numMBs; i++)
    m_multibodies[i]->SetColor(Color4(drand48(), drand48(), drand48(), 1));
}


void
EnvModel::
SetSelectable(bool _s) {
  m_selectable = _s;
  typedef vector<MultiBodyModel*>::iterator MIT;
  for(MIT i = m_multibodies.begin(); i != m_multibodies.end(); ++i)
    (*i)->SetSelectable(_s);
  m_boundary->SetSelectable(_s);
}


void
EnvModel::
GetChildren(list<Model*>& _models) {
  typedef vector<MultiBodyModel*>::iterator MIT;
  for(MIT i = m_multibodies.begin(); i != m_multibodies.end(); ++i)
    if(!(*i)->IsActive())
      _models.push_back(*i);
  typedef vector<RegionModelPtr>::iterator RIT;
  for(RIT i = m_attractRegions.begin(); i != m_attractRegions.end(); ++i)
    _models.push_back(i->get());
  for(RIT i = m_avoidRegions.begin(); i != m_avoidRegions.end(); ++i)
    _models.push_back(i->get());
  for(RIT i = m_nonCommitRegions.begin(); i != m_nonCommitRegions.end(); ++i)
    _models.push_back(i->get());
  typedef vector<UserPathModel*>::iterator PIT;
  for(PIT i = m_userPaths.begin(); i != m_userPaths.end(); ++i)
    _models.push_back(*i);
  _models.push_back(m_boundary);
}


void
EnvModel::
DeleteMBModel(MultiBodyModel* _mbl) {
  vector<MultiBodyModel*>::iterator mbit;
  for(mbit = m_multibodies.begin(); mbit != m_multibodies.end(); mbit++){
    if((*mbit) == _mbl){
      m_multibodies.erase(mbit);
      break;
    }
  }
}


void
EnvModel::
AddMBModel(MultiBodyModel* _m) {
  _m->Build();
  m_multibodies.push_back(_m);
}


bool
EnvModel::
SaveFile(const char* _filename) {
  /// \warning This function is still experimental and has not yet been validated.
  ofstream envFile(_filename);
  if(!envFile.is_open()){
    cout << "Couldn't open the file" << endl;
    return false;
  }
  envFile << "Boundary " << *m_boundary;
  envFile << "\n\n";
  int numMBs = m_multibodies.size();
  envFile << "Multibodies\n" << numMBs << "\n\n";
  vector<MultiBodyModel*> saveMB = GetMultiBodies();
  reverse(saveMB.begin(), saveMB.end());
  while(!saveMB.empty()){
    if(saveMB.back()->IsActive())
      envFile << "Active\n";
    else if(saveMB.back()->IsSurface())
      envFile << "Surface\n";
    else
      envFile << "Passive\n";
    int nB = saveMB.back()->GetNbBodies();
    if(nB!= 0){
      if(saveMB.back()->IsActive())
        envFile << nB << endl;
      vector<BodyModel*> bodies = saveMB.back()->GetBodies();
      reverse(bodies.begin(),bodies.end());
      envFile << "#VIZMO_COLOR" << " " << bodies.back()->GetColor()[0]
              << " " << bodies.back()->GetColor()[1]
              << " " << bodies.back()->GetColor()[2] << endl;
      if(saveMB.back()->IsActive()){
        int nbJoints = 0;
        vector<ConnectionModel*> joints = saveMB.back()->GetJoints();
        reverse(joints.begin(),joints.end());
        while(!bodies.empty()){
          envFile << bodies.back()->GetFilename() << " ";
          if(bodies.back()->IsBaseVolumetric() || bodies.back()->IsBasePlanar()) {
            string baseMovement = "Translational";
            if (bodies.back()->IsBaseRotational())
              baseMovement = "Rotational";
            if(bodies.back()->IsBaseVolumetric())
              envFile << "Volumetric " << baseMovement << endl;
            else
              envFile << "Planar " << baseMovement << endl;
          }
          else if(bodies.back()->IsBaseFixed()){
            envFile << "Fixed ";
            ostringstream transform;
            transform << bodies.back()->GetTransform();
            envFile << SetTransformRight(transform.str()) << endl;
          }
          else{
            envFile << "Joint" << endl;
            nbJoints++;
          }
          bodies.pop_back();
        }
        envFile << "Connections" << endl << nbJoints << endl;
        if(nbJoints!=0){
          while(!joints.empty()){
            pair<double, double> jointLimits[2] = joints.back()->GetJointLimits();
            ostringstream limits;
            string jointType = "NonActuated ";
            if(joints.back()->IsSpherical()){
              jointType = "Spherical ";
              limits << jointLimits[0].first << ":" << jointLimits[0].second
                     << " " << jointLimits[1].first << ":"
                     << jointLimits[1].second;
            }
            else if(joints.back()->IsRevolute()){
              jointType = "Revolute ";
              limits << jointLimits[0].first << ":" << jointLimits[0].second
                     << " ";
            }
            envFile << joints.back()->GetPreviousIndex() << " "
                    << joints.back()->GetNextIndex() << "  "
                    << jointType << limits.str() << endl;
            ostringstream transform;
            transform << joints.back()->TransformToDHFrame();
            envFile << SetTransformRight(transform.str()) << "   ";
            envFile << joints.back()->GetAlpha() << " "
                    << joints.back()->GetA() << " " << joints.back()->GetD()
                    << " " << joints.back()->GetTheta() << "   ";
            ostringstream transform2;
            transform2 << joints.back()->TransformToBody2();
            envFile << SetTransformRight(transform2.str()) << endl;
            joints.pop_back();
          }
        }
      }
      else{
        envFile << bodies.back()->GetFilename() << "  ";
        ostringstream transform;
        transform << bodies.back()->GetTransform();
        envFile << SetTransformRight(transform.str()) << endl;
      }
    }
    envFile << endl;
    saveMB.pop_back();
  }
  envFile.close();
  return true;
}


string
EnvModel::
SetTransformRight(string _transformString) {
  /// \todo This function appears to take a string of 6 numerals and swaps the
  ///       4th and 6th elements. It's not clear why this is needed instead of
  ///       just making the strings correctly in the first place. I think this
  ///       can be improved/removed with a little extra thought.
  stringstream transform;
  istringstream splitTransform(_transformString);
  string splittedTransform[6] = {"","","","","",""};
  int j = 0;
  do {
    splitTransform >> splittedTransform[j];
    j++;
  } while(splitTransform);
  string temp = splittedTransform[3];
  splittedTransform[3] = splittedTransform[5];
  splittedTransform[5] = temp;
  for(int i = 0; i < 6; ++i)
    transform << splittedTransform[i] << " ";
  return transform.str();
}
