#include "AnimationWidget.h"

#include "Models/DebugModel.h"
#include "Models/PathModel.h"
#include "Models/RobotModel.h"
#include "Models/Vizmo.h"

#include "Icons/First.xpm"
#include "Icons/Last.xpm"
#include "Icons/Next.xpm"
#include "Icons/Previous.xpm"
#include "Icons/Play.xpm"
#include "Icons/BackPlay.xpm"
#include "Icons/Pause.xpm"

/////////////////////////////////////////////////////////////////////////////////

AnimationWidget::AnimationWidget(QString _title, QWidget* _parent)
  : QToolBar(_title, _parent), m_name("") {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    CreateGUI();
    setEnabled(false);

    // Initialize the timer
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(Timeout()));
    m_stepSize = 1;
    m_forwardDirection = true;
    m_maxValue = 0;
    m_curValue = 0;
  }

void
AnimationWidget::Reset(){
  PauseAnimate();

  if(GetVizmo().GetPathFileName()!="") {
    m_name = "Path";
    m_maxValue = GetVizmo().GetPath()->GetSize();
  }
  else if(GetVizmo().GetDebugFileName()!="") {
    m_name = "Debug";
    m_maxValue = GetVizmo().GetDebug()->GetSize();
  }
  else {
    m_name = "";
    m_maxValue = 0;
  }

  m_curValue=0;
  m_slider->setRange(0, m_maxValue-1);
  m_slider->setValue(0);
  m_slider->setTickInterval(10);
  m_totalSteps->setNum(m_maxValue);

  //disable/enable this toolbar
  if(m_maxValue == 0)
    setEnabled(false);
  else
    setEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
bool
AnimationWidget::CreateGUI(){

  CreateActions();
  addSeparator();
  CreateSlider();
  addSeparator();
  CreateStepInput();
  addSeparator();
  CreateFrameInput();

  return true;
}

void
AnimationWidget::CreateFrameInput(){

  m_frameField = new QLineEdit(this);             // delete this on cleaning
  m_frameField->setText("0");
  m_frameField->setMaximumSize(55,22);
  m_frameField->setValidator(new QIntValidator(m_frameField));
  connect(m_frameField, SIGNAL(returnPressed()), SLOT(GoToFrame()));
  m_totalSteps = new QLabel("",this);
  m_totalSteps->setNum(0);

  this->addWidget(new QLabel("Frame = ", this));
  this->addWidget(m_frameField);
  this->addWidget(new QLabel(" / ", this));
  this->addWidget(m_totalSteps);
  this->addWidget(new QLabel(" frames ", this));
}

void
AnimationWidget::CreateStepInput(){

  m_stepField = new QLineEdit(this);
  m_stepField->setText("1");
  m_stepField->setMaximumSize(55,22);
  m_stepField->setValidator(new QIntValidator(m_stepField));
  connect(m_stepField,SIGNAL(returnPressed()),SLOT(UpdateStepSize()));

  this->addWidget(new QLabel("Step = ", this));
  this->addWidget(m_stepField);
}

bool AnimationWidget::CreateActions(){

  QAction* playPathAction = new QAction(QIcon(QPixmap(play)), tr("Play"), this);
  connect(playPathAction, SIGNAL(triggered()), SLOT(Animate()));

  QAction* playBackAction = new QAction(QPixmap(backplay), "BackPlay",this);
  connect(playBackAction, SIGNAL(triggered()), SLOT(BackAnimate()));

  QAction* pausePathAction = new QAction(QIcon(QPixmap(pauseIcon)), tr("Pause"),this);
  connect(pausePathAction, SIGNAL(triggered()), SLOT(PauseAnimate()));

  QAction* nextFrameAction = new QAction(QIcon(QPixmap(next)), tr("NextFrame"),this);
  connect(nextFrameAction, SIGNAL(triggered()), SLOT(NextFrame()));

  QAction* previousFrameAction = new QAction(QIcon(QPixmap(previous)), tr("PreviousFrame"),this);
  connect(previousFrameAction, SIGNAL(triggered()), SLOT(PreviousFrame()));

  QAction* firstFrame = new QAction(QIcon(QPixmap(first)), tr("first"),this);
  connect(firstFrame, SIGNAL(triggered()), SLOT(GoToFirst()));

  QAction* lastFrame= new QAction(QIcon(QPixmap(last)), tr("last"),this);
  connect(lastFrame, SIGNAL(triggered()), SLOT(GoToLast()));

  this->addAction(playPathAction);
  this->addAction(playBackAction);
  this->addAction(pausePathAction);
  this->addAction(nextFrameAction);
  this->addAction(previousFrameAction);
  this->addAction(firstFrame);
  this->addAction(lastFrame);

  return true;
}

void
AnimationWidget::UpdateFrameCounter(int _newValue){

  QString result;
  result=result.setNum(_newValue);
  m_frameField->setText(result);
}

void
AnimationWidget::CreateSlider(){

  m_slider = new QSlider(Qt::Horizontal,this);
  m_slider->setRange(0,0);
  m_slider->setFixedSize(300,22);
  m_slider->setTickPosition(QSlider::TicksBelow);

  connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(SliderMoved(int)));
  connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(UpdateFrameCounter(int)));

  this->addWidget(m_slider);
}

void
AnimationWidget::PauseAnimate(){

  m_timer->stop();
}

void
AnimationWidget::BackAnimate(){

  UpdateStepSize();
  m_forwardDirection=false;
  m_timer->start(100);
}

void
AnimationWidget::Animate(){

  // first update step size
  UpdateStepSize();
  m_forwardDirection = true;
  m_timer->start(100);
}

void
AnimationWidget::UpdateCurValue(int _value){

  m_curValue = _value;
  if(m_curValue >= m_maxValue)
    m_curValue = 0;
  else if(m_curValue < 0)
    m_curValue = m_maxValue-1;
}

void
AnimationWidget::UpdateStepSize(){

  QString newValue;
  int newValueint;
  bool conv;
  newValue = m_stepField->text();
  newValueint = newValue.toInt(&conv, 10);
  if(newValueint <= 1) newValueint = 1;
  m_stepField->setText(newValue.setNum(newValueint));
  m_stepSize = newValueint;
  m_slider->setTickInterval(m_stepSize);
}

void
AnimationWidget::GetStepSize(int& _size){
  _size = m_stepSize;
}

void
AnimationWidget::GoToFrame(){
  // Get the number from the frameCoutner
  bool conv;
  QString frame = m_frameField->text();
  int frameInt = frame.toInt(&conv, 10);
  UpdateFrame(frameInt);
}

void
AnimationWidget::UpdateFrame(int _frame){
  // the silde will send the slider moved signal and
  // the robot will update position automatically
  UpdateCurValue(_frame);
  m_slider->setValue(m_curValue);
  QString text;
  m_frameField->setText(text.setNum(m_curValue)); //reset the number
}

void
AnimationWidget::GoToFirst(){
  UpdateCurValue(0);
  m_slider->setValue(m_curValue);
}

void
AnimationWidget::GoToLast(){
  UpdateCurValue(m_maxValue-1);
  m_slider->setValue(m_curValue);
}

void
AnimationWidget::NextFrame(){
  m_curValue += m_stepSize;
  UpdateCurValue(m_curValue);
  m_slider->setValue(m_curValue);
}

void
AnimationWidget::PreviousFrame(){
  m_curValue -= m_stepSize;
  UpdateCurValue(m_curValue);
  m_slider->setValue(m_curValue);
}

void
AnimationWidget::SliderMoved(int _newValue){
  UpdateCurValue(_newValue);
  if(m_name == "Path") {
    const vector<double>& dCfg = GetVizmo().GetPath()->GetConfiguration(_newValue).GetData();
    GetVizmo().GetRobot()->Configure(dCfg);
  }
  else if(m_name == "Debug")
    GetVizmo().GetDebug()->ConfigureFrame(_newValue);
  emit CallUpdate();
}

void
AnimationWidget::Timeout(){

  if(m_forwardDirection)
    m_curValue += m_stepSize;
  else
    m_curValue -= m_stepSize;
  UpdateCurValue(m_curValue);
  m_slider->setValue(m_curValue);
}
