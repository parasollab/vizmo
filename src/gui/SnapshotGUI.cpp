
// GUI for screen shot toolbar

//#include "vizmo2.h"
//#include "MainWin.h"  

#include "SnapshotGUI.h"

#include <GL/gliDump.h>
#include <GL/gliPickBox.h>

///////////////////////////////////////////////////////////////////////////////
//Include Qt Headers



#include <QtGui>

//Added by qt3to4:
#include <Q3ToolBar>
#include <q3grid.h>
#include <q3hbox.h>
#include <q3multilineedit.h>
#include <q3listbox.h>
#include <q3progressdialog.h> 

#include <q3filedialog.h>

///////////////////////////////////////////////////////////////////////////////
//Icons
#include "icon/VideoCamera2.xpm"
#include "icon/Camera.xpm"
#include "icon/Crop.xpm" 
#include "icon/Camcorder.xpm" 

inline QStringList& Filters()
{
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

inline string Filter2Ext(const string filter)
{
    if(filter.find("jpg")!=string::npos) return ".jpg";
    if(filter.find("gif")!=string::npos) return ".gif";
    if(filter.find("eps")!=string::npos) return ".eps";
    if(filter.find("tga")!=string::npos) return ".tga";
    if(filter.find("png")!=string::npos) return ".png";
    if(filter.find("pdf")!=string::npos) return ".pdf";
    if(filter.find("bmp")!=string::npos) return ".bmp";
    if(filter.find("epi")!=string::npos) return ".epi";
    if(filter.find("cgm")!=string::npos) return ".cgm";
    if(filter.find("ico")!=string::npos) return ".ico";
    if(filter.find("pcx")!=string::npos) return ".pcx";
    if(filter.find("ppm")!=string::npos) return ".ppm";
    if(filter.find("pdf")!=string::npos) return ".pdf";
    return "";
}

///////////////////////////////////////////////////////////////////////////////
VizmoScreenShotGUI::VizmoScreenShotGUI(Q3MainWindow *parent, char *name) :Q3ToolBar(parent,name)
{
    this->setLabel("Vizmo ScrenCapture");
    mDialog=NULL;
    CreateGUI();
    boxPicture=false;
}

VizmoScreenShotGUI::~VizmoScreenShotGUI()
{
    delete mDialog;
}

void VizmoScreenShotGUI::reset() //reset every thing
{
    if(mDialog!=NULL){
      if(GetVizmo().GetPathSize()>0){
        mDialog->endIntFrame=GetVizmo().GetPathSize()-1;
      }
      else if(GetVizmo().GetDebugSize()>0){
        mDialog->endIntFrame=GetVizmo().GetDebugSize()-1;
      }
      else{
        mDialog->endIntFrame=-1;
      }
        mDialog->startIntFrame=0;
        mDialog->stepIntSize=10;
        mDialog->updateAttributes();
    }
    
    if(animation!=NULL){
        //disable/enable this toolbar
        if(GetVizmo().GetPathSize()==0 && GetVizmo().GetDebugSize()==0) 
          animation->setEnabled(false);
        
        else{
			animation->setEnabled(true);
			takeBoxPicture->setEnabled(true);
			takePicture->setEnabled(true);
		}
    }
}

bool VizmoScreenShotGUI::CreateGUI()
{
    CreateActions();
    return true;
}


void VizmoScreenShotGUI::CreateActions()
{   
    //Select box region (click and drag) which can be used to take snapshot 
    takeBoxPicture= new QToolButton(QPixmap(crop), "Crop", "Take a snap shot of the selected region", this,
                                    SLOT(takeBoxSnapshot()), this, "selected");
    takeBoxPicture->setUsesTextLabel (true);
    takeBoxPicture->setToggleButton(true);
    takeBoxPicture->setEnabled(false);
     
    //Take the screenshot (whole window or selected region if box is drawn)
    takePicture= new QToolButton(QPixmap(camera), "Picture", "Take a snap shot of whole window", this,
                                 SLOT(takeSnapshot()), this, "picture");
    takePicture->setUsesTextLabel (true);
    takePicture->setEnabled(false);
	
    //Save movie sequence for query, debug file, etc. 
    animation= new QToolButton(QPixmap(camcorder), "Movie", "Save Image Sequence", this,
                               SLOT(takeMoviePictures()), this, "movie");
    animation->setEnabled(false);
    animation->setUsesTextLabel (true);
	
    mDialog=new MovieSaveDialog(this,"movie_save", Qt::Dialog);
    mDialog->setModal(true);
}


void VizmoScreenShotGUI::takeMoviePictures()
{
    ///////////////////////////////////////////////////////////////////////////
    //Pop up the dialog
    int result;
    QString localFileName;
    result=mDialog->exec();
    if(!result) return;
    
    ///////////////////////////////////////////////////////////////////////////
    //Save
    int startFrame=mDialog->startIntFrame;
    int endFrame=mDialog->endIntFrame;
    int stepSize=mDialog->stepIntSize;
    //const char * ExtName=(char *) mDialog->sFileExt.data();
    string s_ExtName = mDialog->sFileExt.toStdString ();
    const char * ExtName = s_ExtName.c_str();

    ///////////////////////////////////////////////////////////////////////////
    int w,h,xOffset,yOffset;
    if(!boxPicture){
        emit getScreenSize(&w,&h);
        xOffset=0;
        yOffset=0;
    }
    else{
        emit getBoxDimensions(&xOffset,&yOffset,&w,&h);
    }
    ///////////////////////////////////////////////////////////////////////////

    QString temp;
    QString qfname;
    char number[32];
    //char *number;
	char cmd[32];
	sprintf(cmd,"%s%d%s","%0",mDialog->frame_digit,"d");

    QProgressDialog progress
	("Saving images...","Abort",startFrame,endFrame,this);
	int FID=0;
    for(int i=startFrame;i<=endFrame;i+=stepSize,FID++)
    {
        progress.setValue(i-startFrame);
        qApp->processEvents();
        if (progress.wasCanceled()) break;
        // dump the image
        emit goToFrame(i);
		// 
        sprintf(number,cmd,FID);
        qfname=mDialog->sFileName+(QString)number;
	const char * char_qfname = (qfname.toStdString()).c_str();
        dump(char_qfname, ExtName, xOffset+1, yOffset+1, w-2, h-2);
    }
    progress.setValue(endFrame-startFrame);
}

void VizmoScreenShotGUI::takeBoxSnapshot()
{
    boxPicture=!boxPicture;
    
    if(!boxPicture)
    {
        emit simulateMouseUp();
    }
    emit callUpdate();
    emit togleSelectionSignal();   
}

void VizmoScreenShotGUI::takeSnapshot()
{
    int w,h,xOffset,yOffset;
    if(!boxPicture){
        emit getScreenSize(&w,&h);
        xOffset=0;
        yOffset=0;
    }
    else{
        emit getBoxDimensions(&xOffset,&yOffset,&w,&h);
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    QFileDialog *fd=new QFileDialog(this,"Choose a name",".",QString::null);
    //fd->setMode(QFileDialog::ExistingFile);
    fd->setFileMode(QFileDialog::AnyFile);
    fd->setFilters(Filters());  
    fd->setAcceptMode(QFileDialog::AcceptSave); 
    QString fileName;
    QString fileExt;
    if(fd->exec()==QDialog::Accepted)
    {
        fileName=fd->selectedFile();
        string fname= fileName.toStdString();
        string ext=Filter2Ext(fd->selectedFilter().toStdString());
        dump(fname.c_str(), ext.c_str(),xOffset+1,yOffset+1,w-2,h-2);
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// MovieSaveDialog class functions
//
///////////////////////////////////////////////////////////////////////////////

MovieSaveDialog::MovieSaveDialog(QWidget *parent, const char *name, Qt::WFlags f):
//MovieSaveDialog::MovieSaveDialog(QWidget *parent, Qt::WFlags f):
QDialog(parent,f)
{
	sFileName="vizmo_movie";
	sFileExt="jpg";
	frame_digit=5;

    QVBoxLayout* vbox = new QVBoxLayout(this,8);
    vbox->setAutoAdd(TRUE);
    Q3Grid* controls = new Q3Grid(2,Qt::Horizontal,this);
    controls->setSpacing(8);
    QLabel* l;
    
    l=new QLabel("StartFrame",controls); l->setAlignment(Qt::AlignCenter);
    startFrame=new QLineEdit(controls);
    startFrame->setValidator(new QIntValidator(startFrame));
    
    l=new QLabel("EndFrame",controls); l->setAlignment(Qt::AlignCenter);
    endFrame = new QLineEdit(controls);
    endFrame->setValidator(new QIntValidator(endFrame));
    
    l=new QLabel("Step Size",controls); l->setAlignment(Qt::AlignCenter);
    stepSize = new QLineEdit(controls);
    stepSize->setValidator(new QIntValidator(stepSize));    
    
    QPushButton *fileNameButton = new QPushButton("Select Name",controls);  
    fnameLabel=new QLabel((sFileName+"#####."+sFileExt),controls); 
	l->setAlignment(Qt::AlignCenter);
    
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

void MovieSaveDialog::updateAttributes()
{
    QString tempStorage;
    
    tempStorage.setNum(startIntFrame);
    startFrame->setText(tempStorage);
    
    tempStorage.setNum(endIntFrame);
    endFrame->setText(tempStorage);
    
    tempStorage.setNum(stepIntSize);
    stepSize->setText(tempStorage);
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
    QFileDialog *fd=new QFileDialog(this,"File name",".",QString::null);
    fd->setFileMode(QFileDialog::AnyFile);  
    fd->setFilters(Filters());
    if(fd->exec()==QDialog::Accepted)
    {
      sFileName=fd->selectedFile(); 
      sFileExt=Filter2Ext(fd->selectedFilter().toStdString()).c_str(); 
      fnameLabel->setText(sFileName+sFileExt); 

		//find digit
		int _s=sFileName.find('#');
		int _g=sFileName.findRev('#');
		frame_digit=_g-_s;
		if(_s==_g) frame_digit=4;
		sFileName=sFileName.left(_s);
    }
}
