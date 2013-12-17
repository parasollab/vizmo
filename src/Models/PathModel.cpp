#include "PathModel.h"

#include "RobotModel.h"
#include "Vizmo.h"
#include "Utilities/IO.h"

PathModel::PathModel(const string& _filename) :
  LoadableModel("Path"),
  m_glPathIndex(-1),
  m_lineWidth(1), m_displayInterval(3) {
    SetFilename(_filename);
    m_renderMode = INVISIBLE_MODE;

    //set default stop colors for path gradient (cyan, green, yellow)
    Color4 cyan(0, 1, 1, 1), green(0, 1, 0, 1), yellow (1, 1, 0, 1);
    m_stopColors.push_back(cyan);
    m_stopColors.push_back(green);
    m_stopColors.push_back(yellow);

    ParseFile();
    BuildModels();
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

  size_t pathSize;
  ifs >> pathSize;
  m_path.resize(pathSize);

  for(size_t i = 0; i < pathSize && ifs; ++i) {
    ifs >> m_path[i];
    m_path[i].SetRenderMode(WIRE_MODE);
  }
}

void
PathModel::BuildModels(){
  CfgModel::Shape tmp = CfgModel::GetShape();
  CfgModel::SetShape(CfgModel::Robot);

  glMatrixMode(GL_MODELVIEW);

  m_glPathIndex = glGenLists(1);
  glNewList(m_glPathIndex, GL_COMPILE);

  typedef vector<CfgModel>::iterator PIT;
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
      m_path[i].SetColor(*cit);
      m_path[i].Draw();
    }
  }

  //gradient may not divide perfectly evenly, so remaining path components are
  //given the last color
  size_t remainder = m_path.size() % allColors.size();
  for(size_t j = 0; j<remainder; ++j){
    if(j%m_displayInterval==0){
      m_path[allColors.size()+j].SetColor(allColors.back());
      m_path[allColors.size()+j].Draw();
    }
  }

  glEndList();

  //set back
  CfgModel::SetShape(tmp);
}

void PathModel::Draw() {
  if(m_renderMode == INVISIBLE_MODE)
    return; //not draw any thing

  //set to line represnet
  glLineWidth(m_lineWidth);
  glCallList(m_glPathIndex);
}

void
PathModel::Print(ostream& _os) const {
  _os << Name() << ": " << GetFilename() << endl
    << m_path.size() << " path frames" << endl;
}


Color4
PathModel::Mix(Color4& _a, Color4& _b, float _percent){
  return _a*(1-_percent) + _b*_percent;
}
