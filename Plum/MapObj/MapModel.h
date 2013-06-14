//////////////////////////////////////////////////////////////////////////////////////
// MapModel.h: interface for the CMapModel class.
// Many things that used to be in Roadmap.h/.cpp are now implemented directly in here
//////////////////////////////////////////////////////////////////////////////////////

#if !defined(_MAPMODEL_H_)
#define _MAPMODEL_H_

#include <math.h>
#include "Gauss.h"
#include "src/EnvObj/Robot.h"
#include "Plum/PlumObject.h" 

#include "MapLoader.h"
#include "CCModel.h"
#include <algorithms/graph_algo_util.h>

using namespace stapl;
using namespace std;

namespace plum {

  template <class Cfg, class WEIGHT>
  class CMapModel : public CGLModel {
      
    private:
      typedef CCModel<Cfg,WEIGHT> myCCModel;
      typedef CMapLoader<Cfg,WEIGHT> Loader;
      typedef graph<DIRECTED,MULTIEDGES,Cfg,WEIGHT> Weg;
      typedef typename Weg::vertex_descriptor VID;
      typedef vector_property_map<Weg, size_t> color_map_t;
      color_map_t cmap;
    
    public:
      //////////////////////////////////////////////////////////////////////
      // Constructor/Destructor
      //////////////////////////////////////////////////////////////////////
      CMapModel();
      virtual ~CMapModel();

      //////////////////////////////////////////////////////////////////////
      // Access functions
      //////////////////////////////////////////////////////////////////////
      void SetMapLoader(Loader* mapLoader){m_mapLoader = mapLoader;}
      void SetRobotModel(OBPRMView_Robot* pRobot){m_pRobot = pRobot;}     
      vector<myCCModel*>& GetCCModels() {return m_CCModels;}
      list<CGLModel*>& GetNodeList() {return m_nodes;} 
      vector<CGLModel*>& GetNodesToConnect() {return m_nodesToConnect;} //prev. Node_Edge  
      //(Some other class besides QLabel*) GetCfgLabel() {return m_cfgLabel;}
      //("") GetRobCfgLabel() {return m_robCfgLabel;}   
      //("") GetMessageLabel() {return m_messageLabel;}
      //("") GetIconLabel() {return m_iconLabel;}   
      void SetMBEditModel(bool _setting) {m_bEditModel = _setting;} 
      void SetSize(double _size) {m_size = _size;}

      myCCModel* GetCCModel(int id) {return m_CCModels[id]; }
      int number_of_CC(){return m_CCModels.size();}

      bool 
      AddCC(int vid){
        
        //get graph
        if(m_mapLoader==NULL) 
          return false;
        typename Loader::Wg * graph = m_mapLoader->GetGraph();
        if(graph==NULL) 
          return false;
        myCCModel * cc=new myCCModel(m_CCModels.size());
        cc->RobotModel(m_pRobot);  
        cc->BuildModels(vid,graph); 
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

      //////////////////////////////////////////////////////////////////////
      // Action functions
      //////////////////////////////////////////////////////////////////////

      virtual bool BuildModels();
      virtual void Draw(GLenum mode);
      void Select(unsigned int* index, vector<gliObj>& sel);
      //set wire/solid/hide
      virtual void SetRenderMode(RenderMode _mode);
      virtual const string GetName() const {return "Map";}

      virtual void 
      GetChildren(list<CGLModel*>& models){

        typedef typename vector<myCCModel*>::iterator CIT;
        for(CIT ic=m_CCModels.begin();ic!=m_CCModels.end();ic++ )
          models.push_back(*ic);
      }  

      void 
      SetProperties(typename myCCModel::Shape s, float size, 
        vector<float> color, bool isNew){
        
        typedef typename vector<myCCModel*>::iterator CIT;//CC iterator
        for(CIT ic=m_CCModels.begin(); ic!=m_CCModels.end(); ic++)
          (*ic)->change_properties(s, size, color, isNew);
      }

      virtual vector<string> GetInfo() const;
      //  void HandleSelect(); ORIGINALLY IN ROADMAP.H/.CPP  
      //  void HandleAddEdge();
      //  void HandleAddNode();
      //  void HandleEditMap();
      //  void MoveNode(); 

      bool m_robCfgOn; //Move to private...hopefully 
      bool m_addNode; 
      bool m_addEdge;

      double* m_cfg; 
      int m_dof; 

    private:

      OBPRMView_Robot* m_pRobot;
      vector<myCCModel*> m_CCModels;
      list<CGLModel*> m_nodes; //moved from obsolete Roadmap.h! 
      vector<CGLModel*> m_nodesToConnect; //nodes to be connected
      Loader* m_mapLoader;
      double m_oldT[3], m_oldR[3];  //old_T 
      double m_size;                //check purpose of this variable  
      //    bool m_robCfgOn;  ALSO FROM ROADMAP 
      //  (Some other class besides QLabel*) m_cfgLabel; //previously l_cfg 
      //  ("") m_robCfgLabel;  //l_robCfg
      //  ("") m_messageLabel;  //l_message or something
      //  ("") m_iconLabel; //l_icon
      string m_cfgString, m_robCfgString; //s_cfg, s_robCfg  
      bool m_bEditModel;
      bool m_noMap; //noMap

    protected:
      //virtual void DumpNode();
      //virtual void SelectNode( bool bSel );
  };

  //////////////////////////////////////////////////////////////////////
  // Template function definitions
  //////////////////////////////////////////////////////////////////////

  template <class Cfg, class WEIGHT>
    CMapModel<Cfg, WEIGHT>::CMapModel() {
      m_mapLoader = NULL;
      m_RenderMode = INVISIBLE_MODE;
      m_pRobot=NULL;
      m_enableSelection=true; //disable selection

      m_bEditModel=false;
      m_addNode=false;
      m_addEdge=false;
      m_robCfgOn = false; 
      m_robCfgString = "";
      m_noMap = false;
      m_size = 0.5; 
    }

  template <class Cfg, class WEIGHT>
    CMapModel<Cfg, WEIGHT>::~CMapModel() {
      typedef typename vector<myCCModel*>::iterator CIT;//CC iterator
      for(CIT ic=m_CCModels.begin();ic!=m_CCModels.end();ic++){
        delete *ic;
      }//end for
    }

  template <class Cfg, class WEIGHT>
    bool CMapModel<Cfg, WEIGHT>::BuildModels() {
      typedef typename vector<myCCModel*>::iterator CCIT;//CC iterator
      //get graph
      if( m_mapLoader==NULL ) return false;
      typename Loader::Wg * graph = m_mapLoader->GetGraph();
      if( graph==NULL ) return false;
      for(CCIT ic = m_CCModels.begin(); ic != m_CCModels.end(); ic++)
        delete (*ic);
      m_CCModels.clear(); //new line Jul-01-05
      //Get CCs
      typedef typename vector< pair<size_t,VID> >::iterator CIT;//CC iterator
      vector<pair<size_t,VID> > ccs;
      cmap.reset();
      get_cc_stats(*graph,cmap,ccs);  


      int CCSize=ccs.size();
      m_CCModels.reserve(CCSize);
      for( CIT ic=ccs.begin();ic!=ccs.end();ic++ ){
        myCCModel* cc = new myCCModel(ic-ccs.begin());
        cc->RobotModel(m_pRobot);    
        cc->BuildModels(ic->second,graph);   
        m_CCModels.push_back(cc);
      }
      return true;
    }

  template <class Cfg, class WEIGHT>
    void CMapModel<Cfg, WEIGHT>::Draw(GLenum mode) {

      if(m_RenderMode == INVISIBLE_MODE) return;
      if(mode==GL_SELECT && !m_enableSelection) return;
      //Draw each CC
      int size = 0;
      typedef typename vector<myCCModel*>::iterator CIT;//CC iterator
      for( CIT ic=m_CCModels.begin();ic!=m_CCModels.end();ic++ ){
        if( mode==GL_SELECT ) glPushName( (*ic)->ID() ); 

        (*ic)->Draw(mode);

        if( mode==GL_SELECT ) glPopName();
        size++;
      }
      //m_pRobot->size = 1;
    }

  template <class Cfg, class WEIGHT>
    void CMapModel<Cfg, WEIGHT>::SetRenderMode(RenderMode _mode) { 
      m_RenderMode = _mode;
      typedef typename vector<myCCModel*>::iterator CIT;//CC iterator
      for(CIT ic = m_CCModels.begin(); ic != m_CCModels.end(); ic++){
        (*ic)->SetRenderMode(_mode);
      }
    }

  template <class Cfg, class WEIGHT>
    vector<string> CMapModel<Cfg, WEIGHT>::GetInfo() const { 
      vector<string> info; 
      info.push_back(m_mapLoader->GetFilename());
      ostringstream temp;
      temp<<"There are "<<m_CCModels.size()<<" connected components";
      info.push_back(temp.str());
      return info;
    }

  template <class Cfg, class WEIGHT>
    void CMapModel<Cfg, WEIGHT>::Select(unsigned int* index, vector<gliObj>& sel){
      if( m_mapLoader==NULL ) return; //status error
      if( index==NULL ) return;
      m_CCModels[index[0]]->Select(&index[1],sel); 
    }

  //Commented out functions below are from Roadmap.h/.cpp and did not work there
  //(or need other attn.) 
  //They should probably be here when fixed. 

  /* template <class Cfg, class WEIGHT>
     void CMapModel<Cfg, WEIGHT>::HandleSelect(){

  //find nodes
  m_nodes.clear();
  vector<gliObj>& sel = this->GetVizmo().GetSelectedItem();
  typedef vector<gliObj>::iterator OIT;

  for(OIT i=sel.begin(); i!=sel.end(); i++){  //GETS THE NODES FROM SELECTED ITEM AND PUTS THEM IN NODE VECTOR 
  string myName = ((CGLModel*)(*i))->GetName();
  if(((CGLModel*)(*i)) != NULL)
  if(myName.find("Node")!=string::npos){
  m_nodes.push_back((CGLModel*)(*i));
  }//end if

  if(m_robCfgOn){
  this->GetVizmo().getRoboCfg();
  //  printRobCfg(); MAY NEED TO RESTORE THIS 
  }
  }//end for

  if(!m_bEditModel){return;}

  if(m_nodes.size() > 0){
  CGLModel* n = m_nodes.front();
  //   printNodeCfg((CCfg*)n); MAY NEED TO RESTORE THIS
  }

  if(m_addEdge)
  this->HandleAddEdge();
  else if(m_addNode)   
  this->HandleAddNode();
  // else
  //   handleEditMap();
  }*/

  /*  template <class Cfg, class WEIGHT>
      void CMapModel<Cfg, WEIGHT>::HandleAddEdge(){

//find two nodes...
PlumObject* m_Map;
m_Map = this->GetVizmo().GetMap();
CCfg *cfg1, *cfg2;

typedef CMapLoader<CCfg,Edge>::Wg WG;
WG* graph;
CMapLoader<CCfg,Edge>* m_loader=(CMapLoader<CCfg,Edge>*)m_Map->getLoader();
graph = m_loader->GetGraph();

//get from m_nodes node1 and node2
//get VID from those Cfgs
//add edge

list<CGLModel*>::const_iterator it;
for(it = m_nodes.begin(); it != m_nodes.end(); it++){
m_nodesToConnect.push_back(*it);
}
if(m_nodesToConnect.size() == 2){
cfg1 = (CCfg*)m_nodesToConnect[0];
cfg2 = (CCfg*)m_nodesToConnect[1];
graph->add_edge(cfg1->GetIndex(), cfg2->GetIndex(),1);  
  //////////  Jun 16-05 ///////////////
  // Add edge to CCModel:
  // get a CC id
  int CC_id = cfg1->GetCC_ID();
  //get mapModel
  CMapModel<CCfg,Edge>* mmodel =(CMapModel<CCfg,Edge>*)m_Map->getModel();
  //get the CCModel of Cfg
  CCModel<CCfg,Edge>* m_CCModel = mmodel->GetCCModel(CC_id);
  //add edge to CC 
  //m_CCModel->addEdge(cfg1, cfg2); Jul 17-12 

  //backUp current prpoperties:
  CCModel<CCfg,Edge>::Shape shape = m_CCModel->getShape();
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

  template <class Cfg, class WEIGHT>
  void CMapModel<Cfg, WEIGHT>::HandleAddNode(){

  vector<gliObj>& sel = this->GetVizmo().GetSelectedItem();
  if(sel.size() !=0){ 
  if(!m_nodes.empty()){
  CGLModel* n = m_nodes.front(); 
  CCfg* cfg = (CCfg*)n;   
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
    CMapLoader<CCfg,Edge>* mloader=new CMapLoader<CCfg,Edge>();
    CMapModel<CCfg,Edge>* mmodel = new CMapModel<CCfg,Edge>();

    mmodel->SetMapLoader(mloader);

    PlumObject* m_Rob;
    m_Rob = this->GetVizmo().GetRobot();
    OBPRMView_Robot* r = (OBPRMView_Robot*)m_Rob->getModel();

    if(r != NULL)
      mmodel->SetRobotModel(r);

    this->GetVizmo().setMapObj(mloader, mmodel); 
    mloader->genGraph();

    //add node to graph
    typedef CMapLoader<CCfg,Edge>::Wg WG;
    WG* graph;
    graph = mloader->GetGraph();
    CCfg* cfgNew = new CCfg(); 
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
    this->GetVizmo().addSelectedItem((CGLModel*)cfgNew);
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
   template <class Cfg, class WEIGHT>
   void CMapModel<Cfg, WEIGHT>::HandleEditMap(){

   if(m_nodes.empty()==false){
   CGLModel* n = m_nodes.front();
   m_oldT[0] = n->tx(); 
   m_oldT[1] = n->ty(); 
   m_oldT[2] = n->tz();

   m_oldR[0] = n->rx(); 
   m_oldR[1] = n->ry(); 
   m_oldR[2] = n->rz();     
   }
   }
   */

/*template <class Cfg, class WEIGHT>
  void CMapModel<Cfg, WEIGHT>::MoveNode(){

  if(m_Nodes.empty()) return;
  CGLModel * n=m_Nodes.front();
  double diff = fabs(m_oldT[0]-n->tx())+
  fabs(m_oldT[1]-n->ty())+
  fabs(m_oldT[2]-n->tz())+
  fabs(m_oldR[0]-n->rx())+
  fabs(m_oldR[1]-n->ry())+
  fabs(m_oldR[2]-n->rz());

  if(diff > 1e-10){
  vector<gliObj>& sel=GetVizmo().GetSelectedItem();
  if(sel.size() !=0){
  GetVizmo().Node_CD((CCfg*)n);
  }

  typedef vector<gliObj>::iterator OIT;
  for(OIT i=sel.begin();i!=sel.end();i++){
  if(((CGLModel*)(*i))->GetName()=="Node") 
  printNodeCfg((CCfg*)n);
  }

  if(nodeGUI!= NULL){
  if(nodeGUI->isVisible()){
  v_cfg = ((CCfg*)n)->GetDataCfg();
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

}//namespace plum

#endif // !defined(_MAPMODEL_H_)

