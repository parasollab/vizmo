/*****************************************************************
* Class for the "Environment" submenu and associated action button
******************************************************************/

#ifndef ENVIRONMENTOPTIONS_H_
#define ENVIRONMENTOPTIONS_H_

#include "OptionsBase.h"

using namespace std;

class ModelSelectionWidget;
class MultiBodyModel;

class EnvironmentOptions : public OptionsBase {

  Q_OBJECT

  public:
    EnvironmentOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

  private slots:
    void RefreshEnv();
    void RandomizeEnvColors();
    void AddObstacle();
    void DeleteObstacle();
    void MoveObstacle();
    void DuplicateObstacles();
    void ChangeBoundaryForm();
    void EditRobot();
    void AddRegionSphere();

  private:
    void CreateActions();
    void SetUpCustomSubmenu();
    void SetUpToolbar(); //Just randomize colors button
    void Reset();
    void SetHelpTips();
    string GetFilename(string _modelFilename);
    string GetFileDir(string _modelFilename, string _filename);
    QMenu* m_obstacleMenu;
    string m_modelFilename;
    string m_modelFileDir;
    string m_filename;

  public:
    ModelSelectionWidget* m_modelSelectionWidget;
    MultiBodyModel* m_multiBodyModel;

  protected:
    double m_xPos;
    double m_yPos;
    double m_zPos;
    double m_xRot;
    double m_yRot;
    double m_zRot;

};

#endif
