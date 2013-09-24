/*******************************************************************
 * Class for the "Robot" submenu
 ********************************************************************/

#ifndef ROBOTOPTIONS_H_
#define ROBOTOPTIONS_H_

#include "OptionsBase.h"

class RobotOptions : public OptionsBase {

  Q_OBJECT

  public:
    RobotOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

  private slots:
    void CreateActions();
    void SetUpToolbar(); //no tool buttons in this case; just a placeholder
    void Reset();
    void SetHelpTips();
    void ResetRobotPosition();
    //	void ShowRobotCfg();
};

#endif


