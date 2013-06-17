// EnvLoader.cpp: implementation of the CEnvLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "EnvLoader.h"
#include "MultiBodyInfo.h"
#include "MapObj/Cfg.h"
#include "EnvObj/BoundingBoxModel.h"
#include "EnvObj/BoundingSphereModel.h"

#define  PI_180 3.1415926535/180 

namespace plum{

  //////////////////////////////////////////////////////////////////////
  // Construction/Destruction
  //////////////////////////////////////////////////////////////////////

  CEnvLoader::CEnvLoader() {
    m_cNumberOfMultiBody = 0;
    m_pMBInfo=NULL;
    m_ContainsSurfaces=false; //this trigger will cause objs to not be offset
    m_boundary = NULL;
  }

  CEnvLoader::~CEnvLoader() {
    Free_Memory();
  }

  //////////////////////////////////////////////////////////////////////
  // Implemetation of core function
  //////////////////////////////////////////////////////////////////////

  bool 
    CEnvLoader::ParseFile() {
      if(CheckCurrentStatus()==false)
	return false;

      //Open file for reading datat
      ifstream ifs(m_filename.c_str());

      if(ParseFileHeader(ifs)==false) 
	return false;

      if(ParseFileBody(ifs)==false) 
	return false;

      ifs.close();

      return true;
    }

  //////////////////////////////////////////////////////////////////////
  //      Protected Member Functions
  //////////////////////////////////////////////////////////////////////

  void CEnvLoader::Free_Memory() {      
    delete [] m_pMBInfo;
    m_pMBInfo=NULL;
  }

  void CEnvLoader::SetModelDataDir( const string strModelDataDir ) {
    m_strModelDataDir=strModelDataDir;
    cout<<"- Geo Dir   : "<<m_strModelDataDir<<endl;
  }

  bool CEnvLoader::ParseFileHeader(ifstream & ifs) {
    //read boundary
    string b = ReadFieldString(ifs, "Boundary Tag");
    if(b != "BOUNDARY") {
      cerr << "Error reading boundary tag." << endl;
      return false;
    }
    if(!ParseBoundary(ifs)){
      cerr << "Error parsing boundary." << endl;
      return false;
    }
    
    //read number of multibodies
    string mb = ReadFieldString(ifs, "Number of Multibodies tag");
    if(mb != "MULTIBODIES"){
      cerr << "Error reading environment multibodies tag." << endl;
      return false;
    }
    m_cNumberOfMultiBody = ReadField<int>(ifs, "Number of Multibodies");
    return true;
  }
  
  bool CEnvLoader::ParseBoundary(ifstream& _ifs) {
    string type = ReadFieldString(_ifs, "Boundary type");
    if(type == "BOX"){
      m_boundary = new BoundingBoxModel();
    }
    else if(type == "SPHERE"){
      m_boundary = new BoundingSphereModel();
    }
    else{
      cerr << "Error reading boundary type " << type << ". Choices are BOX or SPHERE." << endl;
      return false;
    }
    return m_boundary->Parse(_ifs);
  }

  bool CEnvLoader::ParseFileBody( ifstream & ifs ) {
    m_pMBInfo = new CMultiBodyInfo[m_cNumberOfMultiBody];
    if( m_pMBInfo==NULL ) return false;

    for( int iM=0; iM<m_cNumberOfMultiBody; iM++ ) {
      if( ParseMultiBody(ifs, m_pMBInfo[iM])==false )
	return false;
    }
  
    BuildRobotStructure();

    CCfg::m_dof=DoF;
    cout<< "DOF's: "<<CCfg::m_dof<<endl<<flush; 
    
    return true;
  }

  bool CEnvLoader::ParseMultiBody( ifstream & ifs, CMultiBodyInfo & MBInfo ) {
    
    string multibodyType = ReadFieldString(ifs,
        "Multibody Type (Active, Passive, Internal, Surface)");

    if(multibodyType == "ACTIVE"){
      MBInfo.m_active = true;
    }
    else if(multibodyType == "SURFACE") {
      MBInfo.m_surface = true;
      m_ContainsSurfaces = true;
    }

    if(multibodyType == "ACTIVE"){
      MBInfo.m_cNumberOfBody = ReadField<int>(ifs, "Body Count");

      MBInfo.m_pBodyInfo = new CBodyInfo[MBInfo.m_cNumberOfBody];
      if( MBInfo.m_pBodyInfo==NULL ) return false;

      GetColor(ifs);

      for( int iB=0; iB<MBInfo.m_cNumberOfBody; iB++ ) {
        if( ParseActiveBody(ifs, MBInfo.m_pBodyInfo[iB])==false )
          return false;
      }

      //get connection info
      string connectionTag = ReadFieldString(ifs, "Connections tag");
      int numberOfRobotConnections = ReadField<int>(ifs, "Number of Connections");
      MBInfo.m_NumberOfConnections = numberOfRobotConnections;

      int currentBody = 0; //index of current Body

      //Do transformation for body0, the base. This is always needed.	 
      MBInfo.m_pBodyInfo[currentBody].doTransform();

      if( numberOfRobotConnections!=0 ){
        //initialize the MBInfo.m_pBodyInfo[i].m_pConnectionInfo for each body
        for(int i=0; i<MBInfo.m_cNumberOfBody; i++){
          MBInfo.m_pBodyInfo[i].m_pConnectionInfo = 
            new CConnectionInfo[numberOfRobotConnections];

          if( MBInfo.m_pBodyInfo[i].m_pConnectionInfo==NULL ) { 
            cout<<"COULDN'T CREATE CONNECTION INFO"<<endl; return false;}
        }
        for( int iB=0; iB<numberOfRobotConnections; iB++ ){
          if( ParseConnections(ifs, MBInfo)==false )
            return false;
        }
      }
    }
    else if(multibodyType == "INTERNAL" || multibodyType == "SURFACE" ||
        multibodyType == "PASSIVE"){
      MBInfo.m_cNumberOfBody = 1;
      MBInfo.m_pBodyInfo = new CBodyInfo[MBInfo.m_cNumberOfBody];
      if( MBInfo.m_pBodyInfo==NULL ) return false;

      GetColor(ifs);

      if( ParseOtherBody(ifs, MBInfo.m_pBodyInfo[0])==false )
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

  bool CEnvLoader::ParseActiveBody( ifstream & ifs, CBodyInfo & BodyInfo ) {
    BodyInfo.m_bIsFixed = false;
    bool isBase = false;
    Robot::Base baseType;
    Robot::BaseMovement baseMovementType;

    BodyInfo.m_strFileName = ReadFieldString(ifs,
        "Body Filename (geometry file)", false);

    if( !m_strModelDataDir.empty() ) {
      //store just the path of the current directory
      BodyInfo.m_strDirectory = m_strModelDataDir;

      BodyInfo.m_strModelDataFileName += m_strModelDataDir;
      BodyInfo.m_strModelDataFileName += "/";
    }
    BodyInfo.m_strModelDataFileName += BodyInfo.m_strFileName;

    //Read for Base Type. If Planar or Volumetric, read in two more strings
    //If Joint skip this stuff. If Fixed read in positions like an obstacle
    string baseTag = ReadFieldString(ifs,
        "Base Tag (Planar, Volumetric, Fixed, Joint");
    baseType = Robot::GetBaseFromTag(baseTag);

    Vector3D bodyPosition;
    Vector3D bodyRotation;

    if(baseType == Robot::VOLUMETRIC || baseType == Robot::PLANAR){
      isBase = true;
      BodyInfo.m_IsBase = true;
      string rotationalTag = ReadFieldString(ifs,
          "Rotation Tag (Rotational, Translational");
      baseMovementType = Robot::GetMovementFromTag(rotationalTag);
    }
    else if(baseType == Robot::FIXED){
      isBase = true;
      bodyPosition = ReadField<Vector3D>(ifs, "Body Position");
      bodyRotation = ReadField<Vector3D>(ifs, "Body Orientation");
    }

    //save this for when these classes utilize only transformations instead
    //of x, y, z, alpha, beta, gamma, separately.
    /*Orientation bodyOrientation(Orientation::FixedXYZ,
      bodyRotation[2]*PI_180,
      bodyRotation[1]*PI_180,
      bodyRotation[0]*PI_180);
      Transformation transformation(bodyOrientation, bodyPosition);
     */
    BodyInfo.m_IsBase = isBase;
    BodyInfo.isBase = isBase;
    BodyInfo.SetBase(baseType);
    if (isBase) {
      BodyInfo.SetBaseMovement(baseMovementType);
    }

    BodyInfo.m_X = bodyPosition[0];
    BodyInfo.m_Y = bodyPosition[1];
    BodyInfo.m_Z = bodyPosition[2];
    BodyInfo.m_Alpha = bodyRotation[0]*PI_180;
    BodyInfo.m_Beta = bodyRotation[1]*PI_180;
    BodyInfo.m_Gamma = bodyRotation[2]*PI_180;
    BodyInfo.doTransform();

    //set color information
    if(m_color[0] != -1){
      BodyInfo.rgb[0] = m_color[0];
      BodyInfo.rgb[1] = m_color[1];
      BodyInfo.rgb[2] = m_color[2];
    }
    else{
      if(BodyInfo.m_bIsFixed){
        BodyInfo.rgb[0]=0.5;BodyInfo.rgb[1]=0.5;BodyInfo.rgb[2]=0.5;
      }
      else{
        BodyInfo.rgb[0]=1;BodyInfo.rgb[1]=0;BodyInfo.rgb[2]=0;
      }
    }	

    return true;
  }

  bool CEnvLoader::ParseOtherBody( ifstream & ifs, CBodyInfo & BodyInfo ) {
    BodyInfo.m_bIsFixed = true;
    BodyInfo.m_IsBase = true;
    BodyInfo.m_strFileName = ReadFieldString(ifs,
        "Body Filename (geometry file)", false);

    if( !m_strModelDataDir.empty() ) {
      //store just the path of the current directory
      BodyInfo.m_strDirectory = m_strModelDataDir;

      BodyInfo.m_strModelDataFileName += m_strModelDataDir;
      BodyInfo.m_strModelDataFileName += "/";
    }
    BodyInfo.m_strModelDataFileName += BodyInfo.m_strFileName;

    Vector3D bodyPosition = ReadField<Vector3D>(ifs, "Body Position");
    Vector3D bodyRotation = ReadField<Vector3D>(ifs, "Body Orientation");

    //save this for when body utilizes only transformation not
    //x,y,z,alpha,beta,gama
    /*Orientation bodyOrientation(Orientation::FixedXYZ,
      bodyRotation[2]*TWOPI/360.0,
      bodyRotation[1]*TWOPI/360.0,
      bodyRotation[0]*TWOPI/360.0);
      Transformation transformation(bodyOrientation, bodyPosition);
     */

    BodyInfo.m_X = bodyPosition[0];
    BodyInfo.m_Y = bodyPosition[1];
    BodyInfo.m_Z = bodyPosition[2];
    BodyInfo.m_Alpha = bodyRotation[0]*PI_180;
    BodyInfo.m_Beta = bodyRotation[1]*PI_180;
    BodyInfo.m_Gamma = bodyRotation[2]*PI_180;
    BodyInfo.doTransform();

    //set color information
    if(m_color[0] != -1){
      BodyInfo.rgb[0] = m_color[0];
      BodyInfo.rgb[1] = m_color[1];
      BodyInfo.rgb[2] = m_color[2];
    }
    else{
      if(BodyInfo.m_bIsFixed){
        BodyInfo.rgb[0]=0.5;BodyInfo.rgb[1]=0.5;BodyInfo.rgb[2]=0.5;
      }
      else{
        BodyInfo.rgb[0]=1;BodyInfo.rgb[1]=0;BodyInfo.rgb[2]=0;
      }
    }	

    return true;
  }

  bool CEnvLoader::ParseConnections(ifstream & ifs, CMultiBodyInfo & MBInfo) {
    //body indices
    int previousBodyIndex = ReadField<int>(ifs, "Previous Body Index");
    int nextBodyIndex = ReadField<int>(ifs, "Next Body Index");

    //Increment m_cNumberOfConnection for each body
    CBodyInfo& previousBody = MBInfo.m_pBodyInfo[previousBodyIndex];
    previousBody.m_cNumberOfConnection++;

    //Get connection index for this body
    CConnectionInfo& conn = previousBody.m_pConnectionInfo[previousBody.m_cNumberOfConnection - 1];
    Robot::Joint connPtr(&conn);
    MBInfo.jointMap.push_back(connPtr);

    //set global index
    conn.m_globalIndex = CConnectionInfo::m_globalCounter++;

    //set next and pre index
    conn.m_preIndex = previousBodyIndex;
    conn.m_nextIndex = nextBodyIndex;

    //set Actuated/NonActuated
    conn.m_actuated = true;

    //grab the joint type
    string connectionTypeTag = ReadFieldString(ifs, "Connection Type");
    conn.m_jointType = CConnectionInfo::GetJointTypeFromTag(connectionTypeTag);

    //grab the joint limits for revolute and spherical joints
    pair<double, double> jointLimits[2];
    if(conn.m_jointType == CConnectionInfo::REVOLUTE || conn.m_jointType == CConnectionInfo::SPHERICAL){
      jointLimits[0].first = jointLimits[1].first = -1;
      jointLimits[0].second = jointLimits[1].second = 1;
      size_t numRange = (conn.m_jointType == CConnectionInfo::REVOLUTE) ? 1 : 2;
      for(size_t i = 0; i<numRange; i++){
        string tok;
        if(ifs >> tok){
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

    //transformation to DHFrame
    Vector3D positionToDHFrame = ReadField<Vector3D>(ifs, "Translation to DHFrame");
    Vector3D rotationToDHFrame = ReadField<Vector3D>(ifs, "Rotation to DHFrame");

    /*Orientation orientationToDHFrame = Orientation(Orientation::FixedXYZ,
      rotationToDHFrame[2]*TWOPI/360.0,
      rotationToDHFrame[1]*TWOPI/360.0,
      rotationToDHFrame[0]*TWOPI/360.0);*/

    conn.m_pos2X = positionToDHFrame[0];
    conn.m_pos2Y = positionToDHFrame[1];
    conn.m_pos2Z = positionToDHFrame[2];

    conn.m_orient2X = rotationToDHFrame[0]*PI_180;
    conn.m_orient2Y = rotationToDHFrame[1]*PI_180;
    conn.m_orient2Z = rotationToDHFrame[2]*PI_180;

    //DH parameters
    Vector4d dhparameters = ReadField<Vector4d>(ifs, "DH Parameters");

    conn.alpha = dhparameters[0];
    conn.a = dhparameters[1];
    conn.d = dhparameters[2];
    conn.theta = dhparameters[3];

    //save original theta   
    conn.m_theta =
      conn.theta;

    //transformation to next body
    Vector3D positionToNextBody = ReadField<Vector3D>(ifs, "Translation to Next Body");
    Vector3D rotationToNextBody = ReadField<Vector3D>(ifs, "Rotation to Next Body");

    /*Orientation orientationToNextBody = Orientation(Orientation::FixedXYZ,
      rotationToNextBody[2]*TWOPI/360.0, 
      rotationToNextBody[1]*TWOPI/360.0, 
      rotationToNextBody[0]*TWOPI/360.0);*/

    conn.m_posX = positionToNextBody[0];
    conn.m_posY = positionToNextBody[1];
    conn.m_posZ = positionToNextBody[2];

    conn.m_orientX = rotationToNextBody[0]*PI_180;
    conn.m_orientY = rotationToNextBody[1]*PI_180;
    conn.m_orientZ = rotationToNextBody[2]*PI_180;

    return true;
  }


  void CEnvLoader::SetNewMultiBodyInfo(CMultiBodyInfo * mbi){
    for(int i=0; i<m_cNumberOfMultiBody; i++){

      m_pMBInfo[i] = mbi[i];
    }

  }

  void
    CEnvLoader::BuildRobotStructure() {
      DoF = 0;
      int robotIndex = 0;
      for( int i=0; i<m_cNumberOfMultiBody; i++ ) {
        if(m_pMBInfo[i].m_active){
          robotIndex = i;
          break;
        }
      }
      CMultiBodyInfo& robot = m_pMBInfo[robotIndex];
      //int fixedBodyCount = robot -> GetFixedBodyCount();
      //int freeBodyCount = robot->GetFreeBodyCount();
      //int numOfBodies = robot.m_cNumberOfBody;
      for (int i = 0; i < robot.m_cNumberOfBody; i++) {
        m_robotGraph.add_vertex(i);
      }
      //Total amount of bodies in environment: free + fixed
      for (int i = 0; i < robot.m_cNumberOfBody; i++){
        CBodyInfo& body = robot.m_pBodyInfo[i];  
        //For each body, find forward connections and connect them 
        for (int j = 0; j < body.m_cNumberOfConnection; j++) {
          int nextIndex = body.m_pConnectionInfo[j].m_nextIndex;
          m_robotGraph.add_edge(i, nextIndex);
        } 
      }

      //Robot ID typedef
      typedef RobotGraph::vertex_descriptor RID; 
      vector< pair<size_t,RID> > ccs;
      stapl::vector_property_map< RobotGraph,size_t > cmap;
      //Initialize CC information
      get_cc_stats(m_robotGraph,cmap,ccs);
      for (size_t i = 0; i < ccs.size(); i++) {
        cmap.reset();
        vector<RID> cc;
        //Find CCs, construct robot objects
        get_cc(m_robotGraph, cmap, ccs[i].second, cc);
        size_t baseIndx = -1;
        for(size_t j = 0; j<cc.size(); j++){
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
          DoF += 2;
          if(bm == Robot::ROTATIONAL){
            DoF +=1;
          }
        }
        else if(bt == Robot::VOLUMETRIC){
          DoF += 3;
          if(bm == Robot::ROTATIONAL){
            DoF += 3;
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
                DoF += 1;
              }
              else if((*mit)->m_jointType == CConnectionInfo::SPHERICAL){
                DoF += 2;
              }
            }
          }
        }
        robotVec.push_back(Robot(bt, bm, jm, baseIndx));
      }
    }

}//namespace plum

