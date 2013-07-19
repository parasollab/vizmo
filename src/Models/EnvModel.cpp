#include "EnvModel.h"

#include "Plum/EnvObj/MultiBodyInfo.h"
#include "CfgModel.h"
#include "Models/BoundingBoxModel.h"
#include "Models/BoundingSphereModel.h"
#include "Utilities/IOUtils.h"

EnvModel::EnvModel(const string& _filename) :
  m_numMultiBodies(0), m_mBInfo(NULL),
  m_containsSurfaces(false), m_radius(0), m_boundary(NULL) {
    SetFilename(_filename);
    SetModelDataDir(_filename.substr(0, _filename.rfind('/')));
  }

EnvModel::~EnvModel(){

  FreeMemory();
}

//////////Load Functions//////////
void
EnvModel::GetColor(istream& _is){

  string line;
  while(!_is.eof()){

    char c;
    while(isspace(_is.peek()))
      _is.get(c);

    c = _is.peek();
    if(!IsCommentLine(c))
      return;
    else{
      getline(_is, line);
      //colors begin with VIZMO_COLOR
      if(line[7] == 'C'){
        size_t loc = line.find(" ");
        string sub = line.substr(loc+1);
        istringstream iss(sub);
        if(!(iss >> m_color[0] >> m_color[1] >>  m_color[2]))
          cerr << "Warning: Error loading vizmo color." << endl;
      }
    }
  }
}

bool
EnvModel::ParseFile(){

  if(!FileExists(GetFilename()))
    return false;

  //Open file for reading datat
  ifstream ifs(GetFilename().c_str());

  if(ParseFileHeader(ifs) == false)
    return false;

  if(ParseFileBody(ifs) == false)
    return false;

  ifs.close();

  return true;
}

void
EnvModel::SetModelDataDir(const string _modelDataDir){

  m_modelDataDir = _modelDataDir;
  cout<<"- Geo Dir   : "<< m_modelDataDir << endl;
}

void
EnvModel::SetNewMultiBodyInfo(CMultiBodyInfo* _mbi){

  for(int i = 0; i < m_numMultiBodies; i++)
    m_mBInfo[i] = _mbi[i];
}

void
EnvModel::FreeMemory(){

  if(m_mBInfo != NULL)
    delete [] m_mBInfo;
  m_mBInfo = NULL;
}

bool
EnvModel::ParseFileHeader(ifstream& _ifs){

  //Read boundary
  string b = ReadFieldString(_ifs, "Boundary Tag");
  if(b != "BOUNDARY") {
    cerr << "Error reading boundary tag." << endl;
    return false;
  }

  if(!ParseBoundary(_ifs)){
    cerr << "Error parsing boundary." << endl;
    return false;
  }

  //Read number of multibodies
  string mb = ReadFieldString(_ifs, "Number of Multibodies tag");
  if(mb != "MULTIBODIES"){
    cerr << "Error reading environment multibodies tag." << endl;
    return false;
  }

  m_numMultiBodies = ReadField<int>(_ifs, "Number of Multibodies");
  return true;
}

bool
EnvModel::ParseBoundary(ifstream& _ifs){

  string type = ReadFieldString(_ifs, "Boundary type");

  if(type == "BOX")
    m_boundary = new BoundingBoxModel();

  else if(type == "SPHERE")
    m_boundary = new BoundingSphereModel();

  else{
    cerr << "Error reading boundary type " << type << ". Choices are BOX or SPHERE." << endl;
    return false;
  }
  return m_boundary->Parse(_ifs);
}

bool
EnvModel::ParseFileBody(ifstream& _ifs){

  m_mBInfo = new CMultiBodyInfo[m_numMultiBodies];
  if(m_mBInfo == NULL)
    return false;

  for(int i = 0; i < m_numMultiBodies; i++){
    if(ParseMultiBody(_ifs, m_mBInfo[i])== false)
      return false;
  }

  BuildRobotStructure();
  //CfgModel::m_dof = m_dof;
  CfgModel::SetDOF(m_dof);
  //cout<< "DOFs: "<< CfgModel::m_dof << endl << flush;

  return true;
}

bool
EnvModel::ParseMultiBody(ifstream& _ifs, CMultiBodyInfo& _mBInfo){

  string multibodyType = ReadFieldString(_ifs,
    "Multibody Type (Active, Passive, Internal, Surface)");

  if(multibodyType == "ACTIVE")
    _mBInfo.m_active = true;

  else if(multibodyType == "SURFACE"){
    _mBInfo.m_surface = true;
    m_containsSurfaces = true;
  }

  if(multibodyType == "ACTIVE"){
    _mBInfo.m_cNumberOfBody = ReadField<int>(_ifs, "Body Count");
    _mBInfo.m_pBodyInfo = new CBodyInfo[_mBInfo.m_cNumberOfBody];

    if(_mBInfo.m_pBodyInfo == NULL)
      return false;

    GetColor(_ifs);

    for(int i = 0; i < _mBInfo.m_cNumberOfBody; i++){
      if(ParseActiveBody(_ifs, _mBInfo.m_pBodyInfo[i]) == false)
        return false;
    }

    //Get connection info
    string connectionTag = ReadFieldString(_ifs, "Connections tag");
    int numberOfRobotConnections = ReadField<int>(_ifs, "Number of Connections");
    _mBInfo.m_NumberOfConnections = numberOfRobotConnections;

    int currentBody = 0; //index of current Body

    //Do transformation for body0, the base. This is always needed.
    _mBInfo.m_pBodyInfo[currentBody].doTransform();

    if(numberOfRobotConnections != 0){
      //initialize the MBInfo.m_pBodyInfo[i].m_pConnectionInfo for each body
      for(int i = 0; i < _mBInfo.m_cNumberOfBody; i++){
        _mBInfo.m_pBodyInfo[i].m_pConnectionInfo =
          new CConnectionInfo[numberOfRobotConnections];

        if(_mBInfo.m_pBodyInfo[i].m_pConnectionInfo == NULL){
          cout<<"COULDN'T CREATE CONNECTION INFO"<<endl;
          return false;
        }
      }
      for(int iB = 0; iB < numberOfRobotConnections; iB++){
        if(ParseConnections(_ifs, _mBInfo) == false)
          return false;
      }
    }
  }

  else if(multibodyType == "INTERNAL" || multibodyType == "SURFACE" ||
          multibodyType == "PASSIVE"){

    _mBInfo.m_cNumberOfBody = 1;
    _mBInfo.m_pBodyInfo = new CBodyInfo[_mBInfo.m_cNumberOfBody];

    if(_mBInfo.m_pBodyInfo == NULL)
      return false;

    GetColor(_ifs);

    if(ParseOtherBody(_ifs, _mBInfo.m_pBodyInfo[0]) == false)
      return false;

    return true;
  }

  else{
    cerr << "Error:: Unsupported body type" << endl;
    cerr << "Choices are Active, Passive, Internal, or Surface" << endl;
    exit(1);
  }
  return true;
}

bool
EnvModel::ParseActiveBody(ifstream& _ifs, CBodyInfo& _bodyInfo){

  _bodyInfo.m_bIsFixed = false;
  bool isBase = false;
  Robot::Base baseType;
  Robot::BaseMovement baseMovementType;

  _bodyInfo.m_strFileName = ReadFieldString(_ifs,
    "Body Filename (geometry file)", false);

  if(m_modelDataDir.empty() == false){
    //store just the path of the current directory
    _bodyInfo.m_strDirectory = m_modelDataDir;

    _bodyInfo.m_strModelDataFileName += m_modelDataDir;
    _bodyInfo.m_strModelDataFileName += "/";
  }

  _bodyInfo.m_strModelDataFileName += _bodyInfo.m_strFileName;

  //Read for Base Type. If Planar or Volumetric, read in two more strings
  //If Joint skip this stuff. If Fixed read in positions like an obstacle
  string baseTag = ReadFieldString(_ifs, "Base Tag (Planar, Volumetric, Fixed, Joint");
  baseType = Robot::GetBaseFromTag(baseTag);

  Vector3d bodyPosition;
  Vector3d bodyRotation;

  if(baseType == Robot::VOLUMETRIC || baseType == Robot::PLANAR){
    isBase = true;
    _bodyInfo.m_IsBase = true;
    string rotationalTag = ReadFieldString(_ifs, "Rotation Tag (Rotational, Translational");
    baseMovementType = Robot::GetMovementFromTag(rotationalTag);
  }
  else if(baseType == Robot::FIXED){
    isBase = true;
    bodyPosition = ReadField<Vector3d>(_ifs, "Body Position");
    bodyRotation = ReadField<Vector3d>(_ifs, "Body Orientation");
  }

  //save this for when these classes utilize only transformations instead
  //of x, y, z, alpha, beta, gamma, separately.
  _bodyInfo.m_IsBase = isBase;
  _bodyInfo.isBase = isBase;
  _bodyInfo.SetBase(baseType);
  if(isBase)
    _bodyInfo.SetBaseMovement(baseMovementType);

  _bodyInfo.m_X = bodyPosition[0];
  _bodyInfo.m_Y = bodyPosition[1];
  _bodyInfo.m_Z = bodyPosition[2];
  _bodyInfo.m_Alpha = degToRad(bodyRotation[0]);
  _bodyInfo.m_Beta = degToRad(bodyRotation[1]);
  _bodyInfo.m_Gamma = degToRad(bodyRotation[2]);
  _bodyInfo.doTransform();

  //Set color information
  if(m_color[0] != -1){
    _bodyInfo.rgb[0] = m_color[0];
    _bodyInfo.rgb[1] = m_color[1];
    _bodyInfo.rgb[2] = m_color[2];
  }
  else{
    if(_bodyInfo.m_bIsFixed){
      _bodyInfo.rgb[0] = 0.5;
      _bodyInfo.rgb[1] = 0.5;
      _bodyInfo.rgb[2] = 0.5;
    }
    else{
      _bodyInfo.rgb[0] = 1;
      _bodyInfo.rgb[1] = 0;
      _bodyInfo.rgb[2] = 0;
    }
  }
  return true;
}

bool
EnvModel::ParseOtherBody(ifstream& _ifs, CBodyInfo& _bodyInfo){

  _bodyInfo.m_bIsFixed = true;
  _bodyInfo.m_IsBase = true;
  _bodyInfo.m_strFileName = ReadFieldString(_ifs,
    "Body Filename (geometry file)", false);

  if(m_modelDataDir.empty() == false){

    //store just the path of the current directory
    _bodyInfo.m_strDirectory = m_modelDataDir;
    _bodyInfo.m_strModelDataFileName += m_modelDataDir;
    _bodyInfo.m_strModelDataFileName += "/";
  }
  _bodyInfo.m_strModelDataFileName += _bodyInfo.m_strFileName;

  Vector3d bodyPosition = ReadField<Vector3d>(_ifs, "Body Position");
  Vector3d bodyRotation = ReadField<Vector3d>(_ifs, "Body Orientation");

  //save this for when body utilizes only transformation not
  //x,y,z,alpha,beta,gama

  _bodyInfo.m_X = bodyPosition[0];
  _bodyInfo.m_Y = bodyPosition[1];
  _bodyInfo.m_Z = bodyPosition[2];
  _bodyInfo.m_Alpha = degToRad(bodyRotation[0]);
  _bodyInfo.m_Beta = degToRad(bodyRotation[1]);
  _bodyInfo.m_Gamma = degToRad(bodyRotation[2]);
  _bodyInfo.doTransform();

  //Set color information
  if(m_color[0] != -1){
    _bodyInfo.rgb[0] = m_color[0];
    _bodyInfo.rgb[1] = m_color[1];
    _bodyInfo.rgb[2] = m_color[2];
  }
  else{
    if(_bodyInfo.m_bIsFixed){
      _bodyInfo.rgb[0] = 0.5;
      _bodyInfo.rgb[1] = 0.5;
      _bodyInfo.rgb[2] = 0.5;
    }
    else{
      _bodyInfo.rgb[0] = 1;
      _bodyInfo.rgb[1] = 0;
      _bodyInfo.rgb[2] = 0;
    }
  }
  return true;
}

bool
EnvModel::ParseConnections(ifstream& _ifs, CMultiBodyInfo& _mBInfo){

  //body indices
  int previousBodyIndex = ReadField<int>(_ifs, "Previous Body Index");
  int nextBodyIndex = ReadField<int>(_ifs, "Next Body Index");

  //Increment m_cNumberOfConnection for each body
  CBodyInfo& previousBody = _mBInfo.m_pBodyInfo[previousBodyIndex];
  previousBody.m_cNumberOfConnection++;

  //Get connection index for this body
  CConnectionInfo& conn = previousBody.m_pConnectionInfo[previousBody.m_cNumberOfConnection - 1];
  Robot::Joint connPtr(&conn);
  _mBInfo.jointMap.push_back(connPtr);

  //Set global index
  conn.m_globalIndex = CConnectionInfo::m_globalCounter++;

  //Set next and pre index
  conn.m_preIndex = previousBodyIndex;
  conn.m_nextIndex = nextBodyIndex;

  //Set Actuated/NonActuated
  conn.m_actuated = true;

  //Grab the joint type
  string connectionTypeTag = ReadFieldString(_ifs, "Connection Type");
  conn.m_jointType = CConnectionInfo::GetJointTypeFromTag(connectionTypeTag);

  //Grab the joint limits for revolute and spherical joints
  pair<double, double> jointLimits[2];
  if(conn.m_jointType == CConnectionInfo::REVOLUTE || conn.m_jointType == CConnectionInfo::SPHERICAL){
    jointLimits[0].first = jointLimits[1].first = -1;
    jointLimits[0].second = jointLimits[1].second = 1;
    size_t numRange = (conn.m_jointType == CConnectionInfo::REVOLUTE) ? 1 : 2;
    for(size_t i = 0; i<numRange; i++){
      string tok;
      if(_ifs >> tok){
        size_t del = tok.find(":");
        if(del == string::npos){
          cerr << "Error::Reading joint range " << i << ". Should be delimited by ':'." << endl;
          exit(1);
        }
        istringstream minv(tok.substr(0,del)), maxv(tok.substr(del+1, tok.length()));
        if(!(minv>>jointLimits[i].first && maxv>>jointLimits[i].second)){
          cerr << "Error::Reading joint range " << i << "." << endl;
          exit(1);
        }
      }
      else if(numRange == 2 && i==1) { //error. only 1 token provided.
        cerr << "Error::Reading spherical joint ranges. Only one provided." << endl;
        exit(1);
      }
    }
  }

  //Transformation to DHFrame
  Vector3d positionToDHFrame = ReadField<Vector3d>(_ifs, "Translation to DHFrame");
  Vector3d rotationToDHFrame = ReadField<Vector3d>(_ifs, "Rotation to DHFrame");

  conn.m_pos2X = positionToDHFrame[0];
  conn.m_pos2Y = positionToDHFrame[1];
  conn.m_pos2Z = positionToDHFrame[2];

  conn.m_orient2X = degToRad(rotationToDHFrame[0]);
  conn.m_orient2Y = degToRad(rotationToDHFrame[1]);
  conn.m_orient2Z = degToRad(rotationToDHFrame[2]);

  //DH parameters
  Vector4d dhparameters = ReadField<Vector4d>(_ifs, "DH Parameters");

  conn.alpha = dhparameters[0];
  conn.a = dhparameters[1];
  conn.d = dhparameters[2];
  conn.theta = dhparameters[3];

  //Save original theta
  conn.m_theta = conn.theta;

  //Transformation to next body
  Vector3d positionToNextBody = ReadField<Vector3d>(_ifs, "Translation to Next Body");
  Vector3d rotationToNextBody = ReadField<Vector3d>(_ifs, "Rotation to Next Body");

  conn.m_posX = positionToNextBody[0];
  conn.m_posY = positionToNextBody[1];
  conn.m_posZ = positionToNextBody[2];

  conn.m_orientX = degToRad(rotationToNextBody[0]);
  conn.m_orientY = degToRad(rotationToNextBody[1]);
  conn.m_orientZ = degToRad(rotationToNextBody[2]);

  return true;
}

void
EnvModel::BuildRobotStructure(){

  m_dof = 0;
  int robotIndex = 0;
  for(int i = 0; i < m_numMultiBodies; i++){
    if(m_mBInfo[i].m_active){
      robotIndex = i;
      break;
    }
  }
  CMultiBodyInfo& robot = m_mBInfo[robotIndex];
    //int fixedBodyCount = robot -> GetFixedBodyCount();
    //int freeBodyCount = robot->GetFreeBodyCount();
    //int numOfBodies = robot.m_cNumberOfBody;
  for(int i = 0; i < robot.m_cNumberOfBody; i++)
    m_robotGraph.add_vertex(i);

  //Total amount of bodies in environment: free + fixed
  for (int i = 0; i < robot.m_cNumberOfBody; i++){
    CBodyInfo& body = robot.m_pBodyInfo[i];
    //For each body, find forward connections and connect them
    for (int j = 0; j < body.m_cNumberOfConnection; j++){
      int nextIndex = body.m_pConnectionInfo[j].m_nextIndex;
      m_robotGraph.add_edge(i, nextIndex);
    }
  }

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
      if(robot.m_pBodyInfo[index].IsBase()){
        baseIndx = index;
        break;
      }
    }

    if(baseIndx == size_t(-1)){
      cerr << "Each robot must have at least one base. Please fix .env file." << endl;
      exit(1);
    }

    Robot::Base bt = robot.m_pBodyInfo[baseIndx].GetBase();
    Robot::BaseMovement bm = robot.m_pBodyInfo[baseIndx].GetBaseMovement();
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
      int index = m_robotGraph.find_vertex(cc[j])->property();
      typedef Robot::JointMap::iterator MIT;
      for(MIT mit = robot.GetJointMap().begin(); mit!=robot.GetJointMap().end(); mit++){
        if((*mit)->m_preIndex == index){
          jm.push_back(*mit);
          if((*mit)->m_jointType == CConnectionInfo::REVOLUTE){
            m_dof += 1;
          }
          else if((*mit)->m_jointType == CConnectionInfo::SPHERICAL){
            m_dof += 2;
          }
        }
      }
    }

    m_robots.push_back(Robot(bt, bm, jm, baseIndx));
  }
}

//////////Display functions//////////

bool
EnvModel::BuildModels(){

  //Build boundary model
  m_boundary = GetBoundary();
  if(m_boundary == NULL)
    return false;
  if(m_boundary->BuildModels() == false)
    return false;

  //Create MutileBody Model
  int numMBs = GetNumMultiBodies();
  m_mBModels.reserve(numMBs);

  //Build each
  Vector3d com;
  for(int i = 0; i < numMBs; i++){
    MultiBodyModel* m = new MultiBodyModel(GetMultiBodyInfo()[i]);
    if(m == NULL)
      return false;
    if(m->BuildModels() == false)
      return false;
    com = com + (m->GetCOM()-Point3d(0,0,0));
    m_mBModels.push_back(m);
  }
  for(int i = 0; i < 3; i++)
    m_centerOfMass[i] = com[i]/numMBs;

  //Compute radius
  for(int i = 0; i < numMBs; i++){
    double dist = (m_mBModels[i]->GetCOM() - m_centerOfMass).norm()
    + m_mBModels[i]->GetRadius();
    if(m_radius < dist)
      m_radius = dist;
  }
  return true;
}

void
EnvModel::Draw(GLenum _mode){

  if(_mode == GL_SELECT && m_enableSelection == false)
    return;

  int numMBs = m_mBModels.size();
  if(_mode == GL_SELECT)
    glPushName(numMBs);

  m_boundary->Draw(_mode);

  if(_mode == GL_SELECT)
    glPopName();

  glLineWidth(1);
  for(int iP = 0; iP < numMBs; iP++){
    if(m_mBModels[iP]->IsFixed()){
      if(_mode == GL_SELECT)
        glPushName(iP);
      m_mBModels[iP]->Draw(_mode);
      if(_mode == GL_SELECT)
        glPopName();
    }
  }
}

void
EnvModel::ChangeColor(){

  int numMBs = m_mBModels.size();
  float r, g, b;
  for(int iP = 0; iP < numMBs; iP++){
    r  = ((float)rand())/RAND_MAX;
    g = ((float)rand())/RAND_MAX;
    b = ((float)rand())/RAND_MAX;
    m_mBModels[iP]->SetColor(r, g, b, 1);
  }
}

void
EnvModel::Select(unsigned int* _index, vector<GLModel*>& _sel){

  //cout << "selecting env object" << endl;
  //unselect old one
  if(!_index || *_index > m_mBModels.size()) //input error
    return;
  else if(*_index == m_mBModels.size())
    m_boundary->Select(_index+1, _sel);
  else
    m_mBModels[_index[0]]->Select(_index+1, _sel);
}

void
EnvModel::GetChildren(list<GLModel*>& _models){

  typedef vector<MultiBodyModel *>::iterator MIT;
  for(MIT i = m_mBModels.begin(); i != m_mBModels.end(); i++){
    if((*i)->IsFixed())
      _models.push_back(*i);
  }
  _models.push_back(m_boundary);
}

vector<string>
EnvModel::GetInfo() const{

  vector<string> info;
  info.push_back(GetFilename());

  ostringstream temp;
  temp << "There are " << m_mBModels.size() << " multibodies";
  info.push_back(temp.str());

  return info;
}

vector<vector<PolyhedronModel> >
EnvModel::GetMBPolyLists(){

  int numMBs = m_mBModels.size();
  vector<vector<PolyhedronModel> > pPoly;

  for(int i = 0; i < numMBs; i++)
    pPoly.push_back(m_mBModels[i]->GetPolyhedron());

  return pPoly;
}

void
EnvModel::DeleteMBModel(MultiBodyModel* _mbl){

  vector<MultiBodyModel*>::iterator mbit;
  for(mbit = m_mBModels.begin(); mbit != m_mBModels.end(); mbit++){
    if((*mbit) == _mbl){
      m_mBModels.erase(mbit);
      break;
    }
  }
}

void
EnvModel::AddMBModel(CMultiBodyInfo _newMBI){

  int numMBs = GetNumMultiBodies();
  int i = numMBs - 1;

  MultiBodyModel* m = new MultiBodyModel(GetMultiBodyInfo()[i]);
  m->BuildModels();
  Vector3d com;
  com = com+(m->GetCOM()-Point3d(0,0,0));
  m_mBModels.push_back(m);
}

bool
EnvModel::SaveFile(const char* _filename){

  const CMultiBodyInfo* mBI = GetMultiBodyInfo();

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

    if(mBI[i].m_cNumberOfBody != 0){
      int nB = mBI[i].m_cNumberOfBody;
      //write Num. of Bodies in the current MultiBody
      fprintf(envFile,"%d\n", nB);
      //write COLOR tag
      list<GLModel*> tmpList;
      mBModels[i]->GetChildren(tmpList);
      GLModel* om = tmpList.front();
      if(mBI[i].m_active){
        fprintf(envFile,"#VIZMO_COLOR %2f %2f %2f\n",
            mBI[i].m_pBodyInfo[0].rgb[0],
            mBI[i].m_pBodyInfo[0].rgb[1],
            mBI[i].m_pBodyInfo[0].rgb[2]);
      }
      else{
        fprintf(envFile,"#VIZMO_COLOR %2f %2f %2f\n",
                om->GetColor()[0],om->GetColor()[1], om->GetColor()[2]);
      }

      for(int j = 0; j < nB; j++){
        if(mBI[i].m_pBodyInfo[j].m_bIsFixed)
          fprintf(envFile,"FixedBody    ");
        else
          fprintf(envFile,"FreeBody    ");

        fprintf(envFile, "%d  ", mBI[i].m_pBodyInfo[j].m_Index);
        string s_tmp = mBI[i].m_pBodyInfo[j].m_strModelDataFileName;
        const char* st;
        st = s_tmp.c_str();
        const char *pos = strrchr(st, '/');
        int position = pos-st+1;
        string sub_string = s_tmp.substr(position);

        const char* f;
        f = sub_string.c_str();

        if(!mBI[i].m_active){
          string s = mBI[i].m_pBodyInfo[j].m_strFileName;
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

      if(mBI[i].m_NumberOfConnections != 0)
        fprintf(envFile, "\nConnection\n");
      else
        fprintf(envFile,"Connection\n");

      fprintf(envFile, "%d\n", mBI[i].m_NumberOfConnections);

      //write Connection info.
      if(mBI[i].m_NumberOfConnections != 0){
        const char* str;
        int numConn = mBI[i].GetJointMap().size();

        for(int l = 0; l < numConn; l++){
          int indexList = mBI[i].GetJointMap()[l]->GetPreviousBody();
          if(mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo->m_actuated)
            str = "Actuated";
          else
            str = "NonActuated";

          fprintf(envFile,"%d %d  %s\n",indexList,
              mBI[i].GetJointMap()[l]->GetNextBody(), str);

          //get info. from current Body and current connection
          int index = 0;
          for(int b = 0;
              b<mBI[i].m_pBodyInfo[indexList].m_cNumberOfConnection; b++){

            int n = mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[b].m_nextIndex;

            if(mBI[i].GetJointMap()[l]->GetNextBody() == n){
              index = b;
              break;
            }
          }

          fprintf(envFile, "%.1f ",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_posX);
          fprintf(envFile, "%.1f ",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_posY);
          fprintf(envFile, "%.1f ",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_posZ);
          fprintf(envFile, "%.1f ",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orientX*57.29578);
          fprintf(envFile, "%.1f ",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orientY* 57.29578);
          fprintf(envFile, "%.1f\t",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orientZ* 57.29578);

          fprintf(envFile, "%.1f ",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].alpha);
          fprintf(envFile, "%.1f ",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].a);
          fprintf(envFile, "%.1f ",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].d);
          fprintf(envFile, "%.1f        ",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_theta);
          fprintf(envFile, "%.1f ",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_pos2X);
          fprintf(envFile, "%.1f ",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_pos2Y);
          fprintf(envFile, "%.1f ",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_pos2Z);
          fprintf(envFile, "%.1f ",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orient2X* 57.29578);
          fprintf(envFile, "%.1f ",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orient2Y* 57.29578);
          fprintf(envFile, "%.1f\n\n",
              mBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orient2Z* 57.29578);
        }

      }

    }
    fprintf(envFile,"\n");
  }

  fclose(envFile);
  return 1;
}

