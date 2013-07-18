#include "QueryModel.h"

#include "CfgModel.h"
#include "EnvObj/Robot.h"
#include "Utilities/IOUtils.h"
#include "Utilities/GL/gliFont.h"

QueryModel::QueryModel(string _filename) : m_glQueryIndex(-1), m_robotModel(NULL) {
  SetFilename(_filename);
  m_renderMode = INVISIBLE_MODE;
}

QueryModel::~QueryModel() {
  glDeleteLists(m_glQueryIndex, 1);
}

vector<string>
QueryModel::GetInfo() const {
  vector<string> info;
  int dof = m_robotModel->returnDOF();

  info.push_back(GetFilename());

  //output start
  ostringstream temp;
  temp << "Start = ( ";
  for(int i=0; i < dof; i++){
    temp << m_queries[0][i];
    if(i < dof-1)
      temp << ", ";
  }
  temp << " )" << endl;
  info.push_back(temp.str());

  //output goal
  temp.str("");
  temp <<"Goal = ( " ;
  for(int i=0; i<dof; i++){
    temp << m_queries[1][i];
    if(i < dof-1)
      temp << ", ";
  }
  temp << " )" << endl;
  info.push_back(temp.str());

  return info;
}

bool
QueryModel::ParseFile() {
  //check input
  if(!FileExists(GetFilename()))
    return true;

  ifstream ifs(GetFilename().c_str());

  m_queries.clear();

  //TODO: Fix to load arbitrary number of queries
  size_t numLines=2;

  //Build  Model
  int dof  = CfgModel::GetDOF();
  for(size_t i = 0; i < numLines; ++i) {
    vector<double> cfg;

    //read in Robot Index and throw it away for now
    double robotIndex;
    ifs >> robotIndex;

    for(int j=0; j<dof; ++j){
      double d;
      ifs >> d;
      cfg.push_back(d);
    }

    m_queries.push_back(cfg);
  }

  return true;
}

bool
QueryModel::BuildModels(){

  //can't build model without robot
  if(!m_robotModel)
    return false;

  glMatrixMode(GL_MODELVIEW);

  m_robotModel->SetRenderMode(WIRE_MODE);

  vector<float> col = m_robotModel->GetColor(); //remember old color
  vector<float> oldcol = col;

  m_robotModel->SetColor(0, 1, 0, 0);
  //create list
  m_glQueryIndex = glGenLists(1);
  glNewList(m_glQueryIndex, GL_COMPILE);

  for(size_t i=0; i<m_queries.size(); i++ ){
    vector<double> cfg = m_queries[i];
    m_robotModel->Configure(cfg);

    if(i==1)
      m_robotModel->SetColor(1, 0.6, 0, 0);

    m_robotModel->Draw(GL_RENDER);

    //draw text for start and goal
    //TODO: Move to using Qt functions for drawing text to scene
    glColor3d(0.1, 0.1, 0.1);
    if(i==0)
      drawstr(cfg[0]-0.5, cfg[1]-0.5, cfg[2],"S");
    else
      drawstr(cfg[0]-0.2, cfg[1]-0.2, cfg[2],"G");
  }
  glEndList();

  //set back
  m_robotModel->SetColor(oldcol[0], oldcol[1], oldcol[2], oldcol[3]);
  m_robotModel->SetRenderMode(SOLID_MODE);

  return true;
}

void QueryModel::Draw(GLenum _mode) {
  if(_mode == GL_SELECT || m_renderMode == INVISIBLE_MODE)
    return; //not draw anything
  glLineWidth(2.0);
  glCallList(m_glQueryIndex);
}

