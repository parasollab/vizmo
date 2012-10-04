#include <QAction>  
#include <QToolBar>
#include <QPixmap>
#include <QTranslator>  
#include <QFileDialog> 
#include <QProgressDialog>
#include <QApplication> 

#include "CaptureOptions.h"
#include "MainWin.h"
#include "SceneWin.h" 
#include "MovieSaveDialog.h"
#include "AnimationGUI.h" 
#include "vizmo2.h"
#include <GL/gliDump.h>
#include <GL/gliPickBox.h> 
#include "icon/Crop.xpm"
#include "icon/Camera.xpm"
#include "icon/Camcorder.xpm" 

inline QStringList& Filters(){

  static QStringList filters;
     
  filters+="JPEG (*.jpg)";
  filters+="GIF (*.gif)";
  filters+="Encapsulated PostScript file (*.eps)";
  filters+="targa (*.tga)";
  filters+="Portable Network Graphics (*.png)";
  filters+="Microsoft Bitmap (*.bmp)";
  filters+="Computer Graphics Metafile (*.cgm)";
  filters+="PostScript Interchange (*.epi)";
  filters+="Microsoft Icon (*.ico)";
  filters+="Paintbrush File (*.pcx)";
  filters+="Portable Pixmap (*.ppm)";
  filters+="Adobe Portable Document Format (*.pdf)";
 
  return filters;
}

inline string Filter2Ext(const string filter){

  if(filter.find("jpg")!=string::npos) 
    return ".jpg";
  if(filter.find("gif")!=string::npos) 
    return ".gif";
  if(filter.find("eps")!=string::npos) 
    return ".eps";   
  if(filter.find("tga")!=string::npos)
    return ".tga";
  if(filter.find("png")!=string::npos)
    return ".png";
  if(filter.find("pdf")!=string::npos)
    return ".pdf";
  if(filter.find("bmp")!=string::npos)
    return ".bmp";
  if(filter.find("epi")!=string::npos)
    return ".epi";
  if(filter.find("cgm")!=string::npos)
    return ".cgm";
  if(filter.find("ico")!=string::npos)
    return ".ico";
  if(filter.find("pcx")!=string::npos)
    return ".pcx";
  if(filter.find("ppm")!=string::npos)
    return ".ppm";
  if(filter.find("pdf")!=string::npos)
    return ".pdf";
  return "";
}

CaptureOptions::CaptureOptions(QWidget* _parent, VizmoMainWin* _mainWin)
  :OptionsBase(_parent, _mainWin)
{
  m_dialog = NULL; 	
	
  CreateActions(); 
  SetUpSubmenu("Capture"); 
  SetUpToolbar(); 
  SetHelpTips(); 

  m_cropBox = false; 
}

CaptureOptions::~CaptureOptions(){

  delete m_dialog; 
}

void
CaptureOptions::CreateActions(){

  //1. Create actions and add them to map  
  QAction* crop = new QAction(QPixmap(cropIcon), tr("Crop"), this);  
  m_actions["crop"] = crop; 
  QAction* picture = new QAction(QPixmap(cameraIcon), tr("Picture"), this); 
  m_actions["picture"] = picture; 
  QAction* movie = new QAction(QPixmap(camcorderIcon), tr("Movie"), this);
  m_actions["movie"] = movie;  
  m_dialog = new MovieSaveDialog(this, "movie_save", Qt::Dialog); 

  //2. Set other specifications as necessary
  m_actions["crop"]->setEnabled(false); 
  m_actions["crop"]->setCheckable(true);  
  m_actions["crop"]->setStatusTip(tr("Select area for screenshot"));  
  m_actions["picture"]->setEnabled(false); 
  m_actions["picture"]->setStatusTip(tr("Take picture")); 
  m_actions["movie"]->setEnabled(false); 
  m_actions["movie"]->setStatusTip(tr("Save movie")); 

  m_dialog->setModal(true); 

  //3. Make connections 
  connect(m_actions["crop"], SIGNAL(activated()), this, SLOT(CropRegion())); 
  connect(m_actions["picture"], SIGNAL(activated()), this, SLOT(CapturePicture())); 
  connect(m_actions["movie"], SIGNAL(activated()), this, SLOT(CaptureMovie()));

  connect(this, SIGNAL(GetScreenSize(int*, int*)), this, SLOT(GetOpenGLSize(int*, int*))); //???
  connect(this, SIGNAL(ToggleSelectionSignal()), GetMainWin()->GetGLScene(), SLOT(toggleSelectionSlot()));
  connect(this, SIGNAL(GetBoxDimensions(int*, int*, int*, int*)) , GetMainWin()->GetGLScene(), 
    SLOT(getBoxDimensions(int*, int*, int*, int*)));
  connect(this, SIGNAL(CallUpdate()), GetMainWin(), SLOT(updateScreen()));
  connect(this, SIGNAL(GoToFrame(int)), GetMainWin()->GetAnimationGUI(), SLOT(goToFrame(int)));
  connect(this, SIGNAL(GoToFrame(int)), GetMainWin()->GetAnimationDebugGUI(), SLOT(goToFrame(int)));
}

void 
CaptureOptions::SetUpToolbar(){

  m_toolbar = new QToolBar(GetMainWin()); 
  m_toolbar->addAction(m_actions["crop"]); 
  m_toolbar->addAction(m_actions["picture"]); 
  m_toolbar->addAction(m_actions["movie"]); 
}

void
CaptureOptions::Reset(){

  if(m_dialog != NULL){
    if(GetVizmo().GetPathSize() > 0){ 
      m_dialog->m_endIntFrame = GetVizmo().GetPathSize()-1;
    }
    else if(GetVizmo().GetDebugSize() > 0){
      m_dialog->m_endIntFrame = GetVizmo().GetDebugSize()-1;
    }  
    else{
      m_dialog->m_endIntFrame = -1; 
    }
    m_dialog->m_startIntFrame = 0; 
    m_dialog->m_stepIntSize = 10; 
    m_dialog->UpdateAttributes(); 
  }	

  if(m_actions["movie"] != NULL){    
    m_actions["crop"]->setEnabled(true); 
    m_actions["picture"]->setEnabled(true); 
    if(GetVizmo().GetPathSize()==0 && GetVizmo().GetDebugSize()==0)
      m_actions["movie"]->setEnabled(false);
    else	
      m_actions["movie"]->setEnabled(true); 
    
  }
}

void
CaptureOptions::SetHelpTips(){

  m_actions["crop"]->setWhatsThis(tr("Click and drag to specify a"
    " cropping area. You can then use the <b>Picture</b> button to"
    " save a screenshot.")); 
  m_actions["picture"]->setWhatsThis(tr("Click this button to save" 
    " a screenshot of the scene. If no cropping area is specified,"
    " the entire scene will be saved."));
  m_actions["movie"]->setWhatsThis(tr("Click this button to save a"
    " movie.")); //If someone knows a better way to word this, please do.
}

//Slots

void 
CaptureOptions::CropRegion(){

  m_cropBox =! m_cropBox;
  
  if(!m_cropBox)
    emit SimulateMouseUp();
  
  emit CallUpdate();
  emit ToggleSelectionSignal();   
}

void
CaptureOptions::CapturePicture(){

  int w, h, xOffset, yOffset;
   
  if(!m_cropBox){
    emit GetScreenSize(&w,&h);
    xOffset=0;
    yOffset=0;
  }
  else
    emit GetBoxDimensions(&xOffset, &yOffset, &w, &h);

  QFileDialog* fd = new QFileDialog(this,"Choose a name",".",QString::null);
  //fd->setMode(QFileDialog::ExistingFile);
  fd->setFileMode(QFileDialog::AnyFile);
  fd->setFilters(Filters());
  fd->setAcceptMode(QFileDialog::AcceptSave);
  QString fileName;
  QString fileExt;
  
  if(fd->exec() == QDialog::Accepted){
    fileName=fd->selectedFile();
    string fname = fileName.toStdString();
    string ext = Filter2Ext(fd->selectedFilter().toStdString());
    dump(fname.c_str(), ext.c_str(),xOffset+1,yOffset+1,w-2,h-2);
  }
}

void
CaptureOptions::CaptureMovie(){

  //Pop up the dialog
  int result;
  QString localFileName;
  result = m_dialog->exec();
  if(!result) 
  	return;
    
  //Save
  int startFrame = m_dialog->m_startIntFrame;
  int endFrame = m_dialog->m_endIntFrame;
  int stepSize = m_dialog->m_stepIntSize;
  //const char * extName=(char *) mDialog->sFileExt.data();
  string sExtName = m_dialog->m_sFileExt.toStdString();
  const char*  extName = sExtName.c_str();
     
  int w, h, xOffset, yOffset;
  if(!m_cropBox){
    emit GetScreenSize(&w, &h);
    xOffset=0;
    yOffset=0;
  }   
  else
    emit GetBoxDimensions(&xOffset, &yOffset, &w, &h);
   
  QString temp;
  QString qfname;
  char number[32];
  //char *number;
  char cmd[32];
  sprintf(cmd,"%s%d%s","%0", m_dialog->m_frameDigit,"d");
 
  QProgressDialog progress("Saving images...", "Abort", startFrame, endFrame, this);
  int FID=0;
  for(int i=startFrame; i<=endFrame; i+=stepSize, FID++){
    progress.setValue(i-startFrame);
    qApp->processEvents();
    if(progress.wasCanceled()) 
      break;
    // dump the image
    emit GoToFrame(i);
    sprintf(number, cmd, FID);
    qfname = m_dialog-> m_sFileName + (QString)number;
    const char* charQfname = (qfname.toStdString()).c_str();
    dump(charQfname, extName, xOffset+1, yOffset+1, w-2, h-2);
  }
  progress.setValue(endFrame - startFrame);
}

void
CaptureOptions::GetOpenGLSize(int* _w, int* _h){

  GetMainWin()->GetGLScene()->getWidthHeight(_w, _h); 
}














 

 

