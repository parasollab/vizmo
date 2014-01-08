#include "QueryModel.h"

#include "Utilities/Font.h"
#include "Utilities/IO.h"

QueryModel::QueryModel(const string& _filename) :
  LoadableModel("Query"),
  m_glQueryIndex(-1) {
    SetFilename(_filename);
    m_renderMode = INVISIBLE_MODE;

    ParseFile();
    BuildModels();
  }

QueryModel::~QueryModel() {
  glDeleteLists(m_glQueryIndex, 1);
}

void
QueryModel::ParseFile() {
  //check input
  if(!FileExists(GetFilename()))
    throw ParseException(WHERE, "'" + GetFilename() + "' does not exist");

  ifstream ifs(GetFilename().c_str());

  m_cfgs.clear();

  //Build  Model
  CfgModel c;
  while(ifs >> c) {
    m_cfgs.push_back(c);
    m_cfgs.back().SetIsQuery();
  }
}

void
QueryModel::BuildModels() {

  glMatrixMode(GL_MODELVIEW);

  //create list
  glDeleteLists(m_glQueryIndex, 1);
  m_glQueryIndex = glGenLists(1);
  glNewList(m_glQueryIndex, GL_COMPILE);

  double n = m_cfgs.size() - 1;
  for(size_t i = 0; i < m_cfgs.size(); ++i) {
    m_cfgs[i].SetRenderMode(WIRE_MODE);
    m_cfgs[i].SetColor(Color4(1.0 - i/n, 0, i/n, 1));
    m_cfgs[i].DrawRobot();

    //draw text for start and goal
    //TODO: Move to using Qt functions for drawing text to scene
    Point3d pos = m_cfgs[i].GetPoint();
    glColor3d(0.1, 0.1, 0.1);
    if(i==0)
      DrawStr(pos[0]-0.5, pos[1]-0.5, pos[2], "S");
    else {
      stringstream gNum;
      gNum << "G" << i;
      DrawStr(pos[0]-0.5, pos[1]-0.5, pos[2], gNum.str());
    }
  }
  glEndList();
}

void QueryModel::Draw() {
  if(m_renderMode == INVISIBLE_MODE)
    return; //not draw anything

  glLineWidth(2.0);
  glCallList(m_glQueryIndex);
}

void
QueryModel::Print(ostream& _os) const {
  _os << Name() << ": " << GetFilename() << endl;
  for(size_t i = 0; i < m_cfgs.size(); ++i){
    if(i == 0)
      _os << "Start: ( ";
    else
      _os << "G" << i << ": ( ";
    _os << m_cfgs[i] << " )" << endl;
  }
}

void
QueryModel::SaveQuery(const string& _filename) {
  ofstream ofs(_filename.c_str());
  typedef vector<CfgModel>::iterator CIT;
  for(CIT cit = m_cfgs.begin(); cit != m_cfgs.end(); ++cit)
    ofs << *cit << endl;
}

void
QueryModel::AddCfg(int _num) {
  m_cfgs.insert(m_cfgs.begin()+_num, CfgModel());
  m_cfgs[_num+1].SetIsQuery();
}

