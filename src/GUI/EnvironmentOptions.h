/*****************************************************************
* Class for the "Environment" submenu and associated action button
******************************************************************/

#ifndef ENVIRONMENTOPTIONS_H_
#define ENVIRONMENTOPTIONS_H_

#include "OptionsBase.h"

class EnvironmentOptions : public OptionsBase {

  Q_OBJECT

  public:
    EnvironmentOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);
    void CreateActions();
    void SetUpToolbar(); //Just randomize colors button
    void Reset();
    void SetHelpTips();

  private slots:
    void RefreshEnv();
    void RandomizeEnvColors();
    void AddObstacle();
};

#endif
