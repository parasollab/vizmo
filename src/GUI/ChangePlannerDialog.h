#ifndef CHANGE_PLANNER_DIALOG_H_
#define CHANGE_PLANNER_DIALOG_H_

#include <QtGui>
#include <QDialog>

#include <string>

class QButtonGroup;

class MainWindow;

using namespace std;

class ChangePlannerDialog : public QDialog {

  Q_OBJECT

  public:
    ChangePlannerDialog(MainWindow* _mainWindow);
    string GetPlanner();

  private:
    QButtonGroup* m_radioGroup;
};

#endif

