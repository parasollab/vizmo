#include "AnimationWidget.h"

#include "Models/DebugModel.h"
#include "Models/EnvModel.h"
#include "Models/PathModel.h"
#include "Models/Vizmo.h"

#include "Icons/First.xpm"
#include "Icons/Last.xpm"
#include "Icons/Next.xpm"
#include "Icons/Previous.xpm"
#include "Icons/Play.xpm"
#include "Icons/BackPlay.xpm"
#include "Icons/Pause.xpm"

AnimationWidget::
AnimationWidget(QString _title, QWidget* _parent) :
  QToolBar(_title, _parent),
  m_name(""), m_frame(0), m_maxFrame(0), m_stepSize(5), m_forward(true) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    CreateGUI();
    setEnabled(false);

    // Initialize the timer
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(Timeout()));
  }

void
AnimationWidget::
Reset() {
  PauseAnimate();

  if(!GetVizmo().GetPathFileName().empty()) {
    m_name = "Path";
    m_maxFrame = GetVizmo().GetPath()->GetSize();
  }
  else if(!GetVizmo().GetDebugFileName().empty()) {
    m_name = "Debug";
    m_maxFrame = GetVizmo().GetDebug()->GetSize();
  }
  else {
    m_name = "";
    m_maxFrame = 0;
  }

  m_frame = 0;
  m_slider->setRange(1, m_maxFrame);
  m_slider->setValue(m_frame + 1);
  m_slider->setTickInterval(m_stepSize);
  m_totalSteps->setNum(m_maxFrame);
  m_stepValidator->setRange(1, m_maxFrame);
  m_frameValidator->setRange(1, m_maxFrame);

  //disable/enable this toolbar
  if(m_maxFrame == 0)
    setEnabled(false);
  else
    setEnabled(true);
}

void
AnimationWidget::
GoToFrame(int _frame) {
  //update frame value
  m_frame = _frame;
  if(m_frame >= m_maxFrame)
    m_frame -= m_maxFrame;
  else if(m_frame < 0)
    m_frame += m_maxFrame;

  //update slider and frame display
  m_slider->blockSignals(true);
  m_slider->setValue(m_frame + 1);
  m_slider->blockSignals(false);
  m_frameField->setText(QString::number(m_frame + 1)); //reset the number

  //update GUI
  if(m_name == "Path") {
    const CfgModel& cfg = GetVizmo().GetPath()->GetConfiguration(m_frame);
    GetVizmo().GetEnv()->Configure(cfg);
  }
  else if(m_name == "Debug")
    GetVizmo().GetDebug()->ConfigureFrame(m_frame);
  emit CallUpdate();
}

void
AnimationWidget::
BackAnimate() {
  m_forward = false;
  m_timer->start(100);
}

void
AnimationWidget::
Animate() {
  m_forward = true;
  m_timer->start(100);
}

void
AnimationWidget::
PauseAnimate() {
  m_timer->stop();
}

void
AnimationWidget::
PreviousFrame() {
  m_forward = false;
  GoToFrame(m_frame - m_stepSize);
}

void
AnimationWidget::
NextFrame() {
  m_forward = true;
  GoToFrame(m_frame + m_stepSize);
}

void
AnimationWidget::
GoToFirst() {
  GoToFrame(0);
}

void
AnimationWidget::
GoToLast() {
  GoToFrame(m_maxFrame-1);
}

void
AnimationWidget::
SliderMoved(int _frame) {
  GoToFrame(_frame);
}

void
AnimationWidget::
UpdateStepSize() {
  m_stepSize = m_stepField->text().toInt();
  m_slider->setTickInterval(m_stepSize);
}

void
AnimationWidget::
UpdateFrame() {
  GoToFrame(m_frameField->text().toInt() - 1);
}

void
AnimationWidget::
Timeout() {
  if(m_forward)
    GoToFrame(m_frame + m_stepSize);
  else
    GoToFrame(m_frame - m_stepSize);
}

void
AnimationWidget::
CreateGUI() {
  CreateActions();
  addSeparator();
  CreateSlider();
  addSeparator();
  CreateStepInput();
  addSeparator();
  CreateFrameInput();
}

void
AnimationWidget::
CreateActions() {

  QAction* playPathAction = new QAction(QIcon(QPixmap(play)), tr("Play"), this);
  connect(playPathAction, SIGNAL(triggered()), SLOT(Animate()));

  QAction* playBackAction = new QAction(QPixmap(backplay), "BackPlay",this);
  connect(playBackAction, SIGNAL(triggered()), SLOT(BackAnimate()));

  QAction* pausePathAction = new QAction(QIcon(QPixmap(pauseIcon)), tr("Pause"),this);
  connect(pausePathAction, SIGNAL(triggered()), SLOT(PauseAnimate()));

  QAction* nextFrameAction = new QAction(QIcon(QPixmap(next1)), tr("NextFrame"),this);
  connect(nextFrameAction, SIGNAL(triggered()), SLOT(NextFrame()));

  QAction* previousFrameAction = new QAction(QIcon(QPixmap(previous)), tr("PreviousFrame"),this);
  connect(previousFrameAction, SIGNAL(triggered()), SLOT(PreviousFrame()));

  QAction* firstFrame = new QAction(QIcon(QPixmap(first)), tr("first"),this);
  connect(firstFrame, SIGNAL(triggered()), SLOT(GoToFirst()));

  QAction* lastFrame= new QAction(QIcon(QPixmap(last)), tr("last"),this);
  connect(lastFrame, SIGNAL(triggered()), SLOT(GoToLast()));

  addAction(playPathAction);
  addAction(playBackAction);
  addAction(pausePathAction);
  addAction(nextFrameAction);
  addAction(previousFrameAction);
  addAction(firstFrame);
  addAction(lastFrame);
}

void
AnimationWidget::
CreateSlider() {
  m_slider = new QSlider(Qt::Horizontal, this);
  m_slider->setRange(1, m_maxFrame);
  m_slider->setFixedSize(300, 22);
  m_slider->setTickPosition(QSlider::TicksBelow);

  connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(SliderMoved(int)));

  this->addWidget(m_slider);
}

void
AnimationWidget::
CreateStepInput() {
  m_stepField = new QLineEdit(this);
  m_stepField->setText(QString::number(m_stepSize));
  m_stepField->setMaximumSize(55, 22);
  m_stepValidator = new QIntValidator(1, m_maxFrame, m_stepField);
  m_stepField->setValidator(m_stepValidator);
  connect(m_stepField, SIGNAL(returnPressed()), SLOT(UpdateStepSize()));
  addWidget(new QLabel("Step = ", this));
  addWidget(m_stepField);
}

void
AnimationWidget::
CreateFrameInput() {
  m_frameField = new QLineEdit(this);             // delete this on cleaning
  m_frameField->setText(QString::number(m_frame + 1));
  m_frameField->setMaximumSize(55, 22);
  m_frameValidator = new QIntValidator(1, m_maxFrame, m_frameField);
  m_frameField->setValidator(m_frameValidator);
  connect(m_frameField, SIGNAL(returnPressed()), SLOT(UpdateFrame()));

  m_totalSteps = new QLabel(QString::number(m_maxFrame), this);

  addWidget(new QLabel("Frame = ", this));
  addWidget(m_frameField);
  addWidget(new QLabel(" / ", this));
  addWidget(m_totalSteps);
  addWidget(new QLabel(" frames ", this));
}
