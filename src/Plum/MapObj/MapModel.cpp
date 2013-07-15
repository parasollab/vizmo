//////////////////////////////////////////////////////////////////////
// MapModel.cpp: implementation of the CMapModel class.
//////////////////////////////////////////////////////////////////////
/*
#include "MapModel.h"
#include "CfgModel.h"
#include "EdgeModel.h" 

<<<<<<< .mine
#include <math.h>
#include "Gauss.h"
#include "EnvObj/Robot.h"
#include "Plum/PlumObject.h" 

//#include "MapLoader.h"
#include "CCModel.h"

=======
>>>>>>> .r357
namespace plum{
<<<<<<< .mine
  
  template <class CfgModel, class WEIGHT>
  CMapModel<CfgModel, WEIGHT>::CMapModel(){
    
    //m_mapLoader = NULL;
    m_renderMode = INVISIBLE_MODE;
    m_pRobot = NULL;
    m_Graph = NULL; 
    m_enableSelection = true; //disable selection
    m_bEditModel = false;
    m_addNode = false;
    m_addEdge = false;
    m_robCfgOn = false; 
    m_robCfgString = "";
    m_noMap = false;
    m_size = 0.5; 
  }

  template <class CfgModel, class WEIGHT>
  CMapModel<CfgModel, WEIGHT>::~CMapModel(){
  
    typedef typename vector<myCCModel*>::iterator CIT;
    for(CIT ic = m_CCModels.begin(); ic != m_CCModels.end(); ic++)
      delete *ic;
    delete m_Graph; 
    m_Graph = NULL;
=======
  void testMapModel() {
    CMapModel<CfgModel,EdgeModel> model;
>>>>>>> .r357
  }
  //Load functions/////////////////////////////////////////////////////
 
  template <class CfgModel, class WEIGHT>
  bool
  CMapModel<CfgModel, WEIGHT>::CheckCurrentStatus() const{

    //Check if file exists
    ifstream fin(m_filename.c_str());
    if(!fin.good()){
      cerr << "File (" << m_filename << ") not found";
      return false;
    }
    return true;
  }

  template <class CfgModel, class WEIGHT>
  void
  CMapModel<CfgModel, WEIGHT>::GoToNext(istream& _in){
  
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
  CMapModel<CfgModel, WEIGHT>::IsCommentLine(char _c){
    return _c == '#'; 
  }

  template <class CfgModel, class WEIGHT>
  string
  CMapModel<CfgModel, WEIGHT>::GetPathName(const string& _filename){
    
    size_t pos = _filename.rfind('/');
    if(pos == string::npos)
      return "";
    return _filename.substr(0, pos+1);
  }
  
  template <class CfgModel, class WEIGHT>
  bool
  CMapModel<CfgModel, WEIGHT>::ParseHeader(){
  
    if(CheckCurrentStatus() == false)
      return false;

    ifstream fin(m_filename.c_str());
    bool result = ParseHeader(fin);
    fin.close();
    return result;
  }

  template <class CfgModel, class WEIGHT>
  bool 
  CMapModel<CfgModel, WEIGHT>::ParseHeader(istream& _in){
    
    m_strFileDir = GetPathName(m_filename);

    //Open file for reading data
    string s;

    //Get version comment
    GoToNext(_in);
    _in >> s >> s >> s >> s; //Roadmap Version Number 061300
    m_strVersionNumber = s;

    //Get preamble info
    GoToNext(_in);
    getline(_in, s);
    m_strPreamble = s;

    //Get env file name info
    GoToNext(_in);
    getline(_in, s);
    
    //See if we need to add directory
    if(s[0] != '/')
      m_strEnvFileName = m_strFileDir + s; 
    else
      m_strEnvFileName = s;

    unsigned int number = 0;
    //get lp info
    GoToNext(_in);
    _in >> number; //# of lps
    
    for(unsigned int iLP = 0; iLP < number; iLP++){
      GoToNext(_in);
      getline(_in, s);
      if(m_strLPs.size() != number)
        m_strLPs.push_back(s);
    }

    //get cd info
    GoToNext(_in);
    _in >> number; //# of lps
    for(unsigned int iCD = 0; iCD < number; iCD++){
      GoToNext(_in);
      getline(_in, s);
      if(m_strCDs.size() != number)
        m_strCDs.push_back(s);
    }

    //get dm info
    GoToNext(_in);
    _in >> number;
    for(unsigned int iDM = 0; iDM < number; iDM++){
      GoToNext(_in);
      getline(_in, s);
      if(m_strDMs.size() != number)
        m_strDMs.push_back(s);   
    }

    // June 17-05
    // Supports new version of map generation:
    // ask whether this has the RNGSEEDSTART tag:
    string s_ver = m_strVersionNumber;
    if(s_ver == "041805"){
      GoToNext(_in);
      getline(_in, s);
      m_seed = s;
    }
    return true;
  }


  template<class CfgModel, class WEIGHT>
  bool 
  CMapModel<CfgModel, WEIGHT>::ParseFile(){      
      
    if(CheckCurrentStatus() == false)
      return false;

    ifstream fin(m_filename.c_str());
    if(ParseHeader(fin) == false) 
      return false;

    //Get Graph Data
    string s;
    getline(fin, s);
    m_Graph  = new Wg();
    if(m_Graph == NULL){ 
      cout<<"Graph null ..."<<endl;
      return false; 
    }
    read_graph(*m_Graph, fin);

    return true;
  }

  template <class CfgModel, class WEIGHT>
  bool 
  CMapModel<CfgModel, WEIGHT>::WriteMapFile(){

    //m_Graph->WriteGraph(filename);
    return true;
  }

  template <class CfgModel, class WEIGHT>
  //VID
  typename graph<DIRECTED, MULTIEDGES, CfgModel, WEIGHT>::vertex_descriptor 
  CMapModel<CfgModel, WEIGHT>::Cfg2VID(CfgModel _target){
    
    VI vi;
    //VID tvid = -1;
    typename graph<DIRECTED, MULTIEDGES, CfgModel, WEIGHT>::vertex_descriptor tvid = -1; 
    for(vi = m_Graph->begin(); vi != m_Graph->end(); vi++){
      if(_target == (*vi).property()){
        tvid=(*vi).descriptor();   
        break;
      }
    }
    return tvid;
  }
  
  template<class CfgModel, class WEIGHT> 
  void
  CMapModel<CfgModel, WEIGHT>::GenGraph(){
    m_Graph  = new Wg();
  }

  
  //Display functions//////////////////////////////////////////////////

  template <class CfgModel, class WEIGHT>
  bool 
  CMapModel<CfgModel, WEIGHT>::BuildModels() {
      
    typedef typename vector<myCCModel*>::iterator CCIT;//CC iterator
    //typedef graph<DIRECTED,MULTIEDGES,CfgModel,WEIGHT> Wg;
    //get graph
    //if( m_mapLoader==NULL ) return false;
    //typename Wg* graph = GetGraph();  ///NOW REDUNDANT!!!!!!!!!!!11111
    if(m_Graph == NULL) 
      return false;
    for(CCIT ic = m_CCModels.begin(); ic != m_CCModels.end(); ic++)
      delete (*ic);
    m_CCModels.clear(); //new line Jul-01-05
    //Get CCs
    typedef typename vector< pair<size_t,VID> >::iterator CIT;//CC iterator
    vector<pair<size_t,VID> > ccs;
    cmap.reset();
    get_cc_stats(*m_Graph,cmap,ccs);  

    int CCSize=ccs.size();
    m_CCModels.reserve(CCSize);
    for( CIT ic=ccs.begin();ic!=ccs.end();ic++ ){
      myCCModel* cc = new myCCModel(ic-ccs.begin());
      cc->RobotModel(m_pRobot);    
      cc->BuildModels(ic->second,m_Graph);   
      m_CCModels.push_back(cc);
    }
    return true;
  }

  template <class CfgModel, class WEIGHT>
  void 
  CMapModel<CfgModel, WEIGHT>::Draw(GLenum _mode){

      if(m_renderMode == INVISIBLE_MODE)
        return;
      if(_mode==GL_SELECT && !m_enableSelection)
        return; 
      //Draw each CC
      int size = 0;
      typedef typename vector<myCCModel*>::iterator CIT;//CC iterator
      for(CIT ic = m_CCModels.begin(); ic != m_CCModels.end(); ic++){
        if(_mode == GL_SELECT) 
          glPushName((*ic)->ID()); 
        (*ic)->Draw(_mode);
        if(_mode == GL_SELECT) 
          glPopName();
        size++;
      }
  }

  template <class CfgModel, class WEIGHT>
  void 
  CMapModel<CfgModel, WEIGHT>::SetRenderMode(RenderMode _mode){ 
    m_renderMode = _mode;
    typedef typename vector<myCCModel*>::iterator CIT;//CC iterator
    for(CIT ic = m_CCModels.begin(); ic != m_CCModels.end(); ic++){
      (*ic)->SetRenderMode(_mode);
    }
  }


  template <class CfgModel, class WEIGHT>
  bool 
  CMapModel<CfgModel, WEIGHT>::AddCC(int _vid){
    
    //get graph
    //if(m_mapLoader==NULL) 
    //  return false;
    //typename Wg* graph = GetGraph();
    if(m_Graph == NULL) 
      return false;
    myCCModel* cc = new myCCModel(m_CCModels.size());
    cc->RobotModel(m_pRobot);  
    cc->BuildModels(_vid, m_Graph); 
    float size;
    vector<float> color; 
    if(m_CCModels[m_CCModels.size()-1]->getShape() == 0)
      size = m_CCModels[m_CCModels.size()-1]->getRobotSize();
    else if(m_CCModels[m_CCModels.size()-1]->getShape() == 1)
      size = m_CCModels[m_CCModels.size()-1]->getBoxSize();
    else
      size = 0.0;

    color =  m_CCModels[m_CCModels.size()-1]->getColor();
    cc->change_properties(m_CCModels[m_CCModels.size()-1]->getShape(),size,
      color, true);
    m_CCModels.push_back(cc);

    return true;
  }
*/
//  template <class CfgModel, class WEIGHT>
//  /*virtual*/ void 
/*  CMapModel<CfgModel, WEIGHT>::GetChildren(list<GLModel*>& _models){

    typedef typename vector<myCCModel*>::iterator CIT;
    for(CIT ic = m_CCModels.begin(); ic != m_CCModels.end(); ic++)
      _models.push_back(*ic);
  }  

  template <class CfgModel, class WEIGHT>
  vector<string> 
  CMapModel<CfgModel, WEIGHT>::GetInfo() const{ 
  
    vector<string> info; 
    info.push_back(GetFilename());
    ostringstream temp;
    temp<< "There are " << m_CCModels.size() << " connected components";
    info.push_back(temp.str());
    return info;
  }

  template <class CfgModel, class WEIGHT>
  void 
  CMapModel<CfgModel, WEIGHT>::SetProperties(typename myCCModel::Shape _s, float _size, 
                vector<float> _color, bool _isNew){
    
    typedef typename vector<myCCModel*>::iterator CIT;//CC iterator
    for(CIT ic = m_CCModels.begin(); ic!= m_CCModels.end(); ic++)
      (*ic)->change_properties(_s, _size, _color, _isNew);
  }

  template <class CfgModel, class WEIGHT>
  void 
  CMapModel<CfgModel, WEIGHT>::Select(unsigned int* _index, vector<gliObj>& _sel){
  
    //if(m_mapLoader == NULL) 
    //  return; //status error
    if(_index == NULL) 
      return;
    m_CCModels[_index[0]]->Select(&_index[1],_sel); 
  }

*/
  //Commented out functions below are from Roadmap.h/.cpp and did not work there
  //(or need other attn.) 
  //They should probably be here when fixed. 

  /* template <class CfgModel, class WEIGHT>
     void CMapModel<CfgModel, WEIGHT>::HandleSelect(){

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

  if(!m_bEditModel){return;}

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
      void CMapModel<CfgModel, WEIGHT>::HandleAddEdge(){

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
  CMapModel<CfgModel,EdgeModel>* mmodel =(CMapModel<CfgModel,EdgeModel>*)m_Map->getModel();
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
  void CMapModel<CfgModel, WEIGHT>::HandleAddNode(){

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
    CMapModel<CfgModel,EdgeModel>* mmodel = new CMapModel<CfgModel,EdgeModel>();

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
   void CMapModel<CfgModel, WEIGHT>::HandleEditMap(){

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
  void CMapModel<CfgModel, WEIGHT>::MoveNode(){

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
//    CMapModel<CfgModel,EdgeModel> model;
//  }

//}//end namespace plum





























