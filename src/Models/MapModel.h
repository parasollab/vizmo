#if !defined(_MAPMODEL_H_)
#define _MAPMODEL_H_

#include <math.h>

#include <graph.h>
#include <algorithms/graph_algo_util.h>
#include <algorithms/graph_input_output.h>

#include "EnvObj/Robot.h"
#include "Plum/PlumObject.h" 
#include "CCModel.h"
#include "CfgModel.h"
#include "EdgeModel.h"

using namespace stapl;
using namespace std;

template<typename, typename>
class CCModel;

template <class CfgModel, class WEIGHT>
class MapModel : public plum::GLModel{
   
  public: 
    typedef CCModel<CfgModel, WEIGHT> CCM;
    typedef graph<DIRECTED, MULTIEDGES, CfgModel, WEIGHT> Wg;
    typedef typename Wg::vertex_descriptor VID;
    typedef vector_property_map<Wg, size_t> ColorMap;
    ColorMap m_colorMap;
    typedef typename Wg::vertex_iterator VI;
  
    MapModel();
    virtual ~MapModel();

    //Access functions 
    virtual const string GetName() const {return "Map";}
    const string  GetVersionNumber() const{ return m_versionNumber; }
    const string  GetPreamble() const{ return m_preamble; }
    const string  GetEnvFileName() const{ return m_envFileName; }
    const string  GetFileDir() const{ return m_fileDir; }
    const list<string>& GetLPs() const{ return m_lPs; }
    const list<string>& GetCDs() const{ return m_cDs; }
    const list<string>& GetDMs() const{ return m_dMs; }
    const string GetSeed() const {return m_seed; }
    Wg* GetGraph(){ return m_graph; }
    void SetRobotModel(OBPRMView_Robot* _robot){ m_robot = _robot; }     
    vector<CCM*>& GetCCModels(){ return m_cCModels; }
    list<GLModel*>& GetNodeList(){ return m_nodes; } 
    vector<GLModel*>& GetNodesToConnect(){ return m_nodesToConnect; } 
    void SetMBEditModel(bool _setting){ m_editModel = _setting; } 
    //void SetSize(double _size){ m_size = _size; }
    CCM* GetCCModel(int _id){ return m_cCModels[_id]; }
    int NumberOfCC(){ return m_cCModels.size(); }
    bool RobCfgOn() { return m_robCfgOn; }
    void SetAddNode(bool _setting) { m_addNode = _setting; }
    void SetAddEdge(bool _setting) { m_addEdge = _setting; }


    //Load functions
    //Moving generic load functions to virtual in GLModel.h 
    void InitGraph(){ m_graph = new Wg(); }
    //void WriteMapFile(const char *filename);
    bool FileExists() const; 
    void GoToNext(istream& _in); 
    bool IsCommentLine(char _c); 
    string GetPathName(const string& _filename); 
    bool ParseHeader(); 
    bool ParseHeader(istream& _in);
    virtual bool ParseFile(); 
    virtual bool ParseFile(const string& _filename); 
    bool WriteMapFile();
    VID Cfg2VID(CfgModel _target);
    void GenGraph(); 
    
    //Display fuctions 
    virtual bool BuildModels();
    virtual void Draw(GLenum _mode);
    void Select(unsigned int* _index, vector<gliObj>& _sel);
    virtual void SetRenderMode(RenderMode _mode); //Wire, solid, or invisible 
    bool AddCC(int _vid); 
    virtual void GetChildren(list<GLModel*>& _models); 
    virtual vector<string> GetInfo() const;
    void SetProperties(typename CCM::Shape _s, float _size, vector<float> _color, bool _isNew); 
    //  void HandleSelect(); ORIGINALLY IN ROADMAP.H/.CPP  
    //  void HandleAddEdge();
    //  void HandleAddNode();
    //  void HandleEditMap();
    //  void MoveNode(); 

  private:    
    string  m_versionNumber;
    string  m_preamble;
    string  m_envFileName;
    string  m_fileDir;
    list<string> m_lPs;
    list<string> m_cDs;
    list<string> m_dMs;
    string m_seed;
    bool m_robCfgOn;   
    bool m_addNode; 
    bool m_addEdge;
    double* m_cfg; 
    int m_dof; 
    OBPRMView_Robot* m_robot;
    vector<CCM*> m_cCModels;
    list<GLModel*> m_nodes; //moved from obsolete Roadmap.h! 
    vector<GLModel*> m_nodesToConnect; //nodes to be connected
    double m_oldT[3], m_oldR[3];  //old_T 
    //double m_size;                //check purpose of this variable  
    string m_cfgString, m_robCfgString; //s_cfg, s_robCfg  
    bool m_editModel;
    bool m_noMap; 

  protected:
    Wg* m_graph;
    //virtual void DumpNode();
    //virtual void SelectNode( bool bSel );
};

template <class CfgModel, class WEIGHT>
MapModel<CfgModel, WEIGHT>::MapModel(){
  
  m_renderMode = INVISIBLE_MODE;
  m_robot = NULL;
  m_graph = NULL; 
  m_enableSelection = true; //disable selection
  m_editModel = false;
  m_addNode = false;
  m_addEdge = false;
  m_robCfgOn = false; 
  m_robCfgString = "";
  m_noMap = false;
  //m_size = 0.5; 

  //It may be preferable to do all model ParseFiles within constructors,
  //but for now it seems there are some problematic dependencies 
  //ParseFile(_filename); 
}

template <class CfgModel, class WEIGHT>
MapModel<CfgModel, WEIGHT>::~MapModel(){

  typedef typename vector<CCM*>::iterator CIT;
  for(CIT ic = m_cCModels.begin(); ic != m_cCModels.end(); ic++)
    delete *ic;
  delete m_graph; 
  m_graph = NULL;
}

///////////Load functions////////// 

template <class CfgModel, class WEIGHT>
bool
MapModel<CfgModel, WEIGHT>::FileExists() const{

  //Check if file exists
  ifstream fin(GetFilename().c_str());
  if(!fin.good()){
    cerr << "File (" << GetFilename() << ") not found";
    return false;
  }
  return true;
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::GoToNext(istream& _in){

  string line;
  while(!_in.eof()){
    char c;
    while(isspace(_in.peek()))
    _in.get(c);
    
    c = _in.peek();
    if(!IsCommentLine(c))
      return;
    else
      getline(_in, line);
  }
}

template <class CfgModel, class WEIGHT>
bool
MapModel<CfgModel, WEIGHT>::IsCommentLine(char _c){
  return _c == '#'; 
}

template <class CfgModel, class WEIGHT>
string
MapModel<CfgModel, WEIGHT>::GetPathName(const string& _filename){
  
  size_t pos = _filename.rfind('/');
  if(pos == string::npos)
    return "";
  return _filename.substr(0, pos+1);
}

template <class CfgModel, class WEIGHT>
bool
MapModel<CfgModel, WEIGHT>::ParseHeader(){

  if(FileExists() == false)
    return false;

  ifstream fin(GetFilename().c_str());
  bool result = ParseHeader(fin);
  fin.close();
  return result;
}

template <class CfgModel, class WEIGHT>
bool 
MapModel<CfgModel, WEIGHT>::ParseHeader(istream& _in){
  
  m_fileDir = GetPathName(GetFilename());

  //Open file for reading data
  string s;

  //Get version comment
  GoToNext(_in);
  _in >> s >> s >> s >> s; //Roadmap Version Number 061300
  m_versionNumber = s;

  //Get preamble info
  GoToNext(_in);
  getline(_in, s);
  m_preamble = s;

  //Get env file name info
  GoToNext(_in);
  getline(_in, s);
  
  //See if we need to add directory
  if(s[0] != '/')
    m_envFileName = m_fileDir + s; 
  else
    m_envFileName = s;

  unsigned int number = 0;
  //get lp info
  GoToNext(_in);
  _in >> number; //# of lps
  
  for(unsigned int iLP = 0; iLP < number; iLP++){
    GoToNext(_in);
    getline(_in, s);
    if(m_lPs.size() != number)
      m_lPs.push_back(s);
  }

  //get cd info  
  GoToNext(_in);
  _in >> number; //# of lps
  for(unsigned int iCD = 0; iCD < number; iCD++){
    GoToNext(_in);
    getline(_in, s);
    if(m_cDs.size() != number)
      m_cDs.push_back(s);
  }

  //get dm info
  GoToNext(_in);
  _in >> number;
  for(unsigned int iDM = 0; iDM < number; iDM++){
    GoToNext(_in);
    getline(_in, s);
    if(m_dMs.size() != number) 
      m_dMs.push_back(s);   
  }

  // June 17-05
  // Supports new version of map generation:
  // ask whether this has the RNGSEEDSTART tag:
  string s_ver = m_versionNumber;
  if(s_ver == "041805"){
    GoToNext(_in);
    getline(_in, s);
    m_seed = s;
  }
  return true;
}

template<class CfgModel, class WEIGHT>
bool
MapModel<CfgModel,WEIGHT>::ParseFile(){
  return ParseFile(GetFilename());     
}

template<class CfgModel, class WEIGHT>
bool 
MapModel<CfgModel, WEIGHT>::ParseFile(const string& _filename){      
  
  if(FileExists() == false)
    return false;

  ifstream fin(GetFilename().c_str());
  if(ParseHeader(fin) == false) 
    return false;

  //Get Graph Data
  string s;
  getline(fin, s);
  m_graph  = new Wg();
  if(m_graph == NULL) 
    return false; 
  
  read_graph(*m_graph, fin);
  fin.close(); 
  return true;
}

template <class CfgModel, class WEIGHT>
bool 
MapModel<CfgModel, WEIGHT>::WriteMapFile(){

  //m_graph->WriteGraph(filename);
  return true;
}

template <class CfgModel, class WEIGHT>
//VID 
typename graph<DIRECTED, MULTIEDGES, CfgModel, WEIGHT>::vertex_descriptor 
MapModel<CfgModel, WEIGHT>::Cfg2VID(CfgModel _target){
  
  VI vi;
  //typename VID tvid = -1;
  typename graph<DIRECTED, MULTIEDGES, CfgModel, WEIGHT>::vertex_descriptor tvid = -1; 
  for(vi = m_graph->begin(); vi != m_graph->end(); vi++){
    if(_target == (*vi).property()){
      tvid=(*vi).descriptor();   
      break;
    }
  }
  return tvid;
}

template<class CfgModel, class WEIGHT> 
void
MapModel<CfgModel, WEIGHT>::GenGraph(){
  m_graph  = new Wg();
}


//////////Display functions//////////

template <class CfgModel, class WEIGHT>
bool 
MapModel<CfgModel, WEIGHT>::BuildModels() {
  
  typedef typename vector<CCM*>::iterator CCIT; 
  
  if(m_graph == NULL)
    return false;
  
  for(CCIT ic = m_cCModels.begin(); ic != m_cCModels.end(); ic++)
    delete (*ic);
  
  m_cCModels.clear(); //new line Jul-01-05
  
  //Get CCs
  typedef typename vector< pair<size_t,VID> >::iterator CIT; 
  vector<pair<size_t,VID> > ccs;
  m_colorMap.reset();
  get_cc_stats(*m_graph, m_colorMap, ccs);  

  int CCSize = ccs.size();
  m_cCModels.reserve(CCSize);
  for(CIT ic = ccs.begin(); ic != ccs.end(); ic++){
    CCM* cc = new CCM(ic-ccs.begin());
    cc->SetRobotModel(m_robot);    
    cc->BuildModels(ic->second, m_graph);   
    m_cCModels.push_back(cc);
  }
  return true;
}

template <class CfgModel, class WEIGHT>
void 
MapModel<CfgModel, WEIGHT>::Draw(GLenum _mode){

    if(m_renderMode == INVISIBLE_MODE)
      return;
    if(_mode==GL_SELECT && !m_enableSelection)
      return; 
    //Draw each CC
    int size = 0;
    typedef typename vector<CCM*>::iterator CIT;//CC iterator
    for(CIT ic = m_cCModels.begin(); ic != m_cCModels.end(); ic++){
      if(_mode == GL_SELECT) 
        glPushName((*ic)->GetID()); 
      (*ic)->Draw(_mode);
      if(_mode == GL_SELECT) 
        glPopName();
      size++;
    }
}

template <class CfgModel, class WEIGHT>
void 
MapModel<CfgModel, WEIGHT>::SetRenderMode(RenderMode _mode){ 
  m_renderMode = _mode;
  typedef typename vector<CCM*>::iterator CIT;//CC iterator
  for(CIT ic = m_cCModels.begin(); ic != m_cCModels.end(); ic++){
    (*ic)->SetRenderMode(_mode);
  }
}


template <class CfgModel, class WEIGHT>
bool 
MapModel<CfgModel, WEIGHT>::AddCC(int _vid){
  
  if(m_graph == NULL) 
    return false;
  
  CCM* cc = new CCM(m_cCModels.size());
  cc->RobotModel(m_robot);  
  cc->BuildModels(_vid, m_graph); 
  
  float size;
  vector<float> color; 
  if(m_cCModels[m_cCModels.size()-1]->getShape() == 0)
    size = m_cCModels[m_cCModels.size()-1]->getRobotSize();
  else if(m_cCModels[m_cCModels.size()-1]->getShape() == 1)
    size = m_cCModels[m_cCModels.size()-1]->getBoxSize();
  else
    size = 0.0;

  color =  m_cCModels[m_cCModels.size()-1]->getColor();
  cc->change_properties(m_cCModels[m_cCModels.size()-1]->getShape(),size,
    color, true);
  
  m_cCModels.push_back(cc);
  return true;
}

template <class CfgModel, class WEIGHT>
void 
MapModel<CfgModel, WEIGHT>::GetChildren(list<GLModel*>& _models){

  typedef typename vector<CCM*>::iterator CIT;
  for(CIT ic = m_cCModels.begin(); ic != m_cCModels.end(); ic++)
    _models.push_back(*ic);
}  

template <class CfgModel, class WEIGHT>
vector<string> 
MapModel<CfgModel, WEIGHT>::GetInfo() const{ 
  
  vector<string> info;
  info.push_back(GetFilename());
  ostringstream temp;
  temp<< "There are " << m_cCModels.size() << " connected components";
  info.push_back(temp.str());
  return info;
}

template <class CfgModel, class WEIGHT>
void 
MapModel<CfgModel, WEIGHT>::SetProperties(typename CCM::Shape _s, float _size, 
              vector<float> _color, bool _isNew){
  
  typedef typename vector<CCM*>::iterator CIT;//CC iterator
  for(CIT ic = m_cCModels.begin(); ic!= m_cCModels.end(); ic++)
    (*ic)->change_properties(_s, _size, _color, _isNew);
}

template <class CfgModel, class WEIGHT>
void 
MapModel<CfgModel, WEIGHT>::Select(unsigned int* _index, vector<gliObj>& _sel){

  if(_index == NULL) 
    return;
  m_cCModels[_index[0]]->Select(&_index[1],_sel); 
}

  //Commented out functions below are from Roadmap.h/.cpp and did not work there
  //(or need other attn.) 
  //They should probably be here when fixed. 

  /* template <class CfgModel, class WEIGHT>
     void MapModel<CfgModel, WEIGHT>::HandleSelect(){

  //find nodes
  m_nodes.clear();
  vector<gliObj>& sel = this->GetVizmo().GetSelectedItem();
  typedef vector<gliObj>::iterator OIT;

  for(OIT i=sel.begin(); i!=sel.end(); i++){  //GETS THE NODES FROM SELECTED ITEM AND PUTS THEM IN NODE VECTOR 
  string myName = ((GLModel*)(*i))->GetName();
  if(((GLModel*)(*i)) != NULL)
  if(myName.find("Node")!=string::npos){
  m_nodes.push_back((GLModel*)(*i));
  }//end if

  if(m_robCfgOn){
  this->GetVizmo().getRoboCfg();
  //  printRobCfg(); MAY NEED TO RESTORE THIS 
  }
  }//end for

  if(!m_editModel){return;}

  if(m_nodes.size() > 0){
  GLModel* n = m_nodes.front();
  //   printNodeCfg((CfgModel*)n); MAY NEED TO RESTORE THIS
  }

  if(m_addEdge)
  this->HandleAddEdge();
  else if(m_addNode)   
  this->HandleAddNode();
  // else
  //   handleEditMap();
  }*/

  /*  template <class CfgModel, class WEIGHT>
      void MapModel<CfgModel, WEIGHT>::HandleAddEdge(){

//find two nodes...
PlumObject* m_Map;
m_Map = this->GetVizmo().GetMap();
CfgModel *cfg1, *cfg2;

typedef CMapLoader<CfgModel,EdgeModel>::Wg WG;
WG* graph;
CMapLoader<CfgModel,EdgeModel>* m_loader=(CMapLoader<CfgModel,EdgeModel>*)m_Map->getLoader();
graph = m_loader->GetGraph();

//get from m_nodes node1 and node2
//get VID from those Cfgs
//add edge

list<GLModel*>::const_iterator it;
for(it = m_nodes.begin(); it != m_nodes.end(); it++){
m_nodesToConnect.push_back(*it);
}
if(m_nodesToConnect.size() == 2){
cfg1 = (CfgModel*)m_nodesToConnect[0];
cfg2 = (CfgModel*)m_nodesToConnect[1];
graph->add_edge(cfg1->GetIndex(), cfg2->GetIndex(),1);  
  //////////  Jun 16-05 ///////////////
  // Add edge to CCModel:
  // get a CC id
  int CC_id = cfg1->GetCC_ID();
  //get mapModel
  MapModel<CfgModel,EdgeModel>* mmodel =(MapModel<CfgModel,EdgeModel>*)m_Map->getModel();
  //get the CCModel of CfgModel
  CCModel<CfgModel,EdgeModel>* m_CCModel = mmodel->GetCCModel(CC_id);
  //add edge to CC 
  //m_CCModel->addEdge(cfg1, cfg2); Jul 17-12 

  //backUp current prpoperties:
  CCModel<CfgModel,EdgeModel>::Shape shape = m_CCModel->getShape();
  float size;
  if(shape == 0)
  size = m_CCModel->getRobotSize();
  else if (shape == 1)
  size = m_CCModel->getBoxSize();
  else
  size = 0;
  vector<float> rgb; 
  rgb = m_CCModel->getColor();

  mmodel->BuildModels();
  mmodel->SetProperties(shape, size, rgb, false);
  //emit callUpdate();
  //  'UpdateNodeCfg();' //FIX THIS!!! 

  m_nodesToConnect.clear();
  }
  }

  template <class CfgModel, class WEIGHT>
  void MapModel<CfgModel, WEIGHT>::HandleAddNode(){

  vector<gliObj>& sel = this->GetVizmo().GetSelectedItem();
  if(sel.size() !=0){ 
  if(!m_nodes.empty()){
  GLModel* n = m_nodes.front(); 
  CfgModel* cfg = (CfgModel*)n;   
  //get current node's cfg
  vector<double> c = cfg->GetDataCfg();
  m_cfg = new double [c.size()];
  m_dof = c.size();

  for(unsigned int i=0; i<c.size(); i++){
  m_cfg[i] = c[i];
}
//create a window to let user change Cfg:
//  createWindow();
// to avoid add other more nodes every time the user clicks on
// this cfg.
//addNodeAction->setChecked(false);
m_addNode = false;
//  m_messageLabel->clear();
//  m_iconLabel->clear();
}

else{ //no node selected and assumes there is not roadmap....

  if (this->GetVizmo().GetMap() == NULL) {
    CMapLoader<CfgModel,EdgeModel>* mloader=new CMapLoader<CfgModel,EdgeModel>();
    MapModel<CfgModel,EdgeModel>* mmodel = new MapModel<CfgModel,EdgeModel>();

    mmodel->SetMapLoader(mloader);

    PlumObject* m_Rob;
    m_Rob = this->GetVizmo().GetRobot();
    OBPRMView_Robot* r = (OBPRMView_Robot*)m_Rob->getModel();

    if(r != NULL)
      mmodel->SetRobotModel(r);

    this->GetVizmo().setMapObj(mloader, mmodel); 
    mloader->genGraph();

    //add node to graph
    typedef CMapLoader<CfgModel,EdgeModel>::Wg WG;
    WG* graph;
    graph = mloader->GetGraph();
    CfgModel* cfgNew = new CfgModel(); 
    //get robot's current cfg
    m_dof = r->returnDOF();

    vector<double> rCfg = r->getFinalCfg();
    vector<double> tmp;
    tmp.clear();
    for(int i=0; i<m_dof; i++){
      if(i==0)
        tmp.push_back(rCfg[i]+1);
      else
        tmp.push_back(rCfg[i]);
    }
    cfgNew->SetDof(m_dof);
    cfgNew->SetCfg(tmp);
    int vertx = graph->add_vertex(*cfgNew);
    cfgNew->SetIndex(vertx);

    mmodel->BuildModels();
    this->GetVizmo().ShowRoadMap(true);

    cfgNew->SetCCModel(mmodel->GetCCModel(mmodel->number_of_CC()-1));

    //uselect
    this->GetVizmo().cleanSelectedItem();
    //select new node
    this->GetVizmo().addSelectedItem((GLModel*)cfgNew);
    vector<double> cf = cfgNew->GetDataCfg();
    m_cfg = new double [cf.size()];
    for(unsigned int i=0; i<cf.size(); i++)
      m_cfg[i] = cf[i];

    m_noMap = true;
    // createWindow(); 
    //now, shape automatically set upon construction of RoadmapOptions class  
    // setShape(); //clicks the robot button 
    //   emit getSelectedItem();
    this->GetVizmo().Display();
    //addNodeAction->setChecked(false);
    m_addNode = false;
    //  m_messageLabel->clear();
    //  m_iconLabel->clear();
    m_noMap = false;
  }
}
}
}
*/
/*
   template <class CfgModel, class WEIGHT>
   void MapModel<CfgModel, WEIGHT>::HandleEditMap(){

   if(m_nodes.empty()==false){
   GLModel* n = m_nodes.front();
   m_oldT[0] = n->tx(); 
   m_oldT[1] = n->ty(); 
   m_oldT[2] = n->tz();

   m_oldR[0] = n->rx(); 
   m_oldR[1] = n->ry(); 
   m_oldR[2] = n->rz();     
   }
   }
   */

/*template <class CfgModel, class WEIGHT>
  void MapModel<CfgModel, WEIGHT>::MoveNode(){

  if(m_Nodes.empty()) return;
  GLModel * n=m_Nodes.front();
  double diff = fabs(m_oldT[0]-n->tx())+
  fabs(m_oldT[1]-n->ty())+
  fabs(m_oldT[2]-n->tz())+
  fabs(m_oldR[0]-n->rx())+
  fabs(m_oldR[1]-n->ry())+
  fabs(m_oldR[2]-n->rz());

  if(diff > 1e-10){
  vector<gliObj>& sel=GetVizmo().GetSelectedItem();
  if(sel.size() !=0){
  GetVizmo().Node_CD((CfgModel*)n);
  }

  typedef vector<gliObj>::iterator OIT;
  for(OIT i=sel.begin();i!=sel.end();i++){
  if(((GLModel*)(*i))->GetName()=="Node") 
  printNodeCfg((CfgModel*)n);
  }

  if(nodeGUI!= NULL){
  if(nodeGUI->isVisible()){
  v_cfg = ((CfgModel*)n)->GetDataCfg();
  double* new_cfg = new double [v_cfg.size()];
  for(unsigned int v= 0; v<v_cfg.size(); v++)
  new_cfg[v] = v_cfg[v];
  nodeGUI->setNodeVal(v_cfg.size(), new_cfg);
  nodeGUI->filledFirstTime = false;
  }
  }
  m_Map_Changed=true;

//  emit callUpdate(); ****CHECK THIS**********
}   
}*/
  
//  void 
//  TestMapModel(){
//    MapModel<CfgModel,EdgeModel> model;
//  }
//} //namespace plum 

#endif 
