/***************************************************************************
* Class for the "File" submenu and associated action button
***************************************************************************/ 

#ifndef FILE_OPTIONS_H
#define FILE_OPTIONS_H

#include "OptionsBase.h"
#include "vizmo2.h" 

class FileListDialog; 

class FileOptions : public OptionsBase{

  Q_OBJECT  

  public:
    FileOptions(QWidget* _parent = 0, VizmoMainWin* _mainWin = 0); 
    void CreateActions();
    void SetUpToolbar(); //just the folder button in this case  
    void Reset(); 
    void SetHelpTips(); 

  private slots:
    void LoadFile();  
    void UpdateFiles(); 
    void SaveEnv(); 
    void SaveQryFile(); 
    void SaveRoadmap(); 

  private:
    FileListDialog* m_flDialog; 
    void SaveNewRoadmap(const char* _filename); 
};

#endif    
