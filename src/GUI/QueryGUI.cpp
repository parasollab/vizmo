#include "QueryGUI.h"

#include "Models/EnvModel.h"
#include "Models/RobotModel.h"
#include "Models/Vizmo.h"

QueryGUI::QueryGUI(QWidget* _parent, Qt::WFlags _f) : QDialog(_parent) {

  m_nodeCfg = NULL;
  m_filledFirstTime = false;
  typedef vector<Model*>::iterator GIT;
  vector< list<string> > info;
  string objName;
  //to store a cfg
  QString qs;
  //int I=0;
  for(GIT ig= GetVizmo().GetSelectedModels().begin();
      ig!=GetVizmo().GetSelectedModels().end(); ig++)
    objName = (*ig)->Name();

  m_objName = objName;

  RobotModel* robotModel = GetVizmo().GetRobot();
  m_dof = GetVizmo().GetEnv()->GetDOF();

  //here I should put query values...
  vector<double> qCfg = robotModel->CurrentCfg();

  QGridLayout* controls = new QGridLayout();
  controls->addWidget(new QLabel("Configuration ", this),
      0,0, Qt::AlignHCenter);
  controls->addWidget(new QLabel("to be added", this),
      0,1, Qt::AlignHCenter);
  controls->addWidget(new QLabel("<b>X</b>:     ", this),
      1,0 , Qt::AlignRight);
  m_stx = new QDoubleSpinBox(this);
  m_stx->setDecimals(DECIMALS);
  m_stx->setRange(M_MIN, M_MAX);
  m_stx->setSingleStep (STEP);
  m_stx->setSpecialValueText(qs.setNum(qCfg[0]));
  m_stx->setValue(qCfg[0]);
  m_stx->setButtonSymbols(QAbstractSpinBox::PlusMinus);
  connect(m_stx, SIGNAL(valueChanged (const QString&)), this,
      SLOT(newCfg(const QString&)) );

  controls->addWidget(m_stx, 1,1, Qt::AlignRight);
  controls->addWidget(new QLabel("<b>Y</b>:     ", this),
      2,0 , Qt::AlignRight);

  m_sty = new QDoubleSpinBox(this);
  m_sty->setDecimals (DECIMALS);
  m_sty->setRange(M_MIN, M_MAX);
  m_sty->setSingleStep (STEP);
  m_sty->setSpecialValueText(qs.setNum(qCfg[1]));
  m_sty->setValue (qCfg[1]);
  m_sty->setButtonSymbols(QAbstractSpinBox::PlusMinus);

  connect(m_sty, SIGNAL(valueChanged (const QString&)),this,
      SLOT(newCfg(const QString&)) );

  controls->addWidget(m_sty,2,1 , Qt::AlignRight);

  controls->addWidget(new QLabel("<b>Z</b>:     ", this),
      3,0 , Qt::AlignRight);

  m_stz = new QDoubleSpinBox(this);
  m_stz->setRange(M_MIN, M_MAX);
  m_stz->setDecimals (DECIMALS);
  m_stz->setSingleStep (STEP);
  m_stz->setSpecialValueText(qs.setNum(qCfg[2]));
  m_stz->setValue (qCfg[2]);
  m_stz->setButtonSymbols(QAbstractSpinBox::PlusMinus);

  connect(m_stz, SIGNAL(valueChanged (const QString&)),this,
      SLOT(newCfg(const QString&)) );

  controls->addWidget(m_stz,3,1 , Qt::AlignRight);

  QString qst;
  int i, j=0;
  m_spin.clear();
  for(i=3; i<m_dof; i++){
    controls->addWidget(new QLabel("<b>Dof " +qst.setNum(i)+"</b>:", this),
        i+1, 0, Qt::AlignRight);
    m_spin.push_back(new QDoubleSpinBox(this));
    controls->addWidget(m_spin[j], i+1, 1, Qt::AlignRight);
    m_spin[j]->setDecimals(DECIMALS);
    m_spin[j]->setRange(D_MIN, D_MAX);
    m_spin[j]->setSingleStep (STEP_ANGLE);
    m_spin[j]->setSpecialValueText(qs.setNum(qCfg[i]));
    m_spin[j]->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    m_spin[j]->setValue(qCfg[i]);
    connect(m_spin[j], SIGNAL(valueChanged(const QString&)),this,
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

void
QueryGUI::SetQuery(vector<double>& _q){
  m_queryOrg = _q;
}

void
QueryGUI::ResetPointer(){
}

void
QueryGUI::SetNodeVal(int _dof, double* _cfg){
  m_dof = _dof;
  m_nodeCfg = _cfg;

  m_stx->setValue(m_nodeCfg[0]);
  m_sty->setValue(m_nodeCfg[1]);
  m_stz->setValue(m_nodeCfg[2]);

  int j=0;
  for(int i=3; i<m_dof; i++){
    m_spin[j]->setValue(m_nodeCfg[i]);
    j++;
  }
}

///////////////////////////////////////////////////////////////////////////////
///////                        SLOTS
//////////////////////////////////////////////////////////////////////////////

void
QueryGUI::NewCfg(const QString&){
  if(m_objName == "MultiBody"){
    if (isActiveWindow ()){

      vector<double> qCfg2(m_dof);

      RobotModel* robotModel = GetVizmo().GetRobot();
      robotModel->BackUp();

      qCfg2[0] = (m_stx->value());
      qCfg2[1] = (m_sty->value());
      qCfg2[2] = (m_stz->value());

      for(int i = 3, j = 0; i<m_dof; ++i, ++j)
        qCfg2[i] = m_spin[j]->value();

      robotModel->Configure(qCfg2);

      emit CallUpdate();
    }
  }
  else { //node
    if(isVisible() && !m_filledFirstTime){
      vector<double> vCfg;
      vCfg.clear();
      vCfg.push_back(m_stx->value());
      vCfg.push_back(m_sty->value());
      vCfg.push_back(m_stz->value());
      int j=0;
      for(int i=3; i<m_dof; i++){
        vCfg.push_back(m_spin[j]->value());
        j++;
      }
      emit CallUpdate();
    }
  }
}

void
QueryGUI::UpdateQryCfg(){
  RobotModel* robotModel = GetVizmo().GetRobot();
  vector<double> qCfg3  = robotModel->CurrentCfg();

  m_stx->setValue(qCfg3[0]);
  m_sty->setValue(qCfg3[1]);
  m_stz->setValue(qCfg3[2]);

  int j=0;
  for(int i=3; i<m_dof; i++){
    m_spin[j]->setValue(qCfg3[i]);
    j++;
  }
}


void
QueryGUI::ResetCfg(){
  m_stx->setValue(m_queryOrg[0]);
  m_sty->setValue(m_queryOrg[1]);
  m_stz->setValue(m_queryOrg[2]);

  int j=0;
  for(int i=3; i<m_dof; i++){
    m_spin[j]->setValue(m_queryOrg[i]);
    j++;
  }
}

vector<double>
QueryGUI::GetNodeCfg(){
  vector<double> vCfg;
  vCfg.clear();
  vCfg.push_back(m_stx->value());
  vCfg.push_back(m_sty->value());
  vCfg.push_back(m_stz->value());
  int j=0;
  for(int i=3; i<m_dof; i++){
    vCfg.push_back(m_spin[j]->value());
    j++;
  }
  return vCfg;
}
