#include "CaptureOptions.h"

#include <QAction>
#include <QToolBar>
#include <QPixmap>
#include <QFileDialog>
#include <QProgressDialog>
#include <QApplication>

#include "MainWindow.h"
#include "SceneWin.h"
#include "MovieSaveDialog.h"
#include "AnimationGUI.h"
#include "Models/Vizmo.h"
#include "Utilities/ImageFilters.h"

#include "Icons/Crop.xpm"
#include "Icons/Camera.xpm"
#include "Icons/Camcorder.xpm"

CaptureOptions::CaptureOptions(QWidget* _parent, MainWindow* _mainWindow)
  : OptionsBase(_parent, _mainWindow) {
    CreateActions();
    SetUpSubmenu("Capture");
    SetUpToolbar();
    SetHelpTips();
    m_cropBox = false;
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

  //2. Set other specifications as necessary
  m_actions["crop"]->setEnabled(false);
  m_actions["crop"]->setCheckable(true);
  m_actions["crop"]->setStatusTip(tr("Select area for screenshot"));
  m_actions["picture"]->setEnabled(false);
  m_actions["picture"]->setStatusTip(tr("Take picture"));
  m_actions["movie"]->setEnabled(false);
  m_actions["movie"]->setStatusTip(tr("Save movie"));

  //3. Make connections
  connect(m_actions["crop"], SIGNAL(triggered()), this, SLOT(CropRegion()));
  connect(m_actions["picture"], SIGNAL(triggered()), this, SLOT(CapturePicture()));
  connect(m_actions["movie"], SIGNAL(triggered()), this, SLOT(CaptureMovie()));

  connect(this, SIGNAL(ToggleSelectionSignal()), m_mainWindow->GetGLScene(), SLOT(ToggleSelectionSlot()));
  connect(this, SIGNAL(SimulateMouseUp()), m_mainWindow->GetGLScene(), SLOT(SimulateMouseUpSlot()));
  connect(this, SIGNAL(CallUpdate()), m_mainWindow, SLOT(updateScreen()));
  connect(this, SIGNAL(GoToFrame(int)), m_mainWindow->GetAnimationGUI(), SLOT(goToFrame(int)));
}

void
CaptureOptions::SetUpToolbar() {
  m_toolbar = new QToolBar(m_mainWindow);
  m_toolbar->addAction(m_actions["crop"]);
  m_toolbar->addAction(m_actions["picture"]);
  m_toolbar->addAction(m_actions["movie"]);
}

void
CaptureOptions::SetHelpTips() {
  m_actions["crop"]->setWhatsThis(tr("Click and drag to specify a"
        " cropping area. You can then use the <b>Picture</b> button to"
        " save a screenshot."));
  m_actions["picture"]->setWhatsThis(tr("Click this button to save"
        " a screenshot of the scene. If no cropping area is specified,"
        " the entire scene will be saved."));
  m_actions["movie"]->setWhatsThis(tr("Click this button to save a"
        " movie.")); //If someone knows a better way to word this, please do.
}

void
CaptureOptions::Reset(){
  m_actions["crop"]->setEnabled(true);
  m_actions["picture"]->setEnabled(true);
  m_actions["movie"]->setEnabled(GetVizmo().GetPathSize() || GetVizmo().GetDebugSize());
}

//Slots
void
CaptureOptions::CropRegion() {

  m_cropBox =! m_cropBox;

  if(!m_cropBox)
    emit SimulateMouseUp();

  emit CallUpdate();
  emit ToggleSelectionSignal();
}

void
CaptureOptions::CapturePicture(){
  //Set up the file dialog to select image filename
  QFileDialog fd(this, "Choose a name", ".", QString::null);
  fd.setFileMode(QFileDialog::AnyFile);
  fd.setFilters(imageFilters);
  fd.setAcceptMode(QFileDialog::AcceptSave);

  //If filename exists save image
  if(fd.exec() == QDialog::Accepted){
    QStringList files = fd.selectedFiles();
    if(!files.empty()) {
      QString filename = GrabFilename(files[0], fd.selectedFilter());
      m_mainWindow->GetGLScene()->SaveImage(filename, m_cropBox);
    }
  }
}

void
CaptureOptions::CaptureMovie(){
  //Pop up a MovieSaveDialog
  MovieSaveDialog msd(this, Qt::Dialog);
  if(msd.exec() == QDialog::Accepted){
    size_t digits = max(double(msd.m_frameDigits), log10(msd.m_endFrame/msd.m_stepSize) + 2);

    //Create the progress bar for saving images
    QProgressDialog progress("Saving images...", "Abort", msd.m_startFrame, msd.m_endFrame, this);

    //for each frame, update the image, compute a filename, and save the image
    size_t frame = 0;
    for(size_t i = msd.m_startFrame; i <= msd.m_endFrame; i += msd.m_stepSize, ++frame){
      //update progress bar
      progress.setValue(i - msd.m_startFrame);
      qApp->processEvents();
      if(progress.wasCanceled())
        break;

      // update the GLScene
      emit GoToFrame(i);

      //grab string for frame number
      ostringstream oss;
      oss << frame;
      string num = oss.str();
      size_t l = num.length();
      for(size_t j = 0; j < digits - l; ++j)
        num = "0" + num;

      //create the filename
      QString filename = msd.m_filename;
      filename.replace(msd.m_frameDigitStart, msd.m_frameDigits, num.c_str());

      //save the image
      m_mainWindow->GetGLScene()->SaveImage(filename, m_cropBox);
    }
  }
}

