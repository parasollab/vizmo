#include "NodeEditDialog.h"

#include <sstream>

#include "GLWidget.h"
#include "Models/BoundingBoxModel.h"
#include "Models/BoundingSphereModel.h"
#include "Models/CfgModel.h"
#include "Models/QueryModel.h"
#include "Models/MapModel.h"
#include "Models/MultiBodyModel.h"
#include "Models/Vizmo.h"

QValidator::State
NodeEditValidator::validate(QString& _s, int& _i) const {

  if(_s.isEmpty() || _s == "-" || _s == "." || _s == "-.")
    return QValidator::Intermediate;

  bool ok;
  double d = _s.toDouble(&ok);

  if(ok && d >= m_min && d <= m_max)
    return QValidator::Acceptable;
  else
    return QValidator::Invalid;
}

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

  m_dofValue = new QLineEdit(this);
  m_dofValue->setFixedSize(65, 22);
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

void
NodeEditSlider::MoveSlider(QString _inputVal){

 m_slider->setSliderPosition(_inputVal.toDouble() * 100000.0);
}

bool
NodeEditSlider::eventFilter(QObject* _target, QEvent* _event){

  if(_target == m_slider && _event->type() == QEvent::Wheel){
    _event->ignore(); //Prevent mouse wheel from moving sliders
    return true;
  }
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
  m_doneButton = new QPushButton(this);
  m_sliderMapper = new QSignalMapper(this);

  setFixedWidth(390);
  setFixedHeight(300);

  m_overallLayout->addWidget(m_nodeLabel);
  m_overallLayout->addWidget(m_scrollArea);
  m_scrollAreaBoxLayout->setSpacing(0);
  m_scrollAreaBoxLayout->setContentsMargins(3, 7, 3, 7); //L, T, R, B

  connect(m_doneButton, SIGNAL(pressed()), this, SLOT(close()));

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

    double minVal = dofInfo[i].m_minVal;
    double maxVal = dofInfo[i].m_maxVal;
    s->GetSlider()->setRange(100000*minVal, 100000*maxVal);
    s->GetDOFValue()->setValidator(new NodeEditValidator(minVal, maxVal, 5, s->GetDOFValue()));
    //Longer/more informative DOF name pops up when moused over
    s->GetDOFName()->setToolTip(QString::fromStdString(dofInfo[i].m_name));

    connect(s->GetSlider(), SIGNAL(valueChanged(int)), m_sliderMapper, SLOT(map()));
    connect(s->GetDOFValue(), SIGNAL(textEdited(const QString&)), s, SLOT(MoveSlider(QString)));

    m_sliderMapper->setMapping(s->GetSlider(), i);
    m_sliders.push_back(s);
    m_scrollAreaBoxLayout->addWidget(s);
  }

  connect(m_sliderMapper, SIGNAL(mapped(int)), this, SLOT(UpdateDOF(int)));

  m_scrollAreaBox->setLayout(m_scrollAreaBoxLayout);
  m_scrollArea->setWidget(m_scrollAreaBox);
  m_doneButton->setFixedWidth(80);
  m_doneButton->setText("Done");
  m_overallLayout->addWidget(m_doneButton);
  this->setLayout(m_overallLayout);
}

void
NodeEditDialog::SetCurrentNode(CfgModel* _node){
  m_currentNode = _node;

  if(_node->GetCC() != NULL)
    m_nodeLabel->setText(QString::fromStdString(_node->Name()));
  else
    m_nodeLabel->setText("Intermediate configuration");

  const vector<double>& currCfg = _node->GetData();
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

  CollisionCheck();

  if(m_currentNode->IsQuery()){
    GetVizmo().GetQry()->BuildModels();
    GetVizmo().PlaceRobot();
  }
  m_gLScene->updateGL();
}

void
NodeEditDialog::CollisionCheck(){
  //Checks boundaries for now
  //call collision stuff elsewhere!
  vector<MultiBodyModel::DOFInfo>& dofInfo = MultiBodyModel::GetDOFInfo();
  bool collFound = false; //new or existing
  for(size_t i = 0; i < dofInfo.size(); i++){
    if(((*m_currentNode)[i] <= dofInfo[i].m_minVal) ||
     ((*m_currentNode)[i] >= dofInfo[i].m_maxVal)){
      m_currentNode->SetInCollision(true); //test
      collFound = true;
    }
  }
  if(collFound == false)
    m_currentNode->SetInCollision(false);
}
