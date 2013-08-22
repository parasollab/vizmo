#include "QueryGUI.h"

#include "Models/Vizmo.h"

queryGUI::queryGUI(QWidget *parent, Qt::WFlags f) : QDialog(parent) {

  node_cfg = NULL;

  filledFirstTime = false;
  typedef vector<GLModel*>::iterator GIT;
  vector< list<string> > info;
  string objname;
  //to store a cfg
  QString qs;
  //int I=0;
  for(GIT ig= GetVizmo().GetSelectedModels().begin();ig!=GetVizmo().GetSelectedModels().end();ig++){
    GLModel * gl=(GLModel *)(*ig);
    //objname = gl->GetInfo();
    objname = gl->GetName();
  }
  m_objName = objname;

  RobotModel* robotModel = GetVizmo().GetRobot();
  m_dof = robotModel->GetEnvModel()->GetDOF();

  //here I should put query values...
  vector<double> Qcfg = robotModel->getFinalCfg();

  //will be used in newCfg funct.
  QcfgTmp = new double [m_dof];

  QGridLayout* controls = new QGridLayout();
  controls->addWidget(new QLabel("Configuration ", this),
      0,0, Qt::AlignHCenter);
  controls->addWidget(new QLabel("to be added", this),
      0,1, Qt::AlignHCenter);


  controls->addWidget(new QLabel("<b>X</b>:     ", this),
      1,0 , Qt::AlignRight);
  stx = new QDoubleSpinBox(this);
  stx->setDecimals(DECIMALS);
  stx->setRange(M_MIN, M_MAX);
  stx->setSingleStep (STEP);
  stx->setSpecialValueText(qs.setNum(Qcfg[0]));
  stx->setValue(Qcfg[0]);
  stx->setButtonSymbols(QAbstractSpinBox::PlusMinus);
  connect(stx, SIGNAL(valueChanged (const QString&)),this,
      SLOT(newCfg(const QString&)) );

  controls->addWidget(stx, 1,1, Qt::AlignRight);


  controls->addWidget(new QLabel("<b>Y</b>:     ", this),
      2,0 , Qt::AlignRight);


  sty = new QDoubleSpinBox(this);
  sty->setDecimals (DECIMALS);
  sty->setRange(M_MIN, M_MAX);
  sty->setSingleStep (STEP);
  sty->setSpecialValueText(qs.setNum(Qcfg[1]));
  sty->setValue (Qcfg[1]);
  sty->setButtonSymbols(QAbstractSpinBox::PlusMinus);

  connect(sty, SIGNAL(valueChanged (const QString&)),this,
      SLOT(newCfg(const QString&)) );

  controls->addWidget(sty,2,1 , Qt::AlignRight);

  controls->addWidget(new QLabel("<b>Z</b>:     ", this),
      3,0 , Qt::AlignRight);

  stz = new QDoubleSpinBox(this);
  stz->setRange(M_MIN, M_MAX);
  stz->setDecimals ( DECIMALS );
  stz->setSingleStep (STEP);
  stz->setSpecialValueText(qs.setNum(Qcfg[2]));
  stz->setValue (Qcfg[2]);
  stz->setButtonSymbols(QAbstractSpinBox::PlusMinus);

  connect(stz, SIGNAL(valueChanged (const QString&)),this,
      SLOT(newCfg(const QString&)) );

  controls->addWidget(stz,3,1 , Qt::AlignRight);

  QString qst;
  int i, j=0;
  vSpin.clear();
  for(i=3; i<m_dof; i++){
    controls->addWidget(new QLabel("<b>Dof " +qst.setNum(i)+"</b>:", this),
        i+1, 0, Qt::AlignRight);
    vSpin.push_back(new QDoubleSpinBox(this));
    controls->addWidget(vSpin[j], i+1, 1, Qt::AlignRight);
    vSpin[j]->setDecimals(DECIMALS);
    vSpin[j]->setRange(D_MIN, D_MAX);
    vSpin[j]->setSingleStep (STEP_ANGLE);
    vSpin[j]->setSpecialValueText(qs.setNum(Qcfg[i]));
    vSpin[j]->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    vSpin[j]->setValue(Qcfg[i]);
    connect(vSpin[j], SIGNAL(valueChanged(const QString&)),this,
        SLOT(newCfg(const QString&)) );
    j++;
  }

  QPushButton* go = new QPushButton("Set", this);
  connect(go, SIGNAL(clicked()), this, SLOT(SaveSG()));
  controls->addWidget(go, i+1, 0, Qt::AlignHCenter);
  QPushButton* reset = new QPushButton("Reset", this);
  connect(reset, SIGNAL(clicked()), this, SLOT(resetCfg()));
  controls->addWidget(reset, i+1, 1, Qt::AlignHCenter);
  QPushButton* close = new QPushButton("Close", this);
  connect(close, SIGNAL(clicked()), this, SLOT(accept()));
  controls->addWidget(close, i+1, 2, Qt::AlignHCenter);
}

///////////////////////////////////////////////////////////////////////////////
///////                        METHODS
//////////////////////////////////////////////////////////////////////////////

void queryGUI::setQuery(vector<double>& q){
  query_org = q;
}

void queryGUI::resetPointer(){

  for(int i=0; i<m_dof; i++)
    QcfgTmp[i]= 0.0;
}

void queryGUI::setNodeVal(int dof, double *cfg){

  m_dof = dof;
  node_cfg = cfg;

  stx->setValue(node_cfg[0]);
  sty->setValue(node_cfg[1]);
  stz->setValue(node_cfg[2]);

  int j=0;
  for(int i=3; i<m_dof; i++){
    vSpin[j]->setValue(node_cfg[i]);
    j++;
  }
}

///////////////////////////////////////////////////////////////////////////////
///////                        SLOTS
//////////////////////////////////////////////////////////////////////////////

void queryGUI::newCfg(const QString&){

  if(m_objName == "MultiBody") {
    if (isActiveWindow () ){

      double* Qcfg2 = new double[m_dof];

      RobotModel* robotModel = GetVizmo().GetRobot();
      robotModel->BackUp();
      Quaternion qx(0,Vector3d(1,0,0));
      Quaternion qy(0,Vector3d(0,1,0));
      Quaternion qz(0,Vector3d(0,0,1));
      Quaternion nq=qz*qy*qx;

      robotModel->getRobotModel()->q() = nq;

      Qcfg2[0] = (stx->value());
      Qcfg2[1] = (sty->value());
      Qcfg2[2] = (stz->value());


      int j=0;
      for(int i=3; i<m_dof; i++){
        Qcfg2[i] = vSpin[j]->value();
        j++;
      }
      robotModel->Configure(Qcfg2);
      delete [] Qcfg2;

      emit callUpdate();
    }
  }

  else { //node
    if(isVisible() && !filledFirstTime){
      vector<double> Vcfg;
      Vcfg.clear();
      Vcfg.push_back(stx->value());
      Vcfg.push_back(sty->value());
      Vcfg.push_back(stz->value());
      int j=0;
      for(int i=3; i<m_dof; i++){
        Vcfg.push_back(vSpin[j]->value());
        j++;
      }
      emit callUpdate();
    }
  }
}

void queryGUI::updateQryCfg(){

  RobotModel* robotModel = GetVizmo().GetRobot();
  vector<double> Qcfg3  = robotModel->getFinalCfg();

  stx->setValue(Qcfg3[0]);
  sty->setValue(Qcfg3[1]);
  stz->setValue(Qcfg3[2]);

  int j=0;
  for(int i=3; i<m_dof; i++){
    vSpin[j]->setValue(Qcfg3[i]);
    j++;
  }

}


void queryGUI::resetCfg(){

  stx->setValue(query_org[0]);
  sty->setValue(query_org[1]);
  stz->setValue(query_org[2]);

  int j=0;
  for(int i=3; i<m_dof; i++){
    vSpin[j]->setValue(query_org[i]);
    j++;
  }

}

vector<double> queryGUI::getNodeCfg(){

  vector<double> Vcfg;
  Vcfg.clear();
  Vcfg.push_back(stx->value());
  Vcfg.push_back(sty->value());
  Vcfg.push_back(stz->value());
  int j=0;
  for(int i=3; i<m_dof; i++){
    Vcfg.push_back(vSpin[j]->value());
    j++;
  }

  return Vcfg;

}


