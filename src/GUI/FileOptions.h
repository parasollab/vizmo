/***************************************************************************
* Class for the "File" submenu and associated action button
***************************************************************************/

#ifndef FILEOPTIONS_H_
#define FILEOPTIONS_H_

#include "OptionsBase.h"

class FileOptions : public OptionsBase {

  Q_OBJECT

  public:
    FileOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

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
};

#endif
