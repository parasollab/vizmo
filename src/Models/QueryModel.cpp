#include "QueryModel.h"

#include <fstream>

#include "CfgModel.h"
#include "RobotModel.h"
#include "Vizmo.h"
#include "Utilities/Font.h"
#include "Utilities/IO.h"

QueryModel::QueryModel(const string& _filename, RobotModel* _robotModel) :
  LoadableModel("Query"),
  m_glQueryIndex(-1), m_robotModel(_robotModel) {
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

  int dof  = Cfg::DOF();
  //Build  Model
  while(ifs!=NULL){
    vector<double> cfg;

    //read in Robot Index and throw it away for now
    double robotIndex;
    ifs >> robotIndex;

    for(int j=0; j<dof; ++j){
      double d;
      ifs >> d;
      cfg.push_back(d);
    }
    if(ifs!=NULL){
      CfgModel* cfgModel=new CfgModel();
      cfgModel->SetCfg(cfg);
      cfgModel->SetIsQuery();
      m_cfgs.push_back(cfgModel);
    }
  }
}

void
QueryModel::BuildModels(){

  //can't build model without robot
  if(!m_robotModel)
    throw BuildException(WHERE, "RobotModel is null.");

  glMatrixMode(GL_MODELVIEW);

  m_robotModel->SetRenderMode(WIRE_MODE);

  Color4 oldCol = m_robotModel->GetColor(); //remember old color
  m_queries.clear();

  //create list
  m_glQueryIndex = glGenLists(1);
  glNewList(m_glQueryIndex, GL_COMPILE);

  for(size_t i=0; i<m_cfgs.size(); i++ ){
    vector<double> cfg = m_cfgs[i]->GetDataCfg();
    m_robotModel->Configure(cfg);
    m_queries.push_back(cfg);

    m_robotModel->SetColor(Color4(1.0-double(i/(m_cfgs.size()-1.0)),
                                  0,
                                (double(i/(m_cfgs.size()-1.0)))));

    m_robotModel->Draw(GL_RENDER);

    //draw text for start and goal
    //TODO: Move to using Qt functions for drawing text to scene
    glColor3d(0.1, 0.1, 0.1);
    if(i==0)
      DrawStr(cfg[0]-0.5, cfg[1]-0.5, cfg[2],"S");
    else{
      stringstream gNum;
      gNum<<"G"<<i;
      DrawStr(cfg[0]-0.2, cfg[1]-0.2, cfg[2],gNum.str());
    }
  }
  glEndList();

  //set back
  m_robotModel->SetColor(oldCol);
  m_robotModel->SetRenderMode(SOLID_MODE);
}

void QueryModel::Draw(GLenum _mode) {
  if(_mode == GL_SELECT || m_renderMode == INVISIBLE_MODE)
    return; //not draw anything
  glLineWidth(2.0);
  glCallList(m_glQueryIndex);
}

void
QueryModel::Print(ostream& _os) const {
  _os << Name() << ": " << GetFilename() << endl;
  _os << "Start: ( ";
  for(size_t j=0; j<m_cfgs.size(); j++){
    if(j!=0)
      _os << "G"<<j<<": ( " ;
    for(size_t i=0; i < m_queries[j].size(); ++i){
      _os << m_queries[j][i];
      if(i < m_queries[j].size()-1)
        _os << ", ";
    }
    _os << " )" << endl;
  }
}

void
QueryModel::SaveQuery(const string& _filename) {
  ofstream ofs(_filename.c_str());
  for(size_t i = 0; i < m_cfgs.size(); ++i) {
    //output robot index. For now always a 0.
    ofs << "0 ";
    //output dofs
    typedef vector<double>::const_iterator DIT;
    const vector<double>& query = m_cfgs[i]->GetDataCfg();
    for(DIT dit = query.begin(); dit != query.end(); ++dit)
      ofs << *dit << " ";
    ofs << endl;
  }
}

void
QueryModel::AddCfg(int _num){
  vector<double> cfg;
  for(int j=0; j<Cfg::DOF(); ++j)
    cfg.push_back(0);
  CfgModel* cfgModel=new CfgModel();
  cfgModel->SetCfg(cfg);
  cfgModel->SetIsQuery();
  m_cfgs.insert(m_cfgs.begin()+_num, cfgModel);
}

