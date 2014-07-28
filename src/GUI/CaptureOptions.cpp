#include "CaptureOptions.h"

#include "MainWindow.h"
#include "GLWidget.h"
#include "MovieSaveDialog.h"
#include "AnimationWidget.h"
#include "Models/Vizmo.h"
#include "Utilities/AlertUser.h"
#include "Utilities/ImageFilters.h"

#include "Icons/Crop.xpm"
#include "Icons/Camera.xpm"
#include "Icons/Camcorder.xpm"

CaptureOptions::
CaptureOptions(QWidget* _parent, MainWindow* _mainWindow)
    : OptionsBase(_parent, _mainWindow) {
  CreateActions();
  SetUpSubmenu("Capture");
  SetUpToolbar();
  SetHelpTips();
  m_cropBox = false;
}

void
CaptureOptions::
CreateActions() {
  //1. Create actions and add them to map
  m_actions["crop"] = new QAction(QPixmap(cropIcon), tr("Crop"), this);
  m_actions["picture"] = new QAction(QPixmap(cameraIcon), tr("Picture"), this);
  m_actions["movie"] = new QAction(QPixmap(camcorderIcon), tr("Movie"), this);
  m_actions["startLiveRecording"] = new QAction(QPixmap(camcorderIcon), tr("StartLiveRecording"), this);
  m_actions["stopLiveRecording"] = new QAction(QPixmap(camcorderIcon), tr("StopLiveRecording"), this);

  //2. Set other specifications as necessary
  m_actions["crop"]->setEnabled(false);
  m_actions["crop"]->setCheckable(true);
  m_actions["crop"]->setStatusTip(tr("Select area for screenshot"));
  m_actions["picture"]->setEnabled(false);
  m_actions["picture"]->setStatusTip(tr("Take picture"));
  m_actions["movie"]->setEnabled(false);
  m_actions["movie"]->setStatusTip(tr("Save movie"));
  m_actions["startLiveRecording"]->setEnabled(false);
  m_actions["startLiveRecording"]->setStatusTip(tr("Start live recording"));
  m_actions["stopLiveRecording"]->setEnabled(false);
  m_actions["stopLiveRecording"]->setStatusTip(tr("Stop live recording"));

  //3. Make connections
  connect(m_actions["crop"], SIGNAL(triggered()), this, SLOT(CropRegion()));
  connect(m_actions["picture"], SIGNAL(triggered()), this, SLOT(CapturePicture()));
  connect(m_actions["movie"], SIGNAL(triggered()), this, SLOT(CaptureMovie()));
  connect(m_actions["startLiveRecording"], SIGNAL(triggered()), this, SLOT(StartLiveRecording()));
  connect(m_actions["stopLiveRecording"], SIGNAL(triggered()), this, SLOT(StopLiveRecording()));

  connect(this, SIGNAL(ToggleSelectionSignal()), m_mainWindow->GetGLWidget(), SLOT(ToggleSelectionSlot()));
  connect(this, SIGNAL(SimulateMouseUp()), m_mainWindow->GetGLWidget(), SLOT(SimulateMouseUpSlot()));
  connect(this, SIGNAL(CallUpdate()), m_mainWindow, SLOT(UpdateScreen()));
  connect(this, SIGNAL(UpdateFrame(int)), m_mainWindow->GetAnimationWidget(), SLOT(UpdateFrame(int)));
  connect(m_mainWindow->GetGLWidget(), SIGNAL(Record()), this, SLOT(Record()));
}

void
CaptureOptions::
SetUpToolbar() {
  m_toolbar = new QToolBar(m_mainWindow);
  m_toolbar->addAction(m_actions["crop"]);
  m_toolbar->addAction(m_actions["picture"]);
  m_toolbar->addAction(m_actions["movie"]);
  m_toolbar->addAction(m_actions["startLiveRecording"]);
  m_toolbar->addAction(m_actions["stopLiveRecording"]);
}

void
CaptureOptions::
SetHelpTips() {
  m_actions["crop"]->setWhatsThis(tr("Click and drag to specify a"
        " cropping area. You can then use the <b>Picture</b> button to"
        " save a screenshot."));
  m_actions["picture"]->setWhatsThis(tr("Click this button to save"
        " a screenshot of the scene. If no cropping area is specified,"
        " the entire scene will be saved."));
  m_actions["movie"]->setWhatsThis(tr("Click this button to save a"
        " movie.")); //If someone knows a better way to word this, please do.
  m_actions["startLiveRecording"]->setWhatsThis(tr("Click this button to begin recording the GL scene."));
  m_actions["stopLiveRecording"]->setWhatsThis(tr("Click this button to stop recording the GL scene"));
}

void
CaptureOptions::
Reset() {
  m_actions["crop"]->setEnabled(true);
  m_actions["picture"]->setEnabled(true);
  m_actions["movie"]->setEnabled(GetVizmo().GetPath() || GetVizmo().GetDebug());
  m_actions["startLiveRecording"]->setEnabled(true);
  m_actions["stopLiveRecording"]->setEnabled(true);
}

//Slots
void
CaptureOptions::
CropRegion() {
  m_cropBox =! m_cropBox;

  if(!m_cropBox)
    emit SimulateMouseUp();

  emit CallUpdate();
  emit ToggleSelectionSignal();
}

void
CaptureOptions::
CapturePicture() {
  //Set up the file dialog to select image filename
  QFileDialog fd(m_mainWindow, "Choose a name", ".", QString::null);
  fd.setFileMode(QFileDialog::AnyFile);
  fd.setFilters(imageFilters);
  fd.setAcceptMode(QFileDialog::AcceptSave);

  //If filename exists save image
  if(fd.exec() == QDialog::Accepted){
    QStringList files = fd.selectedFiles();
    if(!files.empty()) {
      QString filename = GrabFilename(files[0], fd.selectedFilter());
      m_mainWindow->GetGLWidget()->SaveImage(filename, m_cropBox);
    }
  }
}

void
CaptureOptions::
CaptureMovie() {
  //Pop up a MovieSaveDialog
  MovieSaveDialog msd(m_mainWindow, Qt::Dialog);
  if(msd.exec() == QDialog::Accepted){
    size_t digits = max(double(msd.m_frameDigits),
        log10(msd.m_endFrame/msd.m_stepSize) + 2);

    //Create the progress bar for saving images
    QProgressDialog progress("Saving images...", "Abort",
        msd.m_startFrame, msd.m_endFrame, this);

    //for each frame, update the image, compute a filename, and save the image
    size_t frame = 0;
    for(size_t i = msd.m_startFrame; i <= msd.m_endFrame;
        i += msd.m_stepSize, ++frame) {
      //update progress bar
      progress.setValue(i - msd.m_startFrame);
      qApp->processEvents();
      if(progress.wasCanceled())
        break;

      // update the GLScene
      emit UpdateFrame(i);

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
      m_mainWindow->GetGLWidget()->SaveImage(filename, m_cropBox);
    }
  }
}

void
CaptureOptions::
StartLiveRecording() {
  //Set up the file dialog to select image filename
  QFileDialog fd(m_mainWindow, "Choose a name", ".", QString::null);
  fd.setFileMode(QFileDialog::AnyFile);
  fd.setFilters(imageFilters);
  fd.setAcceptMode(QFileDialog::AcceptSave);

  //if filename exists save image
  if(fd.exec() == QDialog::Accepted) {
    QStringList files = fd.selectedFiles();
    if(!files.isEmpty()) {
      m_filename = GrabFilename(files[0], fd.selectedFilter());

      //find digit
      int f = m_filename.indexOf('#');
      int l = m_filename.lastIndexOf('#');
      m_frameDigits = l - f + 1;
      m_frameDigitStart = f;
      if(m_frameDigitStart == size_t(-1)){
        m_frameDigits = 0;
        m_frameDigitStart = m_filename.lastIndexOf('.');
      }
    }
    m_frame = 0;
    m_mainWindow->GetGLWidget()->SetRecording(true);
  }
  else
    AlertUser("Recording aborted.");
}

void
CaptureOptions::
Record() {
  //grab string for frame number
  ostringstream oss;
  oss << m_frame++;
  string num = oss.str();
  size_t l = num.length();
  for(size_t j = 0; j < m_frameDigits - l; ++j)
    num = "0" + num;

  //create the filename
  QString filename = m_filename;
  filename.replace(m_frameDigitStart, m_frameDigits, num.c_str());

  //save the image
  m_mainWindow->GetGLWidget()->SaveImage(filename, m_cropBox);
}

void
CaptureOptions::
StopLiveRecording() {
  m_mainWindow->GetGLWidget()->SetRecording(false);
}

