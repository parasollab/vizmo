#ifndef GL_WIDGET_OPTIONS_H_
#define GL_WIDGET_OPTIONS_H_

#include "OptionsBase.h"

class CameraPosDialog;

class GLWidgetOptions : public OptionsBase {

  Q_OBJECT

  public:
    GLWidgetOptions(QWidget* _parent, MainWindow* _mainWindow);

  private slots:
    void ResetCamera();
    void SetCameraPosition();
    void ChangeBGColor();
    void ShowGeneralContextMenu(); //right-click generated menu when NOT on an object

  private:
    void CreateActions();
    void SetUpToolbar(); //two buttons: Reset camera and change BG color
    void SetUpToolTab();
    void Reset();
    void SetHelpTips();

    QPointer<QDialog> m_cameraPosDialog;

};

#endif
