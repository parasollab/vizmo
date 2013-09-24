#include "QueryModel.h"

#include "CfgModel.h"
#include "Vizmo.h"
#include "EnvObj/RobotModel.h"
#include "Utilities/IOUtils.h"
#include "Utilities/Exceptions.h"
#include "Utilities/GL/Font.h"

QueryModel::QueryModel(const string& _filename, RobotModel* _robotModel) :
  m_glQueryIndex(-1), m_robotModel(_robotModel) {
    SetFilename(_filename);
    m_renderMode = INVISIBLE_MODE;

    ParseFile();
    BuildModels();
  }

QueryModel::~QueryModel() {
  glDeleteLists(m_glQueryIndex, 1);
}

vector<string>
QueryModel::GetInfo() const {
  vector<string> info;
  int dof = GetVizmo().GetEnv()->GetDOF();

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

void
QueryModel::ParseFile() {
  //check input
  if(!FileExists(GetFilename()))
    throw ParseException(WHERE, "'" + GetFilename() + "' does not exist");

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
}

void
QueryModel::BuildModels(){

  //can't build model without robot
  if(!m_robotModel)
    throw BuildException(WHERE, "RobotModel is null.");

  glMatrixMode(GL_MODELVIEW);

  m_robotModel->SetRenderMode(WIRE_MODE);

  Color4 oldcol = m_robotModel->GetColor(); //remember old color

  m_robotModel->SetColor(Color4(0, 1, 0, 0));
  //create list
  m_glQueryIndex = glGenLists(1);
  glNewList(m_glQueryIndex, GL_COMPILE);

  for(size_t i=0; i<m_queries.size(); i++ ){
    vector<double> cfg = m_queries[i];
    m_robotModel->Configure(cfg);

    if(i==1)
      m_robotModel->SetColor(Color4(1, 0.6, 0, 0));

    m_robotModel->Draw(GL_RENDER);

    //draw text for start and goal
    //TODO: Move to using Qt functions for drawing text to scene
    glColor3d(0.1, 0.1, 0.1);
    if(i==0)
      DrawStr(cfg[0]-0.5, cfg[1]-0.5, cfg[2],"S");
    else
      DrawStr(cfg[0]-0.2, cfg[1]-0.2, cfg[2],"G");
  }
  glEndList();

  //set back
  m_robotModel->SetColor(oldcol);
  m_robotModel->SetRenderMode(SOLID_MODE);
}

void QueryModel::Draw(GLenum _mode) {
  if(_mode == GL_SELECT || m_renderMode == INVISIBLE_MODE)
    return; //not draw anything
  glLineWidth(2.0);
  glCallList(m_glQueryIndex);
}

