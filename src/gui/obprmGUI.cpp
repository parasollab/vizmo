#include "vizmo2.h"
#include "obprmGUI.h"

#include "icon/folder.xpm"
#include "icon/clip.xpm"

///////////////////////////////
// Widgets
//////////////////////////////
#include <q3groupbox.h> 



////////////////////////
// miscelaneous
///////////////////////

//Added by qt3to4:
#include <QTextStream>
#include <QGridLayout>
#include <Q3Frame>
#include <QHBoxLayout>

#include <QtGui>

////////////////////////////////////////////////////////
// CLASS
// obprmGUI
////////////////////////////////////////////////////////

obprmGUI::obprmGUI
( QWidget *parent, const char *name, Qt::WFlags f)
: Q3TabDialog( parent, name, true, f )
{

  this->setPaletteBackgroundColor(QColor( 212, 212, 212 ));
  setupTabFile();
  setupTabBbox();
  setupTabGnodes();
  setupTabCnodes();
  setupTabLp();
  setupTabCD();
 
setOkButton(tr("Generate command"));
setCancelButton(tr("Cancel")); 

  connect( this, SIGNAL( applyButtonPressed() ), this, SLOT( makeCommandLine() ) );
}

void obprmGUI:: getOBPRMpath(){

  if(cb_numJoints->isOn())
    obprmPath = "obprm_serial ";
  else
    obprmPath = "obprm_rigid ";
}


void obprmGUI::makeCommandLine(){

  /////////////////////
  // FILES
  /////////////////////

  getOBPRMpath();
  command <<  obprmPath ;

  if(!fname->text().isEmpty()){
    command << " -f ";
    if(GetVizmo().getEnvFileName() != ""){
      QFileInfo fi(GetVizmo().getEnvFileName().c_str());
      command << fi.baseName() << " ";
    }
    else 
      command << fname->text() << " "; 

    command.append(" \\\n");

  }
  
  /////////////////////
  // CD
  /////////////////////
  command.append("-cd ");
  command << (bg_cd->selected())->text();
  
  command.append(" \\\n");

  /////////////////////
  //-bbox 
  /////////////////////
  if( bGrp->id(bGrp->selected())  == 1){
    if(xmin->text() != ""){
      command << "-bbox [";
      command <<xmin->text()<<","<<ymin->text()<<","<<zmin->text()<<","<<xmax->text()<< ","<<ymax->text()<<","<<zmax->text()<<"]";
      command.append(" \\\n");
    }
    command << "-bbox_scale " <<scale->text() << " \\\n";
  }

  //////////////////////////
  //append -gNodes option
  //////////////////////////
  if(lbMethod->count() !=0){
    for(unsigned int i=0; i<lbMethod->count() ; i++)
      methods << lbMethod->text(i) << " ";
  
    command.append("-gNodes ");

    for ( QStringList::Iterator it = methods.begin(); it != methods.end(); ++it ) {
      //QString myStr = *it;
      command.append(*it);
    }
    command <<" \\\n";
  }

  //////////////////////////
  //append -lp option
  //////////////////////////
  if(lbLP->count() != 0){

    for(unsigned int i=0; i<lbLP->count() ; i++)
      lps << lbLP->text(i) << " ";
    
    command.append("-lp ");

    for ( QStringList::Iterator it = lps.begin(); it != lps.end(); ++it ) {
      command.append(*it);
    }
    command <<" \\\n";
  }

  //////////////////////////
  //append -cNodes option
  //////////////////////////
  if(lbCnodes->count() !=0 ){
    for(unsigned int i =0; i<lbCnodes->count(); i++)
      cNode<< (lbCnodes->item(i))->text() << " ";
  
    command.append("-cNodes ");

    for ( QStringList::Iterator it = cNode.begin(); it != cNode.end(); ++it ) {
      QString myStr = *it;
      command.append(myStr);
    }
    //command <<" \\\n";
  }

  ///////////////////////////////
  // append options: numjoints,
  // posres, orires
  // alledges, partialedge
  //////////////////////////////

  if(cb_numJoints->isOn())
    command<<cb_numJoints->text()<<" "<<numJoints->text()<<" \\\n";
  if(cb_posRes->isOn())
    command<<cb_posRes->text()<<" "<<posRes->text()<<" \\\n";
  if(cb_oriRes->isOn())
    command<<cb_oriRes->text()<<" "<<oriRes->text()<<" \\\n";
  if(cb_allEdges->isOn())
    command<<cb_allEdges->text()<<" "<<addAllEdgs->text()<<" \\\n";;
  if(cb_partEdges->isOn())
    command<<cb_partEdges->text()<<" "<<addPartEdge->text()<<" \\\n";

  createTxtDialog();

  for ( QStringList::Iterator it = command.begin(); it != command.end(); ++it ) {
    strComm+= *it;
  }

  GetVizmo().setCommLine(command);

  if(caller == "mkmp"){
    if(!GetVizmo().isEnvChanged()){
      vizEditor->e->setText(strComm);
      showTxtDilaog();
    }
  }
  else{
    vizEditor->e->setText(strComm);
    showTxtDilaog();
  }

  
}

void obprmGUI::LP1(){
  lbLP->insertItem("straightline "+lp_strLine1->text()+" "+lp_strLine2->text()+" ");
}

void obprmGUI::LP2(){

  lbLP->insertItem("rotate_at_s s "+lp_rotate->text()+" ");
}

void obprmGUI::LP3(){
  lbLP->insertItem("a_star_dist "+lp_starD1->text()+ " " +lp_starD2->text()+ " ");
}

void obprmGUI::LP4(){
  lbLP->insertItem("a_star_clearance "+lp_starC1->text()+" "+lp_starC2->text()+" ");
}

void obprmGUI::LP5(){
  lbLP->insertItem("approx_spheres "+lp_appx->text()+ " ");
}


void obprmGUI::setupTabFile(){


  QWidget *main = new QWidget(this);
  main->setPaletteBackgroundColor(QColor( 230, 230, 250));
  QGridLayout *grid = new QGridLayout( main, 1, 1 );
  //grid->setAlignment( Qt::AlignHCenter);
  grid->setMargin(100);
  grid->setSpacing(15);

  grid->addWidget( new QLabel( "Filename:", main ), 0, 0);

  fname = new QLineEdit( main );
  fname->resize( fname->sizeHint() );
  fname->setText(GetVizmo().getEnvFileName().c_str());
  fname->setFocus();
  grid->addWidget( fname, 0, 1);

  QRadioButton * r1 = new QRadioButton("Other", main);
  connect( r1, SIGNAL(clicked()), this, SLOT(showFileDialog()) );
  grid->addWidget( r1, 1, 0);

  cb_partEdges = new QCheckBox("-addPartialEdge", main);
  grid->addWidget(cb_partEdges, 2, 0 );
  addPartEdge = new QLineEdit(main);
  addPartEdge->setText("0");
  grid->addWidget(addPartEdge, 2, 1 );

  cb_allEdges = new QCheckBox("-addAllEdges", main);
  grid->addWidget(cb_allEdges, 3, 0);
  addAllEdgs = new QLineEdit(main);
  addAllEdgs->setText("0");
  grid->addWidget(addAllEdgs, 3, 1);

  cb_posRes = new QCheckBox("-posres", main);
  grid->addWidget(cb_posRes, 4, 0);
  posRes = new QLineEdit(main);
  posRes->setText("0.05");
  grid->addWidget(posRes, 4, 1);
  grid->addWidget(new QLabel("Position Resolution *calculated*", main), 4, 2);

  cb_oriRes = new QCheckBox("-orires", main);
  grid->addWidget(cb_oriRes, 5, 0);
  oriRes = new QLineEdit(main);
  oriRes->setText("0.05");
  grid->addWidget(oriRes, 5, 1);
  grid->addWidget(new QLabel("Orientation Resolution *HARDCODED*", main), 5, 2);
 
  cb_numJoints = new QCheckBox("-numofjoints", main);
  grid->addWidget(cb_numJoints, 6, 0);
  numJoints = new QLineEdit(main);
  grid->addWidget(numJoints, 6, 1);

  addTab (main, QIcon(icon_folder), "General");

}

void obprmGUI::setupTabBbox(){

  Q3Grid*grid = new Q3Grid(3, Qt::Vertical, this);
  grid->setPaletteBackgroundColor(QColor(238, 232, 170));
  grid->setMargin(50);
  grid->setSpacing(4);
 
  bGrp = new Q3ButtonGroup(0);

  QRadioButton * rb1 = new QRadioButton("Default", grid);
  bGrp->insert(rb1, 0);
  rb1->setChecked( TRUE );
  connect( rb1, SIGNAL(clicked()), this, SLOT(getOption()) );

  QRadioButton * rb2 = new QRadioButton("Set", grid);
  bGrp->insert(rb2, 1);
  connect( rb2, SIGNAL(clicked()), this, SLOT(showBoxes()) );

  addBoxes(grid);
  addTab(grid, "BBox");

}

void obprmGUI::changeOption(const QString &s){

  if(s.compare(s, "BasicPRM") == 0)
    hideBasicObprmGUI();
  else if(s.compare(s, "BasicOBPRM") == 0)
    BasicObprmGUI(); 
  else if(s.compare(s, "OBPRM") == 0)
    ObprmGUI(); 
  else if(s.compare(s, "GaussPRM") == 0)
    GaussGUI(); 
  else if(s.compare(s, "BasicMAPRM") == 0)
    BMaprmGUI();
  else if(s.compare(s, "CSpaceMAPRM") == 0)
    CSpaceGUI();
  else if(s.compare(s, "OBMAPRM") == 0)
    OBMaprmGUI();

}


void obprmGUI::setupTabGnodes(){

  basicWidgt = new QWidget;
  basicWidgt->setPaletteBackgroundColor(QColor(185, 211, 238));
  commonGrid = new QGridLayout( basicWidgt, 6, 2);
  commonGrid->setMargin(10);
  commonGrid->setSpacing(5);
  
  combo = new QComboBox(basicWidgt , "myCombo" );
  combo->insertItem("BasicPRM", 0);
  combo->insertItem("BasicOBPRM", 1);
  combo->insertItem("OBPRM", 2);
  combo->insertItem("GaussPRM", 3);
  combo->insertItem("BasicMAPRM", 4);
  combo->insertItem("CSpaceMAPRM", 5);
  combo->insertItem("OBMAPRM", 6);
  connect( combo, SIGNAL( activated( const QString & ) ),
	   this, SLOT ( changeOption( const QString & ) ) );
  
  commonGrid->addMultiCellWidget( combo, 0, 0, 0, 1,
			    Qt::AlignCenter );

  createElemtsBasicPRM();

//   h = new QHBoxLayout ();
//   commonGrid->addMultiCellLayout(h, 3, 3, 0, 1, 
// 				 Qt::AlignCenter);

  QLabel *label = new QLabel( basicWidgt );
  label->setPaletteForegroundColor(Qt::black);
  label->setFrameStyle( Q3Frame::Panel | Q3Frame::Sunken);
  label->setText( "Double click to delete an item" );
  label->setAlignment(Qt::AlignHCenter);
  QFont f;
  f.setBold( TRUE );
  label->setFont(f);
  commonGrid->addMultiCellWidget(label, 2, 2, 0, 1);
 
  lbMethod = new Q3ListBox(basicWidgt);
  lbMethod->setPaletteForegroundColor(Qt::blue);
  connect(lbMethod, SIGNAL(selected ( const QString & )), 
	  this, SLOT(deleteMethod( const QString & )));
  //  h->addWidget(lbMethod);

  commonGrid->addWidget(lbMethod, 4, 0);

  QPushButton *acceptMethod = new QPushButton(QIcon(icon_clip), 
					      "Add method", basicWidgt);

  connect(acceptMethod, SIGNAL( clicked() ), this, SLOT( addMethod() ) );

  commonGrid->addMultiCellWidget(acceptMethod, 5, 5, 0, 1,
				 Qt::AlignCenter );
  
  lbMethod->insertItem("BasicPRM nodes 100");
  
  addTab(basicWidgt, "-gNodes");
  
}

void obprmGUI::setupTabCnodes(){

  tab4 = new Q3VBox(this);
  tab4->setMargin(3);

  createCNodesGUI();

  addTab(tab4, "-cNodes");

}

void obprmGUI::setupTabLp(){

  
  QWidget *main = new QWidget(this);
  main->setPaletteBackgroundColor(QColor(255, 255, 204));
  QGridLayout *grid = new QGridLayout( main, 1, 1 );
  grid->setMargin(100);
  grid->setSpacing(5);

  QLabel *l;
  l = new QLabel("lineSegmentLength : int", main);
  grid->addWidget(l, 0, 1, Qt::AlignCenter);
  l = new QLabel("binary search : int", main);
  grid->addWidget(l, 0, 2, Qt::AlignCenter);

  pb = new QPushButton(QIcon(icon_clip), "straightline", main);
  connect(pb, SIGNAL(clicked()), this, SLOT(LP1()));
  grid->addWidget(pb, 1, 0);


  lp_strLine1 = new QLineEdit(main);
  lp_strLine1->setText("0");
  grid->addWidget(lp_strLine1, 1, 1);
  lp_strLine2 = new QLineEdit(main);
  lp_strLine2->setText("0");
  grid->addWidget(lp_strLine2, 1, 2);

  pb = new QPushButton(QIcon(icon_clip), "rotate_at_s", main);
  connect(pb, SIGNAL(clicked()), this, SLOT(LP2()));
  grid->addWidget(pb, 3, 0);
 
  l = new QLabel("s : float", main);
  grid->addWidget(l, 2, 1, Qt::AlignCenter); 

  lp_rotate = new QLineEdit(main);
  lp_rotate->setText("0.5");
  grid->addWidget(lp_rotate, 3, 1);


  l = new QLabel("tries : int", main);
  grid->addWidget(l, 4, 1, Qt::AlignCenter);
  l = new QLabel("neighbors : int", main);
  grid->addWidget(l, 4, 2, Qt::AlignCenter);


  pb = new QPushButton(QIcon(icon_clip), "a_star_dist",main);
  connect(pb, SIGNAL(clicked()), this, SLOT(LP3()));
  grid->addWidget(pb, 5, 0);

  lp_starD1 = new QLineEdit(main);
  lp_starD1->setText("6");
  grid->addWidget(lp_starD1, 5, 1);
  lp_starD2 = new QLineEdit(main);
  lp_starD2->setText("3");
  grid->addWidget(lp_starD2, 5, 2);


  l = new QLabel("tries : int", main);
  grid->addWidget(l, 6, 1, Qt::AlignCenter);
  l = new QLabel("neighbors : int", main);
  grid->addWidget(l, 6, 2, Qt::AlignCenter);


  pb = new QPushButton(QIcon(icon_clip), "a_star_clearance", 
			 main, "a_star_clearance");
  connect(pb, SIGNAL(clicked()), this, SLOT(LP4()));
  grid->addWidget(pb, 7, 0);


  lp_starC1 = new QLineEdit(main);
  lp_starC1->setText("6");
  grid->addWidget(lp_starC1, 7, 1);
  lp_starC2 = new QLineEdit(main);
  lp_starC2->setText("3");
  grid->addWidget(lp_starC2, 7, 2);


  l = new QLabel("n : int", main);
  grid->addWidget(l, 8, 1, Qt::AlignCenter);

  pb = new QPushButton(QIcon(icon_clip), "approx_spheres",main);
  connect(pb, SIGNAL(clicked()), this, SLOT(LP5()));
  grid->addWidget(pb, 9, 0);

  lp_appx = new QLineEdit(main);
  lp_appx->setText("3");
  grid->addWidget(lp_appx, 9, 1);

  QLabel *label = new QLabel( main );
  label->setPaletteForegroundColor(Qt::darkGreen);
  label->setFrameStyle( Q3Frame::Panel | Q3Frame::Sunken);
  label->setText( "Double click to delete an item" );
  label->setAlignment(Qt::AlignHCenter);
  QFont f;
  f.setBold( TRUE );
  label->setFont(f);
  grid->addWidget(label,10,1);

  lbLP = new Q3ListBox(main);
  lbLP->setPaletteForegroundColor(Qt::blue);
  connect(lbLP, SIGNAL(selected( const QString & )),
	  this, SLOT(deleteLP(const QString & )));

  lbLP->insertItem("straightline");
  lbLP->insertItem("rotate_at_s s 0.5");
  grid->addMultiCellWidget(lbLP, 11, 11, 0, 2);

  addTab(main, "-lp");
}


void obprmGUI::setupTabCD(){

  Q3Grid *grid = new Q3Grid(2, Qt::Vertical, this);
  grid->setPaletteBackgroundColor(QColor(188, 238, 104));
  grid->setMargin(100);
  grid->setSpacing(0);

  QLabel *l = new QLabel( "Select one Collision Detection method", grid);
  l->setAlignment(Qt::AlignHCenter);
  QFont f;
  f.setBold( TRUE );
  l->setFont(f);

  Q3Grid* g = new Q3Grid(2,Qt::Horizontal,grid);
  g->setFrameStyle( Q3Frame::Box | Q3Frame::Plain);
  g->setSpacing(5);
  g->setMargin(50);

  bg_cd = new Q3ButtonGroup(0);
  QRadioButton *b_cd;

  b_cd = new QRadioButton("RAPID", g);
  bg_cd->insert( b_cd, 0 );
  b_cd->setChecked( true );
  new QLabel("- Robust and Accurate Polygon Interference Detection", g);

  b_cd = new QRadioButton("vclip",g);
  bg_cd->insert( b_cd, 1 );
  new QLabel("- Voronoi Clip", g);

  b_cd = new QRadioButton("PQP",g);
  bg_cd->insert( b_cd, 2);
  new QLabel("- Proximity Query Package", g);


  addTab(grid, "-cd");
}

//////////////////////////////
//Called from setupTabBbox()
//////////////////////////////

void obprmGUI::addBoxes(Q3Grid *grid){

  Q3Grid* g = new Q3Grid(6,Qt::Horizontal,grid);

  g->setFrameStyle( Q3Frame::Box | Q3Frame::Plain);
  g->setSpacing(3);
  g->setMargin(10);

  QLabel* l;
  l = new QLabel( "<b>Xmin:</b>", g);
  xmin = new QLineEdit( g );
  xmin->setDisabled(true);

  l = new QLabel( "<b>Ymin:</b>", g );
  ymin = new QLineEdit( g );
  ymin->setDisabled(true);

  l = new QLabel( "<b>Zmin:</b>", g );
  zmin = new QLineEdit( g );
  zmin->setDisabled(true);

  l= new QLabel( "<b>Xmax:</b>", g );
  xmax = new QLineEdit( g );
  xmax->setDisabled(true);

  l = new QLabel( "<b>Ymax:</b>", g );
  ymax = new QLineEdit( g );
  ymax->setDisabled(true);

  l = new QLabel( "<b>Zmax:</b>", g );
  zmax = new QLineEdit( g );
  zmax->setDisabled(true);

  l = new QLabel("Scale: ", g); 
  scale = new QLineEdit( g );
  scale->setText("2.0");
  scale->setDisabled(true);

}


/////////////////////////////////////////////////////////////
//
// CREATE GUI FOR -cNodes option
//
//  Called from setupTabBbox()
//
////////////////////////////////////////////////////////////

void obprmGUI::createCNodesGUI(){

  QLabel* l;
  QWidget *main = new QWidget(tab4);
  main->setPaletteBackgroundColor(QColor(152, 251, 152));//255, 204, 153
  // make a 1x1 grid; it will auto-expand
  QGridLayout *grid = new QGridLayout( main, 17, 7 );
  grid->setAlignment( Qt::AlignHCenter);
  grid->setSpacing(5);
 
  pbCN = new QPushButton(QIcon(icon_clip),"random",main);
  grid->addWidget(pbCN, 0, 0);
  connect(pbCN, SIGNAL(clicked()), this, SLOT(addRandom()) );

  pbCN = new QPushButton(QIcon(icon_clip),"closest",main);
  grid->addWidget(pbCN, 1, 0);
  connect(pbCN, SIGNAL(clicked()), this, SLOT(addClosest()) );

  closest = new QLineEdit(main);
  closest->setText("10");
  grid->addWidget(closest, 1, 1);
 
  pbCN = new QPushButton(QIcon(icon_clip),"closestVE",main);
  grid->addWidget(pbCN, 2, 0);
  connect(pbCN, SIGNAL(clicked()), this, SLOT(addClosestVE()) ); 

  closestVE = new QLineEdit( main );
  closestVE->setText("5");
  grid->addWidget(closestVE, 2, 1);

  pbCN = new QPushButton(QIcon(icon_clip),"components",main);
  grid->addWidget(pbCN, 4, 0);
  connect(pbCN, SIGNAL(clicked()), this, SLOT(addComp()) );

  l = new QLabel("kpairs:int", main);
  grid->addWidget(l, 3, 1, Qt::AlignCenter);
  l= new QLabel("smallcc:int", main);
  grid->addWidget(l, 3, 2, Qt::AlignCenter);

  componts1 = new QLineEdit(main);
  componts1->setText("3"); 
  grid->addWidget(componts1, 4, 1);

  componts2 = new QLineEdit(main);
  componts2->setText("4"); 
  grid->addWidget(componts2, 4, 2);

  pbCN = new QPushButton(QIcon(icon_clip),"obstBased",main);
  grid->addWidget(pbCN, 6, 0);
  connect(pbCN, SIGNAL(clicked()), this, SLOT(addObst()) );

  l = new QLabel("other:int",main);
  grid->addWidget(l, 5, 1, Qt::AlignCenter);
  l = new QLabel("self:int", main);
  grid->addWidget(l, 5, 2, Qt::AlignCenter);
  
  obstB_o = new QLineEdit(main);
  obstB_o->setText("10");
  grid->addWidget(obstB_o, 6, 1);

  obstB_s = new QLineEdit(main);
  obstB_s->setText("3");
  grid->addWidget(obstB_s, 6, 2);

  pbCN = new QPushButton(QIcon(icon_clip),"RRTexpand",main);
  grid->addWidget(pbCN, 8, 0);
  connect(pbCN, SIGNAL(clicked()), this, SLOT(addRRTe()) );

  l = new QLabel("iter:int", main);
  grid->addWidget(l, 7, 1, Qt::AlignCenter);
  l = new QLabel("step:int", main);
  grid->addWidget(l, 7, 2, Qt::AlignCenter);
  l = new QLabel("smallcc:int", main);
  grid->addWidget(l, 7, 3, Qt::AlignCenter);
  l = new QLabel("obst_clearance:int", main);
  grid->addWidget(l, 7, 4, Qt::AlignCenter);
  l = new QLabel("clearance_from_node:int", main);
  grid->addWidget(l, 7, 5, Qt::AlignCenter);

  rrt1 = new QLineEdit(main);
  rrt1->setText("50");
  grid->addWidget(rrt1, 8, 1);

  rrt2 = new QLineEdit(main);
  rrt2->setText("10000");
  grid->addWidget(rrt2, 8, 2);

  rrt3 = new QLineEdit(main); 
  rrt3->setText("3");
  grid->addWidget(rrt3, 8, 3);

  rrt4 = new QLineEdit(main);
  rrt4->setText("1");
  grid->addWidget(rrt4, 8, 4);
 
  rrt5 = new QLineEdit(main);
  rrt5->setText("1");
  grid->addWidget(rrt5, 8, 5);

  pbCN = new QPushButton(QIcon(icon_clip),"RRTcomponents",main);
  grid->addWidget(pbCN, 10, 0);
  connect(pbCN, SIGNAL(clicked()), this, SLOT(addRRTc()) );


  l = new QLabel("iter:int", main);
  grid->addWidget(l, 9, 1, Qt::AlignCenter);
  l = new QLabel("step:int", main);
  grid->addWidget(l, 9, 2, Qt::AlignCenter);
  l = new QLabel("smallcc:int", main);
  grid->addWidget(l, 9, 3, Qt::AlignCenter);
  l = new QLabel("obst_clearance:int", main);
  grid->addWidget(l, 9, 4, Qt::AlignCenter);
  l = new QLabel("clearance_from_node:int", main);
  grid->addWidget(l, 9, 5, Qt::AlignCenter);

  rrtc1 = new QLineEdit(main);
  rrtc1->setText("50");
  grid->addWidget(rrtc1, 10, 1);

  rrtc2 = new QLineEdit(main);
  rrtc2->setText("10000");
  grid->addWidget(rrtc2, 10, 2);
 
  rrtc3 = new QLineEdit(main);
  rrtc3->setText("3");
  grid->addWidget(rrtc3, 10, 3); 
 
  rrtc4 = new QLineEdit(main);
  rrtc4->setText("1");
  grid->addWidget(rrtc4, 10, 4);

  rrtc5 = new QLineEdit(main);
  rrtc5->setText("1");
  grid->addWidget(rrtc5, 10, 5);   

  pbCN = new QPushButton(QIcon(icon_clip),"RayTracer",main);
  grid->addWidget(pbCN, 12, 0);
  connect(pbCN, SIGNAL(clicked()), this, SLOT(addRT()) );


  l = new QLabel("bouncingMode:string", main); 
  grid->addWidget(l, 11, 1, Qt::AlignCenter);
  l = new QLabel("maxRays:int", main); 
  grid->addWidget(l, 11, 2, Qt::AlignCenter);
  l = new QLabel("maxBounces:int", main); 
  grid->addWidget(l, 11, 3,Qt::AlignCenter );
  l = new QLabel("maxRayLength:int", main); 
  grid->addWidget(l, 11, 4, Qt::AlignCenter);

  rt1 = new QLineEdit(main); 
  rt1->setText("targetOriented");
  grid->addWidget(rt1, 12, 1);
  rt2 = new QLineEdit(main); 
  rt2->setText("1");
  grid->addWidget(rt2, 12, 2);
  rt3 = new QLineEdit(main); 
  rt3->setText("10000");
  grid->addWidget(rt3, 12, 3);
  rt4 = new QLineEdit(main);
  rt4->setText("10000");
  grid->addWidget(rt4, 12, 4);

  l = new QLabel("schedulingMode:int", main); 
  grid->addWidget(l, 13, 1);
  l = new QLabel("scheduleMaxSize:int", main); 
  grid->addWidget(l, 13, 2);
  l = new QLabel("sampleMaxSize:int", main); 
  grid->addWidget(l, 13, 3)
;
  rt5 = new QLineEdit(main); 
  rt5->setText("largestToSmallest");
  grid->addWidget(rt5, 14, 1);
  rt6 = new QLineEdit(main); 
  rt6->setText("20");
  grid->addWidget(rt6, 14, 2);
  rt7 = new QLineEdit(main);
  rt7->setText("10"); 
  grid->addWidget(rt7, 14, 3);

  pbCN = new QPushButton(QIcon(icon_clip),"modifiedLM",main);
  grid->addWidget(pbCN, 16, 0);
  connect(pbCN, SIGNAL(clicked()), this, SLOT(addLM()) );


  l = new QLabel("kpairs:int", main);
  grid->addWidget(l, 15, 1, Qt::AlignCenter);
  l = new QLabel(" add:int", main);
  grid->addWidget(l, 15, 2, Qt::AlignCenter);
  l = new QLabel(" rfactor:int", main);
  grid->addWidget(l, 15, 3, Qt::AlignCenter);
  
  lm_p = new QLineEdit(main);
  lm_p->setText("5");
  grid->addWidget(lm_p, 16, 1);
  lm_a = new QLineEdit(main);
  lm_a->setText("20");
  grid->addWidget(lm_a, 16, 2);
  lm_r = new QLineEdit(main);
  lm_r->setText("2");
  grid->addWidget(lm_r, 16, 3);

  QLabel *label = new QLabel( main );
  label->setPaletteForegroundColor(Qt::black);
  label->setFrameStyle( Q3Frame::Panel | Q3Frame::Sunken );
  label->setText( "Double click to delete an item" );
  label->setAlignment(Qt::AlignHCenter);
  grid->addMultiCellWidget(label,17, 17,0, 6);
  QFont f;
  f.setBold( TRUE );
  label->setFont(f);
    
  hc = new QHBoxLayout ();
  grid->addMultiCellLayout(hc, 18, 18, 0, 6, 
				 Qt::AlignCenter);

  lbCnodes = new Q3ListBox(main);
  lbCnodes->setPaletteForegroundColor(Qt::blue);
  connect(lbCnodes, SIGNAL(selected ( const QString & )), 
	  this, SLOT(deleteCnodes( const QString & )));

  lbCnodes->insertItem("closest 10");
  

  hc->addWidget(lbCnodes);

}

/////////////////////////////////////////////////////////////
//
// CREATE GUI FOR EACH METHOD -gNodes option
//
////////////////////////////////////////////////////////////
void obprmGUI::createElemtsBasicPRM(){
  
  hlay = new QHBoxLayout ();
  commonGrid->addMultiCellLayout( hlay, 1, 1, 0, 1,
			    Qt::AlignCenter );

  //  commonGrid->setRowStretch(1, 60);

  hlay->addWidget( new QLabel("#nodes:",basicWidgt ));
  numNodes = new QLineEdit(basicWidgt);
  numNodes->setText("10");
  hlay->addWidget(numNodes);


  createElemtsBasicObprm();
  createElemtsObprm();
  createElmtsGauss();
  createElmtsBMaprm();

}

void obprmGUI::createElemtsBasicObprm(){

  l_s = new QLabel("shells", basicWidgt);
  hlay->addWidget(l_s);
  numShells = new QLineEdit( basicWidgt );
  numShells->setText("3");
  hlay->addWidget(numShells);

  hideElemtsBasicObprm();
}

void obprmGUI::createElemtsObprm(){

  l_collPair = new QLabel("collPair", basicWidgt);
  l_collPair->setAlignment (Qt::AlignVCenter);
  hlay->addWidget(l_collPair);

  cb_coll1 = new QComboBox(basicWidgt , "collPairCombo1" );
  cb_coll1->insertItem("cM", 0);
  cb_coll1->insertItem("rV", 1);
  cb_coll1->insertItem("rT", 2);
  cb_coll1->insertItem("rE", 3);
  cb_coll1->insertItem("rW", 4);
  cb_coll1->insertItem("cM_rV", 5);
  cb_coll1->insertItem("rV_rT", 6);
  cb_coll1->insertItem("rV_rW", 7);
  cb_coll1->insertItem("N_rT", 8);
  cb_coll1->insertItem("all", 9);
  hlay->addWidget(cb_coll1);

  cb_coll2 = new QComboBox( basicWidgt, "collPairCombo2" );
  cb_coll2->insertItem("cM", 0);
  cb_coll2->insertItem("rV", 1);
  cb_coll2->insertItem("rT", 2);
  cb_coll2->insertItem("rE", 3);
  cb_coll2->insertItem("rW", 4);
  cb_coll2->insertItem("cM_rV", 5);
  cb_coll2->insertItem("rV_rT", 6);
  cb_coll2->insertItem("rV_rW", 7);
  cb_coll2->insertItem("N_rT", 8);
  cb_coll2->insertItem("all", 9);
  cb_coll2->setCurrentItem(2);
  hlay->addWidget(cb_coll2);

 
  l_freePair = new QLabel("freePair",  basicWidgt);
  hlay->addWidget(l_freePair,Qt::AlignCenter);
 
  cb_free1 = new QComboBox( basicWidgt, "collFreeCombo1" );
  cb_free1->insertItem("cM", 0);
  cb_free1->insertItem("rV", 1);
  cb_free1->insertItem("rT", 2);
  cb_free1->insertItem("rE", 3);
  cb_free1->insertItem("rW", 4);
  cb_free1->insertItem("cM_rV", 5);
  cb_free1->insertItem("rV_rT", 6);
  cb_free1->insertItem("rV_rW", 7);
  cb_free1->insertItem("N_rT", 8);
  cb_free1->insertItem("all", 9);
  hlay->addWidget(cb_free1);

  cb_free2 = new QComboBox( basicWidgt, "collFreeCombo2" );
  cb_free2->insertItem("cM", 0);
  cb_free2->insertItem("rV", 1);
  cb_free2->insertItem("rT", 2);
  cb_free2->insertItem("rE", 3);
  cb_free2->insertItem("rW", 4);
  cb_free2->insertItem("cM_rV", 5);
  cb_free2->insertItem("rV_rT", 6);
  cb_free2->insertItem("rV_rW", 7);
  cb_free2->insertItem("N_rT", 8);
  cb_free2->insertItem("all", 9);
  cb_free2->setCurrentItem(2);
  hlay->addWidget(cb_free2);

  l_clear = new QLabel("clearFact",basicWidgt );
  hlay->addWidget(l_clear, Qt::AlignCenter);
  clearFact = new QLineEdit(basicWidgt);
  clearFact->setText("1.0");
  hlay->addWidget(clearFact);
  l_pct = new QLabel("pctSurf", basicWidgt);
  hlay->addWidget(l_pct, Qt::AlignCenter);
  pctSurf = new QLineEdit(basicWidgt);
  pctSurf->setText("1.0");
  hlay->addWidget(pctSurf);

  hideElemtsObprm();
}

void obprmGUI::createElmtsGauss(){
  l_d = new QLabel("distance", basicWidgt);
  hlay->addWidget(l_d);
  distance = new QLineEdit(basicWidgt);
  hlay->addWidget(distance);
  hideElmtsGauss();

}

void obprmGUI::createElmtsBMaprm(){
  l_appx = new QLabel("appx",basicWidgt);
  hlay->addWidget(l_appx);
  appx = new QLineEdit(basicWidgt);
  appx->setText("1");
  hlay->addWidget(appx);

  l_app_ray = new QLabel("app_ray",basicWidgt);
  hlay->addWidget(l_app_ray);
  app_ray = new QLineEdit(basicWidgt);
  app_ray->setText("10");
  hlay->addWidget(app_ray);


  hideElmtsBMaprm();

}

void obprmGUI::createFileDialog(){

  fileDialog =  new obprmFileSelectDialog (this);

}

void obprmGUI::createTxtDialog(){

  //txtDialog = new textEditDialog(this, "textDialog", true);

  vizEditor = new vizmoEditor(this, "VizmoEditor");
  vizEditor->setCaption( "Vizmo++ Editor" );

}

///////////////////////////////////////////////////
// RESET ELEMENTS FOR BMaprm AND CSpace METHODS
// WHICH SHARE VARIABLES
////////////////////////////////////////////////////
void obprmGUI::resetElmtsBMaprm(){
  l_app_ray->setText("app_ray"); 
  app_ray->setText("10");
  l_appx->setText("appx");
  appx->setText("1");
}
void obprmGUI::resetElmtsCSpace(){
  l_app_ray->setText("clearance");
  app_ray->setText("5");
  l_appx->setText("penetration");
  appx->setText("5");
}

/////////////////////////////////////////////////////////////
//
// HIDE-ELEMENT FUNCTIONS
//
////////////////////////////////////////////////////////////
void obprmGUI::hideElemtsBasicPRM(){

  numNodes->hide();
}

void obprmGUI::hideElemtsBasicObprm(){ 
  l_s->hide(); 
  numShells->hide();
}
void obprmGUI::hideElemtsObprm(){

  l_collPair->hide();
  l_freePair->hide();
  cb_coll1->hide();
  cb_free1->hide();
  cb_coll2->hide();
  cb_free2->hide();

  l_clear->hide();
  clearFact->hide();
  l_pct->hide();
  pctSurf->hide();

}
void obprmGUI::hideElmtsGauss(){

  distance->hide();
  l_d->hide();
}
void obprmGUI::hideElmtsBMaprm(){

  l_app_ray->hide();
  app_ray->hide();
  l_appx->hide();
  appx->hide();

}



/////////////////////////////////////////////////////////////
//
// SHOW-ELEMENT FUNCTIONS
//
////////////////////////////////////////////////////////////

void obprmGUI::showElemtsBasicObprm(){

  l_s->show();
  numShells->show();


}
void obprmGUI::showElemtsObprm(){
  l_collPair->show();
  l_freePair->show();
  cb_coll1->show();
  cb_free1->show();
  cb_coll2->show();
  cb_free2->show();

  l_clear->show();
  clearFact->show();
  l_pct->show();
  pctSurf->show();
}
void obprmGUI::showElmtsGauss(){
  distance->show();
  l_d->show();
}
void obprmGUI::showElmtsBMaprm(){
  l_app_ray->show();
  app_ray->show();
  l_appx->show();
  appx->show();
}


////////////////////////////////////////////////////////////
//
//                     SLOTS
//
////////////////////////////////////////////////////////////
void obprmGUI::addLM(){
  lbCnodes->insertItem("modifiedLM "+lm_p->text()+" "
		     +lm_a->text()+" "+lm_r->text());
}
void obprmGUI::addObst(){
  lbCnodes->insertItem("obstBased "+obstB_o->text()+" "+obstB_s->text());
} 
void obprmGUI::addRT(){
    lbCnodes->insertItem("RayTracer "+rt1->text()+" "+rt2->text()+" "+
			 rt3->text()+" "+rt4->text()+" "+
			 rt5->text()+" "+rt6->text()+" "+rt7->text());
}
void obprmGUI::addRandom(){
  lbCnodes->insertItem("random");
}

void obprmGUI::addClosest(){
  lbCnodes->insertItem("closest "+closest->text());
}
void obprmGUI::addClosestVE(){
  lbCnodes->insertItem("closestVE "+closestVE->text());
}
void obprmGUI::addComp(){
  lbCnodes->insertItem("components "+componts1->text()+" "+componts2->text());
}
void obprmGUI::addRRTe(){
  lbCnodes->insertItem("RRTexpand "+rrt1->text()+" "
		     +rrt2->text()+" "+rrt3->text()+" "
		     +rrt4->text()+" "+rrt5->text());
}
void obprmGUI::addRRTc(){
  lbCnodes->insertItem("RRTcomponents "+rrtc1->text()+" "+rrtc2->text()+" "
		       +rrtc3->text()+" "+rrtc4->text()+" "+rrtc5->text());
}
void obprmGUI::deleteCnodes(const QString &s){
  lbCnodes->removeItem(lbCnodes->currentItem());
}


void obprmGUI::deleteLP(const QString &s){

  lbLP->removeItem(lbLP->currentItem());

}


void obprmGUI::deleteMethod(const QString &s){
  
  lbMethod->removeItem(lbMethod->currentItem());

}

void obprmGUI::addMethod(){
  QString s = "";
  s = combo->currentText()+" nodes "+numNodes->text()+" ";
  switch ( (int)combo->currentItem() )
  {
  case 0:
    lbMethod->insertItem(s);
    break;
  case 1: //basicOBPRM
    s+= l_s->text()+" "+numShells->text();
    lbMethod->insertItem( s );
    break;
  case 2: //OBPRM
    s+=l_s->text()+" "+numShells->text()+" ";
    s+=l_collPair->text()+" "+cb_coll1->currentText()+" "+cb_coll2->currentText()+" ";     
    s+=l_freePair->text()+" "+cb_free1->currentText()+" "+cb_free2->currentText()+" ";
    s+=l_clear->text()+" "+clearFact->text()+" "+l_pct->text()+" "+pctSurf->text();
    lbMethod->insertItem( s );
   break;
  case 3: //Gauss
    s+=" d "+distance->text();
    lbMethod->insertItem( s );
    break;
  case 4: //BasicMAPRM
    s+=l_app_ray->text()+" "+app_ray->text()+" "+l_appx->text()+" "+appx->text();
    lbMethod->insertItem( s );
    break;
  case 5: //CSpaceMAPRM
    s+=l_app_ray->text()+" "+app_ray->text()+" "+l_appx->text()+" "+appx->text();
    lbMethod->insertItem( s );
    break;
  case 6: //OBMAPRM
    s+=l_s->text()+" "+numShells->text()+" ";
    s+=l_app_ray->text()+" "+app_ray->text()+" ";
    s+=l_appx->text()+" "+appx->text()+" ";
    s+=l_collPair->text()+" "+cb_coll1->currentText()+" "+cb_coll2->currentText()+" ";     
    s+=l_freePair->text()+" "+cb_free1->currentText()+" "+cb_free2->currentText()+" ";
    s+=l_clear->text()+" "+clearFact->text()+" "+l_pct->text()+" "+pctSurf->text();
    lbMethod->insertItem( s );
    break;
//  default:
   
  }

}

void obprmGUI::hideBasicObprmGUI(){
  //hide all 

  hideElemtsBasicObprm();
  hideElemtsObprm();
  hideElmtsBMaprm();
  hideElmtsGauss();

}

void obprmGUI::BasicObprmGUI(){
  showElemtsBasicObprm();

  hideElemtsObprm();
  hideElmtsBMaprm();
  hideElmtsGauss();

}

void obprmGUI::ObprmGUI(){
  showElemtsBasicObprm();
  showElemtsObprm();

  hideElmtsBMaprm();
  hideElmtsGauss();

}

void obprmGUI::GaussGUI(){
  showElmtsGauss();
 
  hideElemtsBasicObprm();
  hideElemtsObprm();
  hideElmtsBMaprm();


}

void obprmGUI::BMaprmGUI(){
  resetElmtsBMaprm();
  showElmtsBMaprm();

  hideElemtsObprm();
  hideElemtsBasicObprm();
  hideElmtsGauss();

}

void obprmGUI::CSpaceGUI(){

  //CSpaceMAPRM and BasicMAPRM
  //will share variables:
  resetElmtsCSpace();
  showElmtsBMaprm();

  hideElemtsObprm();
  hideElemtsBasicObprm();
  hideElmtsGauss();


}

void obprmGUI::OBMaprmGUI(){
  //OBMAPRM will share gui with:
  // - BasicMAPRM
  // - OBPRM

  resetElmtsCSpace();
  showElmtsBMaprm();
  showElemtsBasicObprm();
  showElemtsObprm();
  
  hideElmtsGauss();

}

void obprmGUI::getOption(){

  xmin->setDisabled(true);
  ymin->setDisabled(true);
  zmin->setDisabled(true);
  xmax->setDisabled(true);
  ymax->setDisabled(true);
  zmax->setDisabled(true);
  scale->setDisabled(true);

 }

void obprmGUI::showBoxes(){
  xmin->setDisabled(false);
  ymin->setDisabled(false);
  zmin->setDisabled(false);
  xmax->setDisabled(false);
  ymax->setDisabled(false);
  zmax->setDisabled(false);
  scale->setDisabled(false);
}

void obprmGUI::showFileDialog(){
 createFileDialog();
  int r;
  r =   fileDialog->exec();
  if(!r) return;

}

void obprmGUI::showTxtDilaog(){

  //createTxtDialog();
  //int r;
  //r =   txtDialog->exec();
  //if(!r) return;

  vizEditor->show();

}

////////////////////////////////////////////////////////
// 
//                     CLASS
//               obprmFileSelectDialog
//
///////////////////////////////////////////////////////

obprmFileSelectDialog::obprmFileSelectDialog (QWidget *parent, Qt::WFlags f)
:QDialog(parent,f)
{


  QGridLayout * controls = new QGridLayout(this, 4, 3);
  controls->setSpacing(1);

  controls->addWidget(new QLabel("<b>Env File</b>:", this), 0, 0);
  Env_label = new QLabel(GetVizmo().getEnvFileName().c_str(),this );
  controls->addWidget(Env_label,0, 1);
  QPushButton * envButton = new QPushButton(QIcon(icon_folder),"Browse",this);  
  connect(envButton,SIGNAL(clicked()),this,SLOT(changeEnv()));
  controls->addWidget(envButton, 0, 2);


  controls->addWidget(new QLabel("<b>IN Map File</b>:", this), 1, 0);
  INMap_label=new QLabel(GetVizmo().getMapFileName().c_str(),this);
  QPushButton * inButton = new QPushButton(QIcon(icon_folder),"Browse",this);  
  connect(inButton,SIGNAL(clicked()),this,SLOT(changeMapIN()));
  controls->addWidget(INMap_label, 1, 1);
  controls->addWidget(inButton, 1, 2);

  controls->addWidget(new QLabel("<b>OUT MapFile</b>:", this), 2, 0);
  OUTMap_label = new QLabel(GetVizmo().getQryFileName().c_str(),this); 
  QPushButton * outButton = new QPushButton(QIcon(icon_folder),"Browse",this);  
  connect(outButton,SIGNAL(clicked()),this,SLOT(changeMapOUT()));
  controls->addWidget(OUTMap_label, 2, 1);
  controls->addWidget(outButton, 2, 2);

  QPushButton *go = new QPushButton("Done",this);
  connect(go,SIGNAL(clicked()),this,SLOT(accept()));
  controls->addWidget(go, 3, 1);

}


void obprmFileSelectDialog::changeMapIN()
{
    QString fn=QFileDialog::getOpenFileName(this, "Choose an INPUT  map file", 
					    QString::null, "Map File (*.map)");
    if ( !fn.isEmpty() ){
		GetVizmo().setMapFileName(fn.toStdString());
        INMap_label->setText(GetVizmo().getMapFileName().c_str());
    }
}
void obprmFileSelectDialog::changeMapOUT()
{
    QString fn=QFileDialog::getOpenFileName(this, "Choose an OUTPUT  map file",
					    QString::null,"Map File (*.map)");
    if ( !fn.isEmpty() ){
		GetVizmo().setMapFileName(fn.toStdString());
        OUTMap_label->setText(GetVizmo().getMapFileName().c_str());
    }
}

void obprmFileSelectDialog::changeEnv()
{
    QString fn=QFileDialog::getOpenFileName(this, "Choose an environment file", 
					    QString::null,"Env File (*.env)");
    if ( !fn.isEmpty() ){
		GetVizmo().setEnvFileName(fn.toStdString());
        Env_label->setText(GetVizmo().getEnvFileName().c_str());
    }
}


////////////////////////////////////////////////////////
// 
//                     CLASS
//               textEditDialog
//
///////////////////////////////////////////////////////

textEditDialog::textEditDialog(QWidget *parent, const char *name, Qt::WFlags f)
:QDialog(parent,name,true,f)
{

  QGridLayout * g = new QGridLayout(this, 15, 15);
  g->setSpacing(1); 

  g->addMultiCellWidget(new QLabel("<b>You can Edit the command line</b>", this), 
			0,1, 0,14,Qt::AlignCenter);

  txt = new Q3TextEdit(this);

  QHBoxLayout *hb = new QHBoxLayout();
  hb->addWidget(txt);

  g->addMultiCellLayout(hb, 2, 13, 0, 14, 
				 Qt::AlignCenter);


  pb = new QPushButton("Save", this);
  connect(pb, SIGNAL(clicked()), this, SLOT(createFile()));

  g->addMultiCellWidget( pb, 14, 14, 1,2,
			 Qt::AlignCenter );
 
  pb = new QPushButton("Exec", this);
  connect(pb, SIGNAL(clicked()), this, SLOT( execFile() ));

  g->addMultiCellWidget( pb, 14, 14, 7, 8,
			 Qt::AlignCenter );

  pb = new QPushButton("Close", this);
  connect(pb, SIGNAL(clicked()), this, SLOT( accept() ));

  g->addMultiCellWidget( pb, 14,14, 13, 14,
			 Qt::AlignCenter );
}

void textEditDialog::createFile(){

  fn = QFileDialog::getSaveFileName(this, "Select a name for the file to be saved", 
				    QString::null, tr( "All Files (*)" ));
  if ( !fn.isEmpty() ) {
    
    QFile file( fn );
    if ( !file.open( QIODevice::WriteOnly ) )
      return;
    QTextStream ts( &file );
    ts <<txt->text();

  }
  QFileInfo fi(fn);
  system("chmod u+x "+fi.baseName());

}

void textEditDialog::execFile(){
  QFileInfo fi(fn);

  int i;
  puts ("Trying to execute command DIR and CHMOD");
  i = system("chmod u+x "+fi.baseName());
  if (i==-1) puts ("Error executing CHMOD");
  i = system (fi.dirPath (TRUE)+"/test_env/l_tunnel/scripTest");
  if (i==-1) puts ("Error executing DIR");
  else puts ("Commands successfully executed");
}
