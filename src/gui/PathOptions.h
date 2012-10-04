/*****************************************************************
 * Class for the "Path" submenu and associated action buttons
 *****************************************************************/ 

#ifndef PATH_OPTIONS_H
#define PATH_OPTIONS_H

#include "OptionsBase.h"

class CustomizePathDialog; 

class PathOptions : public OptionsBase{

  Q_OBJECT

  public:
    PathOptions(QWidget* _parent = 0, VizmoMainWin* _mainWin = 0); 
    void CreateActions();
    void SetUpCustomSubmenu(); //like RoadmapOptions  
    void SetUpToolbar();       //show/hide path and show/hide start/goal
    void Reset(); 
    void SetHelpTips(); 

  private slots:
    void ShowHidePath();
    void ShowHideStartGoal(); 
    void PathDisplayOptions();  

    //void SetQueryStart();  TEMPORARY DISABLE  
    //void SetQueryGoal(); 

  private:
    QMenu* m_setQuery;     //submenu within the submenu 
    CustomizePathDialog* m_pathOptionsInput; //pop-up dialog for gradient input, etc.  

    //queryGUI* m_qrySGUI; //these need some help 
    //queryGUI* m_qryGGUI; 
}; 	 

#endif 




