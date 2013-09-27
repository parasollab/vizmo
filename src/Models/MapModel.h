#ifndef MAPMODEL_H_
#define MAPMODEL_H_

#include <graph.h>
#include <algorithms/graph_algo_util.h>
#include <algorithms/graph_input_output.h>
using namespace stapl;

#include "CCModel.h"
#include "CfgModel.h"
#include "EdgeModel.h"
#include "RobotModel.h"
#include "Utilities/IOUtils.h"

template<typename, typename>
class CCModel;

template <class CfgModel, class WEIGHT>
class MapModel : public LoadableModel {

  public:
    typedef CCModel<CfgModel, WEIGHT> CCM;
    typedef graph<DIRECTED, MULTIEDGES, CfgModel, WEIGHT> Wg;
    typedef typename Wg::vertex_descriptor VID;
    typedef vector_property_map<Wg, size_t> ColorMap;
    ColorMap m_colorMap;
    typedef typename Wg::vertex_iterator VI;
    typedef typename CfgModel::Shape Shape;

    MapModel(RobotModel* _robotModel);
    //constructor only to grab header environment name
    MapModel(const string& _filename);
    MapModel(const string& _filename, RobotModel* _robotModel);
    virtual ~MapModel();

    //Access functions
    const string  GetEnvFileName() const{ return m_envFileName; }
    const string  GetFileDir() const{ return m_fileDir; }
    Wg* GetGraph(){ return m_graph; }
    vector<CCM*>& GetCCModels(){ return m_ccModels; }
    list<Model*>& GetNodeList(){ return m_nodes; }
    vector<Model*>& GetNodesToConnect(){ return m_nodesToConnect; }
    void SetMBEditModel(bool _setting){ m_editModel = _setting; }
    CCM* GetCCModel(int _id){ return m_ccModels[_id]; }
    int NumberOfCC(){ return m_ccModels.size(); }
    bool RobCfgOn() { return m_robCfgOn; }
    void SetEdgeThickness(double _thickness){ EdgeModel::m_edgeThickness = _thickness; }
    void SetNodeShape(Shape _s) { CfgModel::m_shape = _s; }
    void SetAddNode(bool _setting){ m_addNode = _setting; }
    void SetAddEdge(bool _setting){ m_addEdge = _setting; }


    //Load functions
    //Moving generic load functions to virtual in Model.h
    void InitGraph(){ m_graph = new Wg(); }
    void WriteMapFile(const string& _filename);
    void ParseHeader(istream& _in);
    virtual void ParseFile();
    VID Cfg2VID(const CfgModel& _target);
    void GenGraph();

    //Display fuctions
    virtual void SetRenderMode(RenderMode _mode); //Wire, solid, or invisible
    virtual void GetChildren(list<Model*>& _models);

    void BuildModels();
    void Select(GLuint* _index, vector<Model*>& _sel);
    void Draw(GLenum _mode);
    void DrawSelect() {}
    void Print(ostream& _os) const;

    bool AddCC(int _vid);
    //void SetProperties(typename CCM::Shape _s, float _size, vector<float> _color, bool _isNew);
    void ScaleNodes(float _scale);
    //  void HandleSelect(); ORIGINALLY IN ROADMAP.H/.CPP
    //  void HandleAddEdge();
    //  void HandleAddNode();
    //  void HandleEditMap();
    //  void MoveNode();

  private:
    string  m_envFileName;
    string  m_fileDir;
    bool m_robCfgOn;
    bool m_addNode;
    bool m_addEdge;
    //double* m_cfg; (Originally used in Roadmap.h/Roadmap.cpp)
    //int m_dof;
    RobotModel* m_robot;
    vector<CCM*> m_ccModels;
    list<Model*> m_nodes;
    vector<Model*> m_nodesToConnect; //nodes to be connected
    //double m_oldT[3], m_oldR[3];  //old_T
    string m_cfgString, m_robCfgString; //s_cfg, s_robCfg
    bool m_editModel;
    bool m_noMap;

  protected:
    Wg* m_graph;
    //virtual void DumpNode();
    //virtual void SelectNode( bool bSel );
};

template <class CfgModel, class WEIGHT>
MapModel<CfgModel, WEIGHT>::MapModel(RobotModel* _robotModel) : LoadableModel("Map") {
  m_renderMode = INVISIBLE_MODE;
  m_robot = _robotModel;
  m_graph = NULL;
  m_editModel = false;
  m_addNode = false;
  m_addEdge = false;
  m_robCfgOn = false;
  m_robCfgString = "";
  m_noMap = false;
  m_robot = NULL;
}

//constructor only to grab header environment name
template <class CfgModel, class WEIGHT>
MapModel<CfgModel, WEIGHT>::MapModel(const string& _filename) : LoadableModel("Map") {
  SetFilename(_filename);
  if(FileExists(_filename)) {
    ifstream ifs(_filename.c_str());
    ParseHeader(ifs);
  }

  m_graph = NULL;
  m_editModel = false;
  m_addNode = false;
  m_addEdge = false;
  m_robCfgOn = false;
  m_robCfgString = "";
  m_noMap = false;
  m_robot = NULL;
}

template <class CfgModel, class WEIGHT>
MapModel<CfgModel, WEIGHT>::MapModel(const string& _filename, RobotModel* _robotModel) : LoadableModel("Map") {
  SetFilename(_filename);
  m_renderMode = INVISIBLE_MODE;
  m_robot = _robotModel;
  m_graph = NULL;
  m_editModel = false;
  m_addNode = false;
  m_addEdge = false;
  m_robCfgOn = false;
  m_robCfgString = "";
  m_noMap = false;

  ParseFile();
  BuildModels();
}

template <class CfgModel, class WEIGHT>
MapModel<CfgModel, WEIGHT>::~MapModel(){

  typedef typename vector<CCM*>::iterator CIT;
  for(CIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
    delete *ic;
  delete m_graph;
  m_graph = NULL;
}

///////////Load functions//////////

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::ParseHeader(istream& _in){

  m_fileDir = GetPathName(GetFilename());

  //Open file for reading data
  string s;

  //Get env file name info
  GoToNext(_in);
  getline(_in, s);

  //See if we need to add directory
  if(s[0] != '/')
    m_envFileName = m_fileDir + s;
  else
    m_envFileName = s;
}

template<class CfgModel, class WEIGHT>
void
MapModel<CfgModel,WEIGHT>::ParseFile(){
  if(!FileExists(GetFilename()))
    throw ParseException(WHERE, "'" + GetFilename() + "' does not exist");

  ifstream ifs(GetFilename().c_str());

  ParseHeader(ifs);

  //Get Graph Data
  string s;
  getline(ifs, s);
  m_graph  = new Wg();
  read_graph(*m_graph, ifs);
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::WriteMapFile(const string& _filename){

  ofstream outfile(_filename.c_str());

  outfile << "#####ENVFILESTART##### \n";
  outfile << m_envFileName << "\n";
  outfile << "#####ENVFILESTOP##### \n";

  write_graph(*m_graph, outfile);
}

template <class CfgModel, class WEIGHT>
//VID
typename graph<DIRECTED, MULTIEDGES, CfgModel, WEIGHT>::vertex_descriptor
MapModel<CfgModel, WEIGHT>::Cfg2VID(const CfgModel& _target){

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
void
MapModel<CfgModel, WEIGHT>::BuildModels() {
  typedef typename vector<CCM*>::iterator CCIT;
  for(CCIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
    delete (*ic);

  m_ccModels.clear();

  //Get CCs
  typedef typename vector< pair<size_t,VID> >::iterator CIT;
  vector<pair<size_t,VID> > ccs;
  m_colorMap.reset();
  get_cc_stats(*m_graph, m_colorMap, ccs);

  int CCSize = ccs.size();
  m_ccModels.reserve(CCSize);
  for(CIT ic = ccs.begin(); ic != ccs.end(); ic++){
    CCM* cc = new CCM(ic-ccs.begin());
    cc->SetRobotModel(m_robot);
    cc->BuildModels(ic->second, m_graph);
    m_ccModels.push_back(cc);
  }
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::Draw(GLenum _mode){
  if(m_renderMode == INVISIBLE_MODE)
    return;

  //Draw each CC
  typedef typename vector<CCM*>::iterator CIT;//CC iterator
  for(CIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++){
    if(_mode == GL_SELECT)
      glPushName((*ic)->GetID());
    (*ic)->Draw(_mode);
    if(_mode == GL_SELECT)
      glPopName();
  }
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::SetRenderMode(RenderMode _mode){
  m_renderMode = _mode;
  typedef typename vector<CCM*>::iterator CIT;//CC iterator
  for(CIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++){
    (*ic)->SetRenderMode(_mode);
  }
}


template <class CfgModel, class WEIGHT>
bool
MapModel<CfgModel, WEIGHT>::AddCC(int _vid){

  if(m_graph == NULL)
    return false;

  CCM* cc = new CCM(m_ccModels.size());
  cc->RobotModel(m_robot);
  cc->BuildModels(_vid, m_graph);

  float size;
  vector<float> color;
  if(m_ccModels[m_ccModels.size()-1]->getShape() == 0)
    size = m_ccModels[m_ccModels.size()-1]->getRobotSize();
  else if(m_ccModels[m_ccModels.size()-1]->getShape() == 1)
    size = m_ccModels[m_ccModels.size()-1]->getBoxSize();
  else
    size = 0.0;

  color =  m_ccModels[m_ccModels.size()-1]->getColor();
  cc->change_properties(m_ccModels[m_ccModels.size()-1]->getShape(),size,
      color, true);

  m_ccModels.push_back(cc);
  return true;
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::GetChildren(list<Model*>& _models){
  typedef typename vector<CCM*>::iterator CIT;
  for(CIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
    _models.push_back(*ic);
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::Print(ostream& _os) const {
  _os << Name() << ": " << GetFilename() << endl
    << "Connected components: " << m_ccModels.size() << endl;
}

/*template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::SetProperties(typename CCM::Shape _s, float _size,
    vector<float> _color, bool _isNew){
  typedef typename vector<CCM*>::iterator CIT;//CC iterator
  for(CIT ic = m_ccModels.begin(); ic!= m_ccModels.end(); ic++)
    (*ic)->change_properties(_s, _size, _color, _isNew);
}
*/
template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::ScaleNodes(float _scale){

  CfgModel::Scale(_scale);
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::Select(GLuint* _index, vector<Model*>& _sel){
  if(_index == NULL)
    return;
  m_ccModels[_index[0]]->Select(&_index[1],_sel);
}

//Commented out functions below are from Roadmap.h/.cpp and did not work there
//(or need other attn.)
//They should probably be here when fixed.

/* template <class CfgModel, class WEIGHT>
   void MapModel<CfgModel, WEIGHT>::HandleSelect(){

//find nodes
m_nodes.clear();
vector<Model*>& sel = this->GetVizmo().GetSelectedItem();
typedef vector<Model*>::iterator OIT;

for(OIT i=sel.begin(); i!=sel.end(); i++){  //GETS THE NODES FROM SELECTED ITEM AND PUTS THEM IN NODE VECTOR
string myName = ((Model*)(*i))->GetName();
if(((Model*)(*i)) != NULL)
if(myName.find("Node")!=string::npos){
m_nodes.push_back((Model*)(*i));
}//end if

if(m_robCfgOn){
this->GetVizmo().getRoboCfg();
//  printRobCfg(); MAY NEED TO RESTORE THIS
}
}//end for

if(!m_editModel){return;}

if(m_nodes.size() > 0){
Model* n = m_nodes.front();
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
PlumObject* m_map;
m_map = this->GetVizmo().GetMap();
CfgModel *cfg1, *cfg2;

typedef CMapLoader<CfgModel,EdgeModel>::Wg WG;
WG* graph;
CMapLoader<CfgModel,EdgeModel>* m_loader=(CMapLoader<CfgModel,EdgeModel>*)m_map->getLoader();
graph = m_loader->GetGraph();

//get from m_nodes node1 and node2
//get VID from those Cfgs
//add edge

list<Model*>::const_iterator it;
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
MapModel<CfgModel,EdgeModel>* mmodel =(MapModel<CfgModel,EdgeModel>*)m_map->getModel();
//get the CCModel of CfgModel
CCModel<CfgModel,EdgeModel>* m_cCModel = mmodel->GetCCModel(CC_id);
//add edge to CC
//m_cCModel->addEdge(cfg1, cfg2); Jul 17-12

//backUp current prpoperties:
CCModel<CfgModel,EdgeModel>::Shape shape = m_cCModel->getShape();
float size;
if(shape == 0)
size = m_cCModel->getRobotSize();
else if (shape == 1)
size = m_cCModel->getBoxSize();
else
size = 0;
vector<float> rgb;
rgb = m_cCModel->getColor();

mmodel->BuildModels();
mmodel->SetProperties(shape, size, rgb, false);
//emit callUpdate();
//  'UpdateNodeCfg();' //FIX THIS!!!

m_nodesToConnect.clear();
}
}

template <class CfgModel, class WEIGHT>
void MapModel<CfgModel, WEIGHT>::HandleAddNode(){

vector<Model*>& sel = this->GetVizmo().GetSelectedItem();
if(sel.size() !=0){
if(!m_nodes.empty()){
Model* n = m_nodes.front();
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

    PlumObject* m_rob;
    m_rob = this->GetVizmo().GetRobot();
    RobotModel* r = (RobotModel*)m_rob->getModel();

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
    this->GetVizmo().addSelectedItem((Model*)cfgNew);
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
   Model* n = m_nodes.front();
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

  if(m_nodes.empty()) return;
  Model * n=m_nodes.front();
  double diff = fabs(m_oldT[0]-n->tx())+
  fabs(m_oldT[1]-n->ty())+
  fabs(m_oldT[2]-n->tz())+
  fabs(m_oldR[0]-n->rx())+
  fabs(m_oldR[1]-n->ry())+
  fabs(m_oldR[2]-n->rz());

  if(diff > 1e-10){
  vector<Model*>& sel=GetVizmo().GetSelectedItem();
  if(sel.size() !=0){
  GetVizmo().Node_CD((CfgModel*)n);
  }

  typedef vector<Model*>::iterator OIT;
  for(OIT i=sel.begin();i!=sel.end();i++){
  if(((Model*)(*i))->GetName()=="Node")
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
  m_map_Changed=true;

//  emit callUpdate(); ****CHECK THIS**********
}
}*/

//  void
//  TestMapModel(){
//    MapModel<CfgModel,EdgeModel> model;
//  }

#endif
