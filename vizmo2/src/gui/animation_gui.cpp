#include "vizmo2.h"
#include "main_win.h"


#include "animation_gui.h"

///////////////////////////////////////////////////////////////////////////////// Include Qt Headers
#include <qapplication.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qaction.h>
#include <qslider.h>
#include <qlcdnumber.h>
#include <qtimer.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qvalidator.h>
#include <qstring.h>
#include <qmessagebox.h>


//using namespace Qt;

// Icons

//#include "icon/Camera2.xpm"


VizmoAnimationGUI::VizmoAnimationGUI(QMainWindow *parent,char *name)
  :QToolBar(parent,name)

{
 
  //  this->setLabel("Vizmo animation");
   
  CreateGUI();

  


  // Initialize the timer
  QTtimer=new QTimer(this);
  connect(QTtimer,SIGNAL(timeout()),this,SLOT(timeout()));
  stepSize=1;
  forwardDirection=true;


  
}

bool VizmoAnimationGUI::CreateGUI()
{
  CreateActions();



  
  CreateFrameInput();
  CreateStepInput();
  CreateSlider();
  return true;
}



void VizmoAnimationGUI::CreateFrameInput()
{
  
  QLabel *label = new QLabel("Frame ",this);        /// need to delete it once done
  frameCounter=new QLineEdit(this);                    // delete this on cleaning
  frameCounter->setValidator( new QIntValidator( frameCounter ) );
  connect(frameCounter,SIGNAL(returnPressed()),SLOT(goToFrame()));

}

void VizmoAnimationGUI::CreateStepInput()
{
  QLabel *step = new QLabel("Step Size ",this);
  stepField=new QLineEdit(this);
  stepField->setText("1");
  stepField->setValidator(new QIntValidator(stepField));
  connect(stepField,SIGNAL(returnPressed()),SLOT(updateStepSize()));
}



bool VizmoAnimationGUI::CreateActions()
{
  //////////////////////////////////////////////////////////////////////////////
// Setup the play button

 
  //playPathAction=new QAction("Play",QPixmap(Camera),"&Play",CTRL+Key_P,this,"play");

  playPathAction=new QAction("Play","&Play",Qt::CTRL+Qt::Key_P,this,"play");
  connect(playPathAction,SIGNAL(activated()),SLOT(animate2()));
  
  playBackAction = new QAction("BackPlay","&BackPLay",Qt::CTRL+Qt::Key_I,this,"BackPlay");
  connect(playBackAction,SIGNAL(activated()),SLOT(backAnimate()));

  pausePathAction=new QAction("Pause","&Pause",Qt::CTRL+Qt::Key_I,this,"pause");
  connect(pausePathAction,SIGNAL(activated()),SLOT(pauseAnimate()));
  
  nextFrameAction=new QAction("NextFrame","&NextFrame",Qt::CTRL+Qt::Key_I,this,"NextFrame");
  connect(nextFrameAction,SIGNAL(activated()),SLOT(nextFrame()));
  
  previousFrameAction=new QAction("PrevisouFrame","&PreviousFrame",Qt::CTRL+Qt::Key_I,this,"PreviousFrame");
  connect(previousFrameAction,SIGNAL(activated()),SLOT(previousFrame()));


  firstFrame=new QAction("first","&first",Qt::CTRL+Qt::Key_I,this,"first");
  connect(firstFrame,SIGNAL(activated()),SLOT(gotoFirst()));
  lastFrame=new QAction("last","&last",Qt::CTRL+Qt::Key_I,this,"last");
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
  int size;
  size=GetVizmo().getTimer();
  slider=new QSlider(Qt::Horizontal,this,"slider");

  slider->setRange(0,size);
  connect(slider,SIGNAL(valueChanged(int)),this,SLOT(sliderMoved(int)));
  connect(slider,SIGNAL(valueChanged(int)),this,SLOT(updateFrameCounter(int)));
  playPathAction->addTo(this);
  pausePathAction->addTo(this);
  playBackAction->addTo(this);
  nextFrameAction->addTo(this);
  previousFrameAction->addTo(this);
  firstFrame->addTo(this);
  lastFrame->addTo(this);
  
  
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

void VizmoAnimationGUI::updateStepSize()
{
  QString newValue;
  int newValueint;
  bool conv;
  newValue=stepField->text();
  newValueint=newValue.toInt(&conv,10);
  if(newValueint<=0)
    {
        QMessageBox::information(this,"Incorrect Step Size","Step has to be greated than 0",QMessageBox::Ok,QMessageBox::NoButton);
      return;
    } 
  stepSize=newValueint;
}


void VizmoAnimationGUI::goToFrame()
{
  // Get the number from the frameCoutner
  QString frame;
  bool conv;
  int frameint;
  frame=frameCounter->text();
  frameint=frame.toInt(&conv,10);
  if(frameint<0)
    {
      QMessageBox::information(this,"Incorrect Frame Number","Frame cannot be -ive",QMessageBox::Ok,QMessageBox::NoButton);
      return;
    }    
  // the silde will send the slider moved signal and the robot will update position automatically
  slider->setValue(frameint);
  
}


void VizmoAnimationGUI::gotoFirst()
{
  // slider->setValue(0);

  // think of a better way to go to a frame
  GetVizmo().GetConfiguration(0);
  
  emit callUpdate();
  
}


void VizmoAnimationGUI::gotoLast()
{
  slider->setValue(GetVizmo().getTimer());

}


void VizmoAnimationGUI::nextFrame()
{
  // sliderMoved(GetVizmo().GetCurrentCfg()+1);
   slider->setValue(GetVizmo().GetCurrentCfg()+1);
}

void VizmoAnimationGUI::previousFrame()
{
  // sliderMoved(GetVizmo().GetCurrentCfg()-1);
  slider->setValue(GetVizmo().GetCurrentCfg()-1);
}


void VizmoAnimationGUI::sliderMoved(int newValue)
{
  
  if(GetVizmo().GetCurrentCfg()==newValue)
    return;

  if(newValue>GetVizmo().getTimer() || newValue<0)
    return;

  GetVizmo().GetConfiguration(newValue);
  // slider->setValue(GetVizmo().GetCurrentCfg());
 
   emit callUpdate();
 
}


void VizmoAnimationGUI::timeout()
{
  
  // GetVizmo().Animate(true);
  // check to see if the last step has been reached!
  

  int newFrame;
  if(forwardDirection)
    newFrame=GetVizmo().GetCurrentCfg()+stepSize;
  else newFrame=GetVizmo().GetCurrentCfg()-stepSize;

  if(newFrame>GetVizmo().getTimer() || newFrame<0)
    {
      if(forwardDirection)
	newFrame=GetVizmo().getTimer();
      else
	newFrame=0;
      
      pauseAnimate();


    }
  
  GetVizmo().GetConfiguration(newFrame);

  
  emit callUpdate();
  slider->setValue(GetVizmo().GetCurrentCfg());

}








