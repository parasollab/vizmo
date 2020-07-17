#ifndef QUERY_EDIT_DIALOG_H_
#define QUERY_EDIT_DIALOG_H_

#include <QtGui>
#include <QDialog>

#include "Models/QueryModel.h"

using namespace std;

class QListWidget;
class QueryModel;
class MainWindow;

class QueryEditDialog : public QDialog {

  Q_OBJECT

  public:
    QueryEditDialog(MainWindow* _mainWindow, QueryModel* _queryModel);
    ~QueryEditDialog();

  private slots:
    void Delete();
    void Add();
    void EditQuery();
    void SwapUp();
    void SwapDown();

  private:
    //Functions
    void SetUpDialog();
    void RefreshEnv();
    void ShowQuery();
    //Model Variables
    QueryModel* m_queryModel;
    MainWindow* m_mainWindow;
    //Qt Variables
    QListWidget* m_listWidget;
};

#endif
