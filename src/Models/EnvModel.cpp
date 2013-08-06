#include "EnvModel.h"

#include "Plum/EnvObj/ConnectionModel.h"
#include "Plum/EnvObj/BodyModel.h"
#include "CfgModel.h"
#include "Models/BoundingBoxModel.h"
#include "Models/BoundingSphereModel.h"
#include "Utilities/IOUtils.h"
#include "Utilities/Exceptions.h"

EnvModel::EnvModel(const string& _filename) :
  m_containsSurfaces(false), m_radius(0), m_boundary(NULL) {
    SetFilename(_filename);
    SetModelDataDir(_filename.substr(0, _filename.rfind('/')));

    ParseFile();
    BuildModels();
  }

EnvModel::~EnvModel() {
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

  for(size_t i = 0; i < numMultiBodies; i++) {
    MultiBodyModel* m = new MultiBodyModel();
    m->ParseMultiBody(ifs, m_modelDataDir);
    m_multibodies.push_back(m);
  }

  BuildRobotStructure();
  CfgModel::SetDOF(m_dof);
  CfgModel::SetIsPlanarRobot(m_robots[0].m_base == Robot::PLANAR ? true : false);
  CfgModel::SetIsVolumetricRobot(m_robots[0].m_base == Robot::VOLUMETRIC ? true : false);
  CfgModel::SetIsRotationalRobot(m_robots[0].m_baseMovement == Robot::ROTATIONAL ? true : false);
}

void
EnvModel::SetModelDataDir(const string _modelDataDir){
  m_modelDataDir = _modelDataDir;
  cout<<"- Geo Dir   : "<< m_modelDataDir << endl;
}

void
EnvModel::ParseBoundary(ifstream& _ifs) {

  string type = ReadFieldString(_ifs, WHERE, "Failed reading Boundary type.");

  if(type == "BOX")
    m_boundary = new BoundingBoxModel();
  else if(type == "SPHERE")
    m_boundary = new BoundingSphereModel();
  else
    throw ParseException(WHERE,
        "Failed reading boundary type '" + type + "'. Choices are BOX or SPHERE.");

  m_boundary->Parse(_ifs);
}

void
EnvModel::BuildRobotStructure(){

  m_dof = 0;
  int robotIndex = 0;
  for(size_t i = 0; i < m_multibodies.size(); i++){
    if(m_multibodies[i]->IsActive()){
      robotIndex = i;
      break;
    }
  }
  MultiBodyModel* robot = m_multibodies[robotIndex];
    //int fixedBodyCount = robot -> GetFixedBodyCount();
    //int freeBodyCount = robot->GetFreeBodyCount();
    //int numOfBodies = robot.m_numberOfBody;
  for(int i = 0; i < distance(robot->Begin(), robot->End()); i++)
    m_robotGraph.add_vertex(i);

  //Total amount of bodies in environment: free + fixed
  for (MultiBodyModel::BodyIter bit = robot->Begin(); bit!=robot->End(); ++bit)
    //For each body, find forward connections and connect them
    for(BodyModel::ConnectionIter cit = (*bit)->Begin(); cit!=(*bit)->End(); ++cit)
      m_robotGraph.add_edge(bit-robot->Begin(), (*cit)->GetNextIndex());

  //Robot ID typedef
  typedef RobotGraph::vertex_descriptor RID;
  vector<pair<size_t,RID> > ccs;
  stapl::vector_property_map<RobotGraph, size_t> cmap;

  //Initialize CC information
  get_cc_stats(m_robotGraph, cmap, ccs);
  for(size_t i = 0; i < ccs.size(); i++){
    cmap.reset();
    vector<RID> cc;
    //Find CCs, construct robot objects
    get_cc(m_robotGraph, cmap, ccs[i].second, cc);
    size_t baseIndx = -1;

    for(size_t j = 0; j < cc.size(); j++){
      size_t index = m_robotGraph.find_vertex(cc[j])->property();
      if((*(robot->Begin()+index))->IsBase()){
        baseIndx = index;
        break;
      }
    }

    if(baseIndx == size_t(-1))
      throw ParseException(WHERE, "Robot does not have base.");

    const BodyModel* base = *(robot->Begin() + baseIndx);
    Robot::Base bt = base->GetBase();
    Robot::BaseMovement bm = base->GetBaseMovement();
    if(bt == Robot::PLANAR){
      m_dof += 2;
      if(bm == Robot::ROTATIONAL){
        m_dof += 1;
      }
    }
    else if(bt == Robot::VOLUMETRIC){
      m_dof += 3;
      if(bm == Robot::ROTATIONAL){
        m_dof += 3;
      }
    }

    Robot::JointMap jm;
    for(size_t j = 0; j<cc.size(); j++){
      size_t index = m_robotGraph.find_vertex(cc[j])->property();
      typedef Robot::JointMap::const_iterator MIT;
      for(MIT mit = robot->GetJointMap().begin(); mit!=robot->GetJointMap().end(); mit++){
        if((*mit)->GetPreviousIndex() == index){
          jm.push_back(*mit);
          if((*mit)->GetJointType() == ConnectionModel::REVOLUTE){
            m_dof += 1;
          }
          else if((*mit)->GetJointType() == ConnectionModel::SPHERICAL){
            m_dof += 2;
          }
        }
      }
    }

    m_robots.push_back(Robot(bt, bm, jm, baseIndx));
  }
}

//////////Display functions//////////
void
EnvModel::BuildModels(){

  //Build boundary model
  m_boundary = GetBoundary();
  if(!m_boundary)
    throw BuildException(WHERE, "Boundary is NULL");
  m_boundary->BuildModels();

  //Build each
  typedef vector<MultiBodyModel*>::const_iterator MIT;
  for(MIT mit = m_multibodies.begin(); mit!=m_multibodies.end(); ++mit) {
    (*mit)->BuildModels();
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
EnvModel::Draw(GLenum _mode) {
  int numMBs = m_multibodies.size();
  if(_mode == GL_SELECT)
    glPushName(numMBs);

  m_boundary->Draw(_mode);

  if(_mode == GL_SELECT)
    glPopName();

  glLineWidth(1);
  for(int i = 0; i < numMBs; i++){
    if(!m_multibodies[i]->IsActive()){
      if(_mode == GL_SELECT)
        glPushName(i);
      m_multibodies[i]->Draw(_mode);
      if(_mode == GL_SELECT)
        glPopName();
    }
  }
}

void
EnvModel::ChangeColor(){
  int numMBs = m_multibodies.size();
  for(int i = 0; i < numMBs; i++)
    m_multibodies[i]->SetColor(Color4(drand48(), drand48(), drand48(), 1));
}

void
EnvModel::Select(unsigned int* _index, vector<GLModel*>& _sel){
  //unselect old one
  if(!_index || *_index > m_multibodies.size()) //input error
    return;
  else if(*_index == m_multibodies.size())
    m_boundary->Select(_index+1, _sel);
  else
    m_multibodies[_index[0]]->Select(_index+1, _sel);
}

void
EnvModel::GetChildren(list<GLModel*>& _models){
  typedef vector<MultiBodyModel *>::iterator MIT;
  for(MIT i = m_multibodies.begin(); i != m_multibodies.end(); i++){
    if(!(*i)->IsActive())
      _models.push_back(*i);
  }
  _models.push_back(m_boundary);
}

vector<string>
EnvModel::GetInfo() const{
  vector<string> info;
  info.push_back(GetFilename());

  ostringstream temp;
  temp << "There are " << m_multibodies.size() << " multibodies";
  info.push_back(temp.str());

  return info;
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
/*
  const MultiBodyInfo* mBI = GetMultiBodyInfo();

  FILE* envFile;
  if((envFile = fopen(_filename, "a")) == NULL){
    cout<<"Couldn't open the file"<<endl;
    return false;
  }

  int numMBs = GetNumMultiBodies(); //number of objects in env.
  //write num. of Bodies
  fprintf(envFile,"%d\n\n", numMBs);

  //getMBody() and then current position and orientation
  vector<MultiBodyModel*> mBModels = this->GetMultiBodies();

  for(int i = 0; i < numMBs; i++){ //for each body in *.env

    if(mBI[i].m_active)
      fprintf(envFile,"Multibody   Active\n");
    else
      fprintf(envFile,"Multibody   Passive\n");

    if(mBI[i].m_numberOfBody != 0){
      int nB = mBI[i].m_numberOfBody;
      //write Num. of Bodies in the current MultiBody
      fprintf(envFile,"%d\n", nB);
      //write COLOR tag
      list<GLModel*> tmpList;
      mBModels[i]->GetChildren(tmpList);
      GLModel* om = tmpList.front();
      if(mBI[i].m_active){
        fprintf(envFile,"#VIZMO_COLOR %2f %2f %2f\n",
            mBI[i].m_mBodyInfo[0].rgb[0],
            mBI[i].m_mBodyInfo[0].rgb[1],
            mBI[i].m_mBodyInfo[0].rgb[2]);
      }
      else{
        fprintf(envFile,"#VIZMO_COLOR %2f %2f %2f\n",
                om->GetColor()[0],om->GetColor()[1], om->GetColor()[2]);
      }

      for(int j = 0; j < nB; j++){
        if(mBI[i].m_mBodyInfo[j].m_isFixed)
          fprintf(envFile,"FixedBody    ");
        else
          fprintf(envFile,"FreeBody    ");

        fprintf(envFile, "%d  ", mBI[i].m_mBodyInfo[j].m_index);
        string s_tmp = mBI[i].m_mBodyInfo[j].m_modelDataFileName;
        const char* st;
        st = s_tmp.c_str();
        const char *pos = strrchr(st, '/');
        int position = pos-st+1;
        string sub_string = s_tmp.substr(position);

        const char* f;
        f = sub_string.c_str();

        if(!mBI[i].m_active){
          string s = mBI[i].m_mBodyInfo[j].m_fileName;
          f = s.c_str();
          fprintf(envFile,"%s  ",f);
        }
        else
          fprintf(envFile,"%s  ",f);

        //get current (new) rotation

        Quaternion qtmp = mBModels[i]->q();
        //Matrix3x3 mtmp = qtmp.getMatrix();
        //Vector3d vtmp = qtmp.MatrixToEuler(mtmp);

        //get prev. rotation

        list<GLModel*> objList;
        mBModels[i]->GetChildren(objList);
        GLModel* om = objList.front();

        //multiply polyhedron0 and multiBody quaternions
        //to get new rotation
        Quaternion finalQ;
        finalQ = qtmp * om->q();

        EulerAngle e;
        convertFromQuaternion(e, finalQ);

        fprintf(envFile,"%.1f %.1f %.1f %.1f %.1f %.1f\n",
            mBModels[i]->tx(), mBModels[i]->ty(), mBModels[i]->tz(),
            radToDeg(e.alpha()),
            radToDeg(e.beta()),
            radToDeg(e.gamma()));

      }
      //write Connection tag

      if(mBI[i].m_numberOfConnections != 0)
        fprintf(envFile, "\nConnection\n");
      else
        fprintf(envFile,"Connection\n");

      fprintf(envFile, "%d\n", mBI[i].m_numberOfConnections);

      //write Connection info.
      if(mBI[i].m_numberOfConnections != 0){
        const char* str;
        int numConn = mBI[i].GetJointMap().size();

        for(int l = 0; l < numConn; l++){
          int indexList = mBI[i].GetJointMap()[l]->GetPreviousBody();
          if(mBI[i].m_mBodyInfo[indexList].m_connectionInfo->m_actuated)
            str = "Actuated";
          else
            str = "NonActuated";

          fprintf(envFile,"%d %d  %s\n",indexList,
              mBI[i].GetJointMap()[l]->GetNextBody(), str);

          //get info. from current Body and current connection
          int index = 0;
          for(int b = 0;
              b<mBI[i].m_mBodyInfo[indexList].m_numberOfConnection; b++){

            int n = mBI[i].m_mBodyInfo[indexList].m_connectionInfo[b].m_nextIndex;

            if(mBI[i].GetJointMap()[l]->GetNextBody() == n){
              index = b;
              break;
            }
          }

          fprintf(envFile, "%.1f ",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].m_posX);
          fprintf(envFile, "%.1f ",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].m_posY);
          fprintf(envFile, "%.1f ",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].m_posZ);
          fprintf(envFile, "%.1f ",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].m_orientX*57.29578);
          fprintf(envFile, "%.1f ",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].m_orientY* 57.29578);
          fprintf(envFile, "%.1f\t",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].m_orientZ* 57.29578);

          fprintf(envFile, "%.1f ",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].alpha);
          fprintf(envFile, "%.1f ",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].a);
          fprintf(envFile, "%.1f ",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].d);
          fprintf(envFile, "%.1f        ",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].m_theta);
          fprintf(envFile, "%.1f ",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].m_pos2X);
          fprintf(envFile, "%.1f ",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].m_pos2Y);
          fprintf(envFile, "%.1f ",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].m_pos2Z);
          fprintf(envFile, "%.1f ",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].m_orient2X* 57.29578);
          fprintf(envFile, "%.1f ",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].m_orient2Y* 57.29578);
          fprintf(envFile, "%.1f\n\n",
              mBI[i].m_mBodyInfo[indexList].m_connectionInfo[index].m_orient2Z* 57.29578);
        }

      }

    }
    fprintf(envFile,"\n");
  }

  fclose(envFile);
  */
  return 1;
}

