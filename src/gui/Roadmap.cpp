#include "Roadmap.h"
#include "QueryGUI.h"
///////////////////////////////////////////////////////////////////////////////// 
//// Include Qt Headers
#include <qapplication.h>
#include <qaction.h>
#include <qlabel.h>
#include <q3listbox.h>
#include <q3hbox.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qtoolbutton.h>
#include <qwidget.h>
#include <qinputdialog.h>
#include <qcolordialog.h>
#include <qobject.h>
#include <qcursor.h>

#include <q3vbox.h>
#include <qvalidator.h>
#include <qlayout.h>
#include <q3grid.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3Frame>
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////
//Icons
#include "icon/shapes1.xpm"
#include "icon/pallet.xpm"
#include "icon/make.xpm"
#include "icon/bulb.xpm"
#include "icon/cross.xpm"
#include "icon/crossEdge.xpm"
#include "icon/make_solid.xpm" 
#include "icon/make_wired.xpm"
#include "icon/make_invisible.xpm" 
#include "icon/cc_color.xpm"
#include "icon/rcolor.xpm"
//#include "icon/ruler.xpm"
//#include "icon/ccs_one_color.xpm" 

VizmoRoadmapGUI::VizmoRoadmapGUI(Q3MainWindow * parent,char *name)
   :Q3ToolBar(parent, name){

      this->setLabel("CC's features");
      createGUI();
      m_bEditModel=false;
      m_addNode=false;
      m_addEdge=false;

      s_robCfg = "";
      robCfgOn = false;

      noMap = false;

      nodeGUI = NULL;
   }

void VizmoRoadmapGUI::createGUI()
{

   //sizeAction = new QAction (QIcon(QPixmap(icon_ruler)),tr("Size"), this);
   sizeAction = new QAction (QIcon(QPixmap(icon_pallet)),tr("Size"), this);
   sizeAction->setShortcut(tr("CTRL+S"));
   connect(sizeAction,SIGNAL(activated()), this, SLOT(changeSize()));
   sizeAction->addTo(this);
   sizeAction->setEnabled(false);

   colorAction = new QAction (QIcon(QPixmap(icon_rcolor)),tr("Random Color"), this);
   colorAction->setShortcut(tr("CTRL+R"));
   colorAction->setToolTip ("CC's Random Color");
   connect(colorAction,SIGNAL(activated()), this, SLOT(changeColor()));
   colorAction->addTo(this);
   colorAction->setEnabled(false);

   colorSelectAction = new QAction (QIcon(QPixmap(icon_cc_color)),tr("Change Color"), this);
   colorSelectAction->setShortcut(tr("CTRL+C"));
   colorSelectAction->setToolTip ("Change Selected CC's Color");
   connect(colorSelectAction,SIGNAL(activated()), this, SLOT(changeColorOfCCselected()));
   colorSelectAction->addTo(this);
   colorSelectAction->setEnabled(false);

   invisibleSelectNodeAction = new QAction (QIcon(QPixmap(icon_make_invisible)),tr("Change to Invisible"), this);
   invisibleSelectNodeAction->setShortcut(tr("CTRL+N"));
   invisibleSelectNodeAction->setToolTip ("Make Invisible");
   connect(invisibleSelectNodeAction,SIGNAL(activated()), this, SLOT(changeInvisibleOfNodeselected()));
   invisibleSelectNodeAction->addTo(this);
   invisibleSelectNodeAction->setEnabled(false);

   wireSelectNodeAction = new QAction (QIcon(QPixmap(icon_make_wired)),tr("Change to Wire Mode"), this);
   wireSelectNodeAction->setShortcut(tr("CTRL+N"));
   wireSelectNodeAction->setToolTip ("Make Wired");
   connect(wireSelectNodeAction,SIGNAL(activated()), this, SLOT(changeWireOfNodeselected()));
   wireSelectNodeAction->addTo(this);
   wireSelectNodeAction->setEnabled(false);

   solidSelectNodeAction = new QAction (QIcon(QPixmap(icon_make_solid)),tr("Change to Solid Mode"), this);
   solidSelectNodeAction->setShortcut(tr("CTRL+N"));
   solidSelectNodeAction->setToolTip ("Make Solid");
   connect(solidSelectNodeAction,SIGNAL(activated()), this, SLOT(changeSolidOfNodeselected()));
   solidSelectNodeAction->addTo(this);
   solidSelectNodeAction->setEnabled(false);

   colorSelectNodeAction = new QAction (QIcon(QPixmap(icon_pallet)),tr("Change Node Color"), this);
   colorSelectNodeAction->setShortcut(tr("CTRL+N"));
   colorSelectNodeAction->setToolTip ("Change Color");
   connect(colorSelectNodeAction,SIGNAL(activated()), this, SLOT(changeColorOfNodeselected()));
   colorSelectNodeAction->addTo(this);
   colorSelectNodeAction->setEnabled(false);

   editAction = new QAction (QIcon(QPixmap(icon_make)),tr("Edit Map"), this);
   editAction->setShortcut(tr("CTRL+M"));
   editAction->setStatusTip(tr("Edit Map"));
   editAction->setCheckable(true);
   connect(editAction,SIGNAL(triggered ()), this, SLOT(editMap()));
   editAction->addTo(this);
   editAction->setEnabled(false);

   addNodeAction = new QAction (QIcon(QPixmap(icon_cross)),tr("Add &Node"),this);
   addNodeAction->setShortcut(tr("CTRL+M"));
   addNodeAction->setCheckable(true);
   connect(addNodeAction,SIGNAL(activated()), this, SLOT(addNode()));
   addNodeAction->setEnabled(false);

   addEdgeAction = new QAction (QIcon(QPixmap(icon_crossEdge)),tr("Add &Edge"),this);
   addEdgeAction->setShortcut(tr("CTRL+E"));
   addEdgeAction->setCheckable(true);
   connect(addEdgeAction,SIGNAL(activated()), this, SLOT(addEdge()));
   addEdgeAction->setEnabled(false);

   //m_ccsOneColor = new QAction(QIcon(QPixmap(icon_ccs_one_color)), tr("CCs one color"), this); 
   m_ccsOneColor = new QAction(QIcon(QPixmap(icon_pallet)), tr("CCs one color"), this); 
   m_ccsOneColor->setCheckable(true); 
   m_ccsOneColor->setToolTip("CCs one color"); 
   connect(m_ccsOneColor, SIGNAL(activated()), this, SLOT(setSameColor())); 
   m_ccsOneColor->addTo(this); 
   m_ccsOneColor->setEnabled(false); 

   m_nodeView = new QButtonGroup(this);

   m_nodeView->addButton(new QPushButton("Robot", this), 1);
   m_nodeView->button(1)->setFixedWidth(47);
   m_nodeView->button(1)->setEnabled(false);
   m_nodeView->button(1)->setCheckable(true); 
   m_nodeView->addButton(new QPushButton("Box", this), 2);
   m_nodeView->button(2)->setFixedWidth(47); 
   m_nodeView->button(2)->setEnabled(false);
   m_nodeView->button(2)->setCheckable(true); 
   m_nodeView->addButton(new QPushButton("Point", this), 3);  
   m_nodeView->button(3)->setFixedWidth(47); 
   m_nodeView->button(3)->setEnabled(false); 
   m_nodeView->button(3)->setCheckable(true); 

   connect(m_nodeView, SIGNAL(buttonClicked(int)), this, SLOT(getSelectedItem())); 

   size=0.5;

   //Find all toolbar button and show text
   QList<QObject *> l = QObject::queryList("QToolButton");
   QListIterator<QObject *> it(l);             // iterate over the buttons
   QObject * obj;
   while (it.hasNext()) { // for each found object...
      obj=it.next();
      ((QToolButton*)obj)->setUsesTextLabel (true);
   }

   createQGrid();
   createRobotToolBar();

}

void VizmoRoadmapGUI::reset()
{
   //Apr-05-2005
   m_Nodes.clear();
   
   m_nodeView->button(1)->setEnabled(true); 
   m_nodeView->button(2)->setEnabled(true); 
   m_nodeView->button(3)->setEnabled(true); 
   
   if(GetVizmo().IsRoadMapLoaded() && GetVizmo().isRoadMapShown()){
     if(m_nodeView->checkedButton() != 0) 
         emit getSelectedItem();
   }

   editAction->setEnabled(true);
   sizeAction->setEnabled(true);
   colorAction->setEnabled(true);
   colorSelectAction->setEnabled(true);
   solidSelectNodeAction->setEnabled(true);
   wireSelectNodeAction->setEnabled(true);
   invisibleSelectNodeAction->setEnabled(true);
   colorSelectNodeAction->setEnabled(true);
   m_ccsOneColor->setEnabled(true); 
   size = 0.5;
   Node_Edge.clear();
   l_cfg->clear();

   if(robCfgOn==false)
      l_robCfg->clear();

   editAction->setChecked(false);
   addNodeAction->setChecked(false);
   addEdgeAction->setChecked(false);
   l_message->clear();
   l_icon->clear();
   m_bEditModel=false;
   m_addNode=false;
   m_addEdge=false;
}

void 
VizmoRoadmapGUI::getSelectedItem(){

  if(GetVizmo().GetMap() != NULL || GetVizmo().GetDebug() != NULL){ 
    if(m_nodeView->checkedButton() != 0){
      string s = (string)(m_nodeView->checkedButton())->text().ascii(); 
      m_shapeString = s; 
      GetVizmo().ChangeNodesShape(s);
      emit callUpdate(); //set an update event
    }   
  }
}


void 
VizmoRoadmapGUI::changeSize(){

   bool ok = false;
   size = QInputDialog::getDouble(tr("Change Roadmap Node Size"), 
         tr("Enter a positive number to scale the nodes"),
         size, 0, 1, 2,  &ok,  this);
  if(ok){
    string shape = (string)(m_nodeView->checkedButton())->text().ascii(); 
    GetVizmo().ChangeNodesSize(size, shape);
    emit callUpdate(); //set an update event
  }

}

void 
VizmoRoadmapGUI::setSameColor(){

  double R, G, B;
  R=G=B=1;
  string s = "all";
  GetVizmo().oneColor = true;
  QColor color = QColorDialog::getColor(Qt::white, this, "color dialog");
  
  if(color.isValid()){
    R = (double)(color.red()) / 255.0;
    G = (double)(color.green()) / 255.0;
    B = (double)(color.blue()) / 255.0;
  }

  string shape = (string)(m_nodeView->checkedButton())->text().ascii();
  GetVizmo().ChangeCCColor(R, G, B, shape);
  emit callUpdate(); //set an update event
}

void 
VizmoRoadmapGUI::changeColor(){
  GetVizmo().ChangeNodesRandomColor();
   emit callUpdate(); //set an update event
}

void 
VizmoRoadmapGUI::changeColorOfCCselected(){

  double R, G, B;
  R=G=B=1;

  //Check first if there is a CC selected
  vector<gliObj>& sel = GetVizmo().GetSelectedItem();
  typedef vector<gliObj>::iterator SI;
  string m_sO;
  for(SI i = sel.begin(); i!= sel.end(); i++){
    CGLModel *gl = (CGLModel*)(*i);
    m_sO = gl->GetName();
  }
  
  string m_s="NULL";
  size_t position = 0;
  position = m_sO.find("CC",0);
   
  if(position != string::npos){
    QColor color = QColorDialog::getColor(Qt::white, this, "color dialog");
    if (color.isValid()){
      R = (double)(color.red()) / 255.0;
      G = (double)(color.green()) / 255.0;
      B = (double)(color.blue()) / 255.0;
    }
  }

  string s = (string)(m_nodeView->checkedButton())->text().ascii();  
  GetVizmo().ChangeCCColor(R, G, B, s);
  emit callUpdate(); //set an update event
}

void VizmoRoadmapGUI::changeSolidOfNodeselected(){


   string s;
   int k = 0;
   vector<gliObj>& sel = GetVizmo().GetSelectedItem();
   typedef vector<gliObj>::iterator SI;
   for(SI i = sel.begin(); i!= sel.end(); i++){
      CGLModel *gl = (CGLModel*)(*i);
      gl->SetRenderMode(CPlumState::MV_SOLID_MODE);
      s = gl->GetName();
      k++;
   }
   GetVizmo().UpdateSelection();

   emit callUpdate(); //set an update event
}


void VizmoRoadmapGUI::changeWireOfNodeselected(){


   string s;
   vector<gliObj>& sel = GetVizmo().GetSelectedItem();
   typedef vector<gliObj>::iterator SI;
   for(SI i = sel.begin(); i!= sel.end(); i++){
      CGLModel *gl = (CGLModel*)(*i);
      gl->SetRenderMode(CPlumState::MV_WIRE_MODE);
   }
   GetVizmo().UpdateSelection();

   emit callUpdate(); //set an update event
}

void VizmoRoadmapGUI::changeInvisibleOfNodeselected(){

   string s;
   vector<gliObj>& sel = GetVizmo().GetSelectedItem();
   typedef vector<gliObj>::iterator SI;
   for(SI i = sel.begin(); i!= sel.end(); i++){
      CGLModel *gl = (CGLModel*)(*i);
      gl->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);
   }

   GetVizmo().UpdateSelection();

   emit callUpdate(); //set an update event
}


void VizmoRoadmapGUI::changeColorOfNodeselected(){

   double R, G, B;
   R=G=B=1;

   QColor color = QColorDialog::getColor(Qt::white, this, "color dialog");
   if (color.isValid()){
      R = (double)(color.red()) / 255.0;
      G = (double)(color.green()) / 255.0;
      B = (double)(color.blue()) / 255.0;
   }

   vector<gliObj>& sel = GetVizmo().GetSelectedItem();
   typedef vector<gliObj>::iterator SI;
   for(SI i = sel.begin(); i!= sel.end(); i++){
      CGLModel *gl = (CGLModel*)(*i);
      gl->m_RGBA[0]=R;
      gl->m_RGBA[1]=G;
      gl->m_RGBA[2]=B;

   }

   GetVizmo().UpdateSelection();
   emit callUpdate(); //set an update event
}

void VizmoRoadmapGUI::editMap()
{
   m_bEditModel=!m_bEditModel;
   if(m_bEditModel){ 
      m_Map_Changed=false;
      parentWidget()->setCursor(QCursor(Qt::SizeAllCursor));
      if(GetVizmo().GetMap() != NULL)
         GetVizmo().GetMap()->getModel()->EnableSelection(true);
      addEdgeAction->setEnabled(true);
      addEdgeAction->setChecked(false);
      addNodeAction->setEnabled(true);
      addNodeAction->setChecked(false);
   }
   else{
      if(GetVizmo().GetMap() != NULL)
         GetVizmo().GetMap()->getModel()->EnableSelection(false);
      addEdgeAction->setEnabled(false); addEdgeAction->setChecked(false);
      addNodeAction->setEnabled(false); addNodeAction->setChecked(false);
      m_addNode=false;
      m_addEdge=false;
      l_message->clear();
      l_icon->clear();
      l_cfg->clear();

      emit callUpdate();
   }
}

void VizmoRoadmapGUI::addNode()
{   
   m_addNode=!m_addNode;
   //turn off add edge
   m_addEdge=false;
   addEdgeAction->setChecked(false);
   if(m_addNode){
      l_message->setFrameStyle(Q3Frame::Panel | Q3Frame::Sunken);
      l_icon->setPixmap(QPixmap(icon_bulb));
      l_message->setPaletteForegroundColor(Qt::darkRed);
      l_message->setText("Add NODE is ON");        
   }
   else{  
      l_message->clear();
      l_icon->clear();
   }
}

void VizmoRoadmapGUI::addEdge()
{
   m_addEdge=!m_addEdge;
   //turn off add edge anyway
   m_addNode=false;
   addNodeAction->setChecked(false);
   if(m_addEdge){
      l_message->setFrameStyle(Q3Frame::Panel | Q3Frame::Sunken);
      l_icon->setPixmap(QPixmap(icon_bulb));
      l_message->setPaletteForegroundColor(Qt::darkRed);
      l_message->setText("Add EDGE is ON");  
   }
   else{
      l_message->clear();
      l_icon->clear();
   }
}

void VizmoRoadmapGUI::handleSelect()
{
   //find nodes
   m_Nodes.clear();
   vector<gliObj>& sel=GetVizmo().GetSelectedItem();
   typedef vector<gliObj>::iterator OIT;
   
   for(OIT i=sel.begin();i!=sel.end();i++){
      string myName = ((CGLModel*)(*i))->GetName();
      if(((CGLModel*)(*i)) != NULL)
           if(myName.find("Node")!=string::npos){
             m_Nodes.push_back((CGLModel*)(*i));

         }//end if

      if (robCfgOn){
         GetVizmo().getRoboCfg();
         printRobCfg();
      }

   }//end for
   if(!m_bEditModel){return;}
  
   if(m_Nodes.size() > 0){
      CGLModel * n=m_Nodes.front();
      printNodeCfg((CCfg*)n);
   }

   if(m_addEdge){
    
     handleAddEdge();

}
   else if(m_addNode){
          
      handleAddNode();
}
   else{
          
      handleEditMap();
}

}

void VizmoRoadmapGUI::handleAddEdge()
{

   //find two nodes...
   PlumObject * m_Map;
   m_Map = GetVizmo().GetMap();
   CCfg *cfg1, *cfg2;

   typedef CMapLoader<CCfg,Edge>::Wg WG;
   WG * graph;
   CMapLoader<CCfg,Edge> *m_loader=(CMapLoader<CCfg,Edge>*)m_Map->getLoader();
   graph = m_loader->GetGraph();

   //get from m_nodes node1 and node2
   //get VID from those Cfgs
   //add edge

   list<CGLModel*>::const_iterator it;
   for(it = m_Nodes.begin(); it != m_Nodes.end(); it++){
      Node_Edge.push_back(*it);
   }
   if(Node_Edge.size() == 2){
      cfg1 = (CCfg*)Node_Edge[0];
      cfg2 = (CCfg*)Node_Edge[1];
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
      m_CCModel->addEdge(cfg1, cfg2);

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
      emit callUpdate();

      Node_Edge.clear();
   }
}

void VizmoRoadmapGUI::handleAddNode()
{ 
   vector<gliObj>& sel=GetVizmo().GetSelectedItem();

   if(sel.size() !=0){ 
      
      if(!m_Nodes.empty()){
     
         CGLModel * n=m_Nodes.front(); 
         CCfg * cfg = (CCfg*)n;   
         //get current node's cfg
         vector<double> c = cfg->GetDataCfg();
         m_cfg = new double [c.size()];
         m_dof = c.size();

         for(unsigned int i=0; i<c.size(); i++){
            m_cfg[i] = c[i];
         }

         //create a window to let user change Cfg:
         createWindow();

         // to avoid add other more nodes every time the user clicks on
         // this cfg.

         addNodeAction->setChecked(false);
         m_addNode = false;
         l_message->clear();
         l_icon->clear();
      }

      else{ //no node selected and assumes there is not roadmap....

            if (GetVizmo().GetMap() == NULL) {
            CMapLoader<CCfg,Edge> * mloader=new CMapLoader<CCfg,Edge>();
            CMapModel<CCfg,Edge> * mmodel = new CMapModel<CCfg,Edge>();

            mmodel->SetMapLoader(mloader);

            PlumObject * m_Rob;
            m_Rob = GetVizmo().GetRobot();
            OBPRMView_Robot * r = (OBPRMView_Robot*)m_Rob->getModel();

            if(r != NULL)
               mmodel->SetRobotModel(r);

            GetVizmo().setMapObj(mloader, mmodel); 
            mloader->genGraph();

            //add node to graph
            typedef CMapLoader<CCfg,Edge>::Wg WG;
            WG * graph;
            graph = mloader->GetGraph();
            CCfg *cfgNew = new CCfg(); 

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
            GetVizmo().ShowRoadMap(true);

            cfgNew->SetCCModel(mmodel->GetCCModel(mmodel->number_of_CC()-1));

            //uselect
            GetVizmo().cleanSelectedItem();
            //select new node
            GetVizmo().addSelectedItem((CGLModel*)cfgNew);
            vector<double> cf = cfgNew->GetDataCfg();
            m_cfg = new double [cf.size()];
            for(unsigned int i=0; i<cf.size(); i++)
               m_cfg[i] = cf[i];

            noMap = true;

            createWindow();
            setShape();
            emit getSelectedItem();

            GetVizmo().Display();

            addNodeAction->setChecked(false);
            m_addNode = false;
            l_message->clear();
            l_icon->clear();

            noMap = false;
         }
      }
   }
}

void VizmoRoadmapGUI::handleEditMap()
{
   if(m_Nodes.empty()==false){
      CGLModel * n=m_Nodes.front();
      old_T[0]=n->tx(); old_T[1]=n->ty(); old_T[2]=n->tz();
      old_R[0]=n->rx(); old_R[1]=n->ry(); old_R[2]=n->rz();     

   }
}

void VizmoRoadmapGUI::MoveNode()
{
   if(m_Nodes.empty()) return;
   CGLModel * n=m_Nodes.front();
   double diff=fabs(old_T[0]-n->tx())+
      fabs(old_T[1]-n->ty())+
      fabs(old_T[2]-n->tz())+
      fabs(old_R[0]-n->rx())+
      fabs(old_R[1]-n->ry())+
      fabs(old_R[2]-n->rz());
   if(diff>1e-10){

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
            double * new_cfg = new double [v_cfg.size()];
            for(unsigned int v= 0; v<v_cfg.size(); v++)
               new_cfg[v] = v_cfg[v];
            nodeGUI->setNodeVal(v_cfg.size(), new_cfg);
            nodeGUI->filledFirstTime = false;
         }
      }
      m_Map_Changed=true;

      emit callUpdate();
   }   
}


void VizmoRoadmapGUI::createWindow(){

   nodeGUI = new queryGUI(this);  
   nodeGUI->filledFirstTime = true;

   nodeGUI->setNodeVal(m_dof, m_cfg);
   nodeGUI->show();

   connect(nodeGUI, SIGNAL(callUpdate()), this, SLOT(updateNodeCfg()));

   if(noMap == false){
      //translate new node one unit in X
      m_cfg[0]+=1;
      vector<double> newNodeCfg;
      for(int i=0; i<m_dof; i++)
         newNodeCfg.push_back(m_cfg[i]);


      PlumObject * m_Map;
      m_Map = GetVizmo().GetMap();

      typedef CMapLoader<CCfg,Edge>::Wg WG;
      WG * graph;
      CMapLoader<CCfg,Edge> *m_loader=(CMapLoader<CCfg,Edge>*)m_Map->getLoader();
      graph = m_loader->GetGraph();
      int numVert= graph->get_num_vertices();
      CCfg *cfgNew = new CCfg();
      cfgNew->SetCfg(newNodeCfg);
      cfgNew->SetIndex(numVert);
      graph->add_vertex(*cfgNew);

      //get mapModel and add a new elment to m_CCModels
      CMapModel<CCfg,Edge>* mmodel =(CMapModel<CCfg,Edge>*)m_Map->getModel();
      mmodel->AddCC(numVert);    

      cfgNew->SetCCModel(mmodel->GetCCModel(mmodel->number_of_CC()-1));

      GetVizmo().cleanSelectedItem();
      vector<gliObj>& sel=GetVizmo().GetSelectedItem();

      unsigned int *obj = NULL;
      mmodel->Select(&obj[0], sel);
      /////////////////////////////////////////////////    
      //select new node
      GetVizmo().addSelectedItem((CGLModel*)cfgNew);
      emit callUpdate();    
   }

   nodeGUI->filledFirstTime = false;
}

void VizmoRoadmapGUI::SaveNewRoadmap(const char *filename){

   WriteHeader(filename);
}


bool VizmoRoadmapGUI::WriteHeader(const char *filename){
   PlumObject * m_Map;
   m_Map = GetVizmo().GetMap();

   CMapHeaderLoader * maploader=(CMapHeaderLoader*)m_Map->getLoader();

   ofstream outfile (filename);

   if(maploader->ParseHeader()!=false){ //return false;

      const string version = maploader->GetVersionNumber();
      const string preamble = maploader->GetPreamble();
      const string envFile = maploader->GetEnvFileName();
      const list<string> lps = maploader->GetLPs();
      const list<string> cds = maploader->GetCDs();
      const list<string> dms = maploader-> GetDMs();
      const string seed = maploader-> GetSeed();
      outfile<< "Roadmap Version Number "<< version<<"\n";
      outfile<< "#####PREAMBLESTART##### \n";
      outfile<<preamble<<"\n";
      outfile<< "#####PREAMBLESTOP##### \n";
      outfile<< "#####ENVFILESTART##### \n";
      outfile<<envFile<<"\n";
      outfile<< "#####ENVFILESTOP##### \n";
      outfile<< "#####LPSTART##### \n";
      outfile<<lps.size()<<endl;
      list<string>::const_iterator it;
      for(it=lps.begin(); it!=lps.end(); ++it){
         outfile << *it << endl; // each element on a separate line
      }
      outfile<< "#####LPSTOP##### \n";
      outfile<< "#####CDSTART##### \n";
      outfile<<cds.size()<<endl;
      for(it=cds.begin(); it!=cds.end(); ++it){
         outfile << *it << endl; // each element on a separate line
      }
      outfile<< "#####CDSTOP##### \n";
      outfile<< "#####DMSTART##### \n";
      outfile<<dms.size()<<endl;
      for(it=dms.begin(); it!=dms.end(); ++it){
         outfile << *it << endl; // each element on a separate line
      }
      outfile<< "#####DMSTOP#####";

      if(version == "041805"){
         outfile<< "\n#####RNGSEEDSTART##### \n";
         outfile<< seed <<"\n";
         outfile<< "#####RNGSEEDSTOP##### \n";
      }
   }

   typedef CMapLoader<CCfg,Edge>::Wg WG;
   WG * graph;
   CMapLoader<CCfg,Edge> *m_loader=(CMapLoader<CCfg,Edge>*)m_Map->getLoader();
   graph = m_loader->GetGraph();

   write_graph(*graph, outfile);
   outfile.close();
   return true;

}


void VizmoRoadmapGUI::printNodeCfg(CCfg *c){

   vector<gliObj>& sel=GetVizmo().GetSelectedItem();
   typedef vector<gliObj>::iterator OIT;
   for(OIT i=sel.begin();i!=sel.end();i++){
         string myName = ((CGLModel*)(*i))->GetName();
         if(myName.find("Node")!=string::npos){

         sl_cfg.clear();s_cfg="";

         vector<string> info = c->GetNodeInfo();
         typedef vector<string>::iterator SIT;
         for(SIT i=info.begin(); i!=info.end(); i++){
            sl_cfg << QString(i->c_str());
         }

         for (QStringList::Iterator it = sl_cfg.begin(); it != sl_cfg.end(); ++it) {
            s_cfg+= *it;
         }

         l_cfg->setPaletteForegroundColor(Qt::darkGreen);

         if(c->m_coll)
            l_cfg->setPaletteForegroundColor(Qt::red);

         l_cfg->setText(s_cfg);
      }
   }
}


void VizmoRoadmapGUI::printRobCfg(){
   vector<gliObj>& sel=GetVizmo().GetSelectedItem();
   typedef vector<gliObj>::iterator OIT;
   for(OIT i=sel.begin();i!=sel.end();i++){
      if(((CGLModel*)(*i))->GetName()=="MultiBody") {

         GetVizmo().getRoboCfg();

         if(robCfgOn){
            l_robCfg->clear();
            vector<string> info = GetVizmo().getRobCfgText();
            QStringList strList;
            s_robCfg = "";

            typedef vector<string>::iterator SIT;
            for(SIT i=info.begin();i!=info.end();i++)
               strList << QString(i->c_str());

            for (QStringList::Iterator it = strList.begin(); it != strList.end(); ++it)
               s_robCfg+= *it;

            if(GetVizmo().getCD_value())
               l_robCfg->setPaletteForegroundColor(Qt::red);
            else
               l_robCfg->setPaletteForegroundColor(Qt::darkGreen);

            cout<<s_robCfg.toStdString()<<endl;
            l_robCfg->setText(s_robCfg);

         }
         else
            l_robCfg->hide();
      }

   }

}

void VizmoRoadmapGUI::createQGrid(){

   m_vbox = new Q3VBox(this);

   m_grid = new Q3HBox(m_vbox);
   l_icon = new QLabel(m_grid);
   l_message = new QLabel(m_grid);
   l_cfg = new QLabel(m_grid); 
   l_cfg->setEnabled(false);
   l_cfg->hide();
   new QLabel(m_grid);


}

void VizmoRoadmapGUI::createRobotToolBar(){
   l_robCfg = new QLabel(m_vbox);
   l_robCfg->setPaletteForegroundColor(Qt::darkGreen);
   l_robCfg->setFrameStyle(Q3Frame::Panel | Q3Frame::Sunken);
   l_robCfg->setEnabled(false);
   l_robCfg->hide();

}

void VizmoRoadmapGUI::updateNodeCfg(){
   if(nodeGUI!= NULL && nodeGUI->isVisible() && nodeGUI->filledFirstTime==false){
      if(!m_Nodes.empty()){
         CGLModel * n=m_Nodes.front(); 
         CCfg * cfg = (CCfg*)n;   
         vector<double> VNodeCfg;
         VNodeCfg.clear();

         VNodeCfg =  nodeGUI->getNodeCfg();

         cfg->SetCfg(VNodeCfg);
         cfg->GetCC()->ReBuildAll();
         emit callUpdate();
      }
   }
}

void VizmoRoadmapGUI::setShape(){

  m_nodeView->button(1)->click(); 
}



