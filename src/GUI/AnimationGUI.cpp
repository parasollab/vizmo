#include "AnimationGUI.h"

#include <QToolBar>
#include <QPixmap>
#include <QAction>
#include <QSlider>
#include <QTimer>
#include <QLineEdit>
#include <QLabel>
#include <QValidator>
#include <QString>

#include "Models/Vizmo.h"
#include "MainWin.h"

#include "Icons/First.xpm"
#include "Icons/Last.xpm"
#include "Icons/Next.xpm"
#include "Icons/Previous.xpm"
#include "Icons/Play.xpm"
#include "Icons/Playback.xpm"
#include "Icons/Pause.xpm"

/////////////////////////////////////////////////////////////////////////////////

VizmoAnimationGUI::VizmoAnimationGUI(QString _title, QWidget* _parent, string _name)
  : QToolBar(_title, _parent), m_name(_name) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    CreateGUI();
    setEnabled(false);

    // Initialize the timer
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    m_stepSize = 1;
    m_forwardDirection = true;
    m_maxValue = 0;
    m_curValue = 0;
  }

void
VizmoAnimationGUI::reset(){

  pauseAnimate();
  if(m_name == "Path")
    m_maxValue=GetVizmo().GetPathSize();
  else if(m_name == "Debug")
    m_maxValue=GetVizmo().GetDebugSize();
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
VizmoAnimationGUI::CreateGUI(){

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
VizmoAnimationGUI::CreateFrameInput(){

  m_frameLabel = new QLabel("Frame = ",this);     // need to delete it once done
  m_frameField = new QLineEdit(this);             // delete this on cleaning
  m_frameField->setText("0");
  m_frameField->setMaximumSize(55,22);
  m_frameField->setValidator(new QIntValidator(m_frameField));
  connect(m_frameField, SIGNAL(returnPressed()), SLOT(goToFrame()));
  m_slash = new QLabel(" / ",this);
  m_totalSteps = new QLabel("",this);
  m_totalSteps->setNum(0);
  m_framesLast = new QLabel(" frames ",this);

  this->addWidget(m_frameLabel);
  this->addWidget(m_frameField);
  this->addWidget(m_slash);
  this->addWidget(m_totalSteps);
  this->addWidget(m_framesLast);
}

void
VizmoAnimationGUI::CreateStepInput(){

  m_stepLabel = new QLabel("Step = ",this);
  m_stepField = new QLineEdit(this);
  m_stepField->setText("1");
  m_stepField->setMaximumSize(55,22);
  m_stepField->setValidator(new QIntValidator(m_stepField));
  connect(m_stepField,SIGNAL(returnPressed()),SLOT(updateStepSize()));

  this->addWidget(m_stepLabel);
  this->addWidget(m_stepField);
}

bool VizmoAnimationGUI::CreateActions(){

  m_playPathAction=new QAction(QIcon(QPixmap(play)), tr("Play"), this);
  connect(m_playPathAction,SIGNAL(triggered()),SLOT(animate2()));

  m_playBackAction = new QAction(QPixmap(playback),"BackPlay",this);
  connect(m_playBackAction,SIGNAL(triggered()),SLOT(backAnimate()));

  m_pausePathAction=new QAction(QIcon(QPixmap(pauseIcon)),tr("Pause"),this);
  connect(m_pausePathAction,SIGNAL(triggered()),SLOT(pauseAnimate()));

  m_nextFrameAction=new QAction(QIcon(QPixmap(next)),tr("NextFrame"),this);
  connect(m_nextFrameAction,SIGNAL(triggered()),SLOT(nextFrame()));

  m_previousFrameAction=new QAction(QIcon(QPixmap(previous)),tr("PreviousFrame"),this);
  connect(m_previousFrameAction,SIGNAL(triggered()),SLOT(previousFrame()));

  m_firstFrame=new QAction(QIcon(QPixmap(first)),tr("first"),this);
  connect(m_firstFrame,SIGNAL(triggered()),SLOT(gotoFirst()));

  m_lastFrame=new QAction(QIcon(QPixmap(last)),tr("last"),this);
  connect(m_lastFrame,SIGNAL(triggered()),SLOT(gotoLast()));

  this->addAction(m_playPathAction);
  this->addAction(m_playBackAction);
  this->addAction(m_pausePathAction);
  this->addAction(m_nextFrameAction);
  this->addAction(m_previousFrameAction);
  this->addAction(m_firstFrame);
  this->addAction(m_lastFrame);

  return true;
}

void
VizmoAnimationGUI::updateFrameCounter(int newValue){

  QString result;
  result=result.setNum(newValue);
  m_frameField->setText(result);
}

void
VizmoAnimationGUI::CreateSlider(){

  m_slider = new QSlider(Qt::Horizontal,this);
  m_slider->setRange(0,0);
  m_slider->setFixedSize(300,22);
  m_slider->setTickPosition(QSlider::TicksBelow);

  connect(m_slider,SIGNAL(valueChanged(int)),this,SLOT(sliderMoved(int)));
  connect(m_slider,SIGNAL(valueChanged(int)),this,SLOT(updateFrameCounter(int)));

  this->addWidget(m_slider);
}

void
VizmoAnimationGUI::pauseAnimate(){

  m_timer->stop();
}

void
VizmoAnimationGUI::backAnimate(){

  updateStepSize();
  m_forwardDirection=false;
  m_timer->start(100);
}

void
VizmoAnimationGUI::animate2(){

  // first update step size
  updateStepSize();
  m_forwardDirection = true;
  m_timer->start(100);
}

void
VizmoAnimationGUI::UpdateCurValue(int value){

  m_curValue = value;
  if(m_curValue >= m_maxValue)
    m_curValue = 0;
  else if(m_curValue < 0)
    m_curValue = m_maxValue-1;
}

void
VizmoAnimationGUI::updateStepSize(){

  QString newValue;
  int newValueint;
  bool conv;
  newValue = m_stepField->text();
  newValueint = newValue.toInt(&conv,10);
  if(newValueint <= 1) newValueint = 1;
  m_stepField->setText(newValue.setNum(newValueint));
  m_stepSize = newValueint;
  m_slider->setTickInterval(m_stepSize);
}

void
VizmoAnimationGUI::getStepSize(int& size){

  size = m_stepSize;
}

void
VizmoAnimationGUI::goToFrame(){

  // Get the number from the frameCoutner
  bool conv;
  QString frame = m_frameField->text();
  int frameint = frame.toInt(&conv,10);
  goToFrame(frameint);
}

void
VizmoAnimationGUI::goToFrame(int frame){

  // the silde will send the slider moved signal and
  // the robot will update position automatically
  UpdateCurValue(frame);
  m_slider->setValue(m_curValue);
  QString text;
  m_frameField->setText(text.setNum(m_curValue)); //reset the number
}

void
VizmoAnimationGUI::gotoFirst(){

  UpdateCurValue(0);
  m_slider->setValue(m_curValue);
}

void
VizmoAnimationGUI::gotoLast(){

  UpdateCurValue(m_maxValue-1);
  m_slider->setValue(m_curValue);
}

void
VizmoAnimationGUI::nextFrame(){

  m_curValue += m_stepSize;
  UpdateCurValue(m_curValue);
  m_slider->setValue(m_curValue);
}

void
VizmoAnimationGUI::previousFrame(){

  m_curValue -= m_stepSize;
  UpdateCurValue(m_curValue);
  m_slider->setValue(m_curValue);
}

void
VizmoAnimationGUI::sliderMoved(int newValue){

  UpdateCurValue(newValue);
  if(m_name == "Path")
    GetVizmo().Animate(m_curValue);
  else if(m_name == "Debug")
    GetVizmo().AnimateDebug(m_curValue);
  emit callUpdate();
}

void
VizmoAnimationGUI::timeout(){

  if(m_forwardDirection)
    m_curValue += m_stepSize;
  else
    m_curValue -= m_stepSize;
  UpdateCurValue(m_curValue);
  m_slider->setValue(m_curValue);
}


