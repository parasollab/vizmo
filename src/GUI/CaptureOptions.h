/************************************************************************
* Class for the "Capture" submenu and toolbar.
* Both have actions Crop, Picture, and Movie.
*************************************************************************/

#ifndef CAPTUREOPTIONS_H_
#define CAPTUREOPTIONS_H_

#include "OptionsBase.h"

class CaptureOptions : public OptionsBase {

  Q_OBJECT

  public:
    CaptureOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);
    void CreateActions();
    void SetUpToolbar(); //crop, picture, and movie actions
    void SetHelpTips();
    void Reset();

  private slots:
    void CropRegion();
    void CapturePicture();
    void CaptureMovie();

  signals:
    void SimulateMouseUp();
    void ToggleSelectionSignal();
    void CallUpdate();
    void GoToFrame(int);

  private:
    bool m_cropBox;
};

#endif
