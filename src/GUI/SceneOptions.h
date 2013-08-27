#ifndef SCENEOPTIONS_H_
#define SCENEOPTIONS_H_

#include "OptionsBase.h"

class CameraPosDialog;

class SceneOptions : public OptionsBase {

  Q_OBJECT

  public:
    SceneOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

    void CreateActions();
    void SetUpToolbar(); //two buttons: Reset camera and change BG color
    void Reset();
    void SetHelpTips();

  private slots:
    void ShowAxis();
    void ResetCamera();
    void SetCameraPosition();
    void ChangeBGColor();
    void ShowGeneralContextMenu(); //right-click generated menu when NOT on an object

  private:
    CameraPosDialog* m_cameraPosInput; //allow user to manually specify a camera position

};

#endif
