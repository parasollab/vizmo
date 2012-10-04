#ifndef SCENE_OPTIONS_H
#define SCENE_OPTIONS_H

#include "OptionsBase.h"

class CameraPosDialog;  

class SceneOptions : public OptionsBase{

  Q_OBJECT

  public:
    SceneOptions(QWidget* _parent = 0, VizmoMainWin* _mainWin = 0); 
    void CreateActions(); 
    void SetUpToolbar(); //two buttons: Reset camera and change BG color
    void Reset(); 
    void SetHelpTips(); 

  private slots:
    void ShowGrid(); //a slot for every action for cleanliness' sake.. 	
    void ShowAxis(); 
    void ResetCamera(); 
    void SetCameraPosition(); 
    void ChangeBGColor();
    void ShowGeneralContextMenu(); //right-click generated menu when NOT on an object 

  private:
    CameraPosDialog* m_cameraPosInput; //allow user to manually specify a camera position  

}; 

#endif 



