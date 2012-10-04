/************************************************************************
* Class for the "Capture" submenu and toolbar. 
* Both have actions Crop, Picture, and Movie. 
*************************************************************************/

#ifndef CAPTURE_OPTIONS_H
#define CAPTURE_OPTIONS_H

#include "OptionsBase.h"

class MovieSaveDialog;
class gliPickBox;  

class CaptureOptions : public OptionsBase{

  Q_OBJECT

  public:
    CaptureOptions(QWidget* _parent = 0, VizmoMainWin* _mainWin = 0);
    ~CaptureOptions();  
    void CreateActions(); 
    void SetUpToolbar(); //crop, picture, and movie actions
    void Reset(); 
    void SetHelpTips(); 

  private slots:
    void CropRegion();
    void CapturePicture(); 
    void CaptureMovie();
    void GetOpenGLSize(int* _w, int* _h); 

  signals:
    void SimulateMouseUp();
    void ToggleSelectionSignal(); 
    void CallUpdate();  
    void GetScreenSize(int* _w, int* _h); 
    void GetBoxDimensions(int* _xOffset, int* _yOffset, int* _w, int* _h); 
    void GoToFrame(int); 

  private:
    MovieSaveDialog* m_dialog; 
    bool m_cropBox;  

};

#endif

	




