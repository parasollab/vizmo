
// GUI for screen shot toolbar

//#include "vizmo2.h"
//#include "main_win.h"  

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
#include <qtoolbutton.h>
#include <qstringlist.h> 
#include <qprogressdialog.h> 

///////////////////////////////////////////////////////////////////////////////
//Icons
#include "icon/tapes.xpm"
#include "icon/video_camera2.xpm"
#include "icon/Camera.xpm"

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

inline string Filter2Ext( const string filter )
{
    if( filter.find("jpg")!=string::npos ) return ".jpg";
    if( filter.find("gif")!=string::npos ) return ".gif";
    if( filter.find("eps")!=string::npos ) return ".eps";
    if( filter.find("tga")!=string::npos ) return ".tga";
    if( filter.find("png")!=string::npos ) return ".png";
    if( filter.find("pdf")!=string::npos ) return ".pdf";
    if( filter.find("bmp")!=string::npos ) return ".bmp";
    if( filter.find("epi")!=string::npos ) return ".epi";
    if( filter.find("cgm")!=string::npos ) return ".cgm";
    if( filter.find("ico")!=string::npos ) return ".ico";
    if( filter.find("pcx")!=string::npos ) return ".pcx";
    if( filter.find("ppm")!=string::npos ) return ".ppm";
    if( filter.find("pdf")!=string::npos ) return ".pdf";
    return "";
}

///////////////////////////////////////////////////////////////////////////////
VizmoScreenShotGUI::VizmoScreenShotGUI(QMainWindow *parent, char *name) :QToolBar(parent,name)
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
    if( mDialog!=NULL ){
        mDialog->endIntFrame=GetVizmo().GetPathSize()-1;
        mDialog->startIntFrame=0;
        mDialog->stepIntSize=10;
        mDialog->updateAttributes();
    }
    
    if( animation!=NULL ){
        //disable/enable this toolbar
        if( GetVizmo().GetPathSize()==0 ) animation->setEnabled(false);
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
    takePicture= new QToolButton(QPixmap(Camera), "Picture", "Take a snap shot of whole window", this,
                                 SLOT(takeSnapshot()), this, "picture");
    takePicture->setUsesTextLabel ( true );
    takePicture->setEnabled(false);
    
    takeBoxPicture= new QToolButton(QPixmap(tapes), "Crop", "Take a snap shot of the selected region", this,
                                    SLOT(takeBoxSnapshot()), this, "selected");
    takeBoxPicture->setUsesTextLabel ( true );
    takeBoxPicture->setToggleButton(true);
    takeBoxPicture->setEnabled(false);
    animation= new QToolButton(QPixmap(video_camera2), "Movie", "Save Image Sequence", this,
                               SLOT(takeMoviePictures()), this, "movie");
    animation->setEnabled(false);

    animation->setUsesTextLabel ( true );
    mDialog=new MovieSaveDialog(this,"",true);
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
    const char * ExtName=mDialog->sFileExt.data();  
    
    int w,h;
    emit getScreenSize(&w,&h);
    
    QString temp;
    QString qfname;
    QProgressDialog progress( "Saving images...", "Abort", endFrame-startFrame,this, "progress", TRUE );
    for(int i=startFrame;i<=endFrame;i+=stepSize)
    {
        progress.setProgress(i-startFrame);
        qApp->processEvents();
        if ( progress.wasCancelled() ) break;
        // dump the image
        emit goToFrame(i);
        temp=temp.setNum(i);
        qfname=mDialog->sFileName+temp;
        dump(qfname.data(),ExtName,0,0,w,h);
    }
    progress.setProgress( endFrame-startFrame );
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
    if(!boxPicture){
        emit getScreenSize(&w,&h);
        xOffset=0;
        yOffset=0;
    }
    else{
        emit getBoxDimensions(&xOffset,&yOffset,&w,&h);
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    QFileDialog *fd=new QFileDialog(this,"file dialog",TRUE);
    //fd->setMode(QFileDialog::ExistingFile);
    fd->setMode(QFileDialog::AnyFile);
    fd->setFilters(Filters());    
    QString fileName;
    QString fileExt;
    if(fd->exec()==QDialog::Accepted)
    {
        fileName=fd->selectedFile();
        const char *fname=fileName.data();
        string ext=Filter2Ext(fd->selectedFilter().data());
        dump(fname,ext.c_str(),xOffset,yOffset,w,h);
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// MovieSaveDialog class functions
//
///////////////////////////////////////////////////////////////////////////////

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
    QFileDialog *fd=new QFileDialog(this,"file dialog",TRUE);
    fd->setMode(QFileDialog::AnyFile);  
    fd->setFilters(Filters());
    if(fd->exec()==QDialog::Accepted)
    {
        sFileName=fd->selectedFile();
        sFileExt=Filter2Ext(fd->selectedFilter().data()).c_str();
        fnameLabel->setText(sFileName+sFileExt); 
    }
}
