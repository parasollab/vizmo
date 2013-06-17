#include "PathModel.h"
#include "Robot.h"
#include "Plum/Plum.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PathModel::PathModel() 
  : m_glListIndex(-1), m_lineWidth(1),
  m_displayInterval(3), m_pathLoader(NULL), 
  m_robot(NULL) {
    m_RenderMode = INVISIBLE_MODE;
    //set default stop colors for path gradient 
    RGBAcolor cyan, green, yellow;
    cyan.push_back(0);
    cyan.push_back(1); 
    cyan.push_back(1); 
    cyan.push_back(1); 
    green.push_back(0); 
    green.push_back(1); 
    green.push_back(0);
    green.push_back(1); 
    yellow.push_back(1); 
    yellow.push_back(1); 
    yellow.push_back(0);
    yellow.push_back(1); 

    m_stopColors.push_back(cyan); 
    m_stopColors.push_back(green); 
    m_stopColors.push_back(yellow); 
  }

PathModel::~PathModel() {
    m_pathLoader=NULL;
    m_robot=NULL;
}

//////////////////////////////////////////////////////////////////////
// Action Methods
//////////////////////////////////////////////////////////////////////
bool 
PathModel::BuildModels(){   
  //can't build model without model loader
  if(m_robot==NULL || m_pathLoader==NULL) 
    return false;

  //Build Path Model
  size_t iPathSize = m_pathLoader->GetPathSize();
  m_robot->SetRenderMode(WIRE_MODE);
  
  vector<float> col = m_robot->GetColor(); //old color 
  vector<float> oldcol = col; 

  glMatrixMode(GL_MODELVIEW);
  m_glListIndex=glGenLists(1);
  glNewList(m_glListIndex, GL_COMPILE); 

  typedef vector<RGBAcolor>::iterator CIT; //for the stop colors- small vector
  vector<RGBAcolor> allColors; //for the indiv. colors that give the gradation- large vector 
  
  if(m_stopColors.size() > 1){    
    size_t numChunks = m_stopColors.size()-1; 
    size_t chunkSize = iPathSize/numChunks;
    for(CIT cit1=m_stopColors.begin(), cit2=cit1+1; cit2!=m_stopColors.end(); cit1++, cit2++){ 
      for(size_t j=0; j<chunkSize; ++j){
        float percent = (float)j/(float)chunkSize; 
        allColors.push_back(Mix(*cit1, *cit2, percent)); 
      }
    }

  }
  else if(m_stopColors.size() == 1){     
    for(size_t iP=0; iP<iPathSize; iP++)
      allColors.push_back(m_stopColors[0]);
  }

  for(size_t i = 0; i < allColors.size(); i++){
    m_robot->SetColor((allColors[i])[0], (allColors[i])[1],  
        (allColors[i])[2], (allColors[i])[3]); 
    vector<double> Cfg = m_pathLoader->GetConfiguration(i);
    if(i%m_displayInterval==0){
      m_robot->Configure(Cfg);
      m_robot->Draw(GL_RENDER); 
    }
  }
  
  //gradient may not divide perfectly evenly, so remaining path components are
  //given the last color 
  int s = allColors.size()-1; 
  size_t remainder = iPathSize % allColors.size();
  for(size_t j = 0; j<remainder; j++){
    m_robot->SetColor((allColors[s])[0], (allColors[s])[1], 
        (allColors[s])[2], (allColors[s])[3]); 
    vector<double> Cfg = m_pathLoader->GetConfiguration(allColors.size()+(j+1)); 
    if(j%m_displayInterval==0){
      m_robot->Configure(Cfg); 
      m_robot->Draw(GL_RENDER); 
    }
  }
  
  glEndList();

  //set back
  m_robot->SetRenderMode(SOLID_MODE);
  m_robot->SetColor(oldcol[0],oldcol[1],oldcol[2],oldcol[3]);

  return true;
}

void PathModel::Draw(GLenum mode){
  if(mode==GL_SELECT) 
    return; //not draw any thing
  if(m_RenderMode == INVISIBLE_MODE) 
    return;
  if(m_glListIndex==(size_t)-1)
    return;
  //set to line represnet
  glLineWidth(m_lineWidth); 
  glCallList(m_glListIndex);
}

vector<string> 
PathModel::GetInfo() const { 
  vector<string> info; 
  info.push_back(m_pathLoader->GetFilename());
  {
    ostringstream temp;
    temp<<"There are "<<m_pathLoader->GetPathSize()<<" path frames";
    info.push_back(temp.str());
  }	
  return info;
}

PathModel::RGBAcolor 
PathModel::Mix(RGBAcolor& _a, RGBAcolor& _b, float _percent){
  RGBAcolor mix;
  
  float red = _a[0]*(1-_percent) + _b[0]*(_percent);
  float green = _a[1]*(1-_percent) + _b[1]*(_percent);
  float blue = _a[2]*(1-_percent) + _b[2]*(_percent);
  float alpha = _a[3]*(1-_percent) + _b[3]*(_percent);
  
  mix.push_back(red);
  mix.push_back(green); 
  mix.push_back(blue); 
  mix.push_back(alpha);
  
  return mix;
}
