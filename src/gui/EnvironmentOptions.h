/*****************************************************************
* Class for the "Environment" submenu and associated action button
******************************************************************/

#ifndef ENVIRONMENT_OPTIONS_H
#define ENVIRONMENT_OPTIONS_H

#include "OptionsBase.h" 

class EnvironmentOptions : public OptionsBase{
	
  Q_OBJECT

  public:
    EnvironmentOptions(QWidget* _parent = 0, VizmoMainWin* _mainWin = 0); 
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






 
