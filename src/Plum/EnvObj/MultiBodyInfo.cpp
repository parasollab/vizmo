#include "MultiBodyInfo.h"

#include "Utilities/Exceptions.h"
#include "ConnectionModel.h"
#include "BodyModel.h"

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

  m_mBodyInfo = new  BodyModel[m_numberOfBody];

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

