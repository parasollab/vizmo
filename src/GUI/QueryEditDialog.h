#ifndef QUERYEDITDIALOG_H_
#define QUERYEDITDIALOG_H_

#include <QtGui>

#include "Models/QueryModel.h"

using namespace std;

class QPushButton;
class QListWidget;
class QueryModel;
class MainWindow;

class QueryEditDialog : public QDialog {

  Q_OBJECT

  public:
    QueryEditDialog(QueryModel* _queryModel, MainWindow* _mainWindow,  QWidget* _parent);
    ~QueryEditDialog();

  private slots:
    void Delete();
    void Add();
    void EditQuery();
    void SwapUp();
    void SwapDown();

  private:
    //Functions
    void RefreshEnv();
    void ShowQuery();
    //Model Variables
    QueryModel* m_queryModel;
    MainWindow* m_mainWindow;
    //Qt Variables
    QListWidget* m_listWidget;
};

#endif
