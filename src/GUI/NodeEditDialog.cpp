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

NodeEditSlider::NodeEditSlider(QWidget* _parent, string _label, string _details){

  QHBoxLayout* layout = new QHBoxLayout();
  this->setLayout(layout);

  QLabel* dofName = new QLabel(this);
  dofName->setFixedWidth(50);
  dofName->setStyleSheet("font: 9pt;");
  QString label = QString::fromStdString(_label);
  dofName->setText(label);
  dofName->setToolTip(QString::fromStdString(_details));
  layout->addWidget(dofName);

  m_slider = new QSlider(this);
  m_slider->setFixedWidth(180);
  m_slider->setOrientation(Qt::Horizontal);
  m_slider->installEventFilter(this);
  layout->addWidget(m_slider);

  m_dofValue = new QLineEdit(this);
  m_dofValue->setFixedSize(65, 22);
  m_dofValue->setStyleSheet("font: 9pt;");
  layout->addWidget(m_dofValue);

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

  m_glScene = _scene;

  setWindowTitle("Modify Node");
  setFixedWidth(390);
  setFixedHeight(300);

  QLabel* nodeLabel = new QLabel(this);
  QScrollArea* scrollArea = new QScrollArea(this);

  QVBoxLayout* scrollAreaBoxLayout = new QVBoxLayout();
  scrollAreaBoxLayout->setSpacing(0);
  scrollAreaBoxLayout->setContentsMargins(3, 7, 3, 7); //L, T, R, B

  QGroupBox* scrollAreaBox = new QGroupBox(this);
  scrollAreaBox->setLayout(scrollAreaBoxLayout);

  QDialogButtonBox* okayCancel = new QDialogButtonBox(this);
  okayCancel->setOrientation(Qt::Horizontal);
  okayCancel->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  connect(okayCancel, SIGNAL(accepted()), this, SLOT(accept()));
  connect(okayCancel, SIGNAL(rejected()), this, SLOT(reject()));

  QVBoxLayout* overallLayout = new QVBoxLayout();
  overallLayout->addWidget(nodeLabel);
  overallLayout->addWidget(scrollArea);
  overallLayout->addWidget(okayCancel);

  SetUpSliders(m_sliders);
  for(SIT it = m_sliders.begin(); it != m_sliders.end(); it++)
    scrollAreaBoxLayout->addWidget(*it);

  scrollArea->setWidget(scrollAreaBox);
  this->setLayout(overallLayout);

  SetCurrentNode(_node, nodeLabel);
}

NodeEditDialog::~NodeEditDialog(){}

void
NodeEditDialog::SetUpSliders(vector<NodeEditSlider*>& _sliders){

  vector<MultiBodyModel::DOFInfo>& dofInfo = MultiBodyModel::GetDOFInfo();
  QSignalMapper* sliderMapper = new QSignalMapper(this);
  connect(sliderMapper, SIGNAL(mapped(int)), this, SLOT(UpdateDOF(int)));

  for(size_t i = 0; i < dofInfo.size(); i++){
    ostringstream oss;
    oss << "DOF " << i;

    NodeEditSlider* s = new NodeEditSlider(this, oss.str(), dofInfo[i].m_name);
    QSlider* actualSlider = s->GetSlider();
    QLineEdit* dofValue = s->GetDOFValue();

    double minVal = dofInfo[i].m_minVal;
    double maxVal = dofInfo[i].m_maxVal;
    actualSlider->setRange(100000*minVal, 100000*maxVal);
    dofValue->setValidator(new NodeEditValidator(minVal, maxVal, 5, dofValue));

    connect(actualSlider, SIGNAL(valueChanged(int)), sliderMapper, SLOT(map()));
    connect(dofValue, SIGNAL(textEdited(const QString&)), s, SLOT(MoveSlider(QString)));

    sliderMapper->setMapping(actualSlider, i);
    _sliders.push_back(s);
  }
}

void
NodeEditDialog::SetCurrentNode(CfgModel* _node, QLabel* _nodeLabel){

  m_currentNode = _node;

  if(_node->GetCC() != NULL)
    _nodeLabel->setText(QString::fromStdString(_node->Name()));
  else
    _nodeLabel->setText("Intermediate configuration");

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
NodeEditDialog::exec(){

  //Save current DOF values in case user presses "cancel"
  m_oldValues.clear();
  for(SIT it = m_sliders.begin(); it != m_sliders.end(); it++)
    m_oldValues.push_back((*it)->GetSlider()->value());

  QDialog::exec();
}

void
NodeEditDialog::reject(){

  for(size_t i = 0; i < m_sliders.size(); i++)
    (m_sliders[i])->GetSlider()->setSliderPosition(m_oldValues[i]);

  QDialog::reject();
}

void
NodeEditDialog::UpdateDOF(int _id){
  //Also assumes index alignment
  (*m_currentNode)[_id] = m_sliders[_id]->GetSlider()->value() / 100000.0;

  ValidityCheck();

  if(m_currentNode->IsQuery()){
    GetVizmo().GetQry()->BuildModels();
    GetVizmo().PlaceRobot();
  }
  m_glScene->updateGL();
}

void
NodeEditDialog::ValidityCheck(){

  vector<MultiBodyModel::DOFInfo>& dofInfo = MultiBodyModel::GetDOFInfo();
  bool collFound = false; //new or existing
  for(size_t i = 0; i < dofInfo.size(); i++){
    if(((*m_currentNode)[i] <= dofInfo[i].m_minVal) ||
     ((*m_currentNode)[i] >= dofInfo[i].m_maxVal)){
      m_currentNode->SetValidity(false); //test
      collFound = true;
    }
  }
  if(collFound == false){
    m_currentNode->SetValidity(true);
    GetVizmo().CollisionCheck(*m_currentNode);
  }
}
