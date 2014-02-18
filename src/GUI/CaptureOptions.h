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

  private slots:
    void CropRegion();
    void CapturePicture();
    void CaptureMovie();

  signals:
    void SimulateMouseUp();
    void ToggleSelectionSignal();
    void CallUpdate();
    void UpdateFrame(int);

  private:
    void CreateActions();
    void SetUpToolbar(); //crop, picture, and movie actions
    void SetHelpTips();
    void Reset();
    bool m_cropBox;
};

#endif
