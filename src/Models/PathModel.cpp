#include "PathModel.h"

#include "Models/CfgModel.h"
#include "EnvObj/RobotModel.h"
#include "Utilities/IOUtils.h"
#include "Utilities/Exceptions.h"

PathModel::PathModel(const string& _filename, RobotModel* _robotModel)
  : m_glPathIndex(-1), m_robotModel(_robotModel),
  m_lineWidth(1), m_displayInterval(3) {
    SetFilename(_filename);
    m_renderMode = INVISIBLE_MODE;

    //set default stop colors for path gradient (cyan, green, yellow)
    Color4 cyan(0, 1, 1, 1), green(0, 1, 0, 1), yellow (1, 1, 0, 1);
    m_stopColors.push_back(Color4(0, 1, 1, 1));
    m_stopColors.push_back(Color4(0, 1, 0, 1));
    m_stopColors.push_back(Color4(1, 1, 0, 1));

    ParseFile();
    BuildModels();
  }

vector<string>
PathModel::GetInfo() const {
  vector<string> info;
  info.push_back(GetFilename());
  ostringstream temp;
  temp<<"There are " << m_path.size() << " path frames";
  info.push_back(temp.str());
  return info;
}

void
PathModel::ParseFile() {
  m_path.clear();

  //check input filename
  if(!FileExists(GetFilename()))
    throw ParseException(WHERE, "'" + GetFilename() + "' does not exist");

  ifstream ifs(GetFilename().c_str());

  //We throw out the first two lines, which gives path version
  //and which line to start on (always 1)
  string garbage;
  getline(ifs, garbage);
  getline(ifs, garbage);

  size_t pathSize=0;
  ifs >> pathSize;

  int dof = CfgModel::GetDOF();

  for(size_t i = 0; i < pathSize; ++i) {
    vector<double> cfg(dof);

    //need to track robot index
    //for now just discard
    int robotIndex;
    ifs >> robotIndex;

    for(int j=0; j<dof; j++)
      ifs >> cfg[j];

    m_path.push_back(cfg);
  }
}

void
PathModel::BuildModels(){
  //can't build model without robot model
  if(!m_robotModel)
    throw BuildException(WHERE, "RobotModel is null.");

  //Build Path Model
  m_robotModel->SetRenderMode(WIRE_MODE);

  Color4 oldcol = m_robotModel->GetColor(); //old color

  glMatrixMode(GL_MODELVIEW);

  m_glPathIndex = glGenLists(1);
  glNewList(m_glPathIndex, GL_COMPILE);

  typedef vector<vector<double> >::iterator PIT;
  typedef vector<Color4>::iterator CIT; //for the stop colors- small vector
  vector<Color4> allColors; //for the indiv. colors that give the gradation- large vector

  //set up all the colors for each frame along the path
  if(m_stopColors.size() > 1) {
    size_t numChunks = m_stopColors.size()-1;
    size_t chunkSize = m_path.size()/numChunks;
    for(CIT cit1 = m_stopColors.begin(), cit2 = cit1+1; cit2 != m_stopColors.end(); ++cit1, ++cit2){
      for(size_t j=0; j<chunkSize; ++j) {
        float percent = (float)j/(float)chunkSize;
        allColors.push_back(Mix(*cit1, *cit2, percent));
      }
    }
  }
  else {
    for(PIT pit = m_path.begin(); pit!=m_path.end(); ++pit)
      allColors.push_back(m_stopColors[0]);
  }

  //for each color, draw the robot at each display interval
  for(CIT cit = allColors.begin(); cit!=allColors.end(); ++cit) {
    size_t i = cit-allColors.begin();
    if(i % m_displayInterval == 0){
      m_robotModel->SetColor(*cit);
      m_robotModel->Configure(m_path[i]);
      m_robotModel->Draw(GL_RENDER);
    }
  }

  //gradient may not divide perfectly evenly, so remaining path components are
  //given the last color
  size_t remainder = m_path.size() % allColors.size();
  for(size_t j = 0; j<remainder; ++j){
    if(j%m_displayInterval==0){
      m_robotModel->SetColor(allColors.back());
      m_robotModel->Configure(m_path[allColors.size()+j]);
      m_robotModel->Draw(GL_RENDER);
    }
  }

  glEndList();

  //set back
  m_robotModel->SetRenderMode(SOLID_MODE);
  m_robotModel->SetColor(oldcol);
}

void PathModel::Draw(GLenum _mode){
  if(_mode==GL_SELECT || m_renderMode == INVISIBLE_MODE)
    return; //not draw any thing

  //set to line represnet
  glLineWidth(m_lineWidth);
  glCallList(m_glPathIndex);
}

PathModel::Color4
PathModel::Mix(Color4& _a, Color4& _b, float _percent){
  return _a*(1-_percent) + _b*_percent;
}
