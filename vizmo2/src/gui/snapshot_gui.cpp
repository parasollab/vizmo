
// GUI for screen shot toolbar

#include "vizmo2.h"
#include "main_win.h"  

#include "snapshot_gui.h"

#include "GL_Dump.cpp"
#include <GL/gliPickBox.h>



///////////////////////////////////////////////////////////////////////////////
//Include Qt Headers

#include <qapplication.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qaction.h>
#include <qlabel.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qfiledialog.h>


VizmoScreenShotGUI::VizmoScreenShotGUI(QMainWindow *parent, char *name) :QToolBar(parent,name)
{

  this->setLabel("Vizmo ScrenCapture");

  CreateGUI();

  boxPicture=false;
}


bool VizmoScreenShotGUI::CreateGUI()
{
  CreateActions();
 
  return true;
}


void VizmoScreenShotGUI::CreateActions()
{
  takePictureAction=new QAction("Picture","&Picture",Qt::CTRL+Qt::Key_I,this,"picture");
  
  takePictureAction->addTo(this);
  connect(takePictureAction,SIGNAL(activated()),SLOT(takeSnapshot()));

  takeBoxPictureAction=new QAction("BoxPicture","&BoxPicture",Qt::CTRL+Qt::Key_I,this,"BoxPicture");

  takeBoxPictureAction->addTo(this);

  connect(takeBoxPictureAction,SIGNAL(activated()),SLOT(takeBoxSnapshot()));


  

}


void VizmoScreenShotGUI::takeBoxSnapshot()
{
  boxPicture=!boxPicture;
 
  if(!boxPicture)
  {
    emit simulateMouseUp();
    emit callUpdate();
  }
 emit togleSelectionSignal();

}

void VizmoScreenShotGUI::takeSnapshot()
{
  int w,h,xOffset,yOffset;
  if(!boxPicture)
    {
       emit getScreenSize(&w,&h);
       xOffset=0;
       yOffset=0;
    }
  else
    {
      emit getBoxDimensions(&xOffset,&yOffset,&w,&h);
      
    }


  QFileDialog *fd=new QFileDialog(this,"file dialog",TRUE);
  fd->setMode(QFileDialog::AnyFile);
  
  QString fileName;
  if(fd->exec()==QDialog::Accepted)
    fileName=fd->selectedFile();

  const char *fname=fileName.data();

       
  dump(fname,xOffset,yOffset,w,h);
}
