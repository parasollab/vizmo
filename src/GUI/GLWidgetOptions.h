#ifndef GLWIDGETOPTIONS_H_
#define GLWIDGETOPTIONS_H_

#include "OptionsBase.h"

class CameraPosDialog;

class GLWidgetOptions : public OptionsBase {

  Q_OBJECT

  public:
    GLWidgetOptions(QWidget* _parent, MainWindow* _mainWindow);

    void CreateActions();
    void SetUpToolbar(); //two buttons: Reset camera and change BG color
    void Reset();
    void SetHelpTips();

  private slots:
    void ResetCamera();
    void SetCameraPosition();
    void ChangeBGColor();
    void ShowGeneralContextMenu(); //right-click generated menu when NOT on an object

  private:
    CameraPosDialog* m_cameraPosInput; //allow user to manually specify a camera position

};

#endif
