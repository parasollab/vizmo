#include "MultiBodyInfo.h"

#include "Utilities/Exceptions.h"

//////////////////////////////////////////////////////////////////////
//
// MultiBodyInfo
//
//////////////////////////////////////////////////////////////////////

MultiBodyInfo::MultiBodyInfo(){
  m_numberOfBody = 0;
  m_mBodyInfo = NULL;
  m_active = false;
  m_surface = false;
  m_numberOfConnections = 0;
}

MultiBodyInfo::MultiBodyInfo(const MultiBodyInfo& m_other){
  *this = m_other;
}

MultiBodyInfo::~MultiBodyInfo(){
  m_numberOfBody = 0;
  if(m_mBodyInfo != NULL)
    delete [] m_mBodyInfo;
  m_mBodyInfo = NULL;
}

void 
MultiBodyInfo::operator=(const MultiBodyInfo& _other){
  m_numberOfBody = _other.m_numberOfBody;
  m_numberOfConnections = _other.m_numberOfConnections;
  m_active = _other.m_active;
  m_surface = _other.m_surface;

  m_mBodyInfo = new  BodyInfo[m_numberOfBody];

  for(int i = 0; i<m_numberOfBody; i++)
    m_mBodyInfo[i] = _other.m_mBodyInfo[i];
}

ostream& operator<<(ostream& _out, const MultiBodyInfo& _body){
  _out<< "- Number of bodies = " <<_body.m_numberOfBody << endl
    << "- Number of connections = "<<_body.m_numberOfConnections <<endl
    << "- Active/Passive = "<<_body.m_active<<endl
    << "- Surface = "<<_body.m_surface<<endl;

  for(int i = 0; i<_body.m_numberOfBody; i++)
    _out << "- Body["<< i <<"] "<< endl << _body.m_mBodyInfo[i];
  return _out;
}

BodyInfo::BodyInfo(){
  m_isFixed = false;
  m_isBase = false;
  m_transformDone = false;
  m_index    = -1;
  m_x=0;     m_y=0;    m_z=0;
  m_alpha=0; m_beta=0; m_gamma=0;

  m_numberOfConnection =0;
  m_connectionInfo = NULL;
  m_modelDataFileName[0]='\0';
  m_isNew = false;
  m_isSurface = false;
}

BodyInfo::BodyInfo(const BodyInfo& m_other){
  *this = m_other;
  m_isSurface = m_other.m_isSurface;
}

BodyInfo::~BodyInfo(){
  m_isFixed = false;
  m_isBase = false;
  m_index    = -1;
  m_x=0;     m_y=0;    m_z=0;
  m_alpha=0; m_beta=0; m_gamma=0;
  
  m_numberOfConnection =0;
  if(m_connectionInfo != NULL)
    delete [] m_connectionInfo;
  m_connectionInfo=NULL;
}

void 
BodyInfo::doTransform(){
  if(m_isBase){
    Vector3d position(m_x, m_y, m_z);
    Orientation orientation(EulerAngle(m_gamma, m_beta, m_alpha));
    m_currentTransform = Transformation(position, orientation);
  }
}

Transformation BodyInfo::getTransform(){
  return m_currentTransform;
}

void 
BodyInfo::setPrevTransform(Transformation _t){
  m_prevTransform = _t;
}

void 
BodyInfo::computeTransform(BodyInfo&BodyInfo, int _nextBody){

  int NumberOfconnections = BodyInfo.m_numberOfConnection;
  int connection=0;

  for(int j = 0; j<NumberOfconnections; j++){
    if(BodyInfo.m_connectionInfo[j].m_nextIndex == _nextBody){
      connection = j;
      break;
    }
  }

  Transformation dh = BodyInfo.m_connectionInfo[connection].DHTransform();

  Transformation Tbody2 = BodyInfo.m_connectionInfo[connection].transformToBody2();

  Transformation Tdh = BodyInfo.m_connectionInfo[connection].transformToDHframe();

  m_currentTransform = m_prevTransform * Tdh;

  m_currentTransform = m_currentTransform * dh;

  m_currentTransform = m_currentTransform * Tbody2;

}

void 
BodyInfo::operator=(const BodyInfo& _other){
  m_isFixed = _other.m_isFixed;
  m_isBase = _other.m_isBase;
  m_index = _other.m_index;
  m_x=_other.m_x;
  m_y=_other.m_y;
  m_z=_other.m_z;
  m_alpha=_other.m_alpha;
  m_beta=_other.m_beta;
  m_gamma=_other.m_gamma;
  m_isSurface=_other.m_isSurface;

  m_fileName = _other.m_fileName;
  m_directory = _other.m_directory;
  m_isNew = _other.m_isNew;

  m_modelDataFileName[0]='\0';
  m_modelDataFileName=_other.m_modelDataFileName;
  rgb[0] = _other.rgb[0];
  rgb[1] = _other.rgb[1];
  rgb[2] = _other.rgb[2];

  m_numberOfConnection =_other.m_numberOfConnection;

  m_currentTransform = _other.m_currentTransform;
  m_prevTransform = _other.m_prevTransform;

  m_connectionInfo = new ConnectionInfo[m_numberOfConnection];

  for(int i = 0; i<m_numberOfConnection; i++){

    m_connectionInfo[i] = _other.m_connectionInfo[i];

  }
}

ostream& operator<<(ostream& _out, const BodyInfo& _body){
  _out<<"-\t File name (full pth) = "<<_body.m_modelDataFileName<<endl
    <<"-\t File name (subDir and name) = "<<_body.m_fileName<<endl
    <<"-\t Location = ("<<_body.m_x<<", "<<_body.m_y<<", "<<_body.m_z<<")"<<endl
    <<"-\t Orientation = ("<<_body.m_alpha<<", "<<_body.m_beta<<", "<<_body.m_gamma<<")"<<endl
    <<"-\t Number of Connection = "<<_body.m_numberOfConnection <<endl
    <<"-\t Index = "<<_body.m_index<<endl;
  for(int i=0; i<_body.m_numberOfConnection; i++)
    _out << "-\t Connection["<< i <<"] "<< endl <<_body.m_connectionInfo[i];

  return _out;
}

//////////////////////////////////////////////////////////////////////
//
// ConnectionInfo
//
//////////////////////////////////////////////////////////////////////
ConnectionInfo::ConnectionInfo(){
  m_preIndex = -1;
  m_nextIndex= -1;
  m_posX=m_posY=m_posZ=-1;
  m_orientX=m_orientY=m_orientZ=-1;
  alpha=theta=d=a=-1;
  m_pos2X=m_pos2Y=m_pos2Z=-1;
  m_orient2X=m_orient2Y=m_orient2Z=-1;
  m_actuated = true;
}

ConnectionInfo::ConnectionInfo(const ConnectionInfo& m_other){
  *this = m_other;
}

Transformation ConnectionInfo::DHTransform(){
  Vector3d pos(a, -sin(alpha)* d, cos(alpha)* d);
  Matrix3x3 rot;
  getMatrix3x3(rot,
      cos(theta), -sin(theta), 0.0,
      sin(theta)*cos(alpha), cos(theta)*cos(alpha), -sin(alpha),
      sin(theta)*sin(alpha), cos(theta)*sin(alpha), cos(alpha));
  return Transformation(pos, Orientation(rot));
}

Transformation ConnectionInfo::transformToBody2(){
  return Transformation(
      Vector3d(m_posX, m_posY, m_posZ),
      Orientation(EulerAngle(m_orientZ, m_orientY, m_orientX)));
}

Transformation  ConnectionInfo::transformToDHframe(){
  return Transformation(
      Vector3d(m_pos2X, m_pos2Y, m_pos2Z),
      Orientation(EulerAngle(m_orient2Z, m_orient2Y, m_orient2X)));
}

void 
ConnectionInfo::operator=(const ConnectionInfo& _other){
  m_preIndex = _other.m_preIndex;
  m_nextIndex= _other.m_nextIndex;
  m_posX = _other.m_posX;
  m_posY = _other.m_posY;
  m_posZ = _other.m_posZ;
  m_orientX = _other.m_orientX;
  m_orientY = _other.m_orientY;
  m_orientZ = _other.m_orientZ;
  alpha = _other.alpha;
  theta = _other.theta;
  m_theta = _other.m_theta;
  d = _other.d;
  a = _other.a;
  m_actuated = _other.m_actuated;
  m_pos2X = _other.m_pos2X;
  m_pos2Y = _other.m_pos2Y;
  m_pos2Z = _other.m_pos2Z;
  m_orient2X = _other.m_orient2X;
  m_orient2Y = _other.m_orient2Y;
  m_orient2Z = _other.m_orient2Z;
}

ConnectionInfo::JointType
ConnectionInfo::GetJointTypeFromTag(const string _tag){
  if(_tag == "REVOLUTE")
    return ConnectionInfo::REVOLUTE;
  else if (_tag == "SPHERICAL")
    return ConnectionInfo::SPHERICAL;
  else
    throw ParseException(WHERE, "Failed parsing joint type. Choices are Revolute or Spherical.");
}

ostream& operator<<(ostream& _out, const ConnectionInfo& m_con){

  const char* s;;
  if(m_con.m_actuated)
    s = "Actuated";
  else
    s = "NonActuated";

  cout<<"-\t\t Pre Index = "<<m_con.m_preIndex<<endl
    <<"-\t\t Next Index = "<<m_con.m_nextIndex<<endl
    <<"-\t\t "<<s<<endl
    <<"-\t\t Position1 = "<<m_con.m_posX<<", "<<m_con.m_posY<<", "<<m_con.m_posZ<<endl
    <<"-\t\t Orient1 = "<<m_con.m_orientX<<", "<<m_con.m_orientY<<", "<<m_con.m_orientZ<<endl
    <<"-\t\t DH = "<<m_con.alpha<<", "<<m_con.a<<", "<<m_con.d<<", "<<m_con.theta<<endl
    <<"-\t\t Position2= "<<m_con.m_pos2X<<", "<<m_con.m_pos2Y<<", "<<m_con.m_pos2Z<<endl
    <<"-\t\t Orient2 = "<<m_con.m_orient2X<<", "<<m_con.m_orient2Y<<", "<<m_con.m_orient2Z<<endl;
  return _out;
}

size_t ConnectionInfo::m_globalCounter = 0;

