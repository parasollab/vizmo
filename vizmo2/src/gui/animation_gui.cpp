#include "vizmo2.h"
#include "main_win.h"


#include "animation_gui.h"

///////////////////////////////////////////////////////////////////////////////// 
//// Include Qt Headers
#include <qapplication.h>
#include <qpixmap.h>
#include <qaction.h>
#include <qslider.h>
#include <qlcdnumber.h>
#include <qtimer.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qvalidator.h>
#include <qstring.h>
#include <qmessagebox.h>

// Icons
#include "icon/vcr/first.xpm"
#include "icon/vcr/last.xpm"
#include "icon/vcr/next.xpm"
#include "icon/vcr/previous.xpm"
#include "icon/vcr/play.xpm"
#include "icon/vcr/playback.xpm"
#include "icon/vcr/pause.xpm"

VizmoAnimationGUI::VizmoAnimationGUI(QMainWindow *parent,char *name)
:QToolBar("Animation",parent,QMainWindow::Bottom,true,name)

{
    this->setLabel("Vizmo animation");
    
    //  this->setLabel("Vizmo animation");
    CreateGUI();
    setEnabled(false);
    
    // Initialize the timer
    QTtimer=new QTimer(this);
    connect(QTtimer,SIGNAL(timeout()),this,SLOT(timeout()));
    stepSize=1;
    forwardDirection=true;
    max_value=0;
    cur_value=0;
}

void VizmoAnimationGUI::reset()
{
    pauseAnimate();
    max_value=GetVizmo().GetPathSize();
    cur_value=0;
    slider->setRange(0,max_value-1);
    slider->setValue(0);
    slider->setTickInterval(10);
    totalStep->setNum(max_value);
    
    //disable/enable this toolbar
    if( max_value==0 ) setEnabled(false);
    else setEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
bool VizmoAnimationGUI::CreateGUI()
{
    CreateActions();
    
    addSeparator();
    CreateSlider();
    addSeparator();
    CreateStepInput();
    addSeparator();
    CreateFrameInput();
    
    return true;
}

void VizmoAnimationGUI::CreateFrameInput()
{
    
    new QLabel("Frame = ",this);        /// need to delete it once done
    frameCounter=new QLineEdit(this);                   // delete this on cleaning
    frameCounter->setText("0");
    frameCounter->setMaximumSize(55,22);
    frameCounter->setValidator( new QIntValidator(frameCounter) );
    connect(frameCounter,SIGNAL(returnPressed()),SLOT(goToFrame()));
    new QLabel(" / ",this);
    totalStep=new QLabel("",this);
    totalStep->setNum(0);
    new QLabel(" frames ",this);
}

void VizmoAnimationGUI::CreateStepInput()
{
    new QLabel("Step = ",this);
    stepField=new QLineEdit(this);
    stepField->setText("10");
    stepField->setMaximumSize(55,22);
    stepField->setValidator(new QIntValidator(stepField));
    connect(stepField,SIGNAL(returnPressed()),SLOT(updateStepSize()));
}

bool VizmoAnimationGUI::CreateActions()
{
    //////////////////////////////////////////////////////////////////////////////
    // Setup the play button
    
    
    //playPathAction=new QAction("Play",QPixmap(Camera),"&Play",CTRL+Key_P,this,"play");
    
    playPathAction=new QAction("Play",QPixmap(icon_play),"&Play",Qt::CTRL+Qt::Key_P,this,"play");
    connect(playPathAction,SIGNAL(activated()),SLOT(animate2()));
    
    playBackAction = new QAction("BackPlay",QPixmap(icon_playback),"&BackPLay",Qt::CTRL+Qt::Key_I,this,"BackPlay");
    connect(playBackAction,SIGNAL(activated()),SLOT(backAnimate()));
    
    pausePathAction=new QAction("Pause",QPixmap(icon_pause),"&Pause",Qt::CTRL+Qt::Key_I,this,"pause");
    connect(pausePathAction,SIGNAL(activated()),SLOT(pauseAnimate()));
    
    nextFrameAction=new QAction("NextFrame",QPixmap(icon_next),"&NextFrame",Qt::CTRL+Qt::Key_I,this,"NextFrame");
    connect(nextFrameAction,SIGNAL(activated()),SLOT(nextFrame()));
    
    previousFrameAction=new QAction("PrevisouFrame",QPixmap(icon_previous),"&PreviousFrame",Qt::CTRL+Qt::Key_I,this,"PreviousFrame");
    connect(previousFrameAction,SIGNAL(activated()),SLOT(previousFrame()));
    
    
    firstFrame=new QAction("first",QPixmap(icon_first),"&first",Qt::CTRL+Qt::Key_I,this,"first");
    connect(firstFrame,SIGNAL(activated()),SLOT(gotoFirst()));
    lastFrame=new QAction("last",QPixmap(icon_last),"&last",Qt::CTRL+Qt::Key_I,this,"last");
    connect(lastFrame,SIGNAL(activated()),SLOT(gotoLast()));
    
    return true;
}

void VizmoAnimationGUI::updateFrameCounter(int newValue)
{
    QString result;
    result=result.setNum(newValue);
    frameCounter->setText(result);
}

void VizmoAnimationGUI::CreateSlider()
{
    playPathAction->addTo(this);
    playBackAction->addTo(this);
    pausePathAction->addTo(this);
    nextFrameAction->addTo(this);
    previousFrameAction->addTo(this);
    firstFrame->addTo(this);
    lastFrame->addTo(this);
    addSeparator();
    
    slider=new QSlider(Qt::Horizontal,this,"slider");   
    slider->setRange(0,0);
    slider->setFixedSize(300,22);
    slider->setTickmarks(QSlider::Below);
    
    connect(slider,SIGNAL(valueChanged(int)),this,SLOT(sliderMoved(int)));
    connect(slider,SIGNAL(valueChanged(int)),this,SLOT(updateFrameCounter(int)));
    
}

void VizmoAnimationGUI::pauseAnimate()
{
    QTtimer->stop();
}

void VizmoAnimationGUI::backAnimate()
{
    updateStepSize();
    forwardDirection=false;
    QTtimer->start(100);
}

void VizmoAnimationGUI::animate2()
{
    // first update step size
    updateStepSize();
    forwardDirection=true;
    QTtimer->start(100);
}

void VizmoAnimationGUI::UpdateCurValue(int value)
{   
    cur_value=value;
    if(cur_value>=max_value) cur_value=0;
    else if( cur_value<0 ) cur_value=max_value-1;
}

void VizmoAnimationGUI::updateStepSize()
{
    QString newValue;
    int newValueint;
    bool conv;
    newValue=stepField->text();
    newValueint=newValue.toInt(&conv,10);
    if(newValueint<=1) newValueint=1;
    stepField->setText(newValue.setNum(newValueint));
    stepSize=newValueint;
    slider->setTickInterval(stepSize);
}

void VizmoAnimationGUI::getStepSize(int& size)
{
    size=stepSize;
}

void VizmoAnimationGUI::goToFrame()
{
    // Get the number from the frameCoutner
    bool conv;
    QString frame=frameCounter->text();
    int frameint=frame.toInt(&conv,10);
    goToFrame(frameint);
}

void VizmoAnimationGUI::goToFrame(int frame)
{
    // the silde will send the slider moved signal and 
    // the robot will update position automatically
    UpdateCurValue(frame);
    slider->setValue(cur_value);
    QString text;
    frameCounter->setText(text.setNum(cur_value)); //reset the number
}

void VizmoAnimationGUI::gotoFirst()
{
    UpdateCurValue(0);  
    slider->setValue(cur_value);
}

void VizmoAnimationGUI::gotoLast()
{
    UpdateCurValue(max_value-1);
    slider->setValue(cur_value);
}

void VizmoAnimationGUI::nextFrame()
{
    cur_value+=stepSize;
    UpdateCurValue(cur_value);
    slider->setValue(cur_value);
}

void VizmoAnimationGUI::previousFrame()
{
    cur_value-=stepSize;
    UpdateCurValue(cur_value);
    slider->setValue(cur_value);
}

void VizmoAnimationGUI::sliderMoved(int newValue)
{
    UpdateCurValue(newValue);
    GetVizmo().Animate(cur_value);
    emit callUpdate();
}

void VizmoAnimationGUI::timeout()
{   
    if(forwardDirection) cur_value+=stepSize;
    else cur_value-=stepSize;
    UpdateCurValue(cur_value);
    slider->setValue(cur_value);
}