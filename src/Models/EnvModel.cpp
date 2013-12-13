#include "EnvModel.h"

#include <fstream>

#include "BodyModel.h"
#include "BoundingBoxModel.h"
#include "BoundingSphereModel.h"
#include "CfgModel.h"
#include "ConnectionModel.h"
#include "Utilities/VizmoExceptions.h"
#include "Utilities/IO.h"

EnvModel::EnvModel(const string& _filename) : LoadableModel("Environment"),
  m_containsSurfaces(false), m_radius(0), m_boundary(NULL) {

    SetFilename(_filename);
    size_t sl = _filename.rfind('/');
    SetModelDataDir(_filename.substr(0, sl == string::npos ? 0 : sl));

    ParseFile();
    BuildModels();

    m_environment = new Environment();
    m_environment->Read(_filename);
    m_environment->ComputeResolution();
  }

EnvModel::~EnvModel() {
  delete m_boundary;
  typedef vector<MultiBodyModel*>::const_iterator MIT;
  for(MIT mit = m_multibodies.begin(); mit!=m_multibodies.end(); ++mit)
    delete *mit;
}

//////////Load Functions//////////
void
EnvModel::ParseFile(){

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
    MultiBodyModel* m = new MultiBodyModel();
    m->SetEnv(this);
    m->ParseMultiBody(ifs, m_modelDataDir);
    m_multibodies.push_back(m);
  }
}

void
EnvModel::SetModelDataDir(const string _modelDataDir){
  m_modelDataDir = _modelDataDir;
  cout<<"- Geo Dir   : "<< m_modelDataDir << endl;
}

void
EnvModel::ParseBoundary(ifstream& _ifs) {
  string type = ReadFieldString(_ifs, WHERE, "Failed reading Boundary type.");

  if(type == "BOX"){
    m_boundary = new BoundingBoxModel();
    m_boundaryType="BOX";
  }
  else if(type == "SPHERE"){
    m_boundary = new BoundingSphereModel();
    m_boundaryType="SPHERE";
  }
  else
    throw ParseException(WHERE,
      "Failed reading boundary type '" + type + "'. Choices are BOX or SPHERE.");

  m_boundary->Parse(_ifs);
}

void
EnvModel::ChangeBoundary(string _type, istream& _coord){
  if(_type == "SPHERE"){
    m_boundary = new BoundingSphereModel();
    m_boundaryType="SPHERE";
  }
  else if(_type == "BOX"){
    m_boundary = new BoundingBoxModel();
    m_boundaryType="BOX";
  }
  m_boundary->Parse(_coord);

  if(!m_boundary)
    throw BuildException(WHERE, "Boundary is NULL");
  m_boundary->BuildModels();
}

void
EnvModel::BuildModels(){
  //Build boundary model
  if(!m_boundary)
    throw BuildException(WHERE, "Boundary is NULL");
  m_boundary->BuildModels();

  //Build each
  MultiBodyModel::ClearDOFInfo();
  typedef vector<MultiBodyModel*>::const_iterator MIT;
  for(MIT mit = m_multibodies.begin(); mit!=m_multibodies.end(); ++mit) {
    (*mit)->BuildModels();
    m_dof += (*mit)->GetDOF();
    m_centerOfMass += (*mit)->GetCOM();
  }

  m_centerOfMass /= m_multibodies.size();

  //Compute radius
  for(MIT mit = m_multibodies.begin(); mit!=m_multibodies.end(); ++mit) {
    double dist = ((*mit)->GetCOM() - m_centerOfMass).norm() + (*mit)->GetRadius();
    if(m_radius < dist)
      m_radius = dist;
  }
}

void
EnvModel::Select(GLuint* _index, vector<Model*>& _sel){
  //unselect old one
  if(!_index || *_index > m_multibodies.size()) //input error
    return;
  else if(*_index == m_multibodies.size())
    m_boundary->Select(_index+1, _sel);
  else
    m_multibodies[_index[0]]->Select(_index+1, _sel);
}

void
EnvModel::Draw() {
  size_t numMBs = m_multibodies.size();

  glPushName(numMBs);
  m_boundary->Draw();
  glPopName();

  glLineWidth(1);
  for(size_t i = 0; i < numMBs; i++){
    if(!m_multibodies[i]->IsActive()){
      glPushName(i);
      m_multibodies[i]->Draw();
      glPopName();
    }
  }
}

void
EnvModel::Print(ostream& _os) const {
  _os << Name() << ": " << GetFilename() << endl
    << m_multibodies.size() << " multibodies" << endl;
}

void
EnvModel::ChangeColor(){
  int numMBs = m_multibodies.size();
  for(int i = 0; i < numMBs; i++)
    m_multibodies[i]->SetColor(Color4(drand48(), drand48(), drand48(), 1));
}

void
EnvModel::GetChildren(list<Model*>& _models){
  typedef vector<MultiBodyModel *>::iterator MIT;
  for(MIT i = m_multibodies.begin(); i != m_multibodies.end(); i++) {
    if(!(*i)->IsActive())
      _models.push_back(*i);
  }
  _models.push_back(m_boundary);
}

void
EnvModel::DeleteMBModel(MultiBodyModel* _mbl){
  vector<MultiBodyModel*>::iterator mbit;
  for(mbit = m_multibodies.begin(); mbit != m_multibodies.end(); mbit++){
    if((*mbit) == _mbl){
      m_multibodies.erase(mbit);
      break;
    }
  }
}

void
EnvModel::AddMBModel(MultiBodyModel* _m){
  _m->BuildModels();
  m_multibodies.push_back(_m);
}

bool
EnvModel::SaveFile(const char* _filename){
  ofstream envFile(_filename);
  if(!envFile.is_open()){
    cout<<"Couldn't open the file"<<endl;
    return false;
  }
  envFile<<"Boundary "<<m_boundaryType<<" "<<m_boundary->GetCoord();
  envFile<<"\n\n";
  int numMBs = m_multibodies.size();
  envFile<<"Multibodies\n"<<numMBs<<"\n\n";
  vector<MultiBodyModel*> saveMB = GetMultiBodies();
  reverse(saveMB.begin(),saveMB.end());
  while(!saveMB.empty()){
    if(saveMB.back()->IsActive())
      envFile<<"Active\n";
    else if(saveMB.back()->IsSurface())
      envFile<<"Surface\n";
    else
      envFile<<"Passive\n";
    int nB = saveMB.back()->GetNbBodies();
    if(nB!= 0){
      if(saveMB.back()->IsActive())
        envFile<<nB<<endl;
      vector<BodyModel*> bodies = saveMB.back()->GetBodies();
      reverse(bodies.begin(),bodies.end());
      envFile<<"#VIZMO_COLOR"<<" "<<bodies.back()->GetColor()[0]
                             <<" "<<bodies.back()->GetColor()[1]
                             <<" "<<bodies.back()->GetColor()[2]<<endl;
      if(saveMB.back()->IsActive()){
        int nbJoints = 0;
        vector<ConnectionModel*> joints = saveMB.back()->GetJoints();
        reverse(joints.begin(),joints.end());
        while(!bodies.empty()){
          envFile<<bodies.back()->GetFilename()<<" ";
          if(bodies.back()->IsBaseVolumetric()||bodies.back()->IsBasePlanar()){
            string baseMovement = "Translational";
            if (bodies.back()->IsBaseRotational())
              baseMovement = "Rotational";
            if(bodies.back()->IsBaseVolumetric())
              envFile<<"Volumetric "<<baseMovement<<endl;
            else
              envFile<<"Planar "<<baseMovement<<endl;
          }
          else if(bodies.back()->IsBaseFixed()){
            envFile<<"Fixed ";
            ostringstream transform;
            transform<<bodies.back()->GetTransform();
            envFile<<SetTransformRight(transform.str())<<endl;
          }
          else{
            envFile<<"Joint"<<endl;
            nbJoints++;
          }
          bodies.pop_back();
        }
        envFile<<"Connections"<<endl<<nbJoints<<endl;
        if(nbJoints!=0){
          while(!joints.empty()){
            pair<double, double> jointLimits[2] = joints.back()->GetJointLimits();
            ostringstream limits;
            string jointType = "NonActuated ";
            if(joints.back()->IsSpherical()){
              jointType = "Spherical ";
              limits<<jointLimits[0].first<<":"<<jointLimits[0].second<<" "
                    <<jointLimits[1].first<<":"<<jointLimits[1].second;
            }
            else if(joints.back()->IsRevolute()){
              jointType = "Revolute ";
              limits<<jointLimits[0].first<<":"<<jointLimits[0].second<<" ";
            }
            envFile<<joints.back()->GetPreviousIndex()<<" "
                     <<joints.back()->GetNextIndex()<<"  "
                     <<jointType<<limits.str()<<endl;
            ostringstream transform;
            transform<<joints.back()->TransformToDHFrame();
            envFile<<SetTransformRight(transform.str())<<"   ";
            envFile<<joints.back()->GetAlpha()<<" "
                     <<joints.back()->GetA()<<" "<<joints.back()->GetD()<<" "
                     <<joints.back()->GetTheta()<<"   ";
            ostringstream transform2;
            transform2<<joints.back()->TransformToBody2();
            envFile<<SetTransformRight(transform2.str())<<endl;
            joints.pop_back();
          }
        }
      }
      else{
        envFile<<bodies.back()->GetFilename()<<"  ";
        ostringstream transform;
        transform<<bodies.back()->GetTransform();
        envFile<<SetTransformRight(transform.str())<<endl;
      }
    }
    envFile<<endl;
    saveMB.pop_back();
  }
  envFile.close();
  return true;
}


string
EnvModel::SetTransformRight(string _transformString){
  stringstream transform;
  istringstream splitTransform(_transformString);
  string splittedTransform[6]={"","","","","",""};
  int j=0;
  do{
    splitTransform>>splittedTransform[j];
    j++;
  }while(splitTransform);
  string temp;
  temp=splittedTransform[3];
  splittedTransform[3]=splittedTransform[5];
  splittedTransform[5]=temp;
  for(int i=0; i<6; i++)
    transform<<splittedTransform[i]<<" ";
  return transform.str();
}
