/************************************************************************
* Class for the "Capture" submenu and toolbar.
* Both have actions Crop, Picture, and Movie.
*************************************************************************/

#ifndef CAPTURE_OPTIONS_H_
#define CAPTURE_OPTIONS_H_

#include "OptionsBase.h"

class CaptureOptions : public OptionsBase {

  Q_OBJECT

  public:
    CaptureOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

  public slots:
    void Record();

  private slots:
    void CropRegion();
    void CapturePicture();
    void CaptureMovie();
    void StartLiveRecording();
    void StopLiveRecording();

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

    //for live recording
    size_t m_frame;
    size_t m_frameDigits; //number of # in the filename
    size_t m_frameDigitStart; //first index of # in the filename
    QString m_filename; //base filename for movie
};

#endif
