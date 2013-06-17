#include "MultiBodyInfo.h"

namespace plum{

  //////////////////////////////////////////////////////////////////////
  //
  // CMultiBodyInfo
  //
  //////////////////////////////////////////////////////////////////////

  CMultiBodyInfo::CMultiBodyInfo() {
    m_cNumberOfBody = 0;
    m_pBodyInfo = NULL;
    m_active = false;
    m_surface = false;
  }

  CMultiBodyInfo::CMultiBodyInfo( const CMultiBodyInfo & other ) {
    *this = other;
  }

  CMultiBodyInfo::~CMultiBodyInfo() {
    m_cNumberOfBody = 0;
    delete [] m_pBodyInfo;
    m_pBodyInfo = NULL;
  }

  void CMultiBodyInfo::operator=( const CMultiBodyInfo & other ) {
    m_cNumberOfBody = other.m_cNumberOfBody;
    m_NumberOfConnections = other.m_NumberOfConnections;
    m_active = other.m_active;
    m_surface = other.m_surface;

    m_pBodyInfo = new  CBodyInfo[m_cNumberOfBody];

    for( int iM=0; iM<m_cNumberOfBody; iM++ )
      m_pBodyInfo[iM] = other.m_pBodyInfo[iM];
  }

  ostream & operator<<( ostream & out, const CMultiBodyInfo & mbody ){
    out<< "- Number of bodies = " <<mbody.m_cNumberOfBody << endl 
      << "- Number of connections = "<< mbody.m_NumberOfConnections <<endl
      << "- Active/Passive = "<<mbody.m_active<<endl
      << "- Surface = "<<mbody.m_surface<<endl;

    for( int iM=0; iM<mbody.m_cNumberOfBody; iM++ )
      out << "- Body["<< iM <<"] "<< endl << mbody.m_pBodyInfo[iM];
    return out;
  }


  //////////////////////////////////////////////////////////////////////
  //
  // CBodyInfo
  //
  //////////////////////////////////////////////////////////////////////

  CBodyInfo::CBodyInfo() {
    m_bIsFixed = false;
    m_IsBase = false;
    m_transformDone= false;	
    m_Index    = -1;
    m_X=0;     m_Y=0;    m_Z=0;
    m_Alpha=0; m_Beta=0; m_Gamma=0;

    m_cNumberOfConnection =0;
    m_pConnectionInfo     =NULL;
    m_strModelDataFileName[0]='\0';
    m_isNew = false;
    m_IsSurface = false;
  }

  CBodyInfo::CBodyInfo( const CBodyInfo & other ) {
    *this = other;  
    m_IsSurface = other.m_IsSurface; 
  }

  CBodyInfo::~CBodyInfo() {
    m_bIsFixed = false;
    m_IsBase = false;
    m_Index    = -1;
    m_X=0;     m_Y=0;    m_Z=0;
    m_Alpha=0; m_Beta=0; m_Gamma=0;

    m_cNumberOfConnection =0;
    delete [] m_pConnectionInfo;
    m_pConnectionInfo=NULL;
  }

  void CBodyInfo::doTransform(){

    if(m_IsBase){
      Vector3d position;
      position.set(m_X, m_Y, m_Z);
      Orientation orientation(Orientation::FixedXYZ, m_Alpha, m_Beta, m_Gamma);
      m_currentTransform = Transformation(orientation, position);
    }


  }

  Transformation CBodyInfo::getTransform(){
    return m_currentTransform;
  }

  void CBodyInfo::setPrevTransform(Transformation t){
    m_prevTransform = t;
  }


  void CBodyInfo::computeTransform(CBodyInfo &BodyInfo, int nextBody){

    int NumberOfconnections = BodyInfo.m_cNumberOfConnection;
    int connection=0;

    for(int j=0; j<NumberOfconnections; j++){
      if(BodyInfo.m_pConnectionInfo[j].m_nextIndex == nextBody){
	connection = j;
	break;
      }
    }

    Transformation dh(BodyInfo.m_pConnectionInfo[connection].getDH());

    Transformation Tbody2 = BodyInfo.m_pConnectionInfo[connection].transformToBody2();

    Transformation Tdh = BodyInfo.m_pConnectionInfo[connection].transformToDHframe();

    m_currentTransform = m_prevTransform * Tdh;

    m_currentTransform = m_currentTransform * dh;

    m_currentTransform = m_currentTransform * Tbody2;

  }

  void CBodyInfo::operator=( const CBodyInfo & other ) {
    m_bIsFixed = other.m_bIsFixed;
    m_IsBase = other.m_IsBase; 
    m_Index = other.m_Index; 
    m_X=other.m_X; 
    m_Y=other.m_Y; 
    m_Z=other.m_Z; 
    m_Alpha=other.m_Alpha; 
    m_Beta=other.m_Beta; 
    m_Gamma=other.m_Gamma; 
    m_IsSurface=other.m_IsSurface;

    m_strFileName = other.m_strFileName;
    m_strDirectory = other.m_strDirectory;
    m_isNew = other.m_isNew;

    m_strModelDataFileName[0]='\0';
    m_strModelDataFileName=other.m_strModelDataFileName; 
    rgb[0] = other.rgb[0];
    rgb[1] = other.rgb[1];
    rgb[2] = other.rgb[2];

    m_cNumberOfConnection =other.m_cNumberOfConnection;

    m_currentTransform = other.m_currentTransform;
    m_prevTransform = other.m_prevTransform;

    m_pConnectionInfo = new CConnectionInfo[m_cNumberOfConnection];

    for( int iC=0; iC<m_cNumberOfConnection; iC++ ){

      m_pConnectionInfo[iC] = other.m_pConnectionInfo[iC];

    }
  }

  ostream & operator<<( ostream & out, const CBodyInfo & body ){

    out<<"-\t File name (full pth) = "<<body.m_strModelDataFileName<<endl
      <<"-\t File name (subDir and name) = "<<body.m_strFileName<<endl
      <<"-\t Location = ("<<body.m_X<<", "<<body.m_Y<<", "<<body.m_Z<<")"<<endl
      <<"-\t Orientation = ("<<body.m_Alpha<<", "<<body.m_Beta<<", "<<body.m_Gamma<<")"<<endl
      <<"-\t Number of Connection = "<< body.m_cNumberOfConnection <<endl
      <<"-\t Index = "<< body.m_Index<<endl;
    for( int iC=0; iC<body.m_cNumberOfConnection; iC++ )
      out << "-\t Connection["<< iC <<"] "<< endl << body.m_pConnectionInfo[iC];

    return out;
  }

  //////////////////////////////////////////////////////////////////////
  //
  // ConnectionInfo
  //
  //////////////////////////////////////////////////////////////////////
  CConnectionInfo::CConnectionInfo() {
    m_preIndex = -1;
    m_nextIndex= -1;
    m_posX=m_posY=m_posZ=-1;
    m_orientX=m_orientY=m_orientZ=-1;
    alpha=theta=d=a=-1;
    m_pos2X=m_pos2Y=m_pos2Z=-1;
    m_orient2X=m_orient2Y=m_orient2Z=-1;
    m_actuated = true;
  }

  CConnectionInfo::CConnectionInfo( const CConnectionInfo & other ) {
    *this = other;
  }

  Vector4d CConnectionInfo::getDH(){

    Vector4d v; 
    v.set(alpha, a, d, theta);
    return v;
  }

  Transformation CConnectionInfo::transformToBody2(){

    Vector3d position;
    position.set(m_posX, m_posY, m_posZ);

    Orientation orientation(Orientation::FixedXYZ, m_orientX, m_orientY, m_orientZ);
    Transformation x(orientation, position);

    return x;

  }

  Transformation  CConnectionInfo::transformToDHframe(){

    Vector3d position;
    position.set(m_pos2X, m_pos2Y, m_pos2Z);

    Orientation orientation(Orientation::FixedXYZ, m_orient2X, m_orient2Y, m_orient2Z);
    Transformation t(orientation,position);

    return t;

  }


  void CConnectionInfo::operator=( const CConnectionInfo & other ) {
    m_preIndex = other.m_preIndex;
    m_nextIndex= other.m_nextIndex; 
    m_posX = other.m_posX;
    m_posY = other.m_posY;
    m_posZ = other.m_posZ;
    m_orientX = other.m_orientX;
    m_orientY = other.m_orientY;
    m_orientZ = other.m_orientZ;
    alpha = other.alpha;
    theta = other.theta;
    m_theta = other.m_theta;
    d = other.d;
    a = other.a;
    m_actuated = other.m_actuated;
    m_pos2X = other.m_pos2X;
    m_pos2Y = other.m_pos2Y;
    m_pos2Z = other.m_pos2Z;
    m_orient2X = other.m_orient2X;
    m_orient2Y = other.m_orient2Y;
    m_orient2Z = other.m_orient2Z;
  }

  CConnectionInfo::JointType
  CConnectionInfo::GetJointTypeFromTag(const string _tag){
    if(_tag == "REVOLUTE")
      return CConnectionInfo::REVOLUTE;
    else if (_tag == "SPHERICAL")
      return CConnectionInfo::SPHERICAL;
    else {
      cerr << "Error::Incorrect Joint Type Specified::" << _tag << endl;
      cerr << "Choices are:: Revolute or Spherical" << endl;
      exit(1);
    }
  }

  ostream & operator<<( ostream & out, const CConnectionInfo & con ){

    const char* s;;
    if(con.m_actuated)
      s = "Actuated";
    else
      s = "NonActuated";

    cout<<"-\t\t Pre Index = "<<con.m_preIndex<<endl
      <<"-\t\t Next Index = "<<con.m_nextIndex<<endl
      <<"-\t\t "<<s<<endl
      <<"-\t\t Position1 = "<<con.m_posX<<", "<<con.m_posY<<", "<< con.m_posZ<<endl
      <<"-\t\t Orient1 = "<<con.m_orientX<<", "<<con.m_orientY<<", "<<con.m_orientZ<<endl
      <<"-\t\t DH = "<<con.alpha<<", "<<con.a<<", "<<con.d<<", "<<con.theta<<endl
      <<"-\t\t Position2= "<<con.m_pos2X<<", "<<con.m_pos2Y<<", "<<con.m_pos2Z<<endl
      <<"-\t\t Orient2 = "<<con.m_orient2X<<", "<<con.m_orient2Y<<", "<<con.m_orient2Z<<endl;
    return out;
  }

  size_t CConnectionInfo::m_globalCounter = 0;
}//end of plum

