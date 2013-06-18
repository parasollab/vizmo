/*******************************************************************
 * Class for the "Robot" submenu 
 ********************************************************************/

#ifndef ROBOT_OPTIONS_H
#define ROBOT_OPTIONS_H

#include "OptionsBase.h"

class RobotOptions : public OptionsBase{

  Q_OBJECT

  public:
    RobotOptions(QWidget* _parent = 0, VizmoMainWin* _mainWin = 0); 
    void CreateActions(); 
    void SetUpToolbar(); //no tool buttons in this case; just a placeholder
    void Reset(); 
    void SetHelpTips(); 

  private slots:
    void ResetRobotPosition(); 	
    //	void ShowRobotCfg();
};

#endif


