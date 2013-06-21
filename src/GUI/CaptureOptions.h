/************************************************************************
* Class for the "Capture" submenu and toolbar. 
* Both have actions Crop, Picture, and Movie. 
*************************************************************************/

#ifndef CAPTUREOPTIONS_H
#define CAPTUREOPTIONS_H

#include "OptionsBase.h"

class CaptureOptions : public OptionsBase{

  Q_OBJECT

  public:
    CaptureOptions(QWidget* _parent = 0, VizmoMainWin* _mainWin = 0);
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

	




