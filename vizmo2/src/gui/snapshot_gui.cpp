
// GUI for screen shot toolbar

#include "vizmo2.h"
#include "main_win.h"  

#include "snapshot_gui.h"

#include "GL_Dump.h"
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

#include <qimage.h>
#include <qlayout.h>
#include <qgrid.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qcombobox.h>
#include <qmultilineedit.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qvalidator.h>



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

  takeMoviePicturesAction=new QAction("MoviePicture","&MoviePicture",Qt::CTRL+Qt::Key_I,this,"MoviePicture");

  takeMoviePicturesAction->addTo(this);

  connect(takeMoviePicturesAction,SIGNAL(activated()),SLOT(takeMoviePictures()));
  

}


void VizmoScreenShotGUI::takeMoviePictures()
{

  // local variables

  int startFrame;
  int endFrame;
  int stepSize;
  
  int result;
  QString localFileName;

  mDialog=new MovieSaveDialog(this,"blah",0);
  result=mDialog->exec();


  if(!result)
    return;


      startFrame=mDialog->startIntFrame;
      endFrame=mDialog->endIntFrame;
      stepSize=mDialog->stepIntSize;
      
      localFileName=mDialog->sFileName.data();  
    
      // Check for Ranges
 
    
      const char *fext=mDialog->sFilter.data();

      fext++;      // get rid of the *
      
  int w,h;
  
  
  emit getScreenSize(&w,&h);
  
  QString temp;
  QString qfname;

  for(int i=startFrame;i<=endFrame;i+=stepSize)
    {
      // dump the image
      emit goToFrame(i);
      temp=temp.setNum(i);
      qfname=localFileName+temp;
      //  cout<<flush<<qfname;
      const char *fname=qfname.data();
      dump(fname,fext,0,0,w,h);
    }  

    

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

  
  fd->addFilter("Microsoft Bitmap (*.bmp)");
  fd->addFilter("Computer Graphics Metafile (*.CGM)");
  fd->addFilter("Bitmap Image File (*.DIB)");
  fd->addFilter("PostScript Interchange (*.EPI)");
  fd->addFilter("Encapsulated PostScript file (*.EPS)");
  fd->addFilter("Microsoft Icon (*.ico)");
  fd->addFilter("Paintbrush File (*.pcx)");
  fd->addFilter("*.pdf");
  fd->addFilter("*.png");
  fd->addFilter("*.html");
  fd->addFilter("*.tga");
  fd->addFilter("*.gif");
  fd->addFilter("*.jpeg");


  
  QString fileName;
  QString fileExt;
  if(fd->exec()==QDialog::Accepted)
    {
    fileName=fd->selectedFile();
    fileExt=fd->selectedFilter();
  const char *fname=fileName.data();
  const char *fext=fileExt.data();

      fext++;

       

  dump(fname,fext,xOffset,yOffset,w,h);
    }
}



// MovieSaveDialog class functions

MovieSaveDialog::MovieSaveDialog(QWidget *parent, const char *name, WFlags f):
  QDialog(parent,name,f)
{
    QVBoxLayout* vbox = new QVBoxLayout(this,8);
    vbox->setAutoAdd(TRUE);
    QGrid* controls = new QGrid(2,QGrid::Horizontal,this);
    controls->setSpacing(8);
    QLabel* l;

    l=new QLabel("StartFrame",controls); l->setAlignment(AlignCenter);
    startFrame=new QLineEdit(controls);
    startFrame->setValidator(new QIntValidator(startFrame));
    
    l=new QLabel("EndFrame",controls); l->setAlignment(AlignCenter);
    endFrame = new QLineEdit(controls);
    endFrame->setValidator(new QIntValidator(endFrame));

    l=new QLabel("Step Size",controls); l->setAlignment(AlignCenter);
    stepSize = new QLineEdit(controls);
    stepSize->setValidator(new QIntValidator(stepSize));


    
  
    QPushButton *fileNameButton = new QPushButton("Select Name",controls);  
    fnameLabel=new QLabel("File Name",controls); l->setAlignment(AlignCenter);
   

    QPushButton *go = new QPushButton("Go",controls);
    QPushButton *cancel = new QPushButton("Cancel",controls);
    
    connect(fileNameButton,SIGNAL(clicked()),this,SLOT(showFileDialog()));
    connect(cancel,SIGNAL(clicked()),this,SLOT(reject()));
    connect(go,SIGNAL(clicked()),this,SLOT(saveImages()));
    
  
}


void MovieSaveDialog::saveImages()
{
  storeAttributes();
  QDialog::accept();

}

void MovieSaveDialog::storeAttributes()
{
  QString tempStorage;
  bool conv;
  
  tempStorage=startFrame->text();
  startIntFrame=tempStorage.toInt(&conv,10);
  
  tempStorage=endFrame->text();
  endIntFrame=tempStorage.toInt(&conv,10);
  
  tempStorage=stepSize->text();
  stepIntSize=tempStorage.toInt(&conv,10);
  
 
  
  
  

}



void MovieSaveDialog::showFileDialog()
{

  QFileDialog *fd=new QFileDialog(this,"file dialog",TRUE);
  fd->setMode(QFileDialog::AnyFile);
  
  QString fileName;
  
  fd->addFilter("Microsoft Bitmap (*.bmp)");
  fd->addFilter("Computer Graphics Metafile (*.CGM)");
  fd->addFilter("Bitmap Image File (*.DIB)");
  fd->addFilter("PostScript Interchange (*.EPI)");
  fd->addFilter("Encapsulated PostScript file (*.EPS)");
  fd->addFilter("Microsoft Icon (*.ico)");
  fd->addFilter("Paintbrush File (*.pcx)");
  fd->addFilter("*.pdf");
  fd->addFilter("*.png");
  fd->addFilter("*.html");
  fd->addFilter("*.tga");
  fd->addFilter("*.gif");
  fd->addFilter("*.jpeg");


  if(fd->exec()==QDialog::Accepted)
  {
    sFileName=fd->selectedFile();
    sFilter=fd->selectedFilter();
    
    
    fnameLabel->setText(sFileName+sFilter);
  

  }



}
