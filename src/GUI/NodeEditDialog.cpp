#include <sstream>

#include "NodeEditDialog.h"
#include "GLWidget.h"
#include "Models/Vizmo.h"
#include "Models/CfgModel.h"
#include "Models/MapModel.h"
#include "Models/BoundingBoxModel.h"
#include "Models/BoundingSphereModel.h"

NodeEditSlider::NodeEditSlider(QWidget* _parent, string _label){

  m_layout = new QHBoxLayout();
  this->setLayout(m_layout);

  m_dofName = new QLabel(this);
  m_dofName->setFixedWidth(50);
  m_dofName->setStyleSheet("font: 9pt;");
  QString label = QString::fromStdString(_label);
  m_dofName->setText(label);
  m_layout->addWidget(m_dofName);

  m_slider = new QSlider(this);
  m_slider->setFixedWidth(180);
  m_slider->setOrientation(Qt::Horizontal);
  m_slider->installEventFilter(this);
  m_layout->addWidget(m_slider);

  m_dofValue = new QLabel(this);
  m_dofValue->setFixedWidth(65);
  m_dofValue->setStyleSheet("font: 9pt;");
  m_layout->addWidget(m_dofValue);

  connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(UpdateDOFLabel(int)));
}

void
NodeEditSlider::UpdateDOFLabel(int _newVal){

  ostringstream oss;
  oss << (double)_newVal/100000.0;
  QString qs(oss.str().c_str());
  m_dofValue->setText(qs);
}

bool
NodeEditSlider::eventFilter(QObject* _target, QEvent* _event){

  if(_target == m_slider && _event->type() == QEvent::Wheel){
    _event->ignore();
    return true;
  }
  else
    return false;
}

NodeEditDialog::NodeEditDialog(QWidget* _parent, CfgModel* _node, GLWidget* _scene)
: QDialog(_parent) {

  setWindowTitle("Modify Node");

  m_gLScene = _scene;

  m_nodeLabel = new QLabel(this);
  m_overallLayout = new QVBoxLayout();
  m_scrollAreaBoxLayout = new QVBoxLayout();
  m_scrollArea = new QScrollArea(this);
  m_scrollAreaBox = new QGroupBox(this);
  m_setButton = new QPushButton(this);
  m_sliderMapper = new QSignalMapper(this);

  setFixedWidth(390);
  setFixedHeight(300);

  m_overallLayout->addWidget(m_nodeLabel);
  m_overallLayout->addWidget(m_scrollArea);
  m_scrollAreaBoxLayout->setSpacing(0);
  m_scrollAreaBoxLayout->setContentsMargins(7, 7, 7, 7);

  connect(m_setButton, SIGNAL(pressed()), this, SLOT(close()));

  SetUpWidgets();
  SetCurrentNode(_node);
}

NodeEditDialog::~NodeEditDialog(){}

void
NodeEditDialog::SetUpWidgets(){

  vector<MultiBodyModel::DOFInfo>& dofInfo = MultiBodyModel::GetDOFInfo();
  for(size_t i = 0; i < dofInfo.size(); i++){
    ostringstream oss;
    oss << "DOF " << i;
    NodeEditSlider* s = new NodeEditSlider(this, oss.str());
    s->GetSlider()->setRange(100000*dofInfo[i].m_minVal, 100000*dofInfo[i].m_maxVal);
    //Longer/more informative DOF name pops up when moused over
    s->GetDOFName()->setToolTip(QString::fromStdString(dofInfo[i].m_name));
    connect(s->GetSlider(), SIGNAL(valueChanged(int)), m_sliderMapper, SLOT(map()));
    m_sliderMapper->setMapping(s->GetSlider(), i);
    m_sliders.push_back(s);
    m_scrollAreaBoxLayout->addWidget(s);
  }

  connect(m_sliderMapper, SIGNAL(mapped(int)), this, SLOT(UpdateDOF(int)));

  m_scrollAreaBox->setLayout(m_scrollAreaBoxLayout);
  m_scrollArea->setWidget(m_scrollAreaBox);
  m_setButton->setFixedWidth(75);
  m_setButton->setText("Set");
  m_overallLayout->addWidget(m_setButton);
  this->setLayout(m_overallLayout);
}

void
NodeEditDialog::SetCurrentNode(CfgModel* _node){

  m_currentNode = _node;

  ostringstream oss;
  oss << _node->Name() << " --- CCID: " << _node->GetCCID();
  QString qNodeLabel = QString::fromStdString(oss.str());
  m_nodeLabel->setText(qNodeLabel);

  const vector<double>& currCfg = _node->GetDataCfg();
  InitSliderValues(currCfg);
}

void
NodeEditDialog::InitSliderValues(const vector<double>& _vals){
  //Works on assumption that m_sliders and _vals are same size..
  for(size_t i = 0; i < m_sliders.size(); i++){
    (m_sliders[i])->GetSlider()->setSliderPosition(100000*_vals[i]);
    ostringstream oss;
    oss << _vals[i];
    QString qValLabel = QString::fromStdString(oss.str());
    (m_sliders[i])->GetDOFValue()->setText(qValLabel);
  }
}

void
NodeEditDialog::UpdateDOF(int _id){
  //Also assumes index alignment
  (*m_currentNode)[_id] = m_sliders[_id]->GetSlider()->value() / 100000.0;
  //Until GLTransform business is cleaned up
  m_currentNode->Translation()((*m_currentNode)[0], (*m_currentNode)[1],
    m_currentNode->GetIsVolumetricRobot() ? (*m_currentNode)[2] : 0);

  vector<EdgeModel>& cCEdges = m_currentNode->GetCC()->GetEdgesInfo();
  for(size_t i = 0; i < cCEdges.size(); i++){
    if(m_currentNode->GetIndex() == cCEdges[i].GetStartCfg().GetIndex())
      cCEdges[i].SetStartCfg(*m_currentNode);
    else if(m_currentNode->GetIndex() == cCEdges[i].GetEndCfg().GetIndex())
      cCEdges[i].SetEndCfg(*m_currentNode);
  }

  m_gLScene->updateGL();
}
