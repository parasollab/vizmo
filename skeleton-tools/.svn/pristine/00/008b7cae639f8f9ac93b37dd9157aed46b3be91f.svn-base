#include "QueryModel.h"

#include "Utilities/Font.h"
#include "Utilities/IO.h"

/*------------------------------- Construction -------------------------------*/

QueryModel::
QueryModel(const string& _filename) : LoadableModel("Query") {
  SetFilename(_filename);
  m_renderMode = INVISIBLE_MODE;

  ParseFile();
  Build();
}


QueryModel::
~QueryModel() {
  glDeleteLists(m_glQueryIndex, 1);
}

/*------------------------------- Interface ----------------------------------*/

void
QueryModel::
AddCfg(int _num) {
  m_cfgs.insert(m_cfgs.begin() + _num, CfgModel());
  m_cfgs[_num].SetIsQuery();
}


void
QueryModel::
SaveQuery(const string& _filename) {
  ofstream ofs(_filename);
  for(const auto& cfg : m_cfgs)
    ofs << cfg << endl;
}

/*----------------------------- Model Overrides ------------------------------*/

void
QueryModel::
Build() {
  glMatrixMode(GL_MODELVIEW);

  // Create call list.
  glDeleteLists(m_glQueryIndex, 1);
  m_glQueryIndex = glGenLists(1);
  glNewList(m_glQueryIndex, GL_COMPILE);

  double n = m_cfgs.size() - 1;
  for(size_t i = 0; i < m_cfgs.size(); ++i) {
    m_cfgs[i].SetRenderMode(WIRE_MODE);
    m_cfgs[i].SetColor(Color4(1.0 - i/n, 0, i/n, 1));
    m_cfgs[i].DrawRender();

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


void
QueryModel::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  glDisable(GL_LIGHTING);
  glLineWidth(2.0);
  glPointSize(CfgModel::GetPointSize());
  glCallList(m_glQueryIndex);
}


void
QueryModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  glLineWidth(2.0);
  glPointSize(CfgModel::GetPointSize() + 3);
  glCallList(m_glQueryIndex);
}


void
QueryModel::
Print(ostream& _os) const {
  _os << Name() << ": " << GetFilename() << endl;
  for(size_t i = 0; i < m_cfgs.size(); ++i){
    if(i == 0)
      _os << "Start: ( ";
    else
      _os << "G" << i << ": ( ";
    _os << m_cfgs[i] << " )" << endl;
  }
}

/*-------------------------- LoadableModel Overrides -------------------------*/

void
QueryModel::
ParseFile() {
  // Check input.
  if(!FileExists(GetFilename()))
    throw ParseException(WHERE, "'" + GetFilename() + "' does not exist");

  ifstream input(GetFilename());

  m_cfgs.clear();

  CfgModel buffer;
  while(input >> buffer) {
    m_cfgs.push_back(buffer);
    m_cfgs.back().SetIsQuery();
  }
}

/*----------------------------------------------------------------------------*/
